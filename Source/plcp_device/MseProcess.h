/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved锟斤拷
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC锟斤拷
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code锟斤拷
 *
 * FileName    : Common.h
 * Author      :
 * Date        : 2023-09-13
 * Version     : 1.0
 * Description :
 *             :
 * Others      :
 * ModifyRecord:
 *
 ********************************************************************************/
#ifndef __MSEPROCESS_H_
#define __MSEPROCESS_H_
#include "../../Source/plcp_common/Inc/Uapps.h"
/**********************************服务项定义 开始***********************************/
#define MSE_DEVICE_ONPEN            "/_on"  // 打开
#define MSE_DEVICE_CLOSE            "/_off" // 关闭

#define MSE_DEVICE_SET_CONFIG       "/_config" // 设置属性

#define MSE_DEVICE_GROUP_JOIN       "/g/_join" // 设备加入组服务项
#define MSE_DEVICE_GROUP_QUIT       "/g/_quit" // 设备退出组服务项
#define MSE_DEVICE_GROUP_LIST       "/g/_list" // 设备的已加入组号列表服务项

#define MSE_DEVICE_SCENE_JOIN       "/s/_join" // 设备加入场景服务项
#define MSE_DEVICE_SCENE_QUIT       "/s/_quit" // 设备退出场景服务项
#define MSE_DEVICE_SCENE_LIST       "/s/_list" // 设备的已加入场景列表服务项
#define MSE_DEVICE_SCENE_COPY       "/s/_copy" // 设备场景拷贝

#define MSE_DEVICE_PLCP_FACTORY     "/_factory" // 设备恢复出厂设置服务项
#define MSE_DEVICE_PLCP_RESET       "/_rst"     // 设备恢复出厂设置服务项
#define MSE_DEVICE_PLCP_STATE       "/_state"   // 查询状态服务项

#define MSE_DEVICE_PLCP_STATE_ONOFF "/_state?_onoff" // 查询开关状态服务项
#define MSE_DEVICE_PLCP_MCUVER      "/_mcuVer"       // 查询MCU版本服务项

#define MSE_DEVICE_PLCP_BIND        "/_bind"

/**********************************服务项定义 结束***********************************/
/**********************************枚举定义 开始***********************************/
typedef enum {
    DEVICE_STATE_OPEN_E = 0,
    DEVICE_STATE_CLOSE_E,
    DEVICE_STATE_SET_CONFIG_E,

    DEVICE_GROUP_JOIN_e,
    DEVICE_GROUP_QUIT_e,
    DEVICE_GROUP_LIST_e,

    DEVICE_SCENE_JOIN_e,
    DEVICE_SCENE_QUIT_e,
    DEVICE_SCENE_LIST_e,
    DEVICE_SCENE_COPY_e,

    DEVICE_FACTORY_e,
    DEVICE_RESET_e,
    DEVICE_STATE_e,
    DEVICE_STATE_ONOFF_e,
    DEVICE_MCUVER_e,
    DEVICE_BIND_e,

} MSE_RESOURCE_SERVICE_e;

/**********************************枚举定义 开始***********************************/
/**********************************结构体声明 开始***********************************/
struct Function_Config {
    uint8_t (*Control_Open)(void);
    uint8_t (*Control_Close)(void);
    uint8_t (*Set_Config)(uint8_t *buf, uint8_t buf_len);
    void (*Device_Facory)(void);
    void (*Device_Reset)(uint8_t *buf, uint8_t len);

    uint8_t (*Device_Getstate)(uint8_t *buf, uint8_t type, uint16_t bits);
    void (*Device_Putstate)(uint8_t *buf, uint8_t buf_len);
    uint8_t (*Get_Config)(uint8_t *buf, uint8_t type, uint16_t bits);
    uint8_t (*Device_GetMCUver)(uint8_t *buf);
};

void APP_SendACK(UappsMessage *reqMsg, uint16_t payloadFlag, uapps_rw_buffer_t *scratch, uint8_t type, uint8_t repondCode);

void APP_UappsMsgProcessing(uint8_t *inputDataBuff, uint16_t inputDataBuffLen);

uint16_t Aps_UartMessage(UappsMessage *pMsg, uint8_t *sendBuff, uint16_t sendBuffMaxSize);

void CmdTest_MSE_POST_Status(uint8_t *GatewaymacAddr, uint8_t *LocalmacAddr, uint8_t *buf, uint8_t buflen);

void CmdTest_MSE_Load_Product(char *CompanyName, char *model, char *PID);

void CmdTest_MSE_Load_AEInfo(const char *ae_num, uint8_t quantity, char *BrandName, char *id, char *url);
uint8_t APP_SendRSL(char *rslStr, char *from, uint8_t *playload, uint8_t playloadLen);
void mseprecess_init(void);
/***********************************函数声明 结束************************************/

#endif
