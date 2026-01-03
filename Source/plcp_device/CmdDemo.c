
#include "../../Source/device/device_manager.h"
#if defined PLCP_DEVICE
#include "../../Source/plcp_common/Inc/lmexxx_conf.h"
#include "../../Source/usart/usart.h"
#include "../Source/plcp_device/APP_PublicAttribute.h"
#include "MseProcess.h"
#include "gd32e23x.h"

/*---------------------------------------------------------------
函数名称：Uapps_PackRSLWithFromMsg
函数功能：添加from选项
输入参数：sendBuff-发送缓存；rslStr-网关；from-本地；playload-需要上送的数据；playloadLen-数据长度
返回值：  sendBuffLen-发送缓存长度
备 注：
---------------------------------------------------------------*/
uint16_t Uapps_PackRSLWithFromMsg(uint8_t *sendBuff, char *rslStr, char *from, uint8_t *playload, uint8_t playloadLen)
{
    uint16_t sendBuffLen;
    UappsMessage txMsg;
    char RSLtemp[65];

    if (sendBuff == NULL || rslStr == NULL || strlen(rslStr) == 0) {
        return 0;
    }
    if (playloadLen > 0 && playload == NULL) {
        return 0;
    }

    strcpy(RSLtemp, rslStr);
    // memset(&txMsg, 0, sizeof(UappsMessage));
    UappsCreateMessage(&txMsg, UAPPS_TYPE_NON, UAPPS_REQ_PUT, RSLtemp);

    if (from != NULL) {
        if (UAPPS_OK != UappsPutOption(&txMsg, UAPPS_OPT_FROM, (uint8_t *)from, strlen(from))) {
            return 0;
        }
    }
    if (playloadLen > 0) {
        if (UAPPS_OK != UappsPutData(&txMsg, playload, playloadLen, UAPPS_FMT_OCTETS, 0)) {
            return 0;
        }
    }

    memcpy(sendBuff, ESC_STRING, ESC_LEN);
    sendBuffLen = Uapps2Bytes(&txMsg, &sendBuff[ESC_LEN]) + ESC_LEN;
    if (sendBuffLen >= 450) {
        printf("send data too long!\n");
        return 0;
    }

    return sendBuffLen;
}

/*---------------------------------------------------------------
函数名称：CmdTest_MSE_SET_STATE_ON
函数功能：构造打开指令的Uapps报文函数
输入参数：
返回值：   无
备 注：112233445566为对应设备的mac地址，rsi需格式按照文档格式规范
---------------------------------------------------------------*/
void CmdTest_MSE_SET_STATE_ON(void)
{
    UappsMessage testCoap;
    char rsi[50] = "@SE1.112233445566/_on";
    uint8_t uartSendBuff[100];
    uint16_t uartSendBuffLen = 0;

    printf("enter SET_STATE_ON\n");

    // memset(uartSendBuff, 0, 100 * sizeof(uint8_t));
    // memset(&testCoap, 0, sizeof(UappsMessage));
    UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_PUT, rsi);

    // 如果应用层要设置token ID的话
    testCoap.token[0] = 0x11;
    testCoap.token[1] = 0x33;

    uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, 100);
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
}

/*--------------------------------------------------------------
函数名称：CmdTest_MSE_SET_STATE_MODE
函数功能：构造设置模式的Uapps报文函数
输入参数：
返回值：   无
备 注：112233445566为对应设备的mac地址，rsi需格式按照文档格式规范
---------------------------------------------------------------*/
void CmdTest_MSE_SET_STATE_MODE(void)
{
    UappsMessage testCoap;
    char rsi[50] = "@SE1.112233445566/HVAC/_mode";
    uint8_t uartSendBuff[100];
    uint16_t uartSendBuffLen = 0;
    uint8_t date[] = {0x1, 0x2}; // 假设设置的模式数据为1,2.
    uint8_t dataPayload[4];
    printf("enter SET_STATE_MOD\n");

    // memset(&dataPayload, 0, sizeof(dataPayload));
    // memset(&testCoap, 0, sizeof(UappsMessage));
    UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_PUT, rsi);

    // 如果应用层要设置token ID的话
    testCoap.token[0] = 0x11;
    testCoap.token[1] = 0x33;

    memcpy(dataPayload, date, sizeof(date));
    UappsPutData(&testCoap, dataPayload, sizeof(dataPayload), UAPPS_FMT_OCTETS, 0); // 加入载荷
    uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, 100);
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
}

/*--------------------------------------------------------------
函数名称：CmdTest_MSE_GET_MAC
函数功能：构造获取plc模组mac地址的指令Uapps报文函数
输入参数：
返回值：   无
备 注：由于是本地串口直接读取plc模组的信息，因此不需要指定该mac地址。
---------------------------------------------------------------*/
void CmdTest_MSE_GET_MAC(void)
{
    UappsMessage testCoap;
    char rsi[50] = "@SE1./_mac";
    uint8_t uartSendBuff[100];
    uint16_t uartSendBuffLen = 0;

    printf("enter GET_MAC\n");

    memset(&testCoap, 0, sizeof(UappsMessage));
    UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_GET, rsi);

    // 如果应用层要设置token ID的话
    testCoap.token[0] = 0x52;
    testCoap.token[1] = MSE_GET_selfMAC;

    uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, 100);
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
}

/*--------------------------------------------------------------
函数名称：CmdTest_MSE_GET_DID
函数功能：构造获取plc模组DID的指令Uapps报文函数
输入参数：
返回值：   无
备 注：由于是本地串口直接读取plc模组的信息，因此不需要指定该mac地址。
        获取plc模组的did，did为0代表模组未入网，did不为0代表模组已入网
---------------------------------------------------------------*/
void CmdTest_MSE_GET_DID(void)
{
    UappsMessage testCoap;
    char rsi[] = "@SE1./3/1";
    uint8_t uartSendBuff[100];
    uint16_t uartSendBuffLen = 0;

    // printf("enter GET_DID\n");

    memset(&testCoap, 0, sizeof(UappsMessage));
    UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_GET, rsi);

    // 如果应用层要设置token ID的话
    testCoap.token[0] = 0x52;
    testCoap.token[1] = MSE_GET_DID;

    uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, 100);
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
}

/*--------------------------------------------------------------
函数名称：CmdTest_MSE_POST_Status
函数功能：构造主动上送状态的Uapps报文函数
输入参数：GatewaymacAddr-网关的mac地址；LocalmacAddr-本地模组的mac地址； buf-需要上送的数据；buflen-数据长度
返回值：   无
备 注：当温控器状态在本地发生变化时需要主动上送给网关。（本地发生不经过云端的控制时，为了设备与云端状态同步因此需要主动上送发生改变的状态参数）
---------------------------------------------------------------*/
void CmdTest_MSE_POST_Status(uint8_t *GatewaymacAddr, uint8_t *LocalmacAddr, uint8_t *buf, uint8_t buflen)
{
    UappsMessage testCoap;
    char rsi[50];
    char from[30];
    uint8_t uartSendBuff[128];
    uint8_t uartSendBuffLen = 0;
    printf("enter POST_Status\n");
    // memset(rsi, 0, sizeof(rsi));
    // memset(from, 0, sizeof(from));
    // memset(uartSendBuff, 0, sizeof(uartSendBuff));
    memset(&testCoap, 0, sizeof(UappsMessage));

    //	if (PLCP_GetDID() != 0)
    {
        sprintf(rsi, "@SE1.%02x%02x%02x%02x%02x%02x%s",
                GatewaymacAddr[0], GatewaymacAddr[1], GatewaymacAddr[2],
                GatewaymacAddr[3], GatewaymacAddr[4], GatewaymacAddr[5],
                "/_state"); // 网关的mac地址

        sprintf(from, "@SE1.%02x%02x%02x%02x%02x%02x",
                LocalmacAddr[0], LocalmacAddr[1], LocalmacAddr[2],
                LocalmacAddr[3], LocalmacAddr[4], LocalmacAddr[5]); // 本地的mac地址

        uartSendBuffLen = Uapps_PackRSLWithFromMsg(uartSendBuff, rsi, from, buf, buflen);

        // 如果应用层要设置token ID的话
        testCoap.token[0] = 0x11;
        testCoap.token[1] = 0x33;
        MCU_Send_date(uartSendBuff, uartSendBuffLen);
    }
}

/*--------------------------------------------------------------
函数名称：CmdTest_MSE_POST_Config_Status
函数功能：构造主动上送属性状态的Uapps报文函数
输入参数：GatewaymacAddr-网关的mac地址；LocalmacAddr-本地模组的mac地址； buf-需要上送的数据；buflen-数据长度
返回值：   无
备 注：当温控器属性相关状态在本地发生变化时需要主动上送给网关。
（本地发生不经过云端的控制时，为了设备与云端状态同步因此需要主动上送发生改变的状态参数）
---------------------------------------------------------------*/
void CmdTest_MSE_POST_Config_Status(uint8_t *GatewaymacAddr, uint8_t *LocalmacAddr, uint8_t *buf, uint8_t buflen)
{
    UappsMessage testCoap;
    char rsi[50];
    char from[30];
    uint8_t uartSendBuff[128];
    uint8_t uartSendBuffLen = 0;
    printf("enter POST_Config\n");

    //	if (PLCP_GetDID() != 0)
    {
        sprintf(rsi, "@SE1.%02x%02x%02x%02x%02x%02x%s",
                GatewaymacAddr[0], GatewaymacAddr[1], GatewaymacAddr[2],
                GatewaymacAddr[3], GatewaymacAddr[4], GatewaymacAddr[5],
                "/_config"); // 网关的mac地址

        sprintf(from, "@SE1.%02x%02x%02x%02x%02x%02x",
                LocalmacAddr[0], LocalmacAddr[1], LocalmacAddr[2],
                LocalmacAddr[3], LocalmacAddr[4], LocalmacAddr[5]); // 本地的mac地址

        uartSendBuffLen = Uapps_PackRSLWithFromMsg(uartSendBuff, rsi, from, buf, buflen);

        // 如果应用层要设置token ID的话
        testCoap.token[0] = 0x11;
        testCoap.token[1] = 0x33;
        MCU_Send_date(uartSendBuff, uartSendBuffLen);
    }
}

/*--------------------------------------------------------------
函数名称：CmdTest_MSE_GET_CC0MAC
函数功能：构造获取网关mac地址的指令Uapps报文函数
输入参数：
返回值：   无
备 注：由于是本地串口直接读取读取网关的信息，因此不需要指定该mac地址。
---------------------------------------------------------------*/
void CmdTest_MSE_GET_CC0MAC(void)
{
    UappsMessage testCoap;
    char rsi[50] = "@./0/_ccomac";
    uint8_t uartSendBuff[100];
    uint16_t uartSendBuffLen = 0;

    // printf("enter GET_CCOMAC\n");

    // memset(&testCoap, 0, sizeof(UappsMessage));
    UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_GET, rsi);

    // 如果应用层要设置token ID的话
    testCoap.token[0] = 0x52;
    testCoap.token[1] = MSE_GET_CC0MAC;

    uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, 100);
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
}

/*--------------------------------------------------------------
函数名称：CmdTest_MSE_SET_DID
函数功能：构造获取主动退网的指令Uapps报文函数
输入参数：
返回值： 无
备 注：由于是本地串口直接读取读取网关的信息，因此不需要指定该mac地址。
---------------------------------------------------------------*/
void CmdTest_MSE_SET_DID(void)
{
    UappsMessage testCoap;
    char rsi[50] = "@0./3/1";
    uint8_t uartSendBuff[100];
    uint16_t uartSendBuffLen = 0;
    uint8_t did[2] = {0x00, 0x00};
    printf("enter SET_DID\n");

    UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_PUT, rsi);

    // 如果应用层要设置token ID的话
    testCoap.token[0] = 0x52;
    testCoap.token[1] = MSE_SET_DID;

    UappsPutData(&testCoap, did, sizeof(did), UAPPS_FMT_OCTETS, 0); // 加入载荷

    uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, 100);
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
}

/*--------------------------------------------------------------
函数名称：CmdTest_MSE_Factory
函数功能：构造使模组恢复出厂设置的指令Uapps报文函数
输入参数：
返回值：
备 注：由于是本地串口直接读取读取网关的信息，因此不需要指定该mac地址。
---------------------------------------------------------------*/
void CmdTest_MSE_Factory(void)
{
    UappsMessage testCoap;
    char rsi[50] = "@./0/_factory";
    uint8_t uartSendBuff[100];
    uint16_t uartSendBuffLen = 0;

    printf("enter factory\n");

    UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_PUT, rsi);

    // 如果应用层要设置token ID的话
    testCoap.token[0] = 0x52;
    testCoap.token[1] = MSE_SET_Factory;

    uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, 100);
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
}

/*--------------------------------------------------------------
函数名称：CmdTest_MSE_SET_Transfer
函数功能：构造设置模组支持透传功能的指令Uapps报文函数
输入参数：
返回值： 无
备 注：由于是本地串口直接读取读取网关的信息，因此不需要指定该mac地址。
---------------------------------------------------------------*/
void CmdTest_MSE_SET_Transfer(void)
{
    UappsMessage testCoap;
    char rsi[50] = "@./11/1";
    uint8_t uartSendBuff[100];
    uint16_t uartSendBuffLen = 0;
    uint8_t did[1] = {0x01};
    printf("enter SET_Transfer\n");

    UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_PUT, rsi);

    // 如果应用层要设置token ID的话
    testCoap.token[0] = 0x52;
    testCoap.token[1] = MSE_SET_Transfer;

    UappsPutData(&testCoap, did, sizeof(did), UAPPS_FMT_OCTETS, 0); // 加入载荷

    uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, 100);
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
}
#endif
