#include "plcp_panel.h"
#include "../../Source/adc/adc.h"
#include "../../Source/base/base.h"
#include "../../Source/base/debug.h"
#include "../../Source/device/device_manager.h"
#include "../../Source/plcp_device/APP_PublicAttribute.h"
#include "../../Source/plcp_device/plcp_panel/attr_table.h"
#include "../../Source/plcp_device/plcp_panel/panel_adapter.h"
#include "../../Source/plcp_device/plcp_panel/plcp_panel_api.h"
#include "../../Source/plcp_device/plcp_panel/plcp_panel_info.h"
#include "../../Source/pwm/pwm_hw.h"
#include "../../Source/timer/timer.h"

#if defined PLCP_PANEL

// 每个按键的触发电压
typedef struct {
    uint16_t min;
    uint16_t max;
} key_vol_t;

// 用于每个按键的状态
typedef struct {

    bool k_press;
    const key_vol_t vol_range; // 按键电压范围
} panel_status_t;

// 电压缓存结构体
typedef struct
{
    uint8_t buf_idx; // 缓冲区下标
    uint16_t vol;    // 电压值
    uint16_t vol_buf[VOL_BUF_SIZE];
} adc_value_t;

// 面板通用结构体
typedef struct
{
    uint8_t key_short_count;     // 短按计数
    uint16_t key_short_timerout; // 短按超时

    bool key_long_press;      // 长按状态
    uint16_t key_long_count;  // 长按计数
    uint16_t led_filck_count; // 闪烁计数
} common_panel_t;

static panel_status_t my_panel_status[KEY_NUMBER] = {
    PANEL_VOL_RANGE_DEF,
};

static common_panel_t my_common_panel;
static adc_value_t my_adc_value;
static volatile uint16_t timer_5s_count;
static volatile uint16_t timer_1s_count;
static volatile uint16_t timer_2s_count;
static volatile uint16_t timer_3s_count;

static void read_adc_value(void *arg);
static void plcp_panel_tast(void *arg);
static void process_message_queue(void *arg);
static void plcp_panel_blink(bool status);
static void read_adc(panel_status_t *temp_status, common_panel_t *temp_common, adc_value_t *adc_value);

// 向前声明
void APP_PLCSDK_Init(void);
void CmdTest_MSE_GET_DID(void);
void CmdTest_MSE_GET_MAC(void);
void CmdTest_MSE_GET_CC0MAC(void);
void CmdTest_MSE_McuVer(void);

const sPost_wflash *sPost_flag = NULL;
static bool get_cco_mac = false;
static bool get_my_mac = false;
static bool join_net = false;
static bool led_blink = false;
static uint16_t led_blink_count = 0;

void plcp_panel_init(void)
{
    APP_PRINTF("plcp_panel_init\n");

    plcp_panel_pins_init();

    adc_channel_t my_adc_channel = {0};
    my_adc_channel.adc_channel_0 = true;
    app_adc_init(&my_adc_channel);

    app_pwm_hw_init();

    app_pwm_hw_add_pin(PWM_PB0);
    app_pwm_hw_add_pin(PWM_PB1);
    app_pwm_hw_add_pin(PWM_PB6);
    app_pwm_hw_add_pin(PWM_PB7);
    app_pwm_hw_add_pin(PWM_PA6);
    app_pwm_hw_add_pin(PWM_PA7);

    app_timer_start(5, read_adc_value, true, NULL, "read_adc");              // adc 检测
    app_timer_start(10, plcp_panel_tast, true, NULL, "task");                // 初始化状态机
    app_timer_start(50, process_message_queue, true, NULL, "message_queue"); // 消息队列

    APP_PRINTF("plcp_panel_init\n");
    APP_PLCSDK_Init();
    sPost_flag = APP_Postflag_GetPointer();
    CmdTest_MSE_McuVer();
    // app_flash_mass_erase(); // 擦除整个扇区(测试使用)
}

static void read_adc_value(void *arg)
{
    my_adc_value.vol = ADC_TO_VOL(app_get_adc_value()[0]);
    read_adc(my_panel_status, &my_common_panel, &my_adc_value);
}

static void read_adc(panel_status_t *temp_status, common_panel_t *temp_common, adc_value_t *adc_value)
{
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
        if (!p_status->k_press) { // 处理按键按下
            p_status->k_press = true;
            switch_api_button_event_handler(i, 0); // 固定为 按下后松开

            if (i == 0) {
                temp_common->key_short_count++;
                temp_common->key_short_timerout = 0;
                APP_PRINTF("key_short_count:%d\n", temp_common->key_short_count);
            }
            return;
        }
    }

    if (temp_common->key_short_count != 0) {
        temp_common->key_short_timerout++;
        if (temp_common->key_short_timerout >= 200) {
            temp_common->key_short_count = 0;
            APP_PRINTF("clear\n");
        }
        if (temp_common->key_short_count >= 3) {
            temp_common->key_long_press = true;
        }
    }

    // 处理长按
    if (temp_common->key_long_press && ++temp_common->key_long_count >= LONG_PRESS) {
        temp_common->key_long_press = false;
        MCU_Device_Factory();
    }
}

static void process_message_queue(void *arg)
{
    APP_Queue_ListenAndHandleMessage();
}

// 面板初始化状态机
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

// LED 闪烁
static void plcp_panel_blink(bool status)
{
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        switch_led_ctrl(i, status);
    }
}

#endif