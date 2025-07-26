#ifndef SYSTEM_H
#define SYSTEM_H

#include "esp_err.h"
#include "choomi_common.h"

// 系统管理初始化
esp_err_t system_init(void);

// 系统管理去初始化
esp_err_t system_deinit(void);

// 获取系统状态
choomi_state_t system_get_state(void);

// 设置系统状态
esp_err_t system_set_state(choomi_state_t state);

// 系统重启
esp_err_t system_restart(void);

// 获取系统运行时间
uint32_t system_get_uptime_ms(void);

// 获取空闲内存大小
size_t system_get_free_heap_size(void);

// 系统健康检查
esp_err_t system_health_check(void);

// 心跳任务
void system_heartbeat_task(void *pvParameters);

#endif // SYSTEM_H