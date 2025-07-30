#pragma once

#include "esp_err.h"

/**
 * @brief 启动UI布局演示
 * 
 * 这个函数会初始化UI布局系统并启动一个演示任务，
 * 该任务会循环展示不同的界面状态
 * 
 * @return esp_err_t ESP_OK表示成功，其他值表示错误
 */
esp_err_t ui_layout_demo_start(void);

/**
 * @brief 停止UI布局演示
 * 
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t ui_layout_demo_stop(void);

/**
 * @brief 手动切换到下一个演示步骤
 * 
 * 可以用于调试或手动控制演示进度
 */
void ui_layout_demo_next_step(void);

/**
 * @brief 获取当前演示步骤
 * 
 * @return int 当前步骤编号
 */
int ui_layout_demo_get_current_step(void);