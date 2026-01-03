/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved锟斤拷
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC锟斤拷
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code
 *
 * FileName    : APP_UserProcess.h
 * Author      :
 * Date        : 2024-09-06
 * Version     : 1.0
 * Description : 用户实现功能填写内容
 *             :
 * Others      :
 * ModifyRecord:
 *
 ********************************************************************************/
#include "../../Source/device/device_manager.h"
#if defined PLCP_DEVICE
#include "../../Source./plcp_device/plcp_panel/attr_table.h"
#include "../../Source/base/base.h"
#include "../../Source/base/debug.h"
#include "../../Source/plcp_device/MseProcess.h"
#include "../../Source/plcp_device/plcp_linght_ct/light_ct_attr_table.h"
#include "../../Source/plcp_device/plcp_linght_ct/plcp_light_ct_info.h"
#include "../../Source/plcp_device/plcp_panel/panel_adapter.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_callback.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_special_scene.h"
#include "../../Source/usart/usart.h"
#include "APP_PublicAttribute.h"
#include "gd32e23x.h"
#include "systick.h"

/**********************************函数列表  开始************************************
 * 1、   void MCU_Load_Product(void)                   MCU给plc模组写入产品信息
 * 2、   void MCU_Load_AEInfo(void)                    MCU给plc模组写入AE信息
 * 3、   void MCU_Enable_Transmission(void)            MCU使能plc模组透传功能
 * 4、   void APP_PLCSDK_Init(void)                    sdk内部需要初始化的内容
 * 5、   void MCU_Control_Open(void)                   MCU实现开机功能——"/_on"服务项
 * 6、   void MCU_Control_Close(void)                  MCU实现关机功能——"/_off"服务项
 * 7、   void MCU_Control_Set_Temperature(uint8_t *buf, uint8_t len)       MCU实现设置温度的功能——"/_state"服务项
 * 8、   void MCU_Control_Set_Mode(uint8_t *buf, uint8_t len)      MCU实现设置模式的功能——"/_config"服务项
 * 9、      void MCU_Control_Set_Windspeed(uint8_t value)       MCU实现设置风速的功能
 * 10、void MCU_Set_Config(uint8_t *buf, uint8_t len)             MCU实现设置属性
 * 11、   uint16_t MCU_Scene_exe(uint8_t *buf)                     MCU启动场景功能
 * 12、  void MCU_Device_Factory(void)                 MCU实现恢复出厂设置功能——"/_factory"服务项
 * 13、  void MCU_Device_ResetService(uint8_t *buf, uint8_t len)      MCU实现复位功能——"/_rst"服务项
 * 14、  void MCU_deviceOnorOutline(uint8_t value)               MCU获取是否入网状态
 * 15、  void MCU_Proactively_report(void)                  MCU实现主动上报功能
 * 16、  uint8_t MCU_UartReceive(uint8_t *recbuf, uint16_t reclen)         MCU串口接收缓存，添加到接收队列
 * 17、  void MCU_Send_date(uint8_t *SendBuff, uint16_t SendBuffLen)  MCU串口发送缓存，添加到发送队列
 * 18、  void APP_Queue_ListenAndHandleMessage(void)        MCU APP层消息的监听（接收队列、发送队列）和处理函数
 ***********************************函数列表  结束***********************************/

/************************************移植须知:******************************************
                                第一步:初始化
1：上电之后，mcu需要分别给plc模组写入产品信息、ae信息、使能透传功能。（最好是mcu上电3-5s后才开始写入）
    i:   调用MCU_Load_Product();写入产品信息
    ii:  MCU_Load_AEInfo();写入ae信息
    iii: MCU_Enable_Transmission();使能透传功能
    为了防止反复写plc模组flas，在mcu首次上电写入成功后需要将成功标志保存，后续上下电判断该标志判定是否需要重新写入
2: mcu上电需要周期性轮询plc模组的did——did不为0说明已入网，反之；若
3：场景，群组信息等需要掉电保持的信息，上电需要初始化读取
                                第二步:实现功能
1: 对照协议内容与实际需求，将需要实现的服务项功能对照函数列表分别填写
2: 产品信息与AE信息需要用户重新填写的部分，用户可根据实际自行修改宏定义
4: 设备接收和发送需要调用MCU_UartReceive()，MCU_Send_date()添加到消息队列中
5: 单片机进入while循环后调用APP_Queue_ListenAndHandleMessage()函数对消息进行监听，有新消息则处理
                                第三步：存储数据
根据单片机实际的读、写flash函数改写sdk中的例子
1：APP_PublicAttribute.c文件中的APP_SavePostFlag()、APP_ReadPostFlag()需要实现读，写flash操作
2：Scene_demo.c文件中的APP_SaveSceneParameter()、APP_SaveGroupParameter()、APP_ReadParameter()需要实现读写flash操作
3：APP_PublicAttribute.h文件内的FLASH_WRITE_POSTWRIET_FLAG以及Scene_demo.h文件内的FLASH_GROUP_INFO_START_ADD、FLASH_SCENE_INFO_START_ADD存储的地址需要重新规划
******************************************************************************/
#define CompanyName "AODSN"
#define product_model "panel_td_4key"
#define PID "991018"

#define BrandName "6键"
#define id "AODSN_panel_switch"
#define url "https://wowoja.cn/oss/panel/lme/AODSN_panel_switch/v1.0/index.html"

// #define AE_NUM_SIZE   1
// #define AE_NUM_VALUES {"1,2,3,4,5,6"}

#define AE_NUM_SIZE 1
#define AE_NUM_VALUES {"11"}
const char *ae_num[AE_NUM_SIZE] = AE_NUM_VALUES;

void PLCP_Load_Product(char *product);
void MCU_Load_Product(void)
{
    PLCP_Load_Product(PLCP_Callback_Product());
}

void PLCP_Load_AEInfo(char *root);
void MCU_Load_AEInfo(void)
{
    // CmdTest_MSE_Load_AEInfo(ae_num, AE_NUM_SIZE, BrandName, id, url);
    PLCP_Load_AEInfo(PLCP_Callback_Ae());
    // PLCP_Load_AEInfo(ae_srt);
}

// char widgets_srt[] =
//     "{\"widgets\":["
//     "{"
//     "\"name\":\"Switch\","
//     "\"ae\":[\"k1\",\"k2\",\"k3\",\"k4\",\"k5\",\"k6\"],"
//     "\"se\":\"1\","
//     "\"type\":\"switch\","
//     "\"events\":["
//     "{"
//     "\"id\":\"_open\","
//     "\"name\":\"OFF\","
//     "\"data\":\"0\""
//     "},"
//     "{"
//     "\"id\":\"_close\","
//     "\"name\":\"ON\","
//     "\"data\":\"1\""
//     "}"
//     "]"
//     "}"
//     "]}";
void PLCP_Load_Widgets(char *root);
void MCU_Load_Widgets(void)
{
    PLCP_Load_Widgets(PLCP_Callback_Widgets());
}

/*************************************************s****************************
函数名称 : MCU_Enable_Transmission
功能描述 : MCU使能plc模组透传功能
输入参数 : 无
返回参数 : 无
使用说明 : 使能后plc模组才能把恢复出厂命令以及复位命令透传到mcu模组；只要写成功一次即可（需要避免重复写入）
*****************************************************************************/
void CmdTest_MSE_SET_Transfer(void); // 向前声明
void MCU_Enable_Transmission(void)
{
    CmdTest_MSE_SET_Transfer();
}

/*****************************************************************************
函数名称 : APP_PLCSDK_Init
功能描述 : PLC sdk需要初始化的内容
输入参数 : 无
返回参数 : 无
使用说明 : sdk实现进行初始化的功能
*****************************************************************************/
void APP_PLCSDK_Init(void)
{
    APP_TxBuffer_init();
    APP_RxBuffer_init();
    // APP_GroupClr();
    // APP_SceneClr();
    // APP_SpecialSceneinit();

    APP_ReadBindParameter(); // 读取绑定信息
    // APP_Bindinit();          // Bind 订阅事件总线
    // APP_SceneInit();

    APP_ReadDelaySceneParameter(); // 读取延时场景信息
    APP_ReadNightSceneParameter(); // 读取夜灯场景信息

    APP_ReadRecordFlag_Init(); // 初始化入网标志位
    APP_ReadAllSceneInfo();    // 读取所有场景信息
    APP_ReadAllGroupInfo();    // 读取所有群组信息

#if defined PLCP_LIGHT_CT
    APP_Read_light_ct_info();
#endif
}

/*****************************************************************************
函数名称 : MCU_Control_Open
功能描述 : MCU实现开机的功能
输入参数 : 无
返回参数 : 无
使用说明 : 在此实现设备开机的功能
*****************************************************************************/
uint8_t MCU_Control_Open(void)
{
    printf("**MCU_Control_Open**\n");
    return 0;
}

/*****************************************************************************
函数名称 : MCU_Control_Close
功能描述 : MCU实现关机的功能
输入参数 : 无
返回参数 : 无
使用说明 : 在此实现设备关机的功能
*****************************************************************************/
uint8_t MCU_Control_Close(void)
{
    printf("**MCU_Control_Close**\n");
    return 0;
}

/*****************************************************************************
函数名称 : MCU_Control_Set_Temperature
功能描述 : MCU实现设置温度的功能
输入参数 : value：温度数据-2byte, uint8_t len
返回参数 : 无
使用说明 : 在此实现设备设置温度的功能(上位机下发的温度数据已放大10倍，本地处理需还原)
*****************************************************************************/
uint8_t MCU_Control_Set_Temperature(uint8_t *value, uint8_t len)
{
    printf("**MCU_Control_Set_Temperature**\n");
    return 0;
}

/*****************************************************************************
函数名称 : MCU_Control_Set_Mode
功能描述 : MCU实现设置模式的功能
输入参数 : value-模式值（含义参考协议）
返回参数 : 无
使用说明 : 在此实现设置模式的功能
*****************************************************************************/
uint8_t MCU_Control_Set_Mode(uint8_t value)
{
    printf("**MCU_Control_Set_Mode**\n");
    return 0;
}

/*****************************************************************************
函数名称 : MCU_Control_Set_Windspeed
功能描述 : MCU实现设置风速的功能
输入参数 : value-风速值（含义参考协议）
返回参数 : 无
使用说明 : 在此实现设备设置风速的功能
*****************************************************************************/
uint8_t MCU_Control_Set_Windspeed(uint8_t value)
{
    printf("**MCU_Control_Set_Windspeed**\n");
    return 0;
}

//  设置属性
uint8_t MCU_Set_Config(uint8_t *buf, uint8_t buf_len)
{
    APP_PRINTF_BUF("MCU_Set_Config", buf, buf_len);
    uint8_t cmd_type = buf[0];
    uint16_t ctrl_bits = (buf[1] << 8) | buf[2];
    uint8_t idx = 0;

    switch (cmd_type) {
    // 需要逐位处理的类型
    case KJ_MODE: {
        for (uint8_t i = 0; i < KEY_NUMBER; i++) {
            if (ctrl_bits & (1 << (15 - i))) {
                uint8_t status = buf[3 + idx];
                attr_button_mode_table_set(i, status);
                idx++;
            }
        }
    } break;
    case ENABLE_LED: {
        for (uint8_t i = 0; i < KEY_NUMBER; i++) {
            if (ctrl_bits & (1 << (15 - i))) {
                uint8_t status = buf[3 + idx];
                attr_led_relay_table_set(i, status);
                idx++;
            }
        }
    } break;
    case SCENE_LINK: {
        special_scene_set(buf, buf_len);
    } break;
    case RELAY_POWER_UP: {
        for (uint8_t i = 0; i < KEY_NUMBER; i++) {
            if (ctrl_bits & (1 << (15 - i))) {
                uint8_t status = buf[3 + idx];
                attr_relay_power_up_table_set(i, status);
                idx++;
            }
        }
    } break;

    default:
        return 0;
    }

    return 1;
}

/*--------------------------------------------------------------
函数名称：MCU_Scene_On
函数功能：启动场景
输出参数：buf - 场景号清单
返回值：
备 注：根据实际的场景数据运行，数据格式可参考对应的协议
---------------------------------------------------------------*/
void MCU_Scene_exe(uint8_t *buf, uint8_t buf_len)
{
    // APP_PRINTF("MCU_Scene_exe\n");
#if defined PLCP_PANEL
    parse_control_commands(buf, buf_len);
#elif defined PLCP_LIGHT_CT
    light_ct_ctrl(&buf[1], buf_len);
#endif
}

void MCU_SceneKj_exe(uint8_t *buf, uint8_t kj_index, uint8_t kj_type)
{
    if (buf[0] != 0x01) {
        return;
    }
    switch (kj_type) {
    case LED_SCENE:
        attr_led_table_set(kj_index, buf[1]);
        attr_key_state_table_set(kj_index, buf[1]); // 按键状态与LED同步
        break;
    case CH_SCENE:
        attr_relay_table_set(kj_index, buf[1]);
        break;
    default:
        return;
    }
}

// 对于清理勿扰,插卡后恢复

void MCU_dnd_recover(void)
{
    uint8_t k1_status;
    uint8_t k2_status;
#if defined PANEL_3KEY

    k1_status = attr_led_table_get(2);
    k2_status = attr_led_table_get(3);

    attr_led_table_set(2, k1_status);
    attr_led_table_set(3, k2_status);

    // 因为使用 MCU_dnd_recover 跳过了正常的场景执行,对于亚朵3.6的三键竖向面板,卫浴灯在这里做特殊处理
    attr_led_table_set(0, true);
    attr_key_state_table_set(0, true);
    switch_adapter_ad_led_b_ctrl(0, 0, 1);

    switch_adapter_ad_led_b_ctrl(2, k1_status ? 0 : 100, 1);
    switch_adapter_ad_led_b_ctrl(3, k2_status ? 0 : 100, 1);

#elif defined PANEL_4KEY
    k1_status = attr_led_table_get(0);
    k2_status = attr_led_table_get(1);

    attr_led_table_set(0, k1_status);
    attr_led_table_set(3, k1_status);

    switch_adapter_led_b_ctrl(0, !k1_status);
    switch_adapter_led_b_ctrl(3, !k1_status);

    attr_led_table_set(1, k2_status);
    attr_led_table_set(2, k2_status);

    switch_adapter_led_b_ctrl(1, !k2_status);
    switch_adapter_led_b_ctrl(2, !k2_status);
#endif
}

void MCU_Group_State(uint8_t *buf, uint8_t buf_len)
{
    light_ct_ctrl(buf, buf_len);
}

void MCU_GroupKj_On(uint8_t status, uint8_t kj_index, uint8_t kj_type)
{
    switch (kj_type) {
    case LED_SCENE:
        break;
    case CH_SCENE:
        attr_relay_table_set(kj_index, status);
        break;
    case SCENE:
        attr_light_ct_table_set(true);
        break;
    default:
        return;
    }
}

void MCU_GroupKj_Off(uint8_t status, uint8_t kj_index, uint8_t kj_type)
{
    switch (kj_type) {
    case LED_SCENE:
        break;
    case CH_SCENE:
        attr_relay_table_set(kj_index, status);
        break;
    case SCENE:
        attr_light_ct_table_set(status);
        break;
    default:
        return;
    }
}
/*****************************************************************************
函数名称 : MCU_Device_Factory
功能描述 : MCU恢复出厂设置功能
输入参数 : 无
返回参数 : 无
使用说明 : 在此实现设备恢复出厂设置(包括清除群组、场景、写信息成功标志位以及设备本地相关需要清除的数据)
*****************************************************************************/
void MCU_Device_Factory(void)
{
    printf("MCU_Device_Factory\n");
    APP_Attribute_Init();
    APP_SceneGroupClr(); // 清空场景与群组
    PLCP_bindTableClr();
    attr_kj_mode_table_reset(); // 按键类型恢复默认

    delay_1ms(100);
    // NVIC_SystemReset();
}

/*****************************************************************************
函数名称 : MCU_Device_ResetService
功能描述 : MCU复位功能
输入参数 : buf-复位数据（数据结构参考协议），len-数据长度
返回参数 : 无
使用说明 : 在此实现设备复位功能（
*****************************************************************************/
void MCU_Device_ResetService(uint8_t *buf, uint8_t len)
{
    // if (len == 0) {
    //     return;
    // }

    // uint8_t Type = buf[1];
    // for (uint8_t i = 0; i < 8; i++) {
    //     if ((Type >> i) & 0x1) {
    //         switch (i) {
    //             case 2: // 群组初始化
    //                 APP_GroupClr();
    //                 break;
    //             case 3: // 场景初始化
    //                 APP_SceneClr();
    //                 break;
    //         }
    //     }
    // }
    // printf("MCU_Device_ResetService\n");
}

/*****************************************************************************
函数名称 : MCU_deviceOnorOutline
功能描述 : MCU串口接收处理数据函数
输入参数 : value：1-在线；0-不在线
返回参数 : 无
使用说明 : 用户可根据传入值判断设备当前是否在线。
          例如温控面板有入网标示灯，那么本函数的值可以作为判断依据。
*****************************************************************************/
void MCU_deviceOnorOutline(uint8_t value)
{
    // printf("OnorOutline:%d\n", value);
}

/*****************************************************************************
函数名称 : MCU_Proactively_report
功能描述 : MCU主动上报状态接口
输入参数 : 无
返回参数 : 无
使用说明 : 当MCU本地状态发生改变时需要主动上报状态
*****************************************************************************/
void MCU_Proactively_report(void)
{
    /*********************************示例 开始**************************************** */
    uint8_t data[64];
    uint8_t len = 0;
    // uint16_t temp = get_tempSmp();
    // uint16_t setTemp = get_tempSet_ac();
    const sPublic_attribute *pAttributeOfAPP = NULL;
    pAttributeOfAPP = APP_Attribute_GetPointer();
    uint8_t ccoMacAddr[6];
    uint8_t selfMacAddr[6];
    memcpy(selfMacAddr, pAttributeOfAPP->selfMacAddr, sizeof(selfMacAddr));
    memcpy(ccoMacAddr, pAttributeOfAPP->ccoMacAddr, sizeof(ccoMacAddr));

    data[len++] = 1;
    data[len++] = 0xF0;
    data[len++] = 0x00;
    // data[len++] = get_power_ac();
    // data[len++] = setTemp >> 8;
    // data[len++] = setTemp & 0xFF;
    // data[len++] = temp >> 8;
    // data[len++] = temp & 0xFF;
    // data[len++] = get_ac_mode();
    // data[len++] = get_speedSet_ac();
    /*************************************示例 结束************************************* */

    CmdTest_MSE_POST_Status(ccoMacAddr, selfMacAddr, data, len);
}

/*****************************************************************************
函数名称 : MCU_Get_Devicestatus
功能描述 : 获取MCU设备状态
输入参数 : buf：需要上送的状态数据
返回参数 : 无
使用说明 : 上位机获取设备状态，该函数实现将状态数据按照协议要求进行组帧
*****************************************************************************/

uint8_t MCU_Get_Devicestatus(uint8_t *buf, uint8_t type, uint16_t bits)
{
#if defined PLCP_PANEL
    uint8_t count = 0;
    switch (type) {
    case REALY_TYPE: {
        for (uint8_t i = 0; i < 16; i++) {
            if (BIT16_HIGH_TO_LOW(bits, i)) {
                buf[count++] = attr_relay_table_get(i);
            }
        }
        return count;
    } break;
    default:
        break;
    }
#elif defined PLCP_LIGHT_CT
    buf[0] = 0x30;
    buf[1] = 0x00;
    uint16_t brightness;
    uint16_t color_temp;

    const light_ct_t *obj = get_light_info();
    if (obj->brightness_type) {
        brightness = obj->brightness * 100;
    } else {
        brightness = obj->brightness;
    }
    color_temp = obj->color_temp;
    memcpy(&buf[2], &brightness, sizeof(brightness));
    memcpy(&buf[4], &color_temp, sizeof(color_temp));
    return 6;
#endif
    return 0;
}

void MCU_Put_Devicestatus(uint8_t *buf, uint8_t buf_len)
{
    APP_PRINTF_BUF("MCU_Put_Devicestatus", buf, buf_len);
#if defined PLCP_PANEL
    switch_status_by_bits(buf, buf_len);
#elif defined PLCP_LIGHT_CT
    light_ct_ctrl(buf, buf_len);

#endif
}

uint8_t MCU_Get_Config(uint8_t *buf, uint8_t type, uint16_t bits)
{
    APP_PRINTF("MCU_Get_Config\n");
    uint8_t count = 0;
    switch (type) {
    case ENABLE_LED: {
        for (uint8_t i = 0; i < KEY_NUMBER; i++) {
            if (BIT_HIGH_TO_LOW(bits, i)) {
                buf[count++] = attr_led_relay_table_get(i);
            }
        }
        return count;
    } break;
    case RELAY_POWER_UP: {
        for (uint8_t i = 0; i < KEY_NUMBER; i++) {
            if (BIT_HIGH_TO_LOW(bits, i)) {
                buf[count++] = attr_relay_power_up_table_get(i);
            }
        }
        return count;
    } break;
    case KJ_MODE: {
        for (uint8_t i = 0; i < KEY_NUMBER; i++) {
            if (BIT_HIGH_TO_LOW(bits, i)) {
                buf[count++] = attr_kj_mode_table_get(i);
            }
        }
        return count;
    } break;
    default:
        break;
    }
    return 0;
}

/*****************************************************************************
函数名称 : MCU_Get_Ver
功能描述 : 获取MCU固件版本信息数据
输入参数 :
返回参数 : buf：版本数据
使用说明 : 前两个字节-版本号，后四个字节时间日期
*****************************************************************************/
uint8_t MCU_Get_Ver(uint8_t *buf)
{
    APP_PRINTF("MCU_Get_Ver\n");
    /****************************示例**********************************/
    uint8_t softver;
    uint8_t ver[6] = {0x01, 0x00, 0x20, 0x24, 0x09, 0x26};
    // softver = get_version();
    softver = 1;
    ver[0] = softver >> 4;
    ver[1] = softver & 0x0F;
    memcpy(buf, ver, sizeof(ver));
    return sizeof(ver);
    /*****************************************************************/
}

uint8_t MCU_UartReceive(uint8_t *recbuf, uint16_t reclen)
{
    // APP_PRINTF("MCU_UartReceive\n");
#if 1
    RxDataParametersStruct rxDataParam;
    rxDataParam.rxData = recbuf;
    rxDataParam.rxDataLen = reclen;
    return APP_RxBuffer_Add(&rxDataParam);
#else
    APP_UappsMsgProcessing(recbuf, reclen);
#endif
}

void MCU_Send_date(uint8_t *SendBuff, uint16_t SendBuffLen)
{
#if 1
    APP_DataTxTask txTask;
    txTask.nsdu = SendBuff;
    txTask.nsduLength = SendBuffLen;
    APP_TxBuffer_Add(&txTask);
#else

    app_usart_tx_buf(SendBuff, SendBuffLen, USART0);
#endif
}

// #include "../Source/timer/timer.h"
void APP_Queue_ListenAndHandleMessage(void)
{
    RxDataParametersStruct rxDataPointer;
    APP_DataTxTask txTask;
    uint16_t rxBufferLeftAmount = 0;
    rxBufferLeftAmount = APP_RxBuffer_getRxBufferMsgAmount();
    // APP_PRINTF("rxBufferLeftAmount:%d\n", rxBufferLeftAmount);
    if (rxBufferLeftAmount > 0) {                                              // 接受队列
        APP_RxBuffer_GetFirstMsgDataParameters(&rxDataPointer);                // 读取第一条数据
        APP_UappsMsgProcessing(rxDataPointer.rxData, rxDataPointer.rxDataLen); // 处理读取的数据
        APP_RxBuffer_DeleteFirstMsg();                                         // 删除已经处理的数据
    }
    if (APP_TxBuffer_GetFirstMsgDataParameters(&txTask)) { // 检查队列是否 有需要发送的数据
        // APP_PRINTF("time:%d\n", app_timer_get_ticks());
        app_usart_tx_buf(txTask.nsdu, txTask.nsduLength, USART0);
    }
}
#endif
