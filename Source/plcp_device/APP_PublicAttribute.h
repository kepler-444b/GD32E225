/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved锟斤拷
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC锟斤拷
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code锟斤拷
 *
 * FileName    : APP_PublicAttribute.h
 * Author      :
 * Date        : 2024-06-12
 * Version     : 1.0
 * Description : app层公共属性参数
 *             :
 * Others      :
 * ModifyRecord:
 *
 ********************************************************************************/
#include "../../Source/device/device_manager.h"
#if defined PLCP_DEVICE
#ifndef _APP_PUBLICATTRIBUTE_H_
#define _APP_PUBLICATTRIBUTE_H_
#include "MseProcess.h"
#include "../../Source/plcp_common/Inc/lmexxx_conf.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_scene.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_bind.h"

#define FLASH_WRITE_POSTWRIET_FLAG 0x08017C00U // 95 页(存放)

typedef enum {
    MSE_Load_Product = 0x23,
    MSE_GET_selfMAC,
    MSE_GET_DID,
    MSE_GET_CC0MAC,
    MSE_SET_DID,
    MSE_SET_Transfer,
    MSE_Load_AE,
    MSE_Load_Widgets,
    MSE_SET_Factory,
} etoken;

/**********************************结构体声明 开始***********************************/
typedef struct
{
    uint8_t selfMacAddr[6]; /* 模组mac地址 */
    uint8_t ccoMacAddr[6];  /* 网关mac地址 */
    uint16_t did;           /* 模组DID */
} sPublic_attribute;

typedef struct
{
    uint8_t wproductsecflag; /* 产品信息写入成功标志 */
    uint8_t wAEsecflag;      /* AE信息写入成功标志 */
    uint8_t wWidgetsecflag;  /* Widget信息写入成功标志 */
    uint8_t wtranferflag;    /* 透传使能标志 */
} sPost_wflash;

/**********************************结构体声明 结束***********************************/
/***********************************函数声明 开始************************************/
void APP_Attribute_Init(void);
bool APP_SavePostFlag(void);
/*--------------------------------1-----------------------------
函数名称：APP_Attribute_GetPointer
函数功能：获取APP层属性的指针
输入参数：无
返回值：    APP层属性结构体的指针
备 注：
---------------------------------------------------------------*/
const sPublic_attribute *APP_Attribute_GetPointer(void);

/*--------------------------------2-----------------------------
函数名称：APP_Postflag_GetPointer
函数功能：获取主动写信息标志属性的指针
输入参数：无
返回值：    主动写信息标志的指针
备 注：
---------------------------------------------------------------*/
const sPost_wflash *APP_Postflag_GetPointer(void);

/*--------------------------------3-----------------------------
函数名称：APP_Attribute_SetProductSuccessFlag
函数功能：设置公共属性写产品信息成功标志
输入参数：newState-新状态
返回值：   无
备 注：
---------------------------------------------------------------*/
void APP_Attribute_SetProductSuccessFlag(uint16_t newState);

/*--------------------------------4-----------------------------
函数名称：APP_Attribute_SetAESuccessFlag
函数功能：设置公共属性写AE信息成功标志
输入参数：newState-新状态
返回值：   无
备 注：
---------------------------------------------------------------*/
void APP_Attribute_SetAESuccessFlag(uint16_t newState);

/*--------------------------------4-----------------------------
函数名称：APP_Attribute_SetProductSuccessFlag
函数功能：设置公共属性使能模组透传功能成功标志
输入参数：newState-新状态
返回值：   无
备 注：
---------------------------------------------------------------*/
void APP_Attribute_SetTranferSuccessFlag(uint16_t newState);

/*--------------------------------47-----------------------------
函数名称：APP_Attribute_SetselfMacAddress
函数功能：设置主节点MAC地址
输入参数：newValue -新类型值
返回值：   无
备 注：
---------------------------------------------------------------*/
void APP_Attribute_SetselfMacAddress(uint8_t *newValue);

/*--------------------------------47-----------------------------
函数名称：APP_Attribute_SetCCOMacAddress
函数功能：设置主节点MAC地址
输入参数：newValue -新类型值
返回值：   无
备 注：
---------------------------------------------------------------*/
void APP_Attribute_SetCCOMacAddress(uint8_t *newValue);

/*--------------------------------27-----------------------------
函数名称：APP_Attribute_SetPanID
函数功能：设置APP层属性的PanID
输入参数：newValue-新数值
返回值：   无
备 注：
---------------------------------------------------------------*/
void APP_Attribute_SetPanID(uint16_t newValue);

/*--------------------------------4-----------------------------
函数名称：APP_Tx_ProcessOfACKReceived
函数功能：APP层收到回复确认帧的处理函数。
输入参数：uappsMsg--消息指针
返回值：0-ack不匹配,1-ack匹配
备 注：
---------------------------------------------------------------*/
void APP_Tx_ProcessOfACKReceived(UappsMessage *uappsMsg);
void SendNextFrame(void);
/*--------------------------------------------------------------
函数名称：MCU_Load_Product
函数功能：写入plc模组产品信息的接口函数
输入参数：
返回值：   无
备 注：
---------------------------------------------------------------*/
void MCU_Load_Product(void);

/*--------------------------------------------------------------
函数名称：MCU_Load_AEInfo
函数功能：写入plc模组AE信息的接口函数
输入参数：
返回值：   无
备 注：
---------------------------------------------------------------*/
void MCU_Load_AEInfo(void);
void MCU_Load_Widgets(void);
const uint8_t get_ae_packs(void);
/*--------------------------------------------------------------
函数名称：MCU_Enable_Transmission
函数功能：使能透传的接口函数
输入参数：
返回值：   无
备 注：
---------------------------------------------------------------*/
void MCU_Enable_Transmission(void);

/*****************************************************************************
函数名称 : MCU_Control_Open
功能描述 : MCU实现开机的功能
输入参数 : 无
返回参数 : 无
使用说明 : 在此实现设备开机的功能
*****************************************************************************/
uint8_t MCU_Control_Open(void);

/*****************************************************************************
函数名称 : MCU_Control_Close
功能描述 : MCU实现关机的功能
输入参数 : 无
返回参数 : 无
使用说明 : 在此实现设备关机的功能
*****************************************************************************/
uint8_t MCU_Control_Close(void);

/*****************************************************************************
函数名称 : MCU_Control_Set_Temperature
功能描述 : MCU实现设置温度的功能
输入参数 : value：温度数据-2byte, uint8_t len
返回参数 : 无
使用说明 : 在此实现设备设置温度的功能(上位机下发的温度数据已放大10倍，本地处理需还原)
*****************************************************************************/
uint8_t MCU_Control_Set_Temperature(uint8_t *value, uint8_t len);

/*****************************************************************************
函数名称 : MCU_Control_Set_Mode
功能描述 : MCU实现设置模式的功能
输入参数 : value-模式值（含义参考协议）
返回参数 : 无
使用说明 : 在此实现设置模式的功能
*****************************************************************************/
uint8_t MCU_Control_Set_Mode(uint8_t value);

/*****************************************************************************
函数名称 : MCU_Control_Set_Windspeed
功能描述 : MCU实现设置风速的功能
输入参数 : value-风速值（含义参考协议）
返回参数 : 无
使用说明 : 在此实现设备设置风速的功能
*****************************************************************************/
uint8_t MCU_Control_Set_Windspeed(uint8_t value);

/*****************************************************************************
函数名称 : MCU_Set_Config
功能描述 : MCU实现设置属性
输入参数 : buf-属性数据（数据结构参考协议），len-数据长度
返回参数 : 无
使用说明 : MCU实现设置属性
*****************************************************************************/
// uint8_t MCU_Set_Config(uint8_t *buf, uint8_t len);
// uint8_t MCU_Set_Config(uint8_t *buf, uint8_t type, uint8_t bits);
uint8_t MCU_Set_Config(uint8_t *buf, uint8_t buf_len);

/*--------------------------------------------------------------
函数名称：MCU_Scene_exe
函数功能：启动场景
输出参数：buf - 场景号清单
返回值：
备 注：根据实际的场景数据运行，数据格式可参考对应的协议
---------------------------------------------------------------*/
void MCU_Scene_exe(uint8_t *buf, uint8_t buf_len);
void MCU_SceneKj_exe(uint8_t *buf, uint8_t kj_index, uint8_t kj_type);

void MCU_dnd_recover(void);

void MCU_Group_State(uint8_t *buf, uint8_t buf_len);
void MCU_GroupKj_On(uint8_t status, uint8_t kj_index, uint8_t kj_type);
void MCU_GroupKj_Off(uint8_t status, uint8_t kj_index, uint8_t kj_type);

/*****************************************************************************
函数名称 : MCU_Device_Factory
功能描述 : MCU恢复出厂设置功能
输入参数 : 无
返回参数 : 无
使用说明 : 在此实现设备恢复出厂设置逻辑
*****************************************************************************/
void MCU_Device_Factory(void);

/*****************************************************************************
函数名称 : MCU_Device_ResetService
功能描述 : MCU复位功能
输入参数 : buf-复位数据（数据结构参考协议），len-数据长度
返回参数 : 无
使用说明 : 在此实现设备复位功能
*****************************************************************************/
void MCU_Device_ResetService(uint8_t *buf, uint8_t len);
/*****************************************************************************
函数名称 : MCU_Get_Devicestatus
功能描述 : 获取MCU设备状态
输入参数 : buf：需要上送的状态数据
返回参数 : 无
使用说明 : 上位机获取设备状态，该函数实现将状态数据按照协议要求进行组帧
*****************************************************************************/

typedef enum {

    BUTTON_TYPE      = 0x01, // 按键类
    GRAD_TYPE        = 0x02, // 渐变类
    DIAL_TYPE        = 0x03, // 拨码类
    REALY_TYPE       = 0x04, // 继电器类
    BLCK_TYPE        = 0x05, // 按键背光灯类
    COUTN_TYPE       = 0x06, // 倒计时类
    INPUT_CHECK      = 0x07, // 输入检测类
    ADJUST_BLCK_TYPE = 0x08, // 可调按键背光类

} STATE_e;

typedef enum {

    KJ_MODE        = 0x00, // 控件属性
    REPOD_TYPE     = 0x1F, // 上报方式属性
    ENABLE_LED     = 0x01, // 按键的按键灯使能
    RELAY_POWER_UP = 0x04, // 继电器的上电状态
    BATTERY_LEVEL  = 0x05, // 电池电量状态
    SCENE_LINK     = 0x06, // 场景联动属性
} CONFIG_e;

uint8_t MCU_Get_Devicestatus(uint8_t *buf, uint8_t type, uint16_t bits);
// void MCU_Put_Devicestatus(uint8_t *buf, uint8_t type, uint8_t bits);
void MCU_Put_Devicestatus(uint8_t *buf, uint8_t buf_len);

/*****************************************************************************
函数名称 : MCU_Get_Config
功能描述 : 获取MCU属性状态数据
输入参数 : buf：需要上送的属性状态数据
返回参数 : 无
使用说明 : 上位机获取属性状态，该函数实现将属性状态按照协议要求进行组帧
*****************************************************************************/
uint8_t MCU_Get_Config(uint8_t *buf, uint8_t type, uint16_t bits);

/*****************************************************************************
函数名称 : MCU_Get_Ver
功能描述 : 获取MCU固件版本信息数据
输入参数 :
返回参数 : buf：版本数据
使用说明 : 前两个字节-版本号，后四个字节时间日期
*****************************************************************************/
uint8_t MCU_Get_Ver(uint8_t *buf);

/*****************************************************************************
函数名称 : MCU_deviceOnorOutline
功能描述 : MCU串口接收处理数据函数
输入参数 : value：1-在线；0-不在线
返回参数 : 无
使用说明 : 用户可根据传入值判断设备当前是否在线。
          例如温控面板有入网标示灯，那么本函数的值可以作为判断依据。
*****************************************************************************/
void MCU_deviceOnorOutline(uint8_t value);

/*****************************************************************************
函数名称 : MCU_UartReceive
功能描述 : MCU串口接收处理数据函数
输入参数 : recbuf:数据源数据；reclen:数据长度
返回参数 : 无
使用说明 : 在此实现串口接收处理的功能，用调用此函数在串口接收处
*****************************************************************************/
uint8_t MCU_UartReceive(uint8_t *recbuf, uint16_t reclen);

/*****************************************************************************
函数名称 : MCU_Send_date
功能描述 : 将需要MCU串口发送数据添加到发送队列中
输入参数 : SendBuff:数据源数据; SendBuffLen:数据长度
返回参数 : 无
使用说明 : 在此实现将待发送添加到发送队列中
*****************************************************************************/
void MCU_Send_date(uint8_t *SendBuff, uint16_t SendBuffLen);

/*--------------------------------7-----------------------------
函数名称：APP_Queue_ListenAndHandleMessage
函数功能：APP层消息的监听（接收队列、发送队列）和处理函数。
输入参数：无
返回值：无
备 注：该函数需要在主循环中调用
---------------------------------------------------------------*/
void APP_Queue_ListenAndHandleMessage(void);

// 初始化入网标志位
void APP_ReadRecordFlag_Init(void);

/***********************************函数声明 结束************************************/

#endif
#endif