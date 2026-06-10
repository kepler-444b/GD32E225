# GD32E225 固件维护说明

这份文档用于帮助后续维护者快速判断代码入口、模块边界和优先整理方向。当前工程实际启用的是 `PLCP_DEVICE` + `PLCP_PANEL` + `PANEL_4KEY`，也就是 4 键 PLC 面板。

## 启动流程

入口文件是 `Template/main.c`。启动流程分为四段：

1. `app_board_bootstrap()`
   初始化 SysTick，等待模块上电稳定，然后初始化 USART1 和 USART0。

2. `app_core_services_init()`
   初始化事件总线、软定时器、看门狗和串口协议入口。

3. `app_device_init()`
   根据 `Source/device/device_manager.h` 中的宏选择设备实现。当前会进入 `plcp_panel_init()`。

4. `app_main_loop()`
   主循环只做两件事：处理事件总线和处理软定时器回调。

## 当前主要模块

- `Source/device/device_manager.h`
  设备类型和按键数量的编译期配置。

- `Source/plcp_device/plcp_panel/`
  4 键面板的硬件和状态逻辑，包括 ADC 按键、继电器、LED、背光、状态表和 Flash 适配。

- `Source/plcp_device/MseProcess.c`
  Uapps/MSE 服务分发层，负责把 `_on`、`_off`、`_state`、`_config`、群组、场景、绑定等请求分发到业务函数。

- `Source/plcp_device/APP_UserProcess.c`
  MCU 侧业务回调实现。这里承接协议层调用，并最终修改面板状态、执行场景、恢复出厂、收发队列。

- `Source/plcp_device/plcp_user_api/PLCP_scene.c`
  场景和群组存储、查询、执行逻辑。

- `Source/plcp_device/plcp_user_api/PLCP_bind.c`
  绑定表的写入、删除、查询和事件触发上报。

- `Source/protocol/protocol.c`
  USART0 接收入口。当前实际路径是把串口数据交给 `MCU_UartReceive()`，再由 PLC SDK 队列处理。

- `Source/timer/`
  裸机软定时器。TIMER14 产生 1ms tick，中断只标记到期，回调在主循环执行。

## 数据流

```text
USART0
 -> app_proto_check()
 -> MCU_UartReceive()
 -> APP_RxBuffer
 -> APP_Queue_ListenAndHandleMessage()
 -> APP_UappsMsgProcessing()
 -> MSE 服务分发
 -> APP_UserProcess / PLCP_scene / PLCP_bind / attr_table
 -> GPIO / PWM / Flash / ACK / 事件上报
```

## 后续重构建议

优先保持行为不变，按下面顺序逐步整理：

1. 把乱码注释逐步修正为可读中文或简短英文。
2. 给 `MseProcess.c` 的服务分发建立表驱动结构，减少大型 `switch`。
3. 把 `APP_UserProcess.c` 中的状态读写、场景执行、恢复出厂拆成更小的私有函数。
4. 给 Flash 读写建立统一 storage 层，避免业务代码直接关心地址和擦写细节。
5. 将 `PLCP_PANEL` 与 `PLCP_LIGHT_CT` 的差异收敛到设备适配层，减少业务文件中的宏分支。
