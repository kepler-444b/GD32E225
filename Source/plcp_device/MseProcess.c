#include "../../Source/plcp_device/MseProcess.h"
#include "../../Source/base/base.h"
#include "../../Source/base/debug.h"
#include "../../Source/device/device_manager.h"
#include "../../Source/eventbus/eventbus.h"
#include "../../Source/plcp_common/Inc/lmexxx_conf.h"
#include "../../Source/plcp_device/APP_PublicAttribute.h"
#include "../../Source/plcp_device/plcp_panel/plcp_panel_callbacks.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_bind.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_scene.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_special_scene.h"
#include "../../Source/plcp_device/plcp_user_api/plcp_sdk_api.h"
#include "systick.h"

#define MSE_SERVICE_NUM 16
static ServiceEntityAttribute gMSEService[MSE_SERVICE_NUM] =
    {
        MSE_DEVICE_ONPEN,
        MSE_DEVICE_CLOSE,
        MSE_DEVICE_SET_CONFIG,

        MSE_DEVICE_GROUP_JOIN,
        MSE_DEVICE_GROUP_QUIT,
        MSE_DEVICE_GROUP_LIST,

        MSE_DEVICE_SCENE_JOIN,
        MSE_DEVICE_SCENE_QUIT,
        MSE_DEVICE_SCENE_LIST,
        MSE_DEVICE_SCENE_COPY,

        MSE_DEVICE_PLCP_FACTORY,
        MSE_DEVICE_PLCP_RESET,
        MSE_DEVICE_PLCP_STATE,
        MSE_DEVICE_PLCP_STATE_ONOFF,
        MSE_DEVICE_PLCP_MCUVER,
        MSE_DEVICE_PLCP_BIND,
};

const struct Function_Config dev =
    {
        .Control_Open = MCU_Control_Open,
        .Control_Close = MCU_Control_Close,

        .Set_Config = MCU_Set_Config, // 设置属性
        .Get_Config = MCU_Get_Config, // 获取属性

        .Device_Facory = MCU_Device_Factory,
        .Device_Reset = MCU_Device_ResetService,
        .Device_Putstate = MCU_Put_Devicestatus,
        .Device_Getstate = MCU_Get_Devicestatus,

        .Device_GetMCUver = MCU_Get_Ver,
};

uint16_t Aps_UartMessage(UappsMessage *pMsg, uint8_t *sendBuff, uint16_t sendBuffMaxSize)
{
    uint16_t len = 0;
    uint8_t escapeStr[ESC_LEN] = ESC_STRING; // 报文固定帧头

    memset(sendBuff, 0, sendBuffMaxSize * sizeof(uint8_t));
    memcpy(sendBuff, escapeStr, ESC_LEN * sizeof(uint8_t));

    len = Uapps2Bytes(pMsg, &(sendBuff[ESC_LEN]));
    len += ESC_LEN;

    return len;
}

void APP_SendACK(UappsMessage *reqMsg, uint16_t payloadFlag, uapps_rw_buffer_t *scratch, uint8_t type, uint8_t repondCode)
{
    char destaddr[12];
    uint8_t uartSendBuff[256];
    uint16_t uartSendBuffLen = 0;
    uint16_t contentFormat = UAPPS_FMT_OCTETS;
    UappsMessage respondPacket;
    memset(&respondPacket, 0, sizeof(UappsMessage));

    if (payloadFlag) {
        UappsDataResponse(reqMsg, &respondPacket, type, repondCode, scratch->p, scratch->len, contentFormat, 0); // 带载荷进行回复
    } else {
        UappsSimpleResponse(reqMsg, &respondPacket, type, repondCode);
    }
    UappsGetNodaFromOptionFROM(reqMsg, destaddr);
    UappsAddFromOption(&respondPacket, destaddr);
    uartSendBuffLen = Aps_UartMessage(&respondPacket, uartSendBuff, 256);
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
}

void APP_STATE_ON_Processing(UappsMessage *uappsMsg, uapps_rw_buffer_t *scratch, RSL_t *rsl)
{
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uint8_t payloadFlag = 0;

    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) {
        if (PlcSdkCallbackOnOff(rsl->aei, 1)) {
            respondCode = UAPPS_ACK_CHANGED;
        }
    }
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) {
        APP_SendACK(uappsMsg, payloadFlag, scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_STATE_OFF_Processing(UappsMessage *uappsMsg, uapps_rw_buffer_t *scratch, RSL_t *rsl)
{
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uint8_t payloadFlag = 0;

    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) {
        uint16_t optionValueLen = uappsMsg->options[0].opt_len;
        if (PlcSdkCallbackOnOff(rsl->aei, 0)) {
            respondCode = UAPPS_ACK_CHANGED;
        }
    }
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) {
        APP_SendACK(uappsMsg, payloadFlag, scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_STATE_SET_CONFIG(UappsMessage *uappsMsg)
{
    APP_PRINTF("APP_STATE_SET_CONFIG\n");
    uapps_rw_buffer_t scratch;

    uint8_t type = uappsMsg->pl_ptr[0]; // 命令类型

    uint16_t bits = ((uint16_t)uappsMsg->pl_ptr[1] << 8) | uappsMsg->pl_ptr[2]; // 控制字

    uint8_t ackBuf[3 + COUNT_ONES_16(bits)]; // 根据number来定义ACK载荷的长度
    ackBuf[0] = type;
    ackBuf[1] = (bits >> 8) & 0xFF;
    ackBuf[2] = bits & 0xFF;

    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uint16_t ackLen = 0;
    uint8_t payloadFlag = 0;

    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) {
        respondCode = UAPPS_ACK_CHANGED;
        dev.Set_Config(uappsMsg->pl_ptr, uappsMsg->pl_len);

    } else if (uappsMsg->hdr.hdrCode == UAPPS_REQ_GET) {
        respondCode = UAPPS_ACK_CONTENT;
        ackLen = dev.Get_Config(&ackBuf[3], type, bits);
        payloadFlag = 1;
        scratch.p = ackBuf;
        if (ackLen != 0) {
#if defined PLCP_PANEL
            scratch.len = ackLen + 3;
#elif defined PLCP_LIGHT_CT
            scratch.len = ackLen;
#endif
        }
    }
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) // 判断收到的报文内是否需要进行回复
    {
        APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_STATE_DEVICESTATE(UappsMessage *uappsMsg, uapps_rw_buffer_t *scratch)
{
    const sPublic_attribute *pAttributeOfAPP = APP_Attribute_GetPointer();

    uint8_t type = uappsMsg->pl_ptr[0];                                         // 控件类型
    uint16_t bits = ((uint16_t)uappsMsg->pl_ptr[1] << 8) | uappsMsg->pl_ptr[2]; // 控制字
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uint16_t ackLen = 0;
    uint8_t payloadFlag = 0;

#if defined PLCP_PANEL
    uint8_t ackBuf[3 + COUNT_ONES_16(bits)]; // 根据number来定义ACK载荷的长度
    ackBuf[0] = type;
    ackBuf[1] = (bits >> 8) & 0xFF;
    ackBuf[2] = bits & 0xFF;
#elif defined PLCP_LIGHT_CT
    uint8_t ackBuf[6];
#endif
    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_GET) { // 获取状态
        respondCode = UAPPS_ACK_CONTENT;
#if defined PLCP_PANEL
        ackLen = dev.Device_Getstate(&ackBuf[3], type, bits);
#elif defined PLCP_LIGHT_CT
        ackLen = dev.Device_Getstate(ackBuf, type, bits);
#endif
        payloadFlag = 1;
        scratch->p = ackBuf;
        if (ackLen != 0) {
#if defined PLCP_PANEL
            scratch->len = ackLen + 3;
#elif defined PLCP_LIGHT_CT
            scratch->len = ackLen;
#endif
        }
    } else if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) { // 设置状态
        respondCode = UAPPS_ACK_CHANGED;
        dev.Device_Putstate(uappsMsg->pl_ptr, uappsMsg->pl_len);
    }

    // 回复ACK
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) // 判断收到的报文内是否需要进行回复
    {
        APP_SendACK(uappsMsg, payloadFlag, scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_STATE_ONOFF_DEVICESTATE(UappsMessage *uappsMsg)
{
    APP_PRINTF("APP_STATE_ONOFF_DEVICESTATE\n");
}

void APP_RestoreFactorySettings(UappsMessage *uappsMsg)
{
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uapps_rw_buffer_t scratch;
    memset(&scratch, 0, sizeof(scratch));
    uint8_t payloadFlag = 0;
    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) {

        payloadFlag = 1;
        dev.Device_Facory();
    }
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) // 判断收到的报文内是否需要进行回复
    {
        APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_ResetService(UappsMessage *uappsMsg)
{
    LOGINFO0(UART_DEBUG, "Enter Reset\n");

    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) {
        dev.Device_Reset(uappsMsg->pl_ptr, uappsMsg->pl_len);
    }
}

void APP_STATE_GET_MCUVER(UappsMessage *uappsMsg, uapps_rw_buffer_t *scratch)
{
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uint8_t ackBuf[64];
    uint8_t payloadFlag = 0;
    uint16_t ackLen = 0;

    APP_PRINTF("enter GET_MCUVER\n");
    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_GET) {
        respondCode = UAPPS_ACK_CONTENT;
        ackLen = dev.Device_GetMCUver(ackBuf);
        payloadFlag = 1;
        scratch->p = ackBuf;
        if (ackLen != 0) {
#if defined PLCP_PANEL
            scratch->len = ackLen + 3;
#elif defined PLCP_LIGHT_CT
            scratch->len = ackLen;
#endif
        }
    }
    // 回复ACK
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) // 判断收到的报文内是否需要进行回复
    {
        APP_SendACK(uappsMsg, payloadFlag, scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_STATE_GROUP_JOIN(UappsMessage *uappsMsg, uapps_rw_buffer_t *scratch, const char *aei)
{
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uint8_t payloadFlag = 0;
    uint8_t err = 0;

    APP_PRINTF("enter GROUP_JOIN\n");
    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) {
        // 添加群组并保存
        err = APP_Group_Join(uappsMsg->pl_ptr, uappsMsg->pl_len, aei);
    }
    if (err == 0) {
        respondCode = UAPPS_ACK_CHANGED;
    }

    // 回复ACK
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) // 判断收到的报文内是否需要进行回复
    {
        APP_SendACK(uappsMsg, payloadFlag, scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_STATE_GROUP_QUIT(UappsMessage *uappsMsg, uapps_rw_buffer_t *scratch, const char *aei)
{
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uint8_t err = 0;
    uint8_t payloadFlag = 0;

    APP_PRINTF("enter GROUP_QUIT\n");
    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) {
        // 删除指定群组
        err = APP_Group_Quit(uappsMsg->pl_ptr, uappsMsg->pl_len, aei);
    }
    if (err == 0) {
        respondCode = UAPPS_ACK_CHANGED;
    }
    // 回复ACK
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) // 判断收到的报文内是否需要进行回复
    {
        APP_SendACK(uappsMsg, payloadFlag, scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_STATE_GROUP_LIST(UappsMessage *uappsMsg, const char *aei)
{
    uapps_rw_buffer_t scratch;
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uint8_t ackBuf[64];
    uint8_t payloadFlag = 0;
    uint16_t ackLen = 0;
    memset(&scratch, 0, sizeof(uapps_rw_buffer_t));

    APP_PRINTF("enter GROUP_LIST\n");
    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_GET) {
        respondCode = UAPPS_ACK_CONTENT;
        // 将群组列表添加到载荷中进行回复
        ackLen = APP_Group_List(ackBuf, aei);
        payloadFlag = 1;
        scratch.p = ackBuf;
        if (ackLen != 0) {
            scratch.len = ackLen;
        }
    }
    // 回复ACK
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) // 判断收到的报文内是否需要进行回复
    {
        APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_STATE_SCENE_JOIN(UappsMessage *uappsMsg, const char *aei)
{
    uapps_rw_buffer_t scratch;
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uint8_t err = 0;
    uint8_t payloadFlag = 0;
    memset(&scratch, 0, sizeof(scratch));

    APP_PRINTF("enter SCENE_JOIN\n");
    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) {

        err = APP_Scene_Join(uappsMsg->pl_ptr, uappsMsg->pl_len, aei);
    }
    if (err == 0) {
        respondCode = UAPPS_ACK_CHANGED;
    }

    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) // 判断收到的报文内是否需要进行回复
    {
        APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_STATE_SCENE_QUIT(UappsMessage *uappsMsg, const char *aei)
{
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uint8_t payloadFlag = 0;
    uint8_t err = 0;
    uapps_rw_buffer_t scratch;
    memset(&scratch, 0, sizeof(scratch));

    APP_PRINTF("enter SCENE_QUIT\n");
    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) {
        // 删除指定场景
        err = APP_Scene_Quit(uappsMsg->pl_ptr, uappsMsg->pl_len, aei);
        // static test my_test;
        // my_test.buf = uappsMsg->pl_ptr;
        // my_test.len = uappsMsg->pl_len;
        // my_test.aei = aei;
        // app_eventbus_publish(PLCP_COPY_GET, &my_test);
    }
    if (err == 0) {
        respondCode = UAPPS_ACK_CHANGED;
    }

    // 回复ACK
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) // 判断收到的报文内是否需要进行回复
    {
        APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_STATE_SCENE_LIST(UappsMessage *uappsMsg, const char *aei)
{
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uint8_t payloadFlag = 0;
    uint16_t ackLen = 0;
    uint8_t ackBuf[250];
    uapps_rw_buffer_t scratch;
    memset(&scratch, 0, sizeof(scratch));

    APP_PRINTF("enter SCENE_LIST\n");

    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_GET) {
        respondCode = UAPPS_ACK_CONTENT;
        ackLen = APP_Scene_List(ackBuf, aei); // 将场景列表添加到载荷中进行回复
        payloadFlag = 1;
        scratch.p = ackBuf;
        if (ackLen != 0) {
            scratch.len = ackLen;
        }
    }

    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) { // 判断收到的报文内是否需要进行回复
        APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_STATE_SCENE_COPY(UappsMessage *uappsMsg, const char *aei)
{

    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uint8_t payloadFlag = 0;
    uint16_t ackLen = 0;
    uint8_t ackBuf[250];

    uapps_rw_buffer_t scratch;
    memset(&scratch, 0, sizeof(scratch));

    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_GET) {
        respondCode = UAPPS_ACK_CONTENT;
        ackLen = APP_Scene_Copy_Get(ackBuf, aei); // 将场景参数添加到载荷中进行回复
        payloadFlag = 1;
        scratch.p = ackBuf;
        if (ackLen != 0) {
            scratch.len = ackLen;
        }
    }
    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) {
        respondCode = UAPPS_ACK_CONTENT;
        ackLen = APP_Scene_Copy_Get(ackBuf, aei); // 将场景参数添加到载荷中进行回复
        payloadFlag = 1;
        scratch.p = ackBuf;
        if (ackLen != 0) {
            scratch.len = ackLen;
        }
    }
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) // 判断收到的报文内是否需要进行回复
    {
        APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_BIND_Processing(UappsMessage *uappsMsg, uapps_rw_buffer_t *scratch)
{
    uint8_t i;
    uint8_t eventSE;
    char eventAEI[AEI_MAX_LEN];
    char eventID[AEI_MAX_LEN];
    char msg[MAX_BIND_MSG_LEN];
    uint8_t ackBuf[64];

    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uint16_t payloadFlag = 0;
    uint8_t count = 0;

    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) { // 写入绑定信息
        count = PLCP_CountBindDataInAddBindParam(uappsMsg->pl_ptr, uappsMsg->pl_len);
        APP_PRINTF("count:%d\n", count);
        if (count > 0) {
            for (i = 0; i < count; i++) {
                memset(&msg, 0, MAX_BIND_MSG_LEN);
                if (1 == PLCP_GetBindDataInAddBindParam(uappsMsg->pl_ptr, uappsMsg->pl_len, i, &eventSE, eventAEI, eventID, msg)) {
                    APP_PRINTF("PLCP_GetBindDataInAddBindParam\n");
                    if (1 == PLCP_bindTableWrite(eventSE, eventAEI, eventID, msg)) {
                        APP_PRINTF("UAPPS_ACK_CHANGED\n");
                        respondCode = UAPPS_ACK_CHANGED;
                    }
                }
            }
        }
    } else if (uappsMsg->hdr.hdrCode == UAPPS_REQ_DELETE) { // 删除指定事件的绑定信息
        count = PLCP_CountEventInDelBindParam(uappsMsg->pl_ptr, uappsMsg->pl_len);
        APP_PRINTF("count %d\n", count);
        if (count > 0 && count != 0xff) {
            for (i = 0; i < count; i++) {
                if (1 == PLCP_GetEventInDelBindParam(uappsMsg->pl_ptr, uappsMsg->pl_len,
                                                     i, &eventSE, eventAEI, eventID)) {
                    APP_PRINTF("del bind %d\n", i);
                    if (1 == PLCP_bindTableDelet(eventSE, eventAEI, eventID)) {
                        respondCode = UAPPS_ACK_CHANGED;
                    }
                }
            }
        } else if (count == 0xff) { // 删除多个绑定信息
            count = PLCP_CountKJInDelBindParam(uappsMsg->pl_ptr, uappsMsg->pl_len);
            if (count > 0) {
                for (i = 0; i < count; i++) {
                    if (1 == PLCP_GetKJInDelBindParam(uappsMsg->pl_ptr, uappsMsg->pl_len,
                                                      i, &eventSE, eventAEI)) {
                        PLCP_bindTableDeletByAEI(eventSE, eventAEI);
                        respondCode = UAPPS_ACK_CHANGED;
                    }
                }
            }
        }
    } else if (uappsMsg->hdr.hdrCode == UAPPS_REQ_GET) { // 查询绑定信息
        scratch->len = PLCP_GetBindData(uappsMsg->pl_ptr, uappsMsg->pl_len, ackBuf);
        if (scratch->len != 0) {
            scratch->p = ackBuf;
            payloadFlag = 1;
        }
        respondCode = UAPPS_ACK_CONTENT;
    }

    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) {
        APP_SendACK(uappsMsg, payloadFlag, scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_TOPIC_MSEProcessing(UappsMessage *uappsMsg, RSL_t *rsl, uint8_t *Buff, uint8_t len)
{
    // APP_PRINTF("APP_TOPIC_MSEProcessin\n");
    uint8_t i = 0;
    uint8_t index = 0;
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uapps_rw_buffer_t scratch;

    memset(&scratch, 0, sizeof(uapps_rw_buffer_t));
    const sPublic_attribute *pAttributeOfAPP = APP_Attribute_GetPointer();

    for (i = 0; i < MSE_SERVICE_NUM; i++) {
        if (strcmp(rsl->rsi, (char *)gMSEService[i].resourceInfo) == 0) {
            if (i > DEVICE_SCENE_COPY_e) { // 跟控制无关的内容无需判断did
                break;
            }
            if (pAttributeOfAPP->did == 0) {
                if (uappsMsg->hdr.type == UAPPS_TYPE_CON) {
                    uint8_t payloadFlag = 0;
                    respondCode = UAPPS_NOT_FOUND;
                    APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
                }
                return;
            }
            break;
        }
    }
    switch (i) {
    case DEVICE_STATE_OPEN_E: { // 开机
        APP_STATE_ON_Processing(uappsMsg, &scratch, rsl);
        index++;
        break;
    }
    case DEVICE_STATE_CLOSE_E: { // 关机
        APP_STATE_OFF_Processing(uappsMsg, &scratch, rsl);
        index++;
        break;
    }
    case DEVICE_STATE_SET_CONFIG_E: { // 设置属性
        APP_STATE_SET_CONFIG(uappsMsg);
        index++;
        break;
    }
    case DEVICE_FACTORY_e: { // 恢复出厂设置
        APP_RestoreFactorySettings(uappsMsg);
        index++;
        break;
    }
    case DEVICE_RESET_e: { // 复位
        APP_ResetService(uappsMsg);
        index++;
        break;
    }
    case DEVICE_STATE_e: { // 设置state
        APP_STATE_DEVICESTATE(uappsMsg, &scratch);
        index++;
        break;
    }
    case DEVICE_STATE_ONOFF_e: {
        APP_STATE_ONOFF_DEVICESTATE(uappsMsg);
        index++;
    } break;
    case DEVICE_MCUVER_e: {
        APP_STATE_GET_MCUVER(uappsMsg, &scratch);
        index++;
        break;
    }
    case DEVICE_BIND_e: {
        APP_BIND_Processing(uappsMsg, &scratch);
        index++;
    } break;
    }
    if (index > 0) {
        // 本地状态改变保存
    }
}

uint8_t APP_Group_TOPIC_MSEProcessing(UappsMessage *uappsMsg, RSL_t *rsl, uint8_t *Buff, uint8_t len)
{
    uint16_t i;
    uint8_t index = 0;
    uapps_rw_buffer_t scratch;
    memset(&scratch, 0, sizeof(uapps_rw_buffer_t));

    if (night_scene_info_get()->night_enable == 0x01) { // 使能夜灯模式
        if (night_scene_state_get() == 0x02) {          // 即将进入夜灯模式(此时收到任何群组都退出夜灯模式)
            delay_scene_stop();
        }
    }
    for (i = 0; i < MSE_SERVICE_NUM; i++) {
        if (strcmp((char *)rsl->rsi, (char *)gMSEService[i].resourceInfo) == 0) {
            break;
        }
    }
    switch (i) {
    case DEVICE_STATE_e: { // 控制群组开
        APP_Device_GroupState(uappsMsg, rsl->aei);
        index++;
    } break;
    case DEVICE_STATE_CLOSE_E: { // 控制群组关
        APP_Device_GroupClose(uappsMsg, rsl->aei);
        index++;
    } break;
    case DEVICE_GROUP_JOIN_e: { // 添加群组
        APP_STATE_GROUP_JOIN(uappsMsg, &scratch, rsl->aei);
        index++;
        break;
    }
    case DEVICE_GROUP_QUIT_e: { // 删除群组
        APP_STATE_GROUP_QUIT(uappsMsg, &scratch, rsl->aei);
        index++;
        break;
    }
    case DEVICE_GROUP_LIST_e: { // 查询群组
        APP_STATE_GROUP_LIST(uappsMsg, rsl->aei);
        index++;
        break;
    }
    default:
        break;
    }
    return index;
}

void APP_Scene_TOPIC_MSEProcessing(UappsMessage *uappsMsg, RSL_t *rsl, uint8_t *Buff, uint8_t len)
{
    uint16_t i = 0;
    if (night_scene_info_get()->night_enable == 0x01) { // 使能夜灯模式
        if (night_scene_state_get() == 0x02) {          // 即将进入夜灯模式(此时收到任何场景都退出夜灯模式)
            delay_scene_stop();
        }
        if (night_scene_state_get() == 0x01) { // 当前是夜灯模式
            night_scene_close();
        }
    }
    for (i = 0; i < MSE_SERVICE_NUM; i++) {
        if (strcmp((char *)rsl->rsi, (char *)gMSEService[i].resourceInfo) == 0) {
            break;
        }
    }

    switch (i) {
    case DEVICE_STATE_OPEN_E: { // 启动场景
        APP_Device_SceneStart(uappsMsg, rsl->aei);
        break;
    }
    case DEVICE_STATE_CLOSE_E: { //  关闭场景
        APP_Device_SceneClose(uappsMsg, rsl->aei);
    } break;
    case DEVICE_SCENE_JOIN_e: { // 添加场景
        APP_STATE_SCENE_JOIN(uappsMsg, rsl->aei);
        break;
    }
    case DEVICE_SCENE_QUIT_e: { // 删除场景
        APP_STATE_SCENE_QUIT(uappsMsg, rsl->aei);
        break;
    }
    case DEVICE_SCENE_LIST_e: { // 查询场景
        APP_STATE_SCENE_LIST(uappsMsg, rsl->aei);
        break;
    }
    case DEVICE_SCENE_COPY_e: { // 拷贝场景
        APP_STATE_SCENE_COPY(uappsMsg, rsl->aei);
    } break;
    default:
        break;
    }
}

void APP_UappsMsgProcessing(uint8_t *inputDataBuff, uint16_t inputDataBuffLen)
{
    // APP_PRINTF_BUF("inputDataBuff", inputDataBuff, inputDataBuffLen);
    UappsMessage uappsMsg;
    RSL_t rsiMsg;
    uint8_t rsiStr[128];
    uint8_t isUappsMsg = 0;
    uint16_t uappsMsgStartIndex = 0;
    uint16_t uappsMsgLen = 0;
    uint8_t isRSIDataFlag = 0;
    uint16_t optionValueLen = 0;
    uint16_t groupId = 0;
    uint8_t index = 0;

    memset(&uappsMsg, 0, sizeof(UappsMessage));
    memset(&rsiMsg, 0, sizeof(RSL_t));
    memset(rsiStr, 0, sizeof(rsiStr));

    isUappsMsg = isUappsData((char *)inputDataBuff, inputDataBuffLen, &uappsMsgStartIndex);

    if (1 == isUappsMsg) {
        uappsMsgLen = inputDataBuffLen - uappsMsgStartIndex - ESC_LEN;
        isRSIDataFlag = UappsFromBytes(&inputDataBuff[uappsMsgStartIndex + ESC_LEN], uappsMsgLen, &uappsMsg);
        optionValueLen = uappsMsg.options[0].opt_len;
        memcpy(rsiStr, uappsMsg.options[0].opt_val, optionValueLen * sizeof(uint8_t));
        rsiStr[optionValueLen] = '\0';
        RslFromStr(&rsiMsg, (char *)rsiStr);

        if (UAPPS_OK == isRSIDataFlag) {
            if (rsiMsg.se == 201) { // 群组数据
                APP_Group_TOPIC_MSEProcessing(&uappsMsg, &rsiMsg, uappsMsg.pl_ptr, uappsMsg.pl_len);
                return;
#if 0
                if (APP_isGroupExist(groupId, rsiMsg.aei) != 0) // 判断该群组号是否存在
                {
                    APP_PRINTF("groupId did not found!\n");
                    return;
                }
#endif
            }
            if (rsiMsg.se == 202) { // 场景数据
                APP_Scene_TOPIC_MSEProcessing(&uappsMsg, &rsiMsg, uappsMsg.pl_ptr, uappsMsg.pl_len);
                return;
            }
            // 调用用户自定义的处理函数
            APP_TOPIC_MSEProcessing(&uappsMsg, &rsiMsg, uappsMsg.pl_ptr, uappsMsg.pl_len);
            APP_Tx_ProcessOfACKReceived(&uappsMsg);
        } else {
            APP_PRINTF("isRSIDataFlag is not UAPPS_OK!\n");
            return;
        }
    } else {
        // 根据串口接收到的数据解析判断之后不为Uapps指令。如果mcu需要对该数据另外单独进行处理则放到此处进行解析
        uint8_t *target = (uint8_t *)"/_factory";
        size_t target_len = strlen("/_factory");
        if (memcmp(inputDataBuff, target, target_len) == 0) { // 恢复出厂设置
            APP_PRINTF("MCU_Device_Factory\n");
            MCU_Device_Factory();
        }
        return;
    }
}

// uint16_t Uapps_PackRSLWithFromMsg(uint8_t *sendBuff, char *rslStr, char *from, uint8_t *playload, uint8_t playloadLen)
// {
//     uint16_t sendBuffLen;

//     UappsMessage txMsg;
//     char RSLtemp[65];

//     if (sendBuff == NULL || rslStr == NULL || strlen(rslStr) == 0) {
//         return 0;
//     }
//     if (playloadLen > 0 && playload == NULL) {
//         return 0;
//     }

//     strcpy(RSLtemp, rslStr);
//     memset(&txMsg, 0, sizeof(UappsMessage));
//     UappsCreateMessage(&txMsg, UAPPS_TYPE_NON, UAPPS_REQ_PUT, RSLtemp);

//     if (from != NULL) {
//         if (UAPPS_OK != UappsPutOption(&txMsg, UAPPS_OPT_FROM, (uint8_t *)from, strlen(from))) {
//             return 0;
//         }
//     }
//     if (playloadLen > 0) {
//         if (UAPPS_OK != UappsPutData(&txMsg, playload, playloadLen, UAPPS_FMT_OCTETS, 0)) {
//             return 0;
//         }
//     }

//     memcpy(sendBuff, ESC_STRING, ESC_LEN);
//     sendBuffLen = Uapps2Bytes(&txMsg, &sendBuff[ESC_LEN]) + ESC_LEN;
//     if (sendBuffLen >= 450) {
//         APP_PRINTF("send data too long!\n");
//         return 0;
//     }

//     return sendBuffLen;
// }

//  是否包含广播地址
static uint8_t RslFindBroadcastAddr(char *RSL)
{
#define broadcastAddress_1 ".ffffffffffff"
#define broadcastAddress_2 ".FFFFFFFFFFFF"

    char *result;
    result = strstr(RSL, broadcastAddress_1);
    if (NULL != result) {
        return result - RSL;
    }

    result = strstr(RSL, broadcastAddress_2);
    if (NULL != result) {
        return result - RSL;
    }
    return 0;
}

// 是都带有本机地址
static uint8_t RslFindLocalAddr(char *RSL)
{
    uint8_t mac_temp[6];
    char mac_str[16];
    char *result;

    memset(mac_temp, 0, 6);
    plcp_sdk_api_get_self_mac(mac_temp);

    snprintf(mac_str, sizeof(mac_str), ".%02x%02x%02x%02x%02x%02x",
             mac_temp[0], mac_temp[1], mac_temp[2],
             mac_temp[3], mac_temp[4], mac_temp[5]);

    result = strstr(RSL, mac_str);
    if (NULL != result) {
        return result - RSL;
    }

    snprintf(mac_str, sizeof(mac_str), ".%02X%02X%02X%02X%02X%02X",
             mac_temp[0], mac_temp[1], mac_temp[2],
             mac_temp[3], mac_temp[4], mac_temp[5]);

    result = strstr(RSL, mac_str);
    if (NULL != result) {
        return result - RSL;
    }
    return 0;
}

// 向前声明
uint16_t Uapps_PackRSLWithFromMsg(uint8_t *sendBuff, char *rslStr, char *from, uint8_t *playload, uint8_t playloadLen);

uint8_t APP_SendRSL(char *rslStr, char *from, uint8_t *playload, uint8_t playloadLen)
{
    uint8_t uartSendBuff[256];
    uint16_t uartSendBuffLen = 0;
    uartSendBuffLen = Uapps_PackRSLWithFromMsg(uartSendBuff, rslStr, from, playload, playloadLen);
    if (uartSendBuffLen) {
        MCU_Send_date(uartSendBuff, uartSendBuffLen);
        if (RslFindBroadcastAddr(rslStr) != 0 || RslFindLocalAddr(rslStr) != 0) // is Broadcast or self mac
        {
            MCU_UartReceive(uartSendBuff, uartSendBuffLen);
        }
        return 1;
    } else {
        return 0;
    }
}
