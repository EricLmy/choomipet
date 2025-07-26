#ifndef GPIO_CONFIG_H
#define GPIO_CONFIG_H

#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2S音频引脚定义
#define I2S_BCLK_PIN    4
#define I2S_WS_PIN      5
#define I2S_DIN_PIN     18
#define I2S_DOUT_PIN    19

// SPI显示引脚定义
#define SPI_CLK_PIN     7
#define SPI_MOSI_PIN    6
#define LCD_CS_PIN      14
#define LCD_DC_PIN      15
#define LCD_RST_PIN     21
#define LCD_BL_PIN      22

// 按键输入引脚定义
#define BTN_VOL_UP_PIN  1
#define BTN_VOL_DOWN_PIN 2
#define BTN_FUNC_PIN    3

// RGB LED引脚定义
#define RGB_LED_PIN     8

// ADC引脚定义
#define BAT_ADC_PIN     9

// GPIO配置状态枚举
typedef enum {
    GPIO_CONFIG_SUCCESS = 0,
    GPIO_CONFIG_FAIL_I2S,
    GPIO_CONFIG_FAIL_SPI,
    GPIO_CONFIG_FAIL_BUTTON,
    GPIO_CONFIG_FAIL_RGB,
    GPIO_CONFIG_FAIL_ADC
} gpio_config_status_t;

// GPIO引脚测试结果结构体
typedef struct {
    bool i2s_pins_ok;
    bool spi_pins_ok;
    bool button_pins_ok;
    bool rgb_pin_ok;
    bool adc_pin_ok;
} gpio_test_result_t;

/**
 * @brief 配置所有GPIO引脚
 * 
 * @return esp_err_t ESP_OK表示成功，其他值表示失败
 */
esp_err_t gpio_config_all(void);

/**
 * @brief 测试所有GPIO引脚状态
 * 
 * @param result 测试结果结构体指针
 * @return esp_err_t ESP_OK表示成功，其他值表示失败
 */
esp_err_t gpio_test_all_pins(gpio_test_result_t *result);

/**
 * @brief 读取指定GPIO引脚状态
 * 
 * @param pin GPIO引脚号
 * @return int 引脚电平状态（0或1），-1表示错误
 */
int gpio_read_pin_state(gpio_num_t pin);

/**
 * @brief 设置指定GPIO引脚输出电平
 * 
 * @param pin GPIO引脚号
 * @param level 输出电平（0或1）
 * @return esp_err_t ESP_OK表示成功，其他值表示失败
 */
esp_err_t gpio_set_pin_level(gpio_num_t pin, uint32_t level);

/**
 * @brief 验证GPIO配置是否正确
 * 
 * @return esp_err_t ESP_OK表示配置正确，其他值表示配置错误
 */
esp_err_t gpio_verify_config(void);

#ifdef __cplusplus
}
#endif

#endif // GPIO_CONFIG_H