#include "system.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"

static const char *TAG = "SYSTEM";
static choomi_state_t current_state = CHOOMI_STATE_INIT;
static TaskHandle_t heartbeat_task_handle = NULL;

esp_err_t system_init(void) {
    ESP_LOGI(TAG, "System management initializing... - 系统管理初始化中");
    
    // 创建心跳任务
    xTaskCreate(system_heartbeat_task, "heartbeat", 2048, NULL, 5, &heartbeat_task_handle);
    
    // 设置初始状态
    current_state = CHOOMI_STATE_READY;
    
    ESP_LOGI(TAG, "System management initialized - 系统管理初始化完成");
    return ESP_OK;
}

esp_err_t system_deinit(void) {
    ESP_LOGI(TAG, "System management deinitializing... - 系统管理去初始化中");
    
    // 删除心跳任务
    if (heartbeat_task_handle != NULL) {
        vTaskDelete(heartbeat_task_handle);
        heartbeat_task_handle = NULL;
    }
    
    ESP_LOGI(TAG, "System management deinitialized - 系统管理去初始化完成");
    return ESP_OK;
}

choomi_state_t system_get_state(void) {
    return current_state;
}

esp_err_t system_set_state(choomi_state_t state) {
    ESP_LOGI(TAG, "System state changed from %d to %d - 系统状态从 %d 变更为 %d", current_state, state, current_state, state);
    current_state = state;
    return ESP_OK;
}

esp_err_t system_restart(void) {
    ESP_LOGI(TAG, "System restarting... - 系统重启中");
    esp_restart();
    return ESP_OK;
}

uint32_t system_get_uptime_ms(void) {
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

size_t system_get_free_heap_size(void) {
    return esp_get_free_heap_size();
}

esp_err_t system_health_check(void) {
    // 检查内存使用情况
    size_t free_heap = system_get_free_heap_size();
    if (free_heap < 10240) { // 小于10KB时报警
        ESP_LOGW(TAG, "Low memory warning: %d bytes free", free_heap);
    }
    
    // 检查系统运行时间
    uint32_t uptime = system_get_uptime_ms();
    ESP_LOGI(TAG, "System uptime: %lu ms, Free heap: %d bytes", uptime, free_heap);
    
    return ESP_OK;
}

void system_heartbeat_task(void *pvParameters) {
    ESP_LOGI(TAG, "Heartbeat task started");
    
    while (1) {
        // 执行系统健康检查
        system_health_check();
        
        // 等待心跳间隔
        vTaskDelay(pdMS_TO_TICKS(HEARTBEAT_INTERVAL_MS));
    }
}