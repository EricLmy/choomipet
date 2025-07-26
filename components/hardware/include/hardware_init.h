#ifndef HARDWARE_INIT_H
#define HARDWARE_INIT_H

#include "esp_err.h"
#include "gpio_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// 硬件初始化阶段枚举
typedef enum {
    HW_INIT_STAGE_START = 0,
    HW_INIT_STAGE_GPIO,
    HW_INIT_STAGE_CLOCK,
    HW_INIT_STAGE_SPI,
    HW_INIT_STAGE_I2S,
    HW_INIT_STAGE_ADC,
    HW_INIT_STAGE_SELF_TEST,
    HW_INIT_STAGE_COMPLETE,
    HW_INIT_STAGE_ERROR
} hw_init_stage_t;

// 硬件初始化错误码
typedef enum {
    HW_INIT_OK = 0,
    HW_INIT_ERR_GPIO,
    HW_INIT_ERR_CLOCK,
    HW_INIT_ERR_SPI,
    HW_INIT_ERR_I2S,
    HW_INIT_ERR_ADC,
    HW_INIT_ERR_SELF_TEST,
    HW_INIT_ERR_UNKNOWN
} hw_init_error_t;

// 硬件自检结果结构体
typedef struct {
    bool lcd_communication_ok;
    bool i2s_clock_ok;
    bool button_response_ok;
    bool battery_voltage_ok;
    bool rgb_led_ok;
} hw_self_test_result_t;

// 硬件初始化状态结构体
typedef struct {
    hw_init_stage_t current_stage;
    hw_init_error_t last_error;
    bool initialization_complete;
    hw_self_test_result_t self_test_result;
} hw_init_status_t;

/**
 * @brief 启动硬件初始化流程
 * 
 * 按照预定顺序初始化所有硬件模块：
 * 1. GPIO配置
 * 2. 时钟配置
 * 3. SPI初始化
 * 4. I2S初始化
 * 5. ADC初始化
 * 6. 硬件自检
 * 
 * @return esp_err_t ESP_OK表示成功，其他值表示失败
 */
esp_err_t hardware_init_start(void);

/**
 * @brief 执行硬件自检
 * 
 * 检测各硬件模块的基本功能：
 * - SPI通信测试（LCD响应）
 * - I2S时钟输出测试
 * - 按键响应测试
 * - 电池电压检测测试
 * - RGB LED测试
 * 
 * @param result 自检结果结构体指针
 * @return esp_err_t ESP_OK表示自检通过，其他值表示自检失败
 */
esp_err_t hardware_self_test(hw_self_test_result_t *result);

/**
 * @brief 获取当前硬件初始化状态
 * 
 * @param status 状态结构体指针
 * @return esp_err_t ESP_OK表示成功，其他值表示失败
 */
esp_err_t hardware_get_init_status(hw_init_status_t *status);

/**
 * @brief 硬件初始化错误恢复
 * 
 * 尝试从硬件初始化错误中恢复
 * 
 * @param error_code 错误码
 * @return esp_err_t ESP_OK表示恢复成功，其他值表示恢复失败
 */
esp_err_t hardware_init_recovery(hw_init_error_t error_code);

/**
 * @brief 测试LCD通信
 * 
 * @return bool true表示通信正常，false表示通信异常
 */
bool lcd_test_communication(void);

/**
 * @brief 测试I2S时钟输出
 * 
 * @return bool true表示时钟输出正常，false表示时钟输出异常
 */
bool i2s_test_clock_output(void);

/**
 * @brief 测试按键响应
 * 
 * @return bool true表示按键响应正常，false表示按键响应异常
 */
bool button_test_response(void);

/**
 * @brief 测试电池电压检测
 * 
 * @return bool true表示电压检测正常，false表示电压检测异常
 */
bool battery_test_voltage_detection(void);

/**
 * @brief 测试RGB LED功能
 * 
 * @return bool true表示RGB LED正常，false表示RGB LED异常
 */
bool rgb_led_test_function(void);

/**
 * @brief 获取硬件初始化阶段名称
 * 
 * @param stage 初始化阶段
 * @return const char* 阶段名称字符串
 */
const char* hardware_get_stage_name(hw_init_stage_t stage);

/**
 * @brief 获取硬件初始化错误名称
 * 
 * @param error 错误码
 * @return const char* 错误名称字符串
 */
const char* hardware_get_error_name(hw_init_error_t error);

#ifdef __cplusplus
}
#endif

#endif // HARDWARE_INIT_H