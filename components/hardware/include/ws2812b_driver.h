#ifndef WS2812B_DRIVER_H
#define WS2812B_DRIVER_H

#include "esp_err.h"
#include "driver/gpio.h"
#include "led_strip.h"

#ifdef __cplusplus
extern "C" {
#endif

// WS2812B配置参数
#define WS2812B_LED_COUNT       1       // LED数量
#define WS2812B_GPIO_PIN        8       // GPIO引脚

// RGB颜色结构体
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} ws2812b_color_t;

// WS2812B驱动配置
typedef struct {
    gpio_num_t gpio_pin;
    uint16_t led_count;
    led_strip_handle_t led_strip;
} ws2812b_config_t;

/**
 * @brief 初始化WS2812B驱动
 * 
 * @param config WS2812B配置结构体指针
 * @return esp_err_t ESP_OK表示成功，其他值表示失败
 */
esp_err_t ws2812b_init(ws2812b_config_t *config);

/**
 * @brief 设置单个LED颜色
 * 
 * @param config WS2812B配置结构体指针
 * @param led_index LED索引（0开始）
 * @param color RGB颜色
 * @return esp_err_t ESP_OK表示成功，其他值表示失败
 */
esp_err_t ws2812b_set_color(ws2812b_config_t *config, uint16_t led_index, ws2812b_color_t color);

/**
 * @brief 设置所有LED颜色
 * 
 * @param config WS2812B配置结构体指针
 * @param color RGB颜色
 * @return esp_err_t ESP_OK表示成功，其他值表示失败
 */
esp_err_t ws2812b_set_all_color(ws2812b_config_t *config, ws2812b_color_t color);

/**
 * @brief 清除所有LED（设置为黑色）
 * 
 * @param config WS2812B配置结构体指针
 * @return esp_err_t ESP_OK表示成功，其他值表示失败
 */
esp_err_t ws2812b_clear(ws2812b_config_t *config);

/**
 * @brief 刷新LED显示
 * 
 * @param config WS2812B配置结构体指针
 * @return esp_err_t ESP_OK表示成功，其他值表示失败
 */
esp_err_t ws2812b_refresh(ws2812b_config_t *config);

/**
 * @brief 反初始化WS2812B驱动
 * 
 * @param config WS2812B配置结构体指针
 * @return esp_err_t ESP_OK表示成功，其他值表示失败
 */
esp_err_t ws2812b_deinit(ws2812b_config_t *config);

#ifdef __cplusplus
}
#endif

#endif // WS2812B_DRIVER_H