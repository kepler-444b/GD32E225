
#ifndef _BUTTON_STATE_H_
#define _BUTTON_STATE_H_

// #include "lmexxx_conf.h"
#include "../../Source/plcp_common/Inc/lmexxx_conf.h"


U8 button_state_get(U8 id);
void button_state_set(U8 id, U8 on_off);

#endif

