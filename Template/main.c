#include "main.h"
#include "gd32e23x.h"
#include "systick.h"
#include <stdio.h>

#include "../Source/base/base.h"
#include "../Source/base/debug.h"
#include "../Source/device/device_manager.h"
#include "../Source/device/jump_device.h"
#include "../Source/eventbus/eventbus.h"
#include "../Source/plcp_device/APP_PublicAttribute.h"
#include "../Source/protocol/protocol.h"
#include "../Source/pwm/pwm.h"
#include "../Source/timer/timer.h"
#include "../Source/usart/usart.h"
#include "../Source/watchdog/watchdog.h"

int main(void)
{
#ifdef __FIRMWARE_VERSION_DEFINE
    uint32_t fw_ver = 0U;
#endif

#ifdef __FIRMWARE_VERSION_DEFINE
    // fw_ver = gd32e23x_firmware_version_get();
    /* print firmware version */
    printf("\r\nGD32E23x series firmware version: V%d.%d.%d", (uint8_t)(fw_ver >> 24U), (uint8_t)(fw_ver >> 16U), (uint8_t)(fw_ver >> 8U));
#endif /* __FIRMWARE_VERSION_DEFINE */

    systick_config();

    delay_1ms(1000);
    app_usart_init(USART1, 115200);
    app_usart_init(USART0, 115200);
    app_eventbus_init();
    app_timer_init();
    app_watchdog_init();
    app_proto_init();

    app_jump_device_init();
    while (1) {
        app_eventbus_poll();
        app_timer_poll();
    }
}
