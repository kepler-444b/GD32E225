
#ifndef _PLCP_SDK_API_H_
#define _PLCP_SDK_API_H_
#include <stdint.h>

uint8_t plcp_sdk_api_get_cco_mac(uint8_t *ccoMacAddr);
uint16_t plcp_sdk_api_get_did(void);
uint8_t plcp_sdk_api_get_self_mac(uint8_t *selfMacAddr);
uint8_t plcp_sdk_api_uart_rec_process(uint8_t *recbuf, uint16_t reclen);
#endif
