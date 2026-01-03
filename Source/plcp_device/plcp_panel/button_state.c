 
#include "../../Source/plcp_common/Inc/lmexxx_conf.h"

static U8 button_state[8] = {0};

U8 button_state_get(U8 id)
{
	return button_state[id];
}

void button_state_set(U8 id, U8 on_off)
{
	button_state[id] = on_off;
}
