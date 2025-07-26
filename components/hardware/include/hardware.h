#ifndef HARDWARE_H
#define HARDWARE_H

#include "esp_err.h"
#include "driver/gpio.h"
#include "gpio_config.h"
#include "hardware_init.h"

#ifdef __cplusplus
extern "C" {
#endif

// 兼容性定义 - 保持向后兼容
#define GPIO_LED_R          RGB_LED_PIN
#define GPIO_LED_G          RGB_LED_PIN  // 注意：任务2中使用单个WS2812B RGB LED
#define GPIO_LED_B          RGB_LED_PIN
#define GPIO_KEY_RECORD     BTN_FUNC_PIN
#define GPIO_KEY_PLAY       BTN_VOL_UP_PIN
#define GPIO_KEY_MENU       BTN_VOL_DOWN_PIN
#define GPIO_LCD_CS         LCD_CS_PIN
#define GPIO_LCD_DC         LCD_DC_PIN
#define GPIO_LCD_RST        LCD_RST_PIN
#define GPIO_I2S_BCLK       I2S_BCLK_PIN
#define GPIO_I2S_WS         I2S_WS_PIN
#define GPIO_I2S_DIN        I2S_DIN_PIN
#define GPIO_I2S_DOUT       I2S_DOUT_PIN

// 硬件初始化 - 使用新的硬件初始化管理器
esp_err_t hardware_init(void);

// 硬件去初始化
esp_err_t hardware_deinit(void);

// GPIO配置 - 兼容性函数
esp_err_t hardware_gpio_init(void);

// RGB LED控制 - 简化版本（单个WS2812B LED）
esp_err_t hardware_rgb_set_color(uint8_t r, uint8_t g, uint8_t b);

// 按键状态读取
bool hardware_key_read(gpio_num_t key_gpio);

// 系统时钟配置
esp_err_t hardware_clock_config(void);

// 新增功能 - 硬件状态监控
esp_err_t hardware_get_status(hw_init_status_t *status);

// 新增功能 - 硬件自检
esp_err_t hardware_run_self_test(hw_self_test_result_t *result);

#ifdef __cplusplus
}
#endif

#endif // HARDWARE_H