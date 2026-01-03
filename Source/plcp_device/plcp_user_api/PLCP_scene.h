/******************************************************************************
 * (c) CRCright 2017-2018, Leaguer Microelectronics, All Rights Reserved
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF Leaguer Microelectronics, INC.
 * The CRCright notice above does not evidence any actual or intended
 * publication of such source code.
 *
 * file: Scene_demo.c
 * author : qinl@leaguerme.com
 * date   : 2024/03/21
 * version: v1.0
 * description: 本文件为外挂mcu构建群组和场景的基本参考框架
 *
 * modify record:
 *****************************************************************************/
#include "../../Source/device/device_manager.h"
#if defined PLCP_DEVICE
#ifndef _SCENE_DEMO_H_
#define _SCENE_DEMO_H_

#include "../../Source/plcp_common/Inc/lmexxx_conf.h"
#include "../../Source/flash/flash.h"
#include "../../Source/plcp_device/MseProcess.h"

/************************************宏定义 开始*************************************/
#define MAX_GROUP_NUMBERS 32
#define MAX_SCENE_NUMBERS 32
#define MAX_SCENE_TABLE   10

/* 场景数据存储地址*/
#define FLASH_SCENE_INFO_START_ADD1 0x08019000U //  100页
#define FLASH_SCENE_INFO_START_ADD2 0x08019400U //  101页

#define FLASH_GROUP_INFO_START_ADD  0x08019800U //  102页

// #define FLASH_SCENE_INFO_START_ADD4 0x08019C00U //  103页
// #define FLASH_SCENE_INFO_START_ADD4    0x08019C00U //  104页

#define FLASH_CH_SCENE_INFO_START_ADD     0x0801A400U // 105页
#define FLASH_LED_SCENE_INFO_START_ADD    0x0801A800U // 106页
#define FLASH_AD_LED_SCENE_INFO_START_ADD 0x0801AC00U // 107页

#define SCENE1_NUM                        16
#define SCENE2_NUM                        16

#define CH_SCENE_NUM                      4
#define LED_SCENE_NUM                     4
#define AD_LED_SCENE_NUM                  4
#define CH_GROUP_NUM                      4
#define LED_GROUP_NUM                     4

typedef enum {

    CH_SCENE,
    LED_SCENE,
    SCENE,
} scece_type;

typedef struct
{
    uint16_t sceneId;           // 场景号
    uint8_t scenePower[16];     // 进入场景数据，可根据设备实际的场景数据做扩展
    uint8_t quitscenePower[16]; // 退出场景数据
} sDevice_Scene;

typedef struct
{
    uint16_t scene_id;
    uint8_t open_scene[2];
    uint8_t close_scene[2];
} single_ch_scene; // 单个控件的场景列表

typedef struct
{
    single_ch_scene scenes[MAX_SCENE_NUMBERS];
} ch_scene; // 每个继电器的场景列表

typedef struct
{
    uint16_t scene_id;
    uint8_t open_scene[2];
    uint8_t close_scene[2];
} single_led_scene;

typedef struct
{
    uint16_t scene_id;
    uint8_t open_scene[2];
    uint8_t close_scene[2];
} single_ad_led_scene;

typedef struct
{
    single_led_scene scenes[MAX_SCENE_NUMBERS];
} led_scene;

typedef struct
{
    single_ad_led_scene scenes[MAX_SCENE_NUMBERS];
} ad_led_scene;

typedef struct
{
    uint16_t group;
} single_ch_group; // 单个继电器的群组

typedef struct {
    uint16_t group;
} single_led_group; // 单个指示灯的群组

typedef struct {
    single_ch_group groups[MAX_GROUP_NUMBERS];
} ch_group; // 每个继电器的群组列表

typedef struct {
    single_led_group groups[MAX_GROUP_NUMBERS];
} led_group; // 每个指示灯的群组列表

typedef struct
{
    scece_type scene_type; //  ch,led 或默认(scene) 类型
    uint8_t kj_index;      // 控件下标
    uint8_t index;         // 列表下标
    uint16_t scene_id;
    uint8_t *open_scene;
    uint8_t open_scene_len;
    uint8_t *close_scene;
    uint8_t close_scene_len;

} single_scene_data;

typedef struct
{
    uint8_t datelen;        // 数据长度
    uint8_t ver;            // 版本
    uint16_t type;          // 控制字
    uint8_t scenePower[11]; // 场景数据
} sData_Frame;

/**********************************结构体声明 结束***********************************/
/**********************************枚举声明 开始***********************************/
typedef enum {
    Device_OK = 0,
    Device_Err_Len,
    Device_Err_NoFound,
    Device_Err_Full,
} eDeviceErr;

typedef enum {
    state_len   = 0,
    state_value = 1
} eDeviceSceneParam;

typedef struct
{
    uint8_t scene_buf[2];
    uint8_t index;
    scece_type type;

} plcp_ex_t;

void APP_SceneGroupClr(void); // 清除"场景"和"群组"信息

uint8_t APP_isGroupExist(uint16_t groupNum, const char *aei);

uint8_t APP_IsHaveSceneId_Device(uint16_t sceneNumber);

uint8_t APP_Scene_Join(uint8_t *buf, uint16_t len, const char *aei);
uint8_t APP_Scene_Quit(uint8_t *buf, uint16_t len, const char *aei);
uint16_t APP_Scene_List(uint8_t *buf, const char *aei);
uint16_t APP_Scene_Copy_Get(uint8_t *buf, const char *aei);

void APP_Device_SceneStart(UappsMessage *uappsMsg, const char *aei);
void APP_Device_SceneClose(UappsMessage *uappsMsg, const char *aei);

uint8_t APP_Group_Join(uint8_t *buf, uint16_t len, const char *aei);
uint8_t APP_Group_Quit(uint8_t *buf, uint16_t len, const char *aei);
void APP_Device_GroupClose(UappsMessage *uappsMsg, const char *aei);
void APP_Device_GroupState(UappsMessage *uappsMsg, const char *aei);

uint16_t APP_Group_List(uint8_t *buf, const char *aei);

bool APP_ReadAllSceneInfo(void);
bool APP_ReadAllGroupInfo(void);

/***********************************函数声明 结束************************************/
#endif
#endif