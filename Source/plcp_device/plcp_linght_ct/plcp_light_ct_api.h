#ifndef _PLCP_LIGHT_CT_API_H_

#define _PLCP_LIGHT_CT_API_H_
#include <stdio.h>
#include <stdint.h>

void light_api_button_event_handler(uint8_t id, uint8_t event);

void light_api_init(void);

#endif //_PLCP_LIGHT_CT_API_H_