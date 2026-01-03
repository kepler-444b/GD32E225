#include "gd32e23x.h"
#include "quick_box.h"
#include "device_manager.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "../gpio/gpio.h"
#include "../base/debug.h"
#include "../usart/usart.h"
#include "../protocol/protocol.h"
#include "../pwm/pwm.h"
#include "../eventbus/eventbus.h"
#include "../config/config.h"
#include "../base/base.h"
#include "../zero/zero.h"
#include "../device/pcb_device.h"
#include "../timer/timer.h"

#if defined QUICK_BOX

    #define TO_500(x)          ((x) <= 0 ? 0 : ((x) >= 100 ? 500 : ((x) * 500) / 100))
    #define SCALE_5_TO_5000(x) ((x) <= 0 ? 0 : ((x) >= 5 ? 5000 : ((x) * 5000) / 5))

    #define FUNC_PARAMS        at_frame_t *data, const quick_cfg_t *temp_cfg
    #define FUNC_ARGS          data, temp_cfg

    // Traverse
    #define PROCESS(cfg, ...)                             \
        do {                                              \
            for (uint8_t _i = 0; _i < LED_NUMBER; _i++) { \
                const quick_cfg_t *p_cfg = &(cfg)[_i];    \
                __VA_ARGS__                               \
            }                                             \
        } while (0)

    #define FADE_TIME  1000
    #define LONG_PRESS 3000

typedef struct {
    bool key_status;          // 按键状态
    bool led_filck;           // 闪烁
    bool enter_config;        // 进入配置状态
    uint16_t key_long_count;  // 长按计数
    uint32_t led_filck_count; // 闪烁计数
    bool ind_led;             // 通信指示灯

    bool remove_card;           // 拔卡状态
    uint16_t remove_card_count; // 拔卡信号计数

} common_quick_t;
static common_quick_t my_common_quick = {0};

// Function declaration
static void quick_box_data_cb(at_frame_t *data);
static void quick_event_handler(event_type_e event, void *params);
static void quick_box_timer(TimerHandle_t xTimer);
static void quick_ctrl_led_all(bool status);
static void quick_fast_exe(uint8_t len_num, uint16_t lum);
static void process_led_flicker(void);
static void quick_close_all(void);
static void quick_power_status(void);

static void quick_scene_exe(FUNC_PARAMS);
static void quick_all_close(FUNC_PARAMS);
static void quick_all_on_off(FUNC_PARAMS);
static void quick_scene_mode(FUNC_PARAMS);
static void quick_light_mode(FUNC_PARAMS);

void quick_box_init(void)
{
    // APP_PRINTF("quick_box_init\n");
    quick_box_gpio_init();
    app_load_config();
    app_eventbus_subscribe(quick_event_handler); // Subscribe eventbus

    // Initialize a static timer
    static StaticTimer_t QuickStaticBuffer;
    static TimerHandle_t QuickTimerHandle = NULL;

    QuickTimerHandle = xTimerCreateStatic(
        "QuickTimer",
        pdMS_TO_TICKS(1),
        pdTRUE,
        NULL,
        quick_box_timer,
        &QuickStaticBuffer);

    if (xTimerStart(QuickTimerHandle, 0) != true) { // Start timer
        APP_ERROR("QuickTimerHandle error");
    }
    app_pwm_init();
    app_pwm_add_pin(PB7);
    app_pwm_add_pin(PB6);
    app_pwm_add_pin(PB5);

    app_set_pwm_fade(PB5, 0, 5000);
    app_set_pwm_fade(PB6, 0, 5000);
    app_set_pwm_fade(PB7, 0, 5000);

    // APP_SET_GPIO(PB5, true);
    // APP_SET_GPIO(PB6, true);
    // APP_SET_GPIO(PB7, true);

    // APP_SET_GPIO(PB13, true);
    // APP_SET_GPIO(PB14, true);
    // APP_SET_GPIO(PB15, true);
    


    app_zero_init(EXTI_11); // Initialize of zero-crossing detection external interrupt
}

static void quick_box_data_cb(at_frame_t *data)
{
    APP_SET_GPIO(PB1, my_common_quick.ind_led = !my_common_quick.ind_led);
    APP_PRINTF_BUF("[RECV]", data->data, data->length);
    const quick_cfg_t *temp_cfg = app_get_quick_cfg();

    if (data->data[0] == CARD_HEAD &&
        data->data[1] == CARD_CMD) {
        switch (data->data[2]) {
            case INSERT_CAED: // Received insert card signal
                quick_power_status();
                my_common_quick.remove_card       = false;
                my_common_quick.remove_card_count = 0;
                break;
            case REMOVE_CARD: // Received remove card signal
                my_common_quick.remove_card = true;
                break;
            default:
                break;
        }
    } else if (data->data[0] == PANEL_HEAD) {
        switch (data->data[1]) {
            case ALL_CLOSE:
                quick_all_close(FUNC_ARGS);
                break;
            case ALL_ON_OFF:
                quick_all_on_off(FUNC_ARGS);
                break;
            case SCENE_MODE:
                quick_scene_mode(FUNC_ARGS);
                break;
            case LIGHT_MODE:
                quick_light_mode(FUNC_ARGS);
            default:
                return;
        }
    }
}

static void quick_event_handler(event_type_e event, void *params)
{
    switch (event) {
        case EVENT_ENTER_CONFIG: { // Enter configuration mode
            quick_ctrl_led_all(true);
            my_common_quick.enter_config = true;
        } break;
        case EVENT_EXIT_CONFIG: { // Exit configuration mode
            quick_ctrl_led_all(false);
            my_common_quick.enter_config = false;
            NVIC_SystemReset(); // Software reset
        } break;
        case EVENT_SAVE_SUCCESS:
            my_common_quick.led_filck = true;
            break;
        case EVENT_RECEIVE_CMD: {
            at_frame_t *frame = (at_frame_t *)params;
            quick_box_data_cb(frame);
        } break;
        default:
            return;
    }
}

static void quick_box_timer(TimerHandle_t xTimer)
{
    my_common_quick.key_status = APP_GET_GPIO(PA0);

    if (!my_common_quick.key_status) { // Key is pressed
        my_common_quick.key_long_count++;
        if (my_common_quick.key_long_count >= 5000) { // Long press to trigger
            app_send_cmd(0, 0, APPLY_CONFIG, COMMON_CMD, false);
            my_common_quick.key_long_count = 0;
            APP_PRINTF("long_press\n");
        }
    } else if (my_common_quick.key_status && my_common_quick.key_long_count) {
        my_common_quick.key_long_count = 0;
    }
    if (my_common_quick.led_filck) { // Led start to flicker
        process_led_flicker();
    }
    if (my_common_quick.remove_card) { // Delay execution for a period of time
        my_common_quick.remove_card_count++;
        if (my_common_quick.remove_card_count >= 30000) {
            quick_close_all();
            // Both remove_card and remove_card_count are reset
            my_common_quick.remove_card       = false;
            my_common_quick.remove_card_count = 0;
        }
    }
}

static void quick_ctrl_led_all(bool status)
{
    for (uint8_t i = 0; i < 3; i++) {
        app_set_pwm_duty(app_get_quick_cfg()[i].led_pin, status ? 500 : 0);
    }
}

static void process_led_flicker(void) // Led flicker
{
    my_common_quick.led_filck_count++;
    if (my_common_quick.led_filck_count <= 500) {
        quick_ctrl_led_all(false);
    } else {
        quick_ctrl_led_all(true);
        // Both led_filck_count and led_filck are reset
        my_common_quick.led_filck_count = 0;
        my_common_quick.led_filck       = false;
    }
}

static void quick_fast_exe(uint8_t len_num, uint16_t lum)
{
    const quick_cfg_t *temp_cfg = app_get_quick_cfg();
    if (len_num < 3) {
        app_set_pwm_fade(temp_cfg[len_num].led_pin, lum, FADE_TIME);
    } else {
        zero_set_gpio(temp_cfg[len_num].led_pin, lum);
    }
}

static void quick_power_status(void)
{
    const quick_cfg_t *temp_cfg = app_get_quick_cfg();
    for (uint8_t i = 0; i < LED_NUMBER; i++) {
        if (BIT3(temp_cfg[i].perm)) {
            if (i < 3) {
                app_set_pwm_fade(temp_cfg[i].led_pin, 500, FADE_TIME);
            } else {
                zero_set_gpio(temp_cfg[i].led_pin, true);
            }
        }
    }
}

static void quick_close_all(void)
{
    const quick_cfg_t *temp_cfg = app_get_quick_cfg();
    for (uint8_t i = 0; i < LED_NUMBER; i++) {
        if (i < 3) {
            app_set_pwm_fade(temp_cfg[i].led_pin, 0, FADE_TIME);
        } else {
            zero_set_gpio(temp_cfg[i].led_pin, false);
        }
    }
}

/* ***************************************** Exrcute the key ***************************************** */
static void quick_all_close(FUNC_PARAMS)
{
    PROCESS(temp_cfg, {
        if (!BIT5(p_cfg->perm)) {
            continue;
        }
        if (H_BIT(data->data[4]) != H_BIT(p_cfg->area) &&
            H_BIT(data->data[4]) != 0xF) {
            continue;
        }
        quick_fast_exe(_i, 0);
    });
}

static void quick_all_on_off(FUNC_PARAMS)
{
    PROCESS(temp_cfg, {
        if (!BIT5(p_cfg->perm)) {
            continue;
        }
        if (H_BIT(data->data[4]) != 0xF &&
            H_BIT(data->data[4]) != H_BIT(p_cfg->area)) { // Matching partition
            continue;
        }
        quick_fast_exe(_i, data->data[2] ? TO_500(p_cfg->lum) : 0);
    });
}

static void quick_scene_mode(FUNC_PARAMS) // Scene mode
{
    PROCESS(temp_cfg, {
        if (L_BIT(data->data[4]) != 0xF &&
            L_BIT(data->data[4]) != L_BIT(p_cfg->area)) {
            continue; // If the partition is different from this scene's partition and the scene partition is not 0xF,then skpi it
        }
        uint8_t mask = data->data[7] & p_cfg->scene_group;
        if (!mask) { // If this scene_group is not selected,then skip it
            continue;
        }
        uint8_t scene_index = __builtin_ctz(mask); // Get scene index
        if (_i < 3) {
            app_set_pwm_fade(p_cfg->led_pin, data->data[2] ? TO_500(p_cfg->scene_lun[scene_index]) : 0, FADE_TIME);
        } else {
            zero_set_gpio(p_cfg->led_pin, data->data[2] ? p_cfg->scene_lun[scene_index] : 0);
        }
    });
}

static void quick_light_mode(FUNC_PARAMS) // Light mode
{
    PROCESS(temp_cfg, {
        if (data->data[3] != p_cfg->group && data->data[3] != 0xFF) {
            continue;
        }
        if (_i < 3) {
            app_set_pwm_fade(p_cfg->led_pin, data->data[2] ? TO_500(p_cfg->lum) : 0, FADE_TIME);
        } else {
            zero_set_gpio(p_cfg->led_pin, (data->data[2] ? p_cfg->lum : 0));
        }
    });
}
#endif