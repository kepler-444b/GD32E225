#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define MAX_SOFT_TIMERS    10 ///< 最大软定时器数量
#define MAX_TIMER_NAME_LEN 16 ///< 定时器名称最大长度

typedef enum {
    TIMER_STATE_INACTIVE = 0, ///< 未启用
    TIMER_STATE_ACTIVE,       ///< 正在计时
    TIMER_STATE_PENDING       ///< 已到期，等待主循环执行回调
} timer_state_e;

typedef enum {
    TIMER_ERR_SUCCESS = 0,   ///< 操作成功
    TIMER_ERR_INVALID_PARAM, ///< 参数无效
    TIMER_ERR_NO_RESOURCE,   ///< 没有空闲定时器
    TIMER_ERR_NOT_FOUND,     ///< 未找到指定定时器
    TIMER_ERR_NAME_TOO_LONG  ///< 定时器名称过长
} timer_error_e;

/**
 * @brief 软定时器回调函数
 * @param arg 用户参数
 */
typedef void (*SoftTimerCallback)(void *arg);

typedef struct
{
    volatile timer_state_e state;  ///< 中断和主循环共享的状态
    volatile uint32_t start_time;  ///< 启动时间，单位 ms
    bool repeat;                   ///< 是否循环触发
    uint32_t interval_ms;          ///< 定时间隔，单位 ms
    SoftTimerCallback callback;    ///< 到期回调
    void *user_arg;                ///< 用户参数
    char name[MAX_TIMER_NAME_LEN]; ///< 定时器名称
} soft_timer_t;

// 初始化软定时器模块
void app_timer_init(void);

/**
 * @brief 启动一个软定时器
 * @param interval_ms 定时间隔，必须大于 0
 * @param callback 到期回调，不能为 NULL
 * @param repeat 是否循环触发
 * @param arg 用户参数
 * @param name 定时器名称，可选，最大长度 MAX_TIMER_NAME_LEN - 1
 * @return TIMER_ERR_SUCCESS 表示成功，否则返回错误码
 */
timer_error_e app_timer_start(uint32_t interval_ms, SoftTimerCallback callback, bool repeat, void *arg, const char *name);

// 通过名称停止定时器
timer_error_e app_timer_stop(const char *name);

// 通过名称检查定时器是否激活
bool app_timer_is_active(const char *name);

// 处理到期回调，在主循环中调用
void app_timer_poll(void);

// 获取系统运行时间，单位 ms
uint32_t app_timer_get_ticks(void);

#endif
