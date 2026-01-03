#ifndef _PLCP_CALLBACK_H_
#define _PLCP_CALLBACK_H_

#include <stdint.h>
char *PLCP_Callback_Product(void);
char *PLCP_Callback_Ae(void);
char *PLCP_Callback_Widgets(void);
uint8_t PlcSdkCallbackOnOff(char *aei, uint8_t OnOff);

#endif