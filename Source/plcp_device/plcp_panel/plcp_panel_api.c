#include "plcp_panel_api.h"
#include "../../Source/base/debug.h"
#include "../../Source/device/device_manager.h"
#include "../../Source/plcp_device/plcp_panel/attr_table.h"
#include "../../Source/plcp_device/plcp_panel/plcp_panel_info.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_bind.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_scene.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_special_scene.h"
#include "../../Source/pwm/pwm_hw.h"
#include "../../Source/timer/timer.h"
#include "systick.h"
#include <stdint.h>
#if defined PLCP_PANEL

void timer_curtain_exe(void *arg);
void switch_api_init(void)
{
    // attr_relay_table_recover();      // 读取继电器状态
    // attr_bk_table_recover();         // 读取闪烁状态
    // attr_led_table_recover(); // 读取LED状态
    // attr_delay_table_recover();      // 读取延时状态

    // attr_led_enable_table_recover(); // 读取LED与RELAY关联表
    // attr_led_b_state_table_recover();    // 读取背光LED状态
    // attr_ad_led_b_state_table_recover(); // 读取可调背光LED状态
    // attr_relay_power_up_table_recover(); // 读取上电状态
    // relay_table_init();
    // delay_table_init();    // 执行延时
    // relay_powe_up_table_init(); // 执行上电状态
    // led_table_init();

    attr_led_state_table_recover(); // 读取LED控制状态
    attr_kj_mode_table_recover();   // 读取按键模式
    attr_key_state_table_recover();
}

void switch_api_button_event_handler(uint8_t id, uint8_t event) // 0:release 1:press
{
#define KEY_STATE_OPEN 1
#define KEY_STATE_CLOSE 0
#define KEY_STATE_STOP 2

#define EVENT_ID_OPEN "_open"
#define EVENT_ID_CLOSE "_close"
#define EVENT_ID_STOP "_close"

#define EVENT_TYPE_SWITCH "switch"
#define EVENT_TYPE_SCENE "scene"
#define EVENT_TYPE_CURTAIN "curtain"

#define EVENT_SE 1

    uint8_t keyState;
    APP_PRINTF("key_num:%d\n", id);
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

            keyState = !attr_key_state_table_get(id); // 获取当前按键的状态

            APP_SET_GPIO(get_panel_pins()->led_y_pin[id], true);
            delay_1ms(100);
            APP_SET_GPIO(get_panel_pins()->led_y_pin[id], false);
            delay_1ms(100);
            APP_SET_GPIO(get_panel_pins()->led_y_pin[id], !keyState); // 恢复到原来的状态

            strcpy(eventType, EVENT_TYPE_SCENE); // 标记事件为"scene",后续上报

        } else if (attr_kj_mode_table_get(id) == SWITCH_e) { // 开关模式
            APP_PRINTF("SWITCH_e\n");

            keyState = !attr_key_state_table_get(id);
            attr_key_state_table_set(id, keyState);

            if (keyState) {
                attr_led_b_table_set(id, 0);
                attr_led_table_set(id, 1);
            } else {
                attr_led_b_table_set(id, 0x64);
                attr_led_table_set(id, 0);
            }
            strcpy(eventType, EVENT_TYPE_SWITCH);

        } else if (attr_kj_mode_table_get(id) == CURTAIN_e) { // 窗帘模式

            if (!curtain_exe) { // 第一次按下窗帘关按键,开启定时器
                curtain_exe = true;
                APP_Curtain_timer();
            }
            keyState = !attr_key_state_table_get(id);
            const curtain_t *curtain = APP_GetCurtain(id);
            if (curtain->is_exe == true) {
                keyState = KEY_STATE_STOP;
            }
            strcpy(eventType, EVENT_TYPE_CURTAIN);
        }

        if (keyState == KEY_STATE_OPEN) {
            PLCP_WigetEventWithType(EVENT_SE, eventAEI, EVENT_ID_OPEN, keyState, eventType);
        } else if (keyState == KEY_STATE_CLOSE) {
            PLCP_WigetEventWithType(EVENT_SE, eventAEI, EVENT_ID_CLOSE, keyState, eventType);
        } else if (keyState == KEY_STATE_STOP) {
            PLCP_WigetEventWithType(EVENT_SE, eventAEI, EVENT_ID_STOP, keyState, eventType);
        }
    }
}
#endif