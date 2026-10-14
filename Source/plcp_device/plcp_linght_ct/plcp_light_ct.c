#include "plcp_light_ct.h"
#include "../../Source/device/device_manager.h"
#include "../../Source/eventbus/eventbus.h"
#include "../../Source/plcp_device/APP_PublicAttribute.h"
#include "../../Source/plcp_device/plcp_linght_ct/plcp_light_ct_api.h"
#include "../../Source/plcp_device/plcp_linght_ct/plcp_light_ct_info.h"
#include "../../Source/pwm/pwm_hw.h"
#include "../../Source/timer/timer.h"
#include <stdio.h>
#if defined PLCP_LIGHT_CT

// 向前声明
void APP_PLCSDK_Init(void);
void CmdTest_MSE_GET_DID(void);
void CmdTest_MSE_GET_MAC(void);
void CmdTest_MSE_GET_CC0MAC();

// 函数声明
static void process_message_queue(void *arg);
static void plcp_panel_task(void *arg);

static uint16_t timer_8s_count;
static uint16_t timer_7s_count;
static volatile uint16_t timer_5s_count;
static volatile uint16_t timer_1s_count;
static uint16_t timer_2s_count;
static volatile uint16_t timer_3s_count;

static bool get_cco_mac = false;
static bool get_my_mac = false;
static bool join_net = false;
static bool led_blink = false;
static uint16_t led_blink_count = 0;

const sPost_wflash *sPost_flag = NULL;

static volatile bool key_pressed = false; //  按键已经被按下
static uint16_t key_pressed_count = 0;

static volatile bool key_press_pending = false; // 中断触发标记

static uint8_t click_count = 0;         // 按下次数
static uint16_t click_window_ticks = 0; // 3秒窗口倒计时 (300 * 10ms)

void plcp_light_ct_init(void)
{
    APP_PRINTF("plcp_light_ct_init\n");
    plcp_light_ct_pins_init();

    app_pwm_hw_init();
    app_pwm_hw_add_pin(PWM_PB0);
    app_pwm_hw_add_pin(PWM_PB1);

    app_timer_start(10, plcp_panel_task, true, NULL, "task");
    app_timer_start(50, process_message_queue, true, NULL, "message_queue");
    // app_flash_mass_erase();

    APP_PLCSDK_Init();
    sPost_flag = APP_Postflag_GetPointer();
}

static void process_message_queue(void *arg)
{
    APP_Queue_ListenAndHandleMessage();
}

static void plcp_panel_task(void *arg)
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
                if (join_net == false) { // 由为入网变为入网
                    light_api_init();
                    join_net = true; // 入网标识

                    // app_set_pwm_hw_fade(PWM_PB0, 0, 5000); // 暖白 PB0
                }
            } else {
                if (join_net == true) { // 由入网变为未入网
                    join_net = false;
                }
            }
        }
    }
    if (join_net == false) {
        // led_blink_count++;
        // if (led_blink_count >= 100) {
        //     led_blink_count = 0;
        //     led_blink = !led_blink;
        //     if (led_blink) {
        //         app_set_pwm_hw_fade(PWM_PB0, 1000, 1000); // 暖白 PB0
        //         app_set_pwm_hw_fade(PWM_PB1, 1000, 1000); // 白光 PB1
        //     } else {
        //         app_set_pwm_hw_fade(PWM_PB0, 0, 1000); // 暖白 PB0
        //         app_set_pwm_hw_fade(PWM_PB1, 0, 1000); // 白光 PB1
        //     }
        // }
    }

    if (click_window_ticks > 0) {
        click_window_ticks--;
    } else {
        click_count = 0; // 超时清零
        APP_PRINTF("clear\n");
    }

    // 中断触发
    if (key_press_pending) {
        key_press_pending = false; // 清除中断标记
        light_api_button_event_handler(0, 0);
        if (click_count == 0) {
            click_window_ticks = 300; // 第一次按下，启动3秒倒计时(300 * 10ms)
        }

        click_count++;
        APP_PRINTF("Click count: %d\n", click_count);

        if (click_count >= 5) {
            click_count = 0;
            click_window_ticks = 0;
            CmdTest_MSE_Apply_net(DEV_TYPE, KEY_NUMBER); // 申请入网
        }
    }

    if (key_pressed == true) { // 按键按下状态
        key_pressed_count++;
        if (key_pressed_count >= 500) { // 长按 5s
            key_pressed = false;
            CmdTest_MSE_Apply_net(DEV_TYPE, KEY_NUMBER); // 申请入网
        }
    }
}

// 干接点触发中断服务函数
void EXTI0_1_IRQHandler(void)
{
    if (RESET != exti_interrupt_flag_get(EXTI_0)) {
        exti_interrupt_flag_clear(EXTI_0);

        // 读引脚电平,判断是上升沿还是下降沿触发
        if (RESET != gpio_input_bit_get(GPIOA, GPIO_PIN_0)) { // 上升沿(按键抬起)
            key_pressed = false;

        } else if (RESET == gpio_input_bit_get(GPIOA, GPIO_PIN_0)) { // 下降沿(按键按下)
            key_pressed = true;
            key_pressed_count = 0;
            key_press_pending = true;
        }
    }
}
#endif