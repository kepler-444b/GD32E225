#include "../../Source/device/device_manager.h"
#if defined PLCP_DEVICE
#include "../../Source/base/debug.h"
#include "../../Source/plcp_device/APP_PublicAttribute.h"
#include "../Source/flash/flash.h"

static sPublic_attribute Public_attribute;
static sPost_wflash sPost_flag;

void APP_Attribute_Init(void)
{
    memset(&Public_attribute, 0, sizeof(sPublic_attribute));
    memset(&sPost_flag, 0, sizeof(sPost_wflash));
    APP_SavePostFlag();
}

/*--------------------------------3-----------------------------
函数名称：APP_Attribute_GetPointer
函数功能：获取APP层属性的指针
输入参数：无
返回值：    APP层属性结构体的指针
备 注：
---------------------------------------------------------------*/
const sPublic_attribute *APP_Attribute_GetPointer()
{
    return &Public_attribute;
}

/*--------------------------------3-----------------------------
函数名称：APP_Postflag_GetPointer
函数功能：获取主动写信息标志属性的指针
输入参数：无
返回值：    主动写信息标志的指针
备 注：
---------------------------------------------------------------*/
const sPost_wflash *APP_Postflag_GetPointer()
{
    return &sPost_flag;
}

/*--------------------------------4-----------------------------
函数名称：APP_Attribute_SetProductSuccessFlag
函数功能：设置公共属性写产品信息成功标志
输入参数：newState-新状态
返回值：   无
备 注：
---------------------------------------------------------------*/
void APP_Attribute_SetProductSuccessFlag(u16 newState)
{
    sPost_flag.wproductsecflag = newState;
}

/*--------------------------------4-----------------------------
函数名称：APP_Attribute_SetAESuccessFlag
函数功能：设置公共属性写AE信息成功标志
输入参数：newState-新状态
返回值：   无
备 注：
---------------------------------------------------------------*/
void APP_Attribute_SetAESuccessFlag(u16 newState)
{
    sPost_flag.wAEsecflag = newState;
}

/*--------------------------------4-----------------------------
函数名称：APP_Attribute_SetWidgetSuccessFlag
函数功能：设置公共属性写Widget信息成功标志
输入参数：newState-新状态
返回值：   无
备 注：
---------------------------------------------------------------*/
void APP_Attribute_SetWidgetSuccessFlag(u16 newState)
{
    sPost_flag.wWidgetsecflag = newState;
}

/*--------------------------------4-----------------------------
函数名称：APP_Attribute_SetProductSuccessFlag
函数功能：设置公共属性使能模组透传功能成功标志
输入参数：newState-新状态
返回值：   无
备 注：
---------------------------------------------------------------*/
void APP_Attribute_SetTranferSuccessFlag(u16 newState)
{
    sPost_flag.wtranferflag = newState;
}

/*--------------------------------47-----------------------------
函数名称：APP_Attribute_SetselfMacAddress
函数功能：设置主节点MAC地址
输入参数：newValue -新类型值
返回值：   无
备 注：
---------------------------------------------------------------*/
void APP_Attribute_SetselfMacAddress(u8 *newValue)
{
    memcpy(Public_attribute.selfMacAddr, newValue, 6);
}

/*--------------------------------47-----------------------------
函数名称：APP_Attribute_SetCCOMacAddress
函数功能：设置主节点MAC地址
输入参数：newValue -新类型值
返回值：   无
备 注：
---------------------------------------------------------------*/
void APP_Attribute_SetCCOMacAddress(u8 *newValue)
{
    memcpy(Public_attribute.ccoMacAddr, newValue, 6);
}

/*--------------------------------27-----------------------------
函数名称：APP_Attribute_SetPanID
函数功能：设置APP层属性的PanID
输入参数：newValue-新数值
返回值：   无
备 注：
---------------------------------------------------------------*/
void APP_Attribute_SetPanID(u16 newValue)
{
    Public_attribute.did = newValue;
}

bool APP_SavePostFlag(void)
{
    APP_PRINTF("APP_SavePostFlag\n");
    fmc_state_enum ret;
    ret = app_flash_program(FLASH_WRITE_POSTWRIET_FLAG, (uint32_t *)&sPost_flag, sizeof(sPost_wflash), true);
    if (ret != FMC_READY)
        return ret;

    ret = app_flash_program(FLASH_WRITE_POSTWRIET_FLAG + sizeof(sPost_wflash), (uint32_t *)&Public_attribute, sizeof(sPublic_attribute), true);
    if (ret != FMC_READY)
        return ret;
    return ret;
}

bool APP_ReadPostFlag(void)
{
    fmc_state_enum ret;
    ret = app_flash_read(FLASH_WRITE_POSTWRIET_FLAG, (uint32_t *)&sPost_flag, sizeof(sPost_wflash));
    if (ret != FMC_READY) {
        return false;
    }
    ret = app_flash_program(FLASH_WRITE_POSTWRIET_FLAG + sizeof(sPost_wflash), (uint32_t *)&Public_attribute, sizeof(sPublic_attribute), true);
    if (ret != FMC_READY)
        return false;
    return true;
}

void APP_Tx_ProcessOfACKReceived(UappsMessage *uappsMsg)
{
    uint16_t DID = 0;
    if (uappsMsg->hdr.hdrCode == UAPPS_ACK_CHANGED && uappsMsg->token[0] == 0x52) {
        switch (uappsMsg->token[1]) {
        case MSE_Load_Product:
            APP_PRINTF("write product success!\r\n");
            APP_Attribute_SetProductSuccessFlag(1); // 说明写产品信息成功，用户可记录该标志
            break;
        case MSE_SET_Transfer:
            APP_PRINTF("write Tranfer success!\r\n");
            APP_Attribute_SetTranferSuccessFlag(1); // 说明使能透传成功，用户可记录该标志
            break;
        case MSE_Load_AE:
            APP_PRINTF("write AE success!\r\n");
            APP_Attribute_SetAESuccessFlag(1);
            break;
        case MSE_Load_Widgets:
            APP_PRINTF("write Widgets success!\r\n");
            APP_Attribute_SetWidgetSuccessFlag(1); // 说明写Widgets信息成功，用户可记录该标志
            break;
        default:
            break;
        }
        APP_SavePostFlag();
    }
    if (uappsMsg->hdr.hdrCode == UAPPS_ACK_CONTENT && uappsMsg->token[0] == 0x52) {
        switch (uappsMsg->token[1]) {
        case MSE_GET_selfMAC:
            APP_PRINTF("selfMacAddr:%02x%02x%02x%02x%02x%02x\r\n", uappsMsg->pl_ptr[0], uappsMsg->pl_ptr[1], uappsMsg->pl_ptr[2], uappsMsg->pl_ptr[3], uappsMsg->pl_ptr[4], uappsMsg->pl_ptr[5]);

            APP_Attribute_SetselfMacAddress(uappsMsg->pl_ptr); /*记录plc模组的mac地址*/
            break;
        case MSE_GET_DID:
            memcpy(&DID, uappsMsg->pl_ptr, 2 * sizeof(u8));
            APP_Attribute_SetPanID(DID); /*记录plc模组的did*/
            // APP_PRINTF("get DID success:%04x!\r\n", DID);
            if (DID != 0) {
                MCU_deviceOnorOutline(1);
            } else {
                MCU_deviceOnorOutline(0);
            }
            break;
        case MSE_GET_CC0MAC:
            // APP_PRINTF("CCOMacAddr:%02x%02x%02x%02x%02x%02x\r\n", uappsMsg->pl_ptr[0], uappsMsg->pl_ptr[1], uappsMsg->pl_ptr[2],
            //            uappsMsg->pl_ptr[3], uappsMsg->pl_ptr[4], uappsMsg->pl_ptr[5]);
            APP_Attribute_SetCCOMacAddress(uappsMsg->pl_ptr); /*记录plc模组入网的网关mac地址，后续状态上送需要用到*/
            break;
        default:
            break;
        }
    }
}
void APP_ReadRecordFlag_Init(void)
{
    APP_ReadPostFlag();
    if (sPost_flag.wAEsecflag == 0xFF)
        sPost_flag.wAEsecflag = 0;
    if (sPost_flag.wWidgetsecflag == 0xFF)
        sPost_flag.wWidgetsecflag = 0;
    if (sPost_flag.wproductsecflag == 0xFF)
        sPost_flag.wproductsecflag = 0;
    if (sPost_flag.wtranferflag == 0xFF)
        sPost_flag.wtranferflag = 0;
    // APP_PRINTF("wAEsecflag:%d\n", sPost_flag.wAEsecflag);
    // APP_PRINTF("wproductsecflag:%d\n", sPost_flag.wproductsecflag);
    // APP_PRINTF("wWidgetsecflag:%d\n", sPost_flag.wWidgetsecflag);
    // APP_PRINTF("wtranferflag:%d\n", sPost_flag.wtranferflag);
}
#endif