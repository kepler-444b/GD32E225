
#include "../../Source/plcp_device/plcp_user_api/plcp_sdk_api.h"
#include "../../Source/plcp_device/APP_PublicAttribute.h"

uint8_t plcp_sdk_api_get_cco_mac(uint8_t *ccoMacAddr)
{
    const sPublic_attribute *Public_attribute = NULL;
    Public_attribute                          = APP_Attribute_GetPointer();
    if (Public_attribute == NULL) {
        return 0;
    }
    memcpy(ccoMacAddr, Public_attribute->ccoMacAddr, 6);
    return 1;
}

uint8_t plcp_sdk_api_get_self_mac(uint8_t *selfMacAddr)
{
    const sPublic_attribute *Public_attribute = NULL;
    Public_attribute                          = APP_Attribute_GetPointer();
    if (Public_attribute == NULL) {
        return 0;
    }
    memcpy(selfMacAddr, Public_attribute->selfMacAddr, 6);
    return 1;
}

uint16_t plcp_sdk_api_get_did(void)
{
    const sPublic_attribute *Public_attribute;
    Public_attribute = APP_Attribute_GetPointer();
    return Public_attribute->did;
}

uint8_t plcp_sdk_api_uart_rec_process(uint8_t *recbuf, uint16_t reclen)
{
    return MCU_UartReceive(recbuf, reclen);
}
