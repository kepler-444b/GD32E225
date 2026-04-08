#ifndef _DEVICE_MANAGER_H_
#define _DEVICE_MANAGER_H_

#define PLCP_DEVICE
// #define PWM_DIR
#if defined PLCP_DEVICE

// 设备类型(多选一)
#define PLCP_PANEL
// #define PLCP_LIGHT_CT

#if defined PLCP_PANEL

#define PANEL_TD // 横向面板
#define PANEL_4KEY
// #define PANEL_6KEY

#if defined PANEL_4KEY
#define KEY_NUMBER 4
#elif defined PANEL_6KEY
#define KEY_NUMBER 6
#endif

#define RELAY_NUMBER 4
#endif

#if defined PLCP_LIGHT_CT
#define KEY_NUMBER 0
#endif

#endif // PLCP_DEVICE
#endif
