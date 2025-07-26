#include "network.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>

static const char *TAG = "NETWORK";
static network_state_t current_state = NETWORK_STATE_DISCONNECTED;
static network_event_callback_t event_callback = NULL;
static EventGroupHandle_t wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        current_state = NETWORK_STATE_DISCONNECTED;
        if (event_callback) {
            event_callback(NETWORK_EVENT_DISCONNECTED);
        }
        xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        current_state = NETWORK_STATE_CONNECTED;
        if (event_callback) {
            event_callback(NETWORK_EVENT_GOT_IP);
        }
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t network_init(void) {
    ESP_LOGI(TAG, "Network system initializing... - 网络系统初始化中");
    
    // 创建事件组
    wifi_event_group = xEventGroupCreate();
    
    // 初始化网络接口
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    // WiFi初始化
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // 注册事件处理器
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    
    ESP_LOGI(TAG, "Network system initialized - 网络系统初始化完成");
    return ESP_OK;
}

esp_err_t network_deinit(void) {
    ESP_LOGI(TAG, "Network system deinitializing... - 网络系统去初始化中");
    // TODO: 实现网络系统去初始化
    ESP_LOGI(TAG, "Network system deinitialized - 网络系统去初始化完成");
    return ESP_OK;
}

esp_err_t network_wifi_connect(const char *ssid, const char *password) {
    ESP_LOGI(TAG, "Connecting to WiFi: %s - 连接WiFi: %s", ssid, ssid);
    
    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    current_state = NETWORK_STATE_CONNECTING;
    return ESP_OK;
}

esp_err_t network_wifi_disconnect(void) {
    ESP_LOGI(TAG, "Disconnecting from WiFi");
    return esp_wifi_disconnect();
}

network_state_t network_get_state(void) {
    return current_state;
}

esp_err_t network_register_callback(network_event_callback_t callback) {
    if (callback == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    event_callback = callback;
    ESP_LOGI(TAG, "Network event callback registered");
    return ESP_OK;
}

esp_err_t network_get_ip_address(char *ip_str, size_t max_len) {
    // TODO: 实现IP地址获取
    if (ip_str && max_len > 0) {
        strncpy(ip_str, "0.0.0.0", max_len - 1);
        ip_str[max_len - 1] = '\0';
    }
    return ESP_OK;
}