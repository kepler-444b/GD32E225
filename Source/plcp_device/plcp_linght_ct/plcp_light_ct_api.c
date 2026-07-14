#include "plcp_light_ct_api.h"

#include "../../Source/plcp_device/plcp_linght_ct/plcp_light_ct_info.h"
#include "../../Source/plcp_device/plcp_panel/attr_table.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_bind.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_scene.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_special_scene.h"

void light_api_button_event_handler(uint8_t id, uint8_t event) // 0:release 1:press
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
    if (id >= KEY_NUMBER) {
        return;
    }
    for (uint8_t i = 0; i < NIGHT_SCENE_MAX; i++) {
        if (night_scene_state_get(i) == 0x01) { // 如果当前处于某个夜灯的"夜灯模式"
            night_scene_off_send(i);            // 发送该夜灯的关闭夜景
            return;
        }
        if (night_scene_state_get(i) == 0x02) { // 如果当前处于某个夜灯的"即将进入夜灯模式"
            delay_scene_stop(i);                // 则打断延时执行夜灯模式
        }
    }
    // if (night_scene_state_get() == 1) { // 夜灯模式
    //     APP_PRINTF("send close\n");
    //     night_scene_off_send(); // 关闭夜景
    //     return;
    // }
    // if (night_scene_state_get() == 2) { // 即将进入夜灯模式
    //     APP_PRINTF("stop close\n");
    //     delay_scene_stop(); // 停止延时场景任务
    // }

    if (event == 0) {
        static char eventAEI[3];
        static char eventType[16]; // 用于存放本次按钮事件的类型
        memset(eventAEI, 0, sizeof(eventAEI));
        memset(eventType, 0, sizeof(eventType));

        snprintf(eventAEI, 3, "k%d", id + 1); // 构造 AEI

        keyState = !attr_key_state_table_get(id); // 获取当前按键的状态
        strcpy(eventType, EVENT_TYPE_SCENE);      // 标记事件为"scene",后续上报

        if (keyState == KEY_STATE_OPEN) {
            PLCP_WigetEventWithType(EVENT_SE, eventAEI, EVENT_ID_OPEN, keyState, eventType);
        } else if (keyState == KEY_STATE_CLOSE) {
            PLCP_WigetEventWithType(EVENT_SE, eventAEI, EVENT_ID_CLOSE, keyState, eventType);
        }
    }
}

void light_api_init(void)
{
    APP_Read_light_ct_info();
    APP_ReadBindParameter(); // 读取绑定信息
    APP_ReadAllSceneInfo();  // 读取所有场景信息
    APP_ReadAllGroupInfo();  // 读取所有群组信息

    APP_ReadDelaySceneParameter(); // 读取延时场景信息
    APP_ReadNightSceneParameter(); // 读取夜灯场景信息
}