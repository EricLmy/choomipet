#include "rgb_sync.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>

static const char *TAG = "RGB_SYNC";

// 全局同步管理器
static rgb_sync_manager_t g_sync_manager = {0};

// 事件队列
static QueueHandle_t g_event_queue = NULL;
#define EVENT_QUEUE_SIZE 20

// 默认事件处理函数声明
static void default_wifi_handler(rgb_event_data_t *event_data);
static void default_audio_handler(rgb_event_data_t *event_data);
static void default_button_handler(rgb_event_data_t *event_data);
static void default_websocket_handler(rgb_event_data_t *event_data);
static void default_battery_handler(rgb_event_data_t *event_data);
static void default_system_handler(rgb_event_data_t *event_data);

// 同步任务函数
static void rgb_sync_task(void *pvParameters);

// 默认WiFi事件处理
static void default_wifi_handler(rgb_event_data_t *event_data) {
    switch (event_data->type) {
        case RGB_EVENT_WIFI_CONNECTING:
            ESP_LOGD(TAG, "WiFi connecting - showing config status");
            rgb_status_set(RGB_STATUS_CONFIG, RGB_ANIMATION_BREATHING, RGB_PRIORITY_FUNCTION, 0);
            break;
            
        case RGB_EVENT_WIFI_CONNECTED:
            ESP_LOGD(TAG, "WiFi connected - showing normal status");
            rgb_status_set(RGB_STATUS_NORMAL, RGB_ANIMATION_FADE_IN_OUT, RGB_PRIORITY_FUNCTION, 2000);
            break;
            
        case RGB_EVENT_WIFI_DISCONNECTED:
            ESP_LOGD(TAG, "WiFi disconnected - showing warning status");
            rgb_status_set(RGB_STATUS_WARNING, RGB_ANIMATION_BLINKING, RGB_PRIORITY_WARNING, 0);
            break;
            
        default:
            break;
    }
}

// 默认音频事件处理
static void default_audio_handler(rgb_event_data_t *event_data) {
    switch (event_data->type) {
        case RGB_EVENT_AUDIO_PLAYING:
            ESP_LOGD(TAG, "Audio playing - showing playing status");
            rgb_status_set(RGB_STATUS_PLAYING, RGB_ANIMATION_FADE, RGB_PRIORITY_FUNCTION, 0);
            break;
            
        case RGB_EVENT_AUDIO_RECORDING:
            ESP_LOGD(TAG, "Audio recording - showing recording status");
            rgb_status_set(RGB_STATUS_RECORDING, RGB_ANIMATION_BREATHING, RGB_PRIORITY_FUNCTION, 0);
            break;
            
        case RGB_EVENT_AUDIO_STOPPED:
            ESP_LOGD(TAG, "Audio stopped - returning to normal status");
            rgb_status_set(RGB_STATUS_NORMAL, RGB_ANIMATION_NONE, RGB_PRIORITY_NORMAL, 0);
            break;
            
        default:
            break;
    }
}

// 默认按键事件处理
static void default_button_handler(rgb_event_data_t *event_data) {
    switch (event_data->type) {
        case RGB_EVENT_BUTTON_PRESSED:
            ESP_LOGD(TAG, "Button pressed - showing feedback");
            // 短暂的白色闪烁作为按键反馈
            rgb_status_set(RGB_STATUS_STARTUP, RGB_ANIMATION_NONE, RGB_PRIORITY_FUNCTION, 
                          g_sync_manager.feedback_duration_ms);
            break;
            
        case RGB_EVENT_BUTTON_RELEASED:
            ESP_LOGD(TAG, "Button released");
            // 按键释放通常不需要特殊指示
            break;
            
        default:
            break;
    }
}

// 默认WebSocket事件处理
static void default_websocket_handler(rgb_event_data_t *event_data) {
    switch (event_data->type) {
        case RGB_EVENT_WEBSOCKET_CONNECTED:
            ESP_LOGD(TAG, "WebSocket connected - showing brief indication");
            rgb_status_set(RGB_STATUS_CONFIG, RGB_ANIMATION_FADE_IN_OUT, RGB_PRIORITY_FUNCTION, 1000);
            break;
            
        case RGB_EVENT_WEBSOCKET_DISCONNECTED:
            ESP_LOGD(TAG, "WebSocket disconnected - showing warning");
            rgb_status_set(RGB_STATUS_WARNING, RGB_ANIMATION_BLINKING, RGB_PRIORITY_WARNING, 3000);
            break;
            
        case RGB_EVENT_WEBSOCKET_MESSAGE:
            ESP_LOGD(TAG, "WebSocket message - brief flash");
            // 短暂的青色闪烁表示消息活动
            rgb_status_set(RGB_STATUS_PLAYING, RGB_ANIMATION_NONE, RGB_PRIORITY_FUNCTION, 200);
            break;
            
        default:
            break;
    }
}

// 默认电池事件处理
static void default_battery_handler(rgb_event_data_t *event_data) {
    switch (event_data->type) {
        case RGB_EVENT_BATTERY_LOW:
            ESP_LOGD(TAG, "Battery low - showing warning");
            rgb_status_set(RGB_STATUS_WARNING, RGB_ANIMATION_BLINKING, RGB_PRIORITY_WARNING, 0);
            break;
            
        case RGB_EVENT_BATTERY_CHARGING:
            ESP_LOGD(TAG, "Battery charging - showing indication");
            rgb_status_set(RGB_STATUS_CONFIG, RGB_ANIMATION_BREATHING, RGB_PRIORITY_FUNCTION, 0);
            break;
            
        default:
            break;
    }
}

// 默认系统事件处理
static void default_system_handler(rgb_event_data_t *event_data) {
    switch (event_data->type) {
        case RGB_EVENT_SYSTEM_ERROR:
            ESP_LOGD(TAG, "System error - showing error status");
            rgb_status_set(RGB_STATUS_ERROR, RGB_ANIMATION_BLINKING, RGB_PRIORITY_ERROR, 0);
            break;
            
        case RGB_EVENT_SYSTEM_WARNING:
            ESP_LOGD(TAG, "System warning - showing warning status");
            rgb_status_set(RGB_STATUS_WARNING, RGB_ANIMATION_BREATHING, RGB_PRIORITY_WARNING, 0);
            break;
            
        case RGB_EVENT_SYSTEM_STARTUP:
            ESP_LOGD(TAG, "System startup - showing startup animation");
            rgb_status_set(RGB_STATUS_STARTUP, RGB_ANIMATION_RAINBOW, RGB_PRIORITY_FUNCTION, 3000);
            break;
            
        case RGB_EVENT_SYSTEM_SHUTDOWN:
            ESP_LOGD(TAG, "System shutdown - turning off LED");
            rgb_status_set(RGB_STATUS_OFF, RGB_ANIMATION_FADE_IN_OUT, RGB_PRIORITY_FUNCTION, 2000);
            break;
            
        default:
            break;
    }
}

// 同步任务
static void rgb_sync_task(void *pvParameters) {
    rgb_event_data_t event_data;
    
    ESP_LOGI(TAG, "RGB sync task started");
    
    while (1) {
        // 从队列接收事件
        if (xQueueReceive(g_event_queue, &event_data, portMAX_DELAY) == pdTRUE) {
            if (!g_sync_manager.auto_sync_enabled) {
                continue; // 自动同步已禁用
            }
            
            // 调用对应的事件处理器
            if (event_data.type < RGB_EVENT_MAX && 
                g_sync_manager.event_handlers[event_data.type]) {
                g_sync_manager.event_handlers[event_data.type](&event_data);
            }
            
            // 释放事件数据内存（如果有）
            if (event_data.data) {
                free(event_data.data);
            }
        }
    }
}

esp_err_t rgb_sync_init(void) {
    if (g_sync_manager.is_initialized) {
        ESP_LOGW(TAG, "RGB sync manager already initialized");
        return ESP_OK;
    }
    
    // 初始化管理器
    memset(&g_sync_manager, 0, sizeof(rgb_sync_manager_t));
    g_sync_manager.auto_sync_enabled = true;
    g_sync_manager.feedback_brightness = 255;
    g_sync_manager.feedback_duration_ms = 300;
    
    // 注册默认事件处理器
    g_sync_manager.event_handlers[RGB_EVENT_WIFI_CONNECTING] = default_wifi_handler;
    g_sync_manager.event_handlers[RGB_EVENT_WIFI_CONNECTED] = default_wifi_handler;
    g_sync_manager.event_handlers[RGB_EVENT_WIFI_DISCONNECTED] = default_wifi_handler;
    g_sync_manager.event_handlers[RGB_EVENT_AUDIO_PLAYING] = default_audio_handler;
    g_sync_manager.event_handlers[RGB_EVENT_AUDIO_RECORDING] = default_audio_handler;
    g_sync_manager.event_handlers[RGB_EVENT_AUDIO_STOPPED] = default_audio_handler;
    g_sync_manager.event_handlers[RGB_EVENT_BUTTON_PRESSED] = default_button_handler;
    g_sync_manager.event_handlers[RGB_EVENT_BUTTON_RELEASED] = default_button_handler;
    g_sync_manager.event_handlers[RGB_EVENT_WEBSOCKET_CONNECTED] = default_websocket_handler;
    g_sync_manager.event_handlers[RGB_EVENT_WEBSOCKET_DISCONNECTED] = default_websocket_handler;
    g_sync_manager.event_handlers[RGB_EVENT_WEBSOCKET_MESSAGE] = default_websocket_handler;
    g_sync_manager.event_handlers[RGB_EVENT_BATTERY_LOW] = default_battery_handler;
    g_sync_manager.event_handlers[RGB_EVENT_BATTERY_CHARGING] = default_battery_handler;
    g_sync_manager.event_handlers[RGB_EVENT_SYSTEM_ERROR] = default_system_handler;
    g_sync_manager.event_handlers[RGB_EVENT_SYSTEM_WARNING] = default_system_handler;
    g_sync_manager.event_handlers[RGB_EVENT_SYSTEM_STARTUP] = default_system_handler;
    g_sync_manager.event_handlers[RGB_EVENT_SYSTEM_SHUTDOWN] = default_system_handler;
    
    // 创建事件队列
    g_event_queue = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(rgb_event_data_t));
    if (!g_event_queue) {
        ESP_LOGE(TAG, "Failed to create event queue");
        return ESP_ERR_NO_MEM;
    }
    
    // 创建同步任务
    BaseType_t ret = xTaskCreate(
        rgb_sync_task,
        "rgb_sync",
        4096,
        NULL,
        5, // 中等优先级
        &g_sync_manager.sync_task
    );
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create sync task");
        vQueueDelete(g_event_queue);
        return ESP_ERR_NO_MEM;
    }
    
    g_sync_manager.is_initialized = true;
    ESP_LOGI(TAG, "RGB sync manager initialized");
    
    return ESP_OK;
}

esp_err_t rgb_sync_set_auto_sync(bool enable) {
    if (!g_sync_manager.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    g_sync_manager.auto_sync_enabled = enable;
    ESP_LOGI(TAG, "Auto sync %s", enable ? "enabled" : "disabled");
    
    return ESP_OK;
}

esp_err_t rgb_sync_set_feedback_params(uint8_t brightness, uint32_t duration_ms) {
    if (!g_sync_manager.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    g_sync_manager.feedback_brightness = brightness;
    g_sync_manager.feedback_duration_ms = duration_ms;
    
    ESP_LOGD(TAG, "Feedback params set: brightness=%d, duration=%lu ms", brightness, duration_ms);
    return ESP_OK;
}

esp_err_t rgb_sync_register_event_handler(rgb_event_type_t event_type, rgb_event_handler_t handler) {
    if (!g_sync_manager.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (event_type >= RGB_EVENT_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    
    g_sync_manager.event_handlers[event_type] = handler;
    ESP_LOGD(TAG, "Event handler registered for type %d", event_type);
    
    return ESP_OK;
}

esp_err_t rgb_sync_send_event(rgb_event_type_t event_type, void *data, size_t data_len) {
    if (!g_sync_manager.is_initialized || !g_event_queue) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (event_type >= RGB_EVENT_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    
    rgb_event_data_t event_data = {
        .type = event_type,
        .data = NULL,
        .data_len = data_len,
        .timestamp = esp_timer_get_time() / 1000 // 转换为毫秒
    };
    
    // 复制数据（如果有）
    if (data && data_len > 0) {
        event_data.data = malloc(data_len);
        if (!event_data.data) {
            ESP_LOGE(TAG, "Failed to allocate memory for event data");
            return ESP_ERR_NO_MEM;
        }
        memcpy(event_data.data, data, data_len);
    }
    
    // 发送到队列
    if (xQueueSend(g_event_queue, &event_data, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to send event to queue, queue might be full");
        if (event_data.data) {
            free(event_data.data);
        }
        return ESP_ERR_TIMEOUT;
    }
    
    return ESP_OK;
}

esp_err_t rgb_sync_handle_wifi_status(bool connected) {
    rgb_event_type_t event_type = connected ? RGB_EVENT_WIFI_CONNECTED : RGB_EVENT_WIFI_DISCONNECTED;
    return rgb_sync_send_event(event_type, NULL, 0);
}

esp_err_t rgb_sync_handle_audio_status(bool is_playing, bool is_recording) {
    rgb_event_type_t event_type;
    
    if (is_recording) {
        event_type = RGB_EVENT_AUDIO_RECORDING;
    } else if (is_playing) {
        event_type = RGB_EVENT_AUDIO_PLAYING;
    } else {
        event_type = RGB_EVENT_AUDIO_STOPPED;
    }
    
    return rgb_sync_send_event(event_type, NULL, 0);
}

esp_err_t rgb_sync_handle_button_event(uint8_t button_id, bool pressed) {
    rgb_event_type_t event_type = pressed ? RGB_EVENT_BUTTON_PRESSED : RGB_EVENT_BUTTON_RELEASED;
    return rgb_sync_send_event(event_type, &button_id, sizeof(button_id));
}

esp_err_t rgb_sync_handle_websocket_status(bool connected) {
    rgb_event_type_t event_type = connected ? RGB_EVENT_WEBSOCKET_CONNECTED : RGB_EVENT_WEBSOCKET_DISCONNECTED;
    return rgb_sync_send_event(event_type, NULL, 0);
}

esp_err_t rgb_sync_handle_websocket_message(const char *message_type) {
    size_t len = message_type ? strlen(message_type) + 1 : 0;
    return rgb_sync_send_event(RGB_EVENT_WEBSOCKET_MESSAGE, (void*)message_type, len);
}

esp_err_t rgb_sync_handle_battery_status(uint8_t battery_level, bool is_charging) {
    rgb_event_type_t event_type;
    
    if (is_charging) {
        event_type = RGB_EVENT_BATTERY_CHARGING;
    } else if (battery_level < 20) { // 电量低于20%
        event_type = RGB_EVENT_BATTERY_LOW;
    } else {
        return ESP_OK; // 正常电量不需要特殊指示
    }
    
    return rgb_sync_send_event(event_type, &battery_level, sizeof(battery_level));
}

esp_err_t rgb_sync_handle_system_error(uint32_t error_code, const char *error_msg) {
    return rgb_sync_send_event(RGB_EVENT_SYSTEM_ERROR, &error_code, sizeof(error_code));
}

esp_err_t rgb_sync_handle_system_warning(uint32_t warning_code, const char *warning_msg) {
    return rgb_sync_send_event(RGB_EVENT_SYSTEM_WARNING, &warning_code, sizeof(warning_code));
}

esp_err_t rgb_sync_handle_system_startup(void) {
    return rgb_sync_send_event(RGB_EVENT_SYSTEM_STARTUP, NULL, 0);
}

esp_err_t rgb_sync_handle_system_shutdown(void) {
    return rgb_sync_send_event(RGB_EVENT_SYSTEM_SHUTDOWN, NULL, 0);
}

bool rgb_sync_is_auto_sync_enabled(void) {
    return g_sync_manager.is_initialized && g_sync_manager.auto_sync_enabled;
}

esp_err_t rgb_sync_deinit(void) {
    if (!g_sync_manager.is_initialized) {
        return ESP_OK;
    }
    
    // 停止同步任务
    if (g_sync_manager.sync_task) {
        vTaskDelete(g_sync_manager.sync_task);
    }
    
    // 删除事件队列
    if (g_event_queue) {
        vQueueDelete(g_event_queue);
        g_event_queue = NULL;
    }
    
    // 重置管理器
    memset(&g_sync_manager, 0, sizeof(rgb_sync_manager_t));
    
    ESP_LOGI(TAG, "RGB sync manager deinitialized");
    return ESP_OK;
}