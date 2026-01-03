#ifndef _PLCP_PANEL_API_H_
#define _PLCP_PANEL_API_H_

#include <stdint.h>
void switch_api_button_event_handler(uint8_t id, uint8_t event);
void switch_api_init(void);
#endif