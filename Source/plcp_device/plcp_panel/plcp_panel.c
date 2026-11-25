#include "plcp_panel.h"
#include "../../Source/adc/adc.h"
#include "../../Source/base/base.h"
#include "../../Source/base/debug.h"
#include "../../Source/device/device_manager.h"
#include "../../Source/eventbus/eventbus.h"
#include "../../Source/plcp_device/APP_PublicAttribute.h"
#include "../../Source/plcp_device/plcp_panel/attr_table.h"
#include "../../Source/plcp_device/plcp_panel/panel_adapter.h"
#include "../../Source/plcp_device/plcp_panel/plcp_panel_api.h"
#include "../../Source/plcp_device/plcp_panel/plcp_panel_info.h"
#include "../../Source/pwm/pwm_hw.h"
#include "../../Source/timer/timer.h"
#include "systick.h"

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
static volatile uint16_t timer_5s_count = 200;
static volatile uint16_t timer_1s_count = 0;
static volatile uint16_t timer_2s_count = 50;

static void read_adc_value(void *arg);
static void plcp_panel_tast(void *arg);
static void process_message_queue(void *arg);
static void plcp_panel_event(event_type_e event, void *params);
static void plcp_panel_blink(bool status);
static void plcp_panel_blink_bled(bool status);
static void plcp_panel_key_status(bool status);
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

// #define PLCP_FACTORY // plcp 恢复出厂设置

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
    app_eventbus_subscribe(plcp_panel_event);

    APP_PRINTF("plcp_panel_init\n");
    APP_PLCSDK_Init();
    sPost_flag = APP_Postflag_GetPointer();

    CmdTest_MSE_McuVer(); // 写入软件版本
    plcp_panel_key_status(false);

    attr_led_state_table_recover(); // 读取LED控制状态
    attr_kj_mode_table_recover();   // 读取按键模式
}

static void read_adc_value(void *arg)
{
    my_adc_value.vol = ADC_TO_VOL(app_get_adc_value()[0]);
    read_adc(my_panel_status, &my_common_panel, &my_adc_value);
}

static void read_adc(panel_status_t *temp_status, common_panel_t *temp_common, adc_value_t *adc_value)
{
    // APP_PRINTF("valeu:%d\n", adc_value->vol);
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

#if defined PLCP_FACTORY

            if (i == 0) {
                temp_common->key_short_count++;
                temp_common->key_short_timerout = 0;
                APP_PRINTF("key_short_count:%d\n", temp_common->key_short_count);
            }
#else
            temp_common->key_long_press = true;
#endif
            return;
        }
    }

#if defined PLCP_FACTORY

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
#endif

    // 处理长按
    if (temp_common->key_long_press && ++temp_common->key_long_count >= LONG_PRESS) {
        temp_common->key_long_press = false;

#if defined PLCP_FACTORY
        led_blink = true;
        CmdTest_MSE_Factory();
#else
        CmdTest_MSE_Apply_net(DEV_TYPE, KEY_NUMBER);
#endif

        // MCU_Device_Factory();
        // CmdTest_MSE_Apply_net(DEV_TYPE, KEY_NUMBER);
        // APP_PRINTF("CmdTest_MSE_Apply_net\n");
    }
}

static void plcp_panel_event(event_type_e event, void *params)
{
    switch (event) {
    case EVENT_NETWORK_SUCC:
        led_blink = true;
        APP_PRINTF("EVENT_NETWORK_SUCC\n");
        // plcp_panel_blink_bled(false);
        // // delay_1ms(100);
        // plcp_panel_blink_bled(true);
        break;
    default:
        break;
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
    if (sPost_flag->wtranferflag == 1 && get_my_mac != true) { // 获取自己的 mac
        if (!get_my_mac) {
            timer_1s_count++;
            if (timer_1s_count >= 100) {
                timer_1s_count = 0;
                uint8_t selfMacAddr[6];
                memcpy(selfMacAddr, APP_Attribute_GetPointer()->selfMacAddr, 6);
                if (selfMacAddr[0] == 0 && selfMacAddr[1] == 0 && selfMacAddr[2] == 0 && selfMacAddr[3] == 0 && selfMacAddr[4] == 0 && selfMacAddr[5] == 0) {
                    CmdTest_MSE_GET_MAC();
                } else {
                    get_my_mac = true;
                }
            }
        }
    }
    if (get_my_mac == true && get_cco_mac != true) { // 获取CCO 的 mac
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
                if (join_net == false) { // 由未入网变为入网
                    // plcp_panel_blink(false); // 关闭所有指示灯
                    switch_api_init();

                    join_net = true;
                }
            } else {
                if (join_net == true) { // 有入网变为为入网
                    join_net = false;   // 指示灯闪烁
                }
            }

            uint8_t selfMacAddr[6];
            memcpy(selfMacAddr, APP_Attribute_GetPointer()->selfMacAddr, 6);

            if (selfMacAddr[0] == 0 && selfMacAddr[1] == 0 && selfMacAddr[2] == 0 && selfMacAddr[3] == 0 && selfMacAddr[4] == 0 && selfMacAddr[5] == 0) {
                // MAC 地址全为 0
                // CmdTest_MSE_GET_MAC();
            }
        }
    }

    if (join_net == false) {
#if 0 // 不再闪烁
        led_blink_count++;
        if (led_blink_count >= 100) {
            led_blink_count = 0;
            led_blink = !led_blink;
            plcp_panel_blink(led_blink);
        }
#endif
    }

    if (led_blink) { // 背光灯闪烁
        led_blink_count++;

#if defined PLCP_FACTORY
        if (led_blink_count == 1) {
            plcp_panel_blink_bled(false); // 灭
        }
        if (led_blink_count == 50) {     // 每10次执行一次
            plcp_panel_blink_bled(true); // 亮
        }
        if (led_blink_count == 100) {     // 每20次执行一次
            plcp_panel_blink_bled(false); // 灭
        }
        if (led_blink_count >= 150) {
            plcp_panel_blink_bled(true);
        }
        if (led_blink_count >= 200) {
            led_blink_count = 0;
            plcp_panel_blink_bled(false);
            led_blink = false;
            NVIC_SystemReset();
        }
#else
        if (led_blink_count == 1) {
            plcp_panel_blink_bled(false); // 灭
        }
        if (led_blink_count == 20) {     // 每10次执行一次
            plcp_panel_blink_bled(true); // 亮
        }
        if (led_blink_count == 40) {      // 每20次执行一次
            plcp_panel_blink_bled(false); // 灭
        }
        if (led_blink_count >= 60) { // 总共闪3次
            led_blink_count = 0;
            led_blink = false;
            plcp_panel_blink_bled(true);
        }
#endif
    }
}

// LED 闪烁
static void plcp_panel_blink(bool status)
{
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        switch_led_ctrl(i, status);
    }
}

// 背光灯闪烁
static void plcp_panel_blink_bled(bool status)
{
    // const plcp_panel_pin_t *p_pins = get_panel_pins();

    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        switch_led_b_ctrl(i, status ? 100 : 0, 0);

        // pwm_hw_set_duty(get_panel_pins()->led_w_pin[i], status ? 1000 : 0);
    }
}

static void plcp_panel_key_status(bool status)
{
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        attr_key_state_table_set(i, status);
    }
}

#endif