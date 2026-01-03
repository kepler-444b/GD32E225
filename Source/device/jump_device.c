#include <stdio.h>
#include "device_manager.h"
#include "jump_device.h"
#include "../Source/plcp_device/plcp_panel/plcp_panel.h"
#include "../Source/plcp_device/plcp_linght_ct/plcp_light_ct.h"

void app_jump_device_init(void)
{
#if defined PLCP_PANEL
    plcp_panel_init();
#elif defined PLCP_LIGHT_CT
    plcp_light_ct_init();
#endif
}
