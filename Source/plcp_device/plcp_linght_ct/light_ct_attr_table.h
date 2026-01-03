#ifndef _LIGHT_CT_ATTR_TABLE_H_
#define _LIGHT_CT_ATTR_TABLE_H_
#include <stdint.h>
uint8_t attr_light_ct_table_set(uint8_t onoff);
void light_ct_ctrl(uint8_t *buf, uint8_t len);
#endif