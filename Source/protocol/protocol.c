#include "protocol.h"
#include "../../Source/plcp_common/Inc/APP_RxBuffer.h"
#include "../../Source/plcp_device/APP_PublicAttribute.h"
#include "../../Source/plcp_device/MseProcess.h"
#include "../../Source/timer/timer.h"
#include "../base/base.h"
#include "../base/debug.h"
#include "../config/config.h"
#include "../device/device_manager.h"
#include "../eventbus/eventbus.h"
#include "../flash/flash.h"
#include "../usart/usart.h"
#include "gd32e23x.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 函数声明
static void app_proto_check(usart0_rx_buf_t *buf);
static void app_proto_test(usart1_rx_buf_t *buf);

void app_proto_init(void)
{
    app_usart0_rx_callback(app_proto_check);
    // app_usart1_rx_callback(app_proto_test);
}

// usart1 接收到的数据首先回调在这里处理
static void app_proto_test(usart1_rx_buf_t *buf)
{
    app_usart_tx_buf(buf->buffer, buf->length, USART1);
}

// usart0 接收到数据首先回调在这里处理
static void app_proto_check(usart0_rx_buf_t *buf)
{
    MCU_UartReceive(buf->buffer, buf->length);
}
