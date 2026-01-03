#include "plcp_panel_callbacks.h"
#include "../../Source/device/device_manager.h"
#include "../../Source/plcp_device/plcp_panel/attr_table.h"
#include <string.h>

// uint8_t PlcSdkCallbackOnOff(char *aei, uint8_t OnOff)
// {
// #if defined PLCP_PANEL
//     uint8_t id;
//     if (NULL != strstr(aei, "ch_")) {
//         id = aei[3] - '0' - 1;
//         if (id >= KEY_NUMBER) {
//             return 0;
//         }
//         return attr_relay_table_set(id, OnOff);
//     } else if (NULL != strstr(aei, "led_")) {
//         id = aei[4] - '0' - 1;
//         if (id >= KEY_NUMBER) {
//             return 0;
//         }
//         return attr_led_table_set(id, OnOff);
//     } else {
//         return 0;
//     }
// #else
//     return 0;
// #endif
// }

uint8_t PlcSdkCallbackAeiToIndex(char *aei)
{
#if defined PLCP_PANEL
    uint8_t index = 0xff;
    if (strlen(aei) == 0) {
        return 0;
    }
    if (NULL != strstr(aei, "ch_")) {
        index = aei[3] - '0' - 1;
        if (index > RELAY_NUMBER) {
            return 0xff;
        }
    } else if (NULL != strstr(aei, "led_")) {
        index = aei[4] - '0' - 1;
        if (index > RELAY_NUMBER) {
            return 0xff;
        }
        index += RELAY_NUMBER;
    }
    return index;

#else
    return 0;
#endif
}