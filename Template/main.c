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

static void app_board_bootstrap(void)
{
    systick_config();

    delay_1ms(1000);
    app_usart_init(USART1, 115200);
    app_usart_init(USART0, 115200);
}

static void app_core_services_init(void)
{
    app_eventbus_init();
    app_timer_init();
    app_watchdog_init();
    app_proto_init();
}

static void app_device_init(void)
{
    app_jump_device_init();
}

static void app_main_loop(void)
{
    while (1) {
        app_eventbus_poll();
        app_timer_poll();
    }
}

int main(void)
{
    app_board_bootstrap();
    app_core_services_init();
    app_device_init();
    app_main_loop();
}
