#ifndef NETWORK_H
#define NETWORK_H

#include "esp_err.h"
#include "esp_wifi.h"

// 网络连接状态
typedef enum {
    NETWORK_STATE_DISCONNECTED,
    NETWORK_STATE_CONNECTING,
    NETWORK_STATE_CONNECTED,
    NETWORK_STATE_ERROR
} network_state_t;

// 网络事件类型
typedef enum {
    NETWORK_EVENT_CONNECTED,
    NETWORK_EVENT_DISCONNECTED,
    NETWORK_EVENT_GOT_IP,
    NETWORK_EVENT_LOST_IP
} network_event_type_t;

// 网络事件回调函数类型
typedef void (*network_event_callback_t)(network_event_type_t event_type);

// 网络系统初始化
esp_err_t network_init(void);

// 网络系统去初始化
esp_err_t network_deinit(void);

// WiFi连接
esp_err_t network_wifi_connect(const char *ssid, const char *password);

// WiFi断开连接
esp_err_t network_wifi_disconnect(void);

// 获取网络连接状态
network_state_t network_get_state(void);

// 注册网络事件回调
esp_err_t network_register_callback(network_event_callback_t callback);

// 获取IP地址
esp_err_t network_get_ip_address(char *ip_str, size_t max_len);

#endif // NETWORK_H