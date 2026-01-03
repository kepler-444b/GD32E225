#ifndef _DEVICE_MANAGER_H_
#define _DEVICE_MANAGER_H_

#define PLCP_DEVICE
// #define PWM_DIR
#if defined PLCP_DEVICE

// 设备类型(多选一)
#define PLCP_PANEL
// #define PLCP_LIGHT_CT

#if defined PLCP_PANEL

// #define DND_MODE_PANEL // 特殊用法,对于清理勿扰,插卡后要恢复原来的状态(临时使用)

// #define PANEL_1KEY
// #define PANEL_2KEY // 用于亚朵3.6的 清理勿扰
// #define PANEL_3KEY // 用于亚朵3.6的 入户门面板(竖向,3键)
#define PANEL_4KEY

#if defined PANEL_1KEY
#define KEY_NUMBER 6
#elif defined PANEL_2KEY
#define KEY_NUMBER 4
#elif defined PANEL_3KEY
#define KEY_NUMBER 4
#elif defined PANEL_4KEY
#define KEY_NUMBER 4
#elif defined PANEL_6KEY
#define KEY_NUMBER 6
#endif
#define RELAY_NUMBER 4
#endif

#if defined PLCP_LIGHT_CT
#define KEY_NUMBER 0
#endif

#endif

#endif