#ifndef _LIGHT_CT_ADAPTER_H_
#define _LIGHT_CT_ADAPTER_H_
#include <stdint.h>
#include "../../Source/plcp_device/plcp_linght_ct/plcp_light_ct_info.h"

void light_adapter_ctrl(const light_ct_t *ctrl);
void light_adapter_close(uint8_t grad_time);
#endif