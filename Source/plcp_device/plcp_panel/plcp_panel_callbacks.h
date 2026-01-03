#ifndef _PLCP_PANEL_CALLBACKS_H
#define _PLCP_PANEL_CALLBACKS_H
#include <stdint.h>
uint8_t PlcSdkCallbackOnOff(char *aei, uint8_t OnOff);
uint8_t PlcSdkCallbackAeiToIndex(char *aei);
#endif