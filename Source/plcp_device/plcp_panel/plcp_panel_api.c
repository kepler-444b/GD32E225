#include "plcp_panel_api.h"
#include "../../Source/base/debug.h"
#include "../../Source/device/device_manager.h"
#include "../../Source/plcp_device/plcp_panel/attr_table.h"
#include "../../Source/plcp_device/plcp_panel/button_state.h"
#include "../../Source/plcp_device/plcp_panel/plcp_panel_info.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_bind.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_scene.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_special_scene.h"
#include "../../Source/pwm/pwm_hw.h"
#include "../../Source/timer/timer.h"
#include <stdint.h>
#if defined PLCP_PANEL

// static uint8_t curtain_hold[KEY_NUMBER];

// 此结构体用于管理窗帘按键
typedef struct {
    bool tag_status;
    bool src_status; // 用于边沿检测
    uint8_t count;
} curtain_hold_t;
static curtain_hold_t my_curtain_hold[KEY_NUMBER];
static bool curtain_exe; // 是否已开启定时器

// 窗帘按键互斥映射表
static const uint8_t opposite[KEY_NUMBER] = {3, 2, 1, 0};

void timer_curtain_exe(void *arg);
void switch_api_init(void)
{
    // attr_relay_table_recover();      // 读取继电器状态
    // attr_bk_table_recover();         // 读取闪烁状态
    // attr_led_table_recover(); // 读取LED状态
    // attr_delay_table_recover();      // 读取延时状态
    attr_led_state_table_recover(); // 读取LED控制状态
    // attr_led_enable_table_recover(); // 读取LED与RELAY关联表
    // attr_led_b_state_table_recover();    // 读取背光LED状态
    // attr_ad_led_b_state_table_recover(); // 读取可调背光LED状态
    // attr_relay_power_up_table_recover(); // 读取上电状态
    attr_kj_mode_table_recover(); // 读取按键模式

    // relay_table_init();
    // delay_table_init();    // 执行延时
    // bk_table_init();       // 执行闪烁
    ad_led_b_table_init(); // 执行可调背光LED
    // led_table_init();
    // relay_powe_up_table_init(); // 执行上电状态
}

void switch_api_button_event_handler(uint8_t id, uint8_t event) // 0:release 1:press
{
#define KEY_STATE_OPEN 1
#define KEY_STATE_CLOSE 0
#define KEU_STATE_STOP 2

#define EVENT_ID_OPEN "_open"
#define EVENT_ID_CLOSE "_close"
#define EVENT_ID_STOP "_stop"

#define EVENT_TYPE_SWITCH "switch"
#define EVENT_TYPE_SCENE "scene"
#define EVENT_TYPE_CURTAIN "curtain"

#define EVENT_SE 1

    uint8_t keyState;

    if (id >= KEY_NUMBER) {
        return;
    }

    if (night_scene_state_get() == 1) { // 夜灯模式
        APP_PRINTF("send close\n");
        night_scene_off_send(); // 关闭夜景
        return;
    }
    if (night_scene_state_get() == 2) { // 即将进入夜灯模式
        APP_PRINTF("stop close\n");
        delay_scene_stop(); // 停止延时场景任务
    }

    if (event == 0) {
        static char eventAEI[3];
        static char eventType[16]; // 用于存放本次按钮事件的类型

        memset(eventAEI, 0, sizeof(eventAEI));
        memset(eventType, 0, sizeof(eventType));

        snprintf(eventAEI, 3, "k%d", id + 1);        // 构造 AEI
        if (attr_kj_mode_table_get(id) == SCENE_e) { // 场景模式
            // APP_PRINTF("SCENE_e\n");
            keyState = !attr_key_state_table_get(id);
            APP_PRINTF("keyState:%d\n", keyState);
            attr_key_state_table_set(id, keyState);
            strcpy(eventType, EVENT_TYPE_SCENE); // 标记事件为"scene",后续上报

        } else if (attr_kj_mode_table_get(id) == SWITCH_e) { // 开关模式
            APP_PRINTF("SWITCH_e\n");
            keyState = !attr_key_state_table_get(id);
            attr_key_state_table_set(id, keyState);
            delay_timer_active(id, !attr_relay_table_get(id));
            strcpy(eventType, EVENT_TYPE_SWITCH);

        } else if (attr_kj_mode_table_get(id) == CURTAIN_e) { // 窗帘模式
            APP_PRINTF("CURTAIN_e\n");
            if (!curtain_exe) { // 第一次按下窗帘关按键,开启定时器
                curtain_exe = true;
                app_timer_start(100, timer_curtain_exe, true, NULL, "curtain_hold");
            }
            curtain_hold_t *c = &my_curtain_hold[id];

            if (c->tag_status) { // 已点亮 → STOP
                keyState = KEU_STATE_STOP;
                c->tag_status = false;
                c->src_status = true;
            } else { // 未点亮 → OPEN
                keyState = KEY_STATE_OPEN;
                c->tag_status = true;
                c->src_status = false;
                c->count = 0;

                // 关闭互斥按键
                uint8_t opp = opposite[id];
                my_curtain_hold[opp].tag_status = false;
                my_curtain_hold[opp].src_status = true;
                my_curtain_hold[opp].count = 0;
            }
            strcpy(eventType, EVENT_TYPE_CURTAIN);
        }

        if (keyState == KEY_STATE_OPEN) {
            PLCP_WigetEventWithType(EVENT_SE, eventAEI, EVENT_ID_OPEN, 0xffffffff, eventType);
        } else if (keyState == KEY_STATE_CLOSE) {
            PLCP_WigetEventWithType(EVENT_SE, eventAEI, EVENT_ID_CLOSE, 0xffffffff, eventType);
        } else if (keyState == KEU_STATE_STOP) {
            PLCP_WigetEventWithType(EVENT_SE, eventAEI, EVENT_ID_STOP, 0xffffffff, eventType);
        }
    }
}

void timer_curtain_exe(void *arg)
{
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {

        curtain_hold_t *c = &my_curtain_hold[i];

        bool rising = (c->tag_status && !c->src_status);  // 上升沿
        bool falling = (!c->tag_status && c->src_status); // 下降沿

        if (rising) {
            APP_SET_GPIO(get_panel_pins()->led_y_pin[i], true);
            pwm_hw_set_duty(get_panel_pins()->led_w_pin[i], 0);
            c->count = 0;
        }

        if (c->tag_status) {
            c->count++;
            if (c->count >= 30) { // 3 秒自动关闭
                APP_SET_GPIO(get_panel_pins()->led_y_pin[i], false);
                pwm_hw_set_duty(get_panel_pins()->led_w_pin[i], 1000);
                c->tag_status = false;
                c->src_status = false;
                c->count = 0;
            }
        }

        if (falling) {
            APP_SET_GPIO(get_panel_pins()->led_y_pin[i], false);
            pwm_hw_set_duty(get_panel_pins()->led_w_pin[i], 1000);
            c->tag_status = false;
            c->src_status = false;
            c->count = 0;
        }

        // 更新上一轮状态
        c->src_status = c->tag_status;
    }
}
#endif