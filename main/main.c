#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"
#include "choomi_common.h"
#include "hardware.h"
#include "audio.h"
#include "display.h"
#include "input.h"
#include "network.h"
#include "system.h"
#include "common.h"

static const char *TAG = "CHOOMI";

// 硬件初始化函数声明（本地实现）
void hardware_init_main(void);

void app_main(void) {
    // NVS Flash初始化
    ESP_LOGI(TAG, "NVS Flash initialization started - NVS Flash初始化开始");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS Flash initialization completed - NVS Flash初始化完成");
    
    // 日志系统初始化
    ESP_LOGI(TAG, "Log system initialization started - 日志系统初始化开始");
    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_LOGI(TAG, "Choomi Hardware Starting... - Choomi硬件启动中...");
    ESP_LOGI(TAG, "Log system initialization completed - 日志系统初始化完成");
    
    // 看门狗配置 (ESP-IDF已自动初始化)
    ESP_LOGI(TAG, "Watchdog configuration started - 看门狗配置开始");
    // 添加当前任务到看门狗监控
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    ESP_LOGI(TAG, "Watchdog configuration completed - 看门狗配置完成");
    
    // 进入硬件初始化阶段
    ESP_LOGI(TAG, "Entering hardware initialization phase - 进入硬件初始化阶段");
    hardware_init_main();
    
    ESP_LOGI(TAG, "System initialization completed successfully - 系统初始化成功完成");
    
    // 主任务循环，保持任务运行并重置看门狗
    ESP_LOGI(TAG, "Entering main task loop - 进入主任务循环");
    while(1) {
        esp_task_wdt_reset();  // 重置看门狗
        hardware_rgb_set_color(255, 0, 0);  // 红色表示错误
        vTaskDelay(pdMS_TO_TICKS(1000));  // 延时1秒
        hardware_rgb_set_color(0, 0, 255);  // 红色表示错误
    }
}

// 硬件初始化函数实现
void hardware_init_main(void) {
    ESP_LOGI(TAG, "Hardware initialization started - 硬件初始化开始");
    
    // 1. 硬件层初始化（使用新的硬件初始化管理器）
    esp_err_t ret = hardware_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Hardware initialization failed: %s - 硬件初始化失败", esp_err_to_name(ret));
        
        // 显示错误状态
        hardware_rgb_set_color(255, 0, 0);  // 红色表示错误
        
        // 尝试错误恢复
        ESP_LOGW(TAG, "Attempting hardware initialization recovery... - 尝试硬件初始化恢复");
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // 重试一次
        ret = hardware_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Hardware initialization recovery failed, system halted - 硬件初始化恢复失败，系统停止");
            while(1) {
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }
    }
    
    // 2. 系统管理初始化
    ESP_ERROR_CHECK(system_init());
    
    // 3. 音频系统初始化
    ESP_ERROR_CHECK(audio_init());
    
    // 4. 显示系统初始化
    ESP_ERROR_CHECK(display_init());
    
    // 5. 输入系统初始化
    ESP_ERROR_CHECK(input_init());
    
    // 6. 网络系统初始化
    ESP_ERROR_CHECK(network_init());
    
    // 7. 最终硬件状态检查
    hw_init_status_t hw_status;
    if (hardware_get_status(&hw_status) == ESP_OK) {
        ESP_LOGI(TAG, "Hardware status - Stage: %s, Complete: %s - 硬件状态检查 - 阶段: %s, 完成: %s", 
                 hardware_get_stage_name(hw_status.current_stage),
                 hw_status.initialization_complete ? "YES" : "NO",
                 hardware_get_stage_name(hw_status.current_stage),
                 hw_status.initialization_complete ? "YES" : "NO");
    }
    
    // 8. 设置RGB LED为绿色表示初始化完成
    hardware_rgb_set_color(0, 255, 0);
    ESP_LOGI(TAG, "RGB LED set to green (R:0 G:255 B:0) - RGB LED设置为绿色");
    
    ESP_LOGI(TAG, "Hardware initialization completed successfully - 硬件初始化成功完成");
}