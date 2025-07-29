#include "ws2812b_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "WS2812B";

// LED颜色缓冲区
static ws2812b_color_t *led_buffer = NULL;
static ws2812b_config_t *global_config = NULL;

esp_err_t ws2812b_init(ws2812b_config_t *config)
{
    esp_err_t ret = ESP_OK;
    
    if (!config) {
        ESP_LOGE(TAG, "Config is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    // 分配LED缓冲区
    led_buffer = calloc(config->led_count, sizeof(ws2812b_color_t));
    if (!led_buffer) {
        ESP_LOGE(TAG, "No memory for LED buffer");
        return ESP_ERR_NO_MEM;
    }
    
    // LED strip初始化配置
    led_strip_config_t strip_config = {
        .strip_gpio_num = config->gpio_pin,
        .max_leds = config->led_count,
    };
    
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    
    ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &config->led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Create LED strip failed");
        goto err;
    }
    
    // 清除所有LED
    led_strip_clear(config->led_strip);
    
    global_config = config;
    ESP_LOGI(TAG, "RGB LED driver initialized on GPIO%d with %d LEDs", config->gpio_pin, config->led_count);
    return ESP_OK;
    
err:
    if (led_buffer) {
        free(led_buffer);
        led_buffer = NULL;
    }
    return ret;
}

esp_err_t ws2812b_set_color(ws2812b_config_t *config, uint16_t led_index, ws2812b_color_t color)
{
    if (!config || !led_buffer) {
        ESP_LOGE(TAG, "WS2812B not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (led_index >= config->led_count) {
        ESP_LOGE(TAG, "LED index out of range");
        return ESP_ERR_INVALID_ARG;
    }
    
    led_buffer[led_index] = color;
    
    // 设置LED颜色并刷新
    led_strip_set_pixel(config->led_strip, led_index, color.r, color.g, color.b);
    led_strip_refresh(config->led_strip);
    
    return ESP_OK;
}

esp_err_t ws2812b_set_all_color(ws2812b_config_t *config, ws2812b_color_t color)
{
    if (!config || !led_buffer) {
        ESP_LOGE(TAG, "WS2812B not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    for (uint16_t i = 0; i < config->led_count; i++) {
        led_buffer[i] = color;
        led_strip_set_pixel(config->led_strip, i, color.r, color.g, color.b);
    }
    
    led_strip_refresh(config->led_strip);
    return ESP_OK;
}

esp_err_t ws2812b_clear(ws2812b_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "Config is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    led_strip_clear(config->led_strip);
    
    if (led_buffer) {
        memset(led_buffer, 0, config->led_count * sizeof(ws2812b_color_t));
    }
    
    return ESP_OK;
}

esp_err_t ws2812b_refresh(ws2812b_config_t *config)
{
    if (!config || !led_buffer) {
        ESP_LOGE(TAG, "WS2812B not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    // 更新所有LED颜色
    for (uint16_t i = 0; i < config->led_count; i++) {
        led_strip_set_pixel(config->led_strip, i, led_buffer[i].r, led_buffer[i].g, led_buffer[i].b);
    }
    
    return led_strip_refresh(config->led_strip);
}

esp_err_t ws2812b_deinit(ws2812b_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "Config is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (config->led_strip) {
        led_strip_del(config->led_strip);
        config->led_strip = NULL;
    }
    
    if (led_buffer) {
        free(led_buffer);
        led_buffer = NULL;
    }
    
    global_config = NULL;
    ESP_LOGI(TAG, "WS2812B driver deinitialized");
    return ESP_OK;
}