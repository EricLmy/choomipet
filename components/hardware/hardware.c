#include "hardware.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "gpio_config.h"
#include "hardware_init.h"
#include "ws2812b_driver.h"

static const char *TAG = "HARDWARE";

// WS2812B配置
static ws2812b_config_t ws2812b_config = {
    .gpio_pin = RGB_LED_PIN,
    .led_count = 1,
    .rmt_channel = NULL,
    .rmt_encoder = NULL
};

esp_err_t hardware_init(void) {
    ESP_LOGI(TAG, "Hardware initializing (using new hardware init manager)... - 硬件初始化中（使用新的硬件初始化管理器）");
    
    // 使用新的硬件初始化管理器
    esp_err_t ret = hardware_init_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Hardware initialization failed: %s - 硬件初始化失败", esp_err_to_name(ret));
        return ret;
    }
    
    // 初始化WS2812B驱动
    ret = ws2812b_init(&ws2812b_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WS2812B initialization failed: %s - WS2812B初始化失败", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Hardware initialized successfully - 硬件初始化成功");
    return ESP_OK;
}

esp_err_t hardware_deinit(void) {
    ESP_LOGI(TAG, "Hardware deinitializing... - 硬件去初始化中");
    // TODO: 实现硬件去初始化
    ESP_LOGI(TAG, "Hardware deinitialized - 硬件去初始化完成");
    return ESP_OK;
}

esp_err_t hardware_gpio_init(void) {
    ESP_LOGI(TAG, "GPIO initializing (compatibility function)... - GPIO初始化中（兼容性函数）");
    
    // 使用新的GPIO配置函数
    esp_err_t ret = gpio_config_all();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO configuration failed: %s - GPIO配置失败", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "GPIO initialized successfully - GPIO初始化成功");
    return ESP_OK;
}

esp_err_t hardware_rgb_set_color(uint8_t r, uint8_t g, uint8_t b) {
    // 使用WS2812B驱动设置颜色
    ws2812b_color_t color = {
        .r = r,
        .g = g,
        .b = b
    };
    
    esp_err_t ret = ws2812b_set_all_color(&ws2812b_config, color);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WS2812B color");
        return ret;
    }
    
    ret = ws2812b_refresh(&ws2812b_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to refresh WS2812B");
        return ret;
    }
    
    ESP_LOGI(TAG, "RGB LED set to R:%d G:%d B:%d", r, g, b);
    return ESP_OK;
}

bool hardware_key_read(gpio_num_t key_gpio) {
    // 按键按下时为低电平（因为使用了上拉电阻）
    return !gpio_get_level(key_gpio);
}

esp_err_t hardware_clock_config(void) {
    ESP_LOGI(TAG, "Clock configuration...");
    // TODO: 实现系统时钟配置（CPU频率160MHz、FreeRTOS tick 1000Hz）
    ESP_LOGI(TAG, "Clock configured");
    return ESP_OK;
}

// 新增功能函数
esp_err_t hardware_get_status(hw_init_status_t *status) {
    return hardware_get_init_status(status);
}

esp_err_t hardware_run_self_test(hw_self_test_result_t *result) {
    return hardware_self_test(result);
}