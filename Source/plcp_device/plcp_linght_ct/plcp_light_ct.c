#include "plcp_light_ct.h"
#include <stdio.h>
#include "../../Source/pwm/pwm_hw.h"
#include "../../Source/device/device_manager.h"
#include "../../Source/timer/timer.h"
#include "../../Source/plcp_device/APP_PublicAttribute.h"
#include "../../Source/plcp_device/plcp_linght_ct/plcp_light_ct_info.h"
#include "../../Source/plcp_device/plcp_linght_ct/light_ct_attr_table.h"
#if defined PLCP_LIGHT_CT

// 向前声明
void APP_PLCSDK_Init(void);
void CmdTest_MSE_GET_DID(void);
void CmdTest_MSE_GET_MAC(void);
void CmdTest_MSE_GET_CC0MAC();

// 函数声明
static void process_message_queue(void *arg);
static void plcp_panel_tast(void *arg);

static uint16_t timer_8s_count;
static uint16_t timer_7s_count;
static volatile uint16_t timer_5s_count;
static volatile uint16_t timer_1s_count;
static uint16_t timer_2s_count;
static volatile uint16_t timer_3s_count;

static bool get_cco_mac         = false;
static bool get_my_mac          = false;
static bool join_net            = false;
static bool led_blink           = false;
static uint16_t led_blink_count = 0;

const sPost_wflash *sPost_flag = NULL;
void plcp_light_ct_init(void)
{
    plcp_light_ct_pins_init();
    app_pwm_hw_init();
    app_pwm_hw_add_pin(PWM_PB0);
    app_pwm_hw_add_pin(PWM_PB1);

    app_timer_start(10, plcp_panel_tast, true, NULL, "task");
    app_timer_start(50, process_message_queue, true, NULL, "message_queue");
    // app_flash_mass_erase();
    APP_PLCSDK_Init();
    attr_light_ct_table_set(true);
    sPost_flag = APP_Postflag_GetPointer();
}

static void process_message_queue(void *arg)
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
            if (APP_Attribute_GetPointer()->did != 0x0000) { 
                join_net = true; // 入网标识
            }
        }
    }
    if (join_net == false) {
        led_blink_count++;
        if (led_blink_count >= 600) {
            led_blink_count = 0;
            led_blink       = !led_blink;
            if (led_blink) {
                app_set_pwm_hw_fade(PWM_PB0, 1000, 5000); // 暖白 PB0
                app_set_pwm_hw_fade(PWM_PB1, 1000, 5000); // 白光 PB1
            } else {
                app_set_pwm_hw_fade(PWM_PB0, 0, 5000); // 暖白 PB0
                app_set_pwm_hw_fade(PWM_PB1, 0, 5000); // 白光 PB1
            }
        }
    }
}

#endif