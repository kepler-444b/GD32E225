#ifndef _DEVICE_MANAGER_H_
#define _DEVICE_MANAGER_H_

// 设备类型(多选一)
#define PLCP_PANEL // 灯控面板
// #define PLCP_LIGHT_CT // 灯驱

#if defined PLCP_PANEL
#define DEV_TYPE 0x00 // 产品类型(面板)

#define PANEL_TD   // 横向面板
#define PANEL_4KEY // 4键面板
// #define PANEL_6KEY // 6键面板

#if defined PANEL_4KEY
#define KEY_NUMBER 4
#elif defined PANEL_6KEY
#define KEY_NUMBER 6
#endif
#define RELAY_NUMBER 4

#elif defined PLCP_LIGHT_CT

#define ONE_CHJANNEL

#define KEY_NUMBER 0
#else

#endif
#endif
