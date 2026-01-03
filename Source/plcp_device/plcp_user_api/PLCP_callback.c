#include "PLCP_callback.h"
#include "../../Source/base/debug.h"
#include "../../Source/device/device_manager.h"
#include "../../Source/plcp_device/plcp_linght_ct/light_ct_attr_table.h"
#include "../../Source/plcp_device/plcp_panel/attr_table.h"
#include <string.h>

const char product_str[] =
#if defined PLCP_PANEL
    "{\"product\":{"
    "\"vendor\":{\"name\":\"AODSN\"},"
    "\"ver\":\"1.0\","
#if defined PANEL_6KEY
    "\"name\":\"6key_panel\","
#elif defined PANEL_4KEY
    "\"name\":\"4key_panel\","
#endif
    "\"model\":\"PLCP_Panel\","
    "\"pid\":\"annogytd\","
    "\"license\":\"1234567\""
    "}}";
#elif defined PLCP_LIGHT_CT
    "{\"product\":{"
    "\"vendor\":{\"name\":\"AODSN\"},"
    "\"ver\":\"1.0\","
    "\"name\":\"Light\","
    "\"model\":\"PLCP_light\","
    "\"pid\":\"annogytd\","
    "\"license\":\"1234567\""
    "}}";
#endif

char ae_str[] =
#if defined PLCP_PANEL
    "{\"AE\":["
    "{\"ae\":[\"1\",\"2\",\"3\",\"4\"],"
    "\"name\":\"button\","
    "\"se\":\"31\","
    "\"panel\":{"
    "\"id\":\"PLCP_Panel_switch\","
    "\"url\":\"https://wowoja.cn/oss/panel/lme/PLCP_Panel_switch/v1.0/index.html\""
    "}"
    "},"
    "{\"ae\":[\"ch_1\",\"ch_2\",\"ch_3\",\"ch_4\"],"
    "\"name\":\"relay\","
    "\"se\":\"0\","
    "\"panel\":{"
    "\"id\":\"PLCP_Relay\","
    "\"url\":\"https://wowoja.cn/oss/panel/lme/PLCP_Relay/v1.0/index.html\""
    "},"
    "\"active\":\"1\""
    "}"
    "]}";
#elif defined PLCP_LIGHT_CT
    "{\"AE\":["
    "{"
    "\"name\":\"Light\","
    "\"se\":\"1\","
    "\"panel\":{"
    "\"id\":\"LME_Downlight\","
    "\"url\":\"https://wowoja.cn/oss/panel/lme/LME_Downlight/v1.0/index.html\""
    "},"
    "\"ae\":[\"1\"]"
    "}"
    "]}";
#endif

char widgets_str[] =
#if defined PLCP_PANEL
    "{\"widgets\":["
    "{"
    "\"name\":\"Switch\","
#if defined PANEL_6KEY
    "\"ae\":[\"k1\",\"k2\",\"k3\",\"k4\",\"k5\",\"k6\"],"
#elif defined PANEL_4KEY
    "\"ae\":[\"k1\",\"k2\",\"k3\",\"k4\"],"
#endif
    "\"se\":\"1\","
    "\"type\":\"switch\","
    "\"events\":["
    "{"
    "\"id\":\"_open\","
    "\"name\":\"OFF\","
    "\"data\":\"0\""
    "},"
    "{"
    "\"id\":\"_close\","
    "\"name\":\"ON\","
    "\"data\":\"1\""
    "}"
    "]"
    "}"
    "]}";
#elif defined PLCP_LIGHT_CT
    " ";
#endif

char *PLCP_Callback_Product(void)
{
    return (char *)product_str;
}

char *PLCP_Callback_Ae(void)
{
    return (char *)ae_str;
}

char *PLCP_Callback_Widgets(void)
{
    return (char *)widgets_str;
}

uint8_t PlcSdkCallbackOnOff(char *aei, uint8_t OnOff)
{
#if defined PLCP_PANEL
    uint8_t id;
    if (NULL != strstr(aei, "ch_")) {
        id = aei[3] - '0' - 1;
        if (id >= KEY_NUMBER) {
            return 0;
        }
        return attr_relay_table_set(id, OnOff);
    } else if (NULL != strstr(aei, "led_")) {
        id = aei[4] - '0' - 1;
        if (id >= KEY_NUMBER) {
            return 0;
        }
        return attr_led_table_set(id, OnOff);
    } else {
        return 0;
    }
#elif defined PLCP_LIGHT_CT
    attr_light_ct_table_set(OnOff);

    return 1;
#endif
}