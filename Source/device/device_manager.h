#ifndef _DEVICE_MANAGER_H_
#define _DEVICE_MANAGER_H_

// 设备类型(多选一)
#define PLCP_PANEL // 灯控面板
// #define PLCP_LIGHT_CT // 灯驱

#if defined PLCP_PANEL

#define DEV_TYPE   0x00 // 产品类型(面板)

#define PWM_HIGH   // pwm高电平有效

#define PANEL_TD   // 横向面板
#define PANEL_4KEY // 4键面板
// #define PANEL_6KEY // 6键面板

#if defined PANEL_4KEY
#define KEY_NUMBER 4
#elif defined PANEL_6KEY
#define KEY_NUMBER 6
#endif
#define RELAY_NUMBER 4

// ************************************* 默认配置 *************************************
// #define DEFAUTL_CURTAIN_PANEL // 窗帘开 窗帘关 窗纱开 窗纱关
// #define DEFAUTL_DND_CLEAR_PANEL // 勿扰 清理 卫浴灯 排气扇
// #define DEFAUTL_MIRROR_LINGHT_PANEL // 镜前灯
// #define DEFAUTL_ALL_OPEN_BATH_PANEL // 全开|关 卫浴 柔光 夜灯
// #define DEFAUTL_BATH_ALL_OPEN_PANEL // 卫浴 全开|关 夜灯 柔6光

// 亚朵(见野)
// #define DEFAUTL_MIRROR_LINGHT_PANEL_JY // 镜前灯
// #define DEFAUTL_DND_CLEAR_PANEL_JY // 勿扰 清理 卫浴灯 排气扇

// 亚朵(新野)
// #define DEFAUTL_AROM_LINGHT_PANEL // 香薰灯
// #define DEFAUTL_AROM_FIRE_PANEL // 香薰 壁炉
// #define DEFAUTL_CLEAR_DND_TOILET_PANEL // 清理 勿扰 马桶间 排气扇
// #define DEFAUTL_BATH_EXHA_PANEL // 卫浴 排气扇
// #define DEFAUTL_CLEAR_DND_PANEL // 清理 清理 勿扰 勿扰
#define DEFAUTL_ALL_OPEN_ALL_CLOSE // 全开 全关 马桶间 排气扇

// ***********************************************************************************

#elif defined PLCP_LIGHT_CT
// #define PWM_HIGH   // pwm高电平有效6

// #define DEFAUTL_MIRROR_LIGHT_CT // 台盆镜灯
// #define DEFAUTL_BED_LIGHT_CT_L // 床头花灯(左)
// #define DEFAUTL_BED_LIGHT_CT_R // 床头花灯(右)

#define DEV_TYPE   0x17 // 产品类型(双色温灯驱)

#define KEY_NUMBER 1
#else

#endif
#endif
