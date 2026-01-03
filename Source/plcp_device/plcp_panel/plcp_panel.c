#include "plcp_panel.h"
#include "../../Source/adc/adc.h"
#include "../../Source/base/base.h"
#include "../../Source/base/debug.h"
#include "../../Source/device/device_manager.h"
#include "../../Source/eventbus/eventbus.h"
#include "../../Source/plcp_device/APP_PublicAttribute.h"
#include "../../Source/plcp_device/MseProcess.h"
#include "../../Source/plcp_device/plcp_panel/panel_adapter.h"
#include "../../Source/plcp_device/plcp_panel/plcp_panel_api.h"
#include "../../Source/plcp_device/plcp_panel/plcp_panel_info.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_bind.h"
#include "../../Source/pwm/pwm_hw.h"
#include "../../Source/timer/timer.h"
#include "../../Source/zero/zero.h"
#include "systick.h"
#include <stdio.h>

#define SYSTEM_CLOCK_FREQ 64000000 // 系统时钟频率(72MHz)
#define TIMER_PERIOD 100           // 50ms 触发一次中断

#define ADC_TO_VOL(adc_val) ((adc_val) * 330 / 4096) // adc值转电压

#if defined PLCP_PANEL
#define VOL_BUF_SIZE 10
#define LONG_PRESS 120 // 长按时间
#define MIN_VOL 329    // 无按键按下时的最小电压值
#define MAX_VOL 330    // 无按键按下时的最大电压值

#define ADC_PARAMS panel_status_t *temp_status, \
                   common_panel_t *temp_common, \
                   adc_value_t *adc_value

typedef struct
{
    uint16_t min;
    uint16_t max;
} key_vol_t;

typedef struct
{ // 用于每个按键的状态

    bool k_press;
    const key_vol_t vol_range; // 按键电压范围
} panel_status_t;

typedef struct
{
    uint8_t buf_idx; // 缓冲区下标
    uint16_t vol;    // 电压值
    uint16_t vol_buf[VOL_BUF_SIZE];
} adc_value_t;

typedef struct
{
    bool bl_close;           // Turn on the backlight
    bool bl_open;            // Turn off the backlight
    bool bl_status;          // Backlight status
    uint16_t bl_delay_count; // 总关背光延时执行

    bool led_filck;           // 闪烁
    bool key_long_press;      // 长按状态
    bool enter_config;        // 进入配置状态
    uint16_t key_long_count;  // 长按计数
    uint16_t led_filck_count; // 闪烁计数
    bool remove_card;
    uint16_t remove_card_count;
} common_panel_t;

static panel_status_t my_panel_status[KEY_NUMBER] = {
    PANEL_VOL_RANGE_DEF,
};
static common_panel_t my_common_panel;
static adc_value_t my_adc_value;

static uint16_t timer_8s_count;
static uint16_t timer_7s_count;
static volatile uint16_t timer_5s_count;
static volatile uint16_t timer_1s_count;
static uint16_t timer_2s_count;
static volatile uint16_t timer_3s_count;

static void read_adc_value(void *arg);
static void plcp_panel_tast(void *arg);
void process_message_queue(void *arg);
static void plcp_panel_event_handler(event_type_e event, void *params);
static void read_adc(ADC_PARAMS);
static void plcp_panel_blink(bool status);

// 向前声明
void APP_PLCSDK_Init(void);
void CmdTest_MSE_GET_DID(void);
void CmdTest_MSE_GET_MAC(void);
void CmdTest_MSE_GET_CC0MAC();

const sPost_wflash *sPost_flag = NULL;

static bool get_cco_mac = false;
static bool get_my_mac = false;
static bool join_net = false;
static bool led_blink = false;
static uint16_t led_blink_count = 0;
void plcp_panel_init(void)
{
    plcp_panel_pins_init();
    adc_channel_t my_adc_channel = {0};
    my_adc_channel.adc_channel_0 = true;
    app_adc_init(&my_adc_channel);
    // app_zero_init(EXTI_11);

    app_pwm_hw_init();
    app_pwm_hw_add_pin(PWM_PB0);
    app_pwm_hw_add_pin(PWM_PB1);
    app_pwm_hw_add_pin(PWM_PB6);
    app_pwm_hw_add_pin(PWM_PB7);
    app_pwm_hw_add_pin(PWM_PA8);

    app_eventbus_subscribe(plcp_panel_event_handler);
    app_timer_start(5, read_adc_value, true, NULL, "read_adc");
    app_timer_start(10, plcp_panel_tast, true, NULL, "task");
    app_timer_start(50, process_message_queue, true, NULL, "message_queue");

    APP_PRINTF("plcp_panel_init\n");
    APP_PLCSDK_Init();
    sPost_flag = APP_Postflag_GetPointer();
    // APP_SceneGroupClr();

    //  app_flash_mass_erase();  // 擦除整个扇区(测试使用)
}

static void read_adc_value(void *arg)
{
    my_adc_value.vol = ADC_TO_VOL(app_get_adc_value()[0]);
    read_adc(my_panel_status, &my_common_panel, &my_adc_value);
}

static void read_adc(ADC_PARAMS)
{
    // APP_PRINTF("vol:%d\n", adc_value->vol);
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        panel_status_t *p_status = &temp_status[i];
        if (adc_value->vol < p_status->vol_range.min || adc_value->vol > p_status->vol_range.max) {
            if (adc_value->vol >= MIN_VOL && adc_value->vol <= MAX_VOL) {
                p_status->k_press = false;
                temp_common->key_long_press = false;
                temp_common->key_long_count = 0;
            }
            continue;
        }
        adc_value->vol_buf[adc_value->buf_idx++] = adc_value->vol;
        if (adc_value->buf_idx < VOL_BUF_SIZE) {
            continue;
        }
        adc_value->buf_idx = 0;

        uint16_t new_value = app_calculate_average(adc_value->vol_buf, VOL_BUF_SIZE);
        if (new_value < p_status->vol_range.min || new_value > p_status->vol_range.max) {
            continue; // 检查平均值是否在有效范围
        }
        if (!p_status->k_press && !temp_common->enter_config) { // 处理按键按下

#if defined PANEL_3KEY
            uint8_t main_key;

            if (i == 0 || i == 1) {
                main_key = 0;
                APP_SET_GPIO(get_panel_pins()->led_y_pin[0], true);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[1], true);
            } else {
                main_key = i;
                APP_SET_GPIO(get_panel_pins()->led_y_pin[main_key], true);
            }

            delay_1ms(50);
            if (i == 0 || i == 1) {
                main_key = 0;
                APP_SET_GPIO(get_panel_pins()->led_y_pin[0], false);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[1], false);
            } else {
                main_key = i;
                APP_SET_GPIO(get_panel_pins()->led_y_pin[main_key], false);
            }
            p_status->k_press = true;
            temp_common->key_long_press = true;
            temp_common->key_long_count = 0;

            switch_api_button_event_handler(main_key, 0);
#elif defined PANEL_1KEY
            uint8_t main_key;

            if (i == 0 || i == 1 || i == 2 || i == 3 || i == 4 || i == 5) {
                main_key = 0;
                APP_SET_GPIO(get_panel_pins()->led_y_pin[0], true);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[1], true);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[2], true);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[3], true);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[4], true);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[5], true);
            } else {
                main_key = i;
                APP_SET_GPIO(get_panel_pins()->led_y_pin[main_key], true);
            }

            delay_1ms(50);
            if (i == 0 || i == 1 || i == 2 || i == 3 || i == 4 || i == 5) {
                main_key = 0;
                APP_SET_GPIO(get_panel_pins()->led_y_pin[0], false);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[1], false);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[2], false);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[3], false);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[4], false);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[5], false);
            } else {
                main_key = i;
                APP_SET_GPIO(get_panel_pins()->led_y_pin[main_key], false);
            }
            p_status->k_press = true;
            temp_common->key_long_press = true;
            temp_common->key_long_count = 0;

            switch_api_button_event_handler(main_key, 0);
#elif defined PANEL_2KEY
            uint8_t main_key;
            if (i == 0 || i == 3) {
                main_key = 0;
                APP_SET_GPIO(get_panel_pins()->led_y_pin[0], true);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[3], true);
            } else if (i == 1 || i == 2) {
                main_key = 1;
                APP_SET_GPIO(get_panel_pins()->led_y_pin[1], true);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[2], true);
            } else {
                main_key = i;
                APP_SET_GPIO(get_panel_pins()->led_y_pin[main_key], true);
            }

            delay_1ms(100);
            if (i == 0 || i == 3) {
                main_key = 0;
                APP_SET_GPIO(get_panel_pins()->led_y_pin[0], false);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[3], false);
            } else if (i == 1 || i == 2) {
                main_key = 1;
                APP_SET_GPIO(get_panel_pins()->led_y_pin[1], false);
                APP_SET_GPIO(get_panel_pins()->led_y_pin[2], false);
            } else {
                main_key = i;
                APP_SET_GPIO(get_panel_pins()->led_y_pin[main_key], false);
            }
            p_status->k_press = true;
            temp_common->key_long_press = true;
            temp_common->key_long_count = 0;
            switch_api_button_event_handler(main_key, 0);
#else
            APP_SET_GPIO(get_panel_pins()->led_y_pin[i], true);
            delay_1ms(100);
            APP_SET_GPIO(get_panel_pins()->led_y_pin[i], false);
            p_status->k_press = true;
            temp_common->key_long_press = true;
            temp_common->key_long_count = 0;
            switch_api_button_event_handler(i, 0); // 固定为 按下后松开
#endif
            continue;
        }
        // 处理长按
        if (temp_common->key_long_press && ++temp_common->key_long_count >= LONG_PRESS) {

            APP_PRINTF("key_long_press\n");
            // MCU_Device_Factory();
            temp_common->key_long_press = false;
        }
    }
}

void process_message_queue(void *arg)
{
    APP_Queue_ListenAndHandleMessage();
}

static void plcp_panel_tast(void *arg)
{
    timer_5s_count++;
    if (timer_5s_count >= 500) { // 5s 写产品信息
        timer_5s_count = 0;
        if (sPost_flag->wproductsecflag != 1) {
            MCU_Load_Product();
        }
    }
    if (sPost_flag->wproductsecflag == 1 && sPost_flag->wAEsecflag != 1) { // 产品信息写成功,写AE信息
        timer_1s_count++;
        if (timer_1s_count >= 100) {
            timer_1s_count = 0;
            MCU_Load_AEInfo();
        }
    }
    if (sPost_flag->wAEsecflag == 1 && sPost_flag->wWidgetsecflag != 1) { // AE信息写成功,写 Widgets
        timer_1s_count++;
        if (timer_1s_count >= 100) {
            timer_1s_count = 0;
            MCU_Load_Widgets();
        }
    }
    if (sPost_flag->wWidgetsecflag == 1 && sPost_flag->wtranferflag != 1) { // Widgets写成功,使能透传功能
        timer_1s_count++;
        if (timer_1s_count >= 100) {
            timer_1s_count = 0;
            MCU_Enable_Transmission();
        }
    }
    if (sPost_flag->wtranferflag == 1 && get_my_mac != true) { // 查询CCO地址
        if (!get_my_mac) {
            timer_1s_count++;
            if (timer_1s_count >= 100) {
                timer_1s_count = 0;
                CmdTest_MSE_GET_MAC();
                get_my_mac = true;
            }
        }
    }
    if (get_my_mac == true && get_cco_mac != true) {
        if (!get_my_mac) {
            timer_1s_count++;
            if (timer_1s_count >= 100) {
                timer_1s_count = 0;
                CmdTest_MSE_GET_CC0MAC();
                get_my_mac = true;
            }
        }
    }
    if (get_my_mac == true) {
        timer_2s_count++;
        if (timer_2s_count >= 200) {
            timer_2s_count = 0;
            CmdTest_MSE_GET_DID(); 
            CmdTest_MSE_GET_CC0MAC();
            // APP_PRINTF("APP_RxBuffer_getRxBufferMsgAmount :%d\n", APP_RxBuffer_getRxBufferMsgAmount());
            if (APP_Attribute_GetPointer()->did != 0x0000) {
                if (join_net == false) {
                    plcp_panel_blink(false);
                    switch_api_init();
                    join_net = true;
                }
            }
        }
    }
    if (join_net == false) {
        led_blink_count++;
        if (led_blink_count >= 100) {
            led_blink_count = 0;
            led_blink = !led_blink;
            plcp_panel_blink(led_blink);
        }
    }
}

static void plcp_panel_blink(bool status)
{
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        switch_adapter_led_ctrl(i, status);
    }
}

static void plcp_panel_event_handler(event_type_e event, void *params)
{
}
#endif