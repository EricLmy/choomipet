#ifndef RGB_SYNC_H
#define RGB_SYNC_H

#include "rgb_status.h"
#include "esp_err.h"
#include "esp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

// 系统事件类型枚举
typedef enum {
    RGB_EVENT_WIFI_CONNECTING = 0,  // WiFi连接中
    RGB_EVENT_WIFI_CONNECTED,       // WiFi已连接
    RGB_EVENT_WIFI_DISCONNECTED,    // WiFi断开连接
    RGB_EVENT_AUDIO_PLAYING,        // 音频播放中
    RGB_EVENT_AUDIO_STOPPED,        // 音频停止
    RGB_EVENT_AUDIO_RECORDING,      // 音频录制中
    RGB_EVENT_BUTTON_PRESSED,       // 按键按下
    RGB_EVENT_BUTTON_RELEASED,      // 按键释放
    RGB_EVENT_WEBSOCKET_CONNECTED,  // WebSocket连接
    RGB_EVENT_WEBSOCKET_DISCONNECTED, // WebSocket断开
    RGB_EVENT_WEBSOCKET_MESSAGE,    // WebSocket消息
    RGB_EVENT_BATTERY_LOW,          // 电池电量低
    RGB_EVENT_BATTERY_CHARGING,     // 电池充电中
    RGB_EVENT_SYSTEM_ERROR,         // 系统错误
    RGB_EVENT_SYSTEM_WARNING,       // 系统警告
    RGB_EVENT_SYSTEM_STARTUP,       // 系统启动
    RGB_EVENT_SYSTEM_SHUTDOWN,      // 系统关闭
    RGB_EVENT_MAX
} rgb_event_type_t;

// 事件数据结构体
typedef struct {
    rgb_event_type_t type;
    void *data;
    size_t data_len;
    uint32_t timestamp;
} rgb_event_data_t;

// 事件处理回调函数类型
typedef void (*rgb_event_handler_t)(rgb_event_data_t *event_data);

// RGB同步管理器结构体
typedef struct {
    bool is_initialized;
    rgb_event_handler_t event_handlers[RGB_EVENT_MAX];
    esp_event_loop_handle_t event_loop;
    TaskHandle_t sync_task;
    bool auto_sync_enabled;
    uint8_t feedback_brightness;
    uint32_t feedback_duration_ms;
} rgb_sync_manager_t;

/**
 * @brief 初始化RGB同步管理器
 * 
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_init(void);

/**
 * @brief 启用/禁用自动同步
 * 
 * @param enable true启用，false禁用
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_set_auto_sync(bool enable);

/**
 * @brief 设置用户反馈参数
 * 
 * @param brightness 反馈亮度 (0-255)
 * @param duration_ms 反馈持续时间
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_set_feedback_params(uint8_t brightness, uint32_t duration_ms);

/**
 * @brief 注册事件处理器
 * 
 * @param event_type 事件类型
 * @param handler 事件处理回调函数
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_register_event_handler(rgb_event_type_t event_type, rgb_event_handler_t handler);

/**
 * @brief 发送系统事件
 * 
 * @param event_type 事件类型
 * @param data 事件数据（可选）
 * @param data_len 数据长度
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_send_event(rgb_event_type_t event_type, void *data, size_t data_len);

/**
 * @brief 处理WiFi状态变化
 * 
 * @param connected true表示已连接，false表示断开
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_handle_wifi_status(bool connected);

/**
 * @brief 处理音频状态变化
 * 
 * @param is_playing true表示播放中，false表示停止
 * @param is_recording true表示录制中，false表示停止
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_handle_audio_status(bool is_playing, bool is_recording);

/**
 * @brief 处理按键事件
 * 
 * @param button_id 按键ID
 * @param pressed true表示按下，false表示释放
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_handle_button_event(uint8_t button_id, bool pressed);

/**
 * @brief 处理WebSocket状态变化
 * 
 * @param connected true表示已连接，false表示断开
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_handle_websocket_status(bool connected);

/**
 * @brief 处理WebSocket消息事件
 * 
 * @param message_type 消息类型
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_handle_websocket_message(const char *message_type);

/**
 * @brief 处理电池状态变化
 * 
 * @param battery_level 电池电量 (0-100)
 * @param is_charging true表示充电中
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_handle_battery_status(uint8_t battery_level, bool is_charging);

/**
 * @brief 处理系统错误
 * 
 * @param error_code 错误代码
 * @param error_msg 错误消息（可选）
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_handle_system_error(uint32_t error_code, const char *error_msg);

/**
 * @brief 处理系统警告
 * 
 * @param warning_code 警告代码
 * @param warning_msg 警告消息（可选）
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_handle_system_warning(uint32_t warning_code, const char *warning_msg);

/**
 * @brief 处理系统启动完成
 * 
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_handle_system_startup(void);

/**
 * @brief 处理系统关闭
 * 
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_handle_system_shutdown(void);

/**
 * @brief 获取当前同步状态
 * 
 * @return bool true表示自动同步已启用
 */
bool rgb_sync_is_auto_sync_enabled(void);

/**
 * @brief 反初始化RGB同步管理器
 * 
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_sync_deinit(void);

// 便捷宏定义
#define RGB_SYNC_WIFI_CONNECTING()      rgb_sync_handle_wifi_status(false)
#define RGB_SYNC_WIFI_CONNECTED()       rgb_sync_handle_wifi_status(true)
#define RGB_SYNC_WIFI_DISCONNECTED()    rgb_sync_handle_wifi_status(false)
#define RGB_SYNC_AUDIO_PLAY()           rgb_sync_handle_audio_status(true, false)
#define RGB_SYNC_AUDIO_STOP()           rgb_sync_handle_audio_status(false, false)
#define RGB_SYNC_AUDIO_RECORD_START()   rgb_sync_handle_audio_status(false, true)
#define RGB_SYNC_AUDIO_RECORD_STOP()    rgb_sync_handle_audio_status(false, false)
#define RGB_SYNC_BUTTON_PRESS(id)       rgb_sync_handle_button_event(id, true)
#define RGB_SYNC_BUTTON_RELEASE(id)     rgb_sync_handle_button_event(id, false)
#define RGB_SYNC_WS_CONNECTED()         rgb_sync_handle_websocket_status(true)
#define RGB_SYNC_WS_DISCONNECTED()      rgb_sync_handle_websocket_status(false)
#define RGB_SYNC_SYSTEM_READY()         rgb_sync_handle_system_startup()

#ifdef __cplusplus
}
#endif

#endif // RGB_SYNC_H