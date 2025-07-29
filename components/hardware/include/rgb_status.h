#ifndef RGB_STATUS_H
#define RGB_STATUS_H

#include "ws2812b_driver.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#ifdef __cplusplus
extern "C" {
#endif

// 设备状态枚举
typedef enum {
    RGB_STATUS_ERROR = 0,       // 错误状态 - 红色
    RGB_STATUS_WARNING,         // 警告状态 - 黄色
    RGB_STATUS_NORMAL,          // 正常状态 - 绿色
    RGB_STATUS_CONFIG,          // 配置状态 - 蓝色
    RGB_STATUS_RECORDING,       // 录音状态 - 紫色
    RGB_STATUS_PLAYING,         // 播放状态 - 青色
    RGB_STATUS_STARTUP,         // 启动状态 - 白色
    RGB_STATUS_OFF,             // 关闭状态 - 黑色
    RGB_STATUS_MAX
} rgb_status_t;

// 动画效果枚举
typedef enum {
    RGB_ANIMATION_NONE = 0,     // 无动画，静态显示
    RGB_ANIMATION_BREATHING,    // 呼吸效果
    RGB_ANIMATION_BLINKING,     // 闪烁效果
    RGB_ANIMATION_FADE,         // 渐变效果
    RGB_ANIMATION_RAINBOW,      // 彩虹效果
    RGB_ANIMATION_FADE_IN_OUT,  // 淡入淡出效果
    RGB_ANIMATION_MAX
} rgb_animation_t;

// 状态优先级枚举（数值越大优先级越高）
typedef enum {
    RGB_PRIORITY_NORMAL = 0,    // 正常优先级
    RGB_PRIORITY_FUNCTION = 1,  // 功能状态优先级
    RGB_PRIORITY_WARNING = 2,   // 警告优先级
    RGB_PRIORITY_ERROR = 3      // 错误优先级（最高）
} rgb_priority_t;

// RGB状态配置结构体
typedef struct {
    rgb_status_t status;        // 当前状态
    rgb_animation_t animation;  // 动画效果
    rgb_priority_t priority;    // 优先级
    uint8_t brightness;         // 亮度 (0-255)
    uint32_t duration_ms;       // 动画持续时间 (0表示永久)
    bool auto_brightness;       // 自动亮度调节
} rgb_status_config_t;

// RGB状态管理器结构体
typedef struct {
    ws2812b_config_t *ws2812b_config;
    TimerHandle_t animation_timer;
    rgb_status_config_t current_config;
    rgb_status_config_t pending_config;
    bool is_initialized;
    bool is_animation_running;
    uint32_t animation_frame;
    uint8_t global_brightness;
} rgb_status_manager_t;

/**
 * @brief 初始化RGB状态管理器
 * 
 * @param ws2812b_config WS2812B配置指针
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_status_init(ws2812b_config_t *ws2812b_config);

/**
 * @brief 设置RGB状态
 * 
 * @param status 设备状态
 * @param animation 动画效果
 * @param priority 优先级
 * @param duration_ms 持续时间（0表示永久）
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_status_set(rgb_status_t status, rgb_animation_t animation, rgb_priority_t priority, uint32_t duration_ms);

/**
 * @brief 设置全局亮度
 * 
 * @param brightness 亮度值 (0-255)
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_status_set_brightness(uint8_t brightness);

/**
 * @brief 启用/禁用自动亮度调节
 * 
 * @param enable true启用，false禁用
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_status_set_auto_brightness(bool enable);

/**
 * @brief 获取当前状态
 * 
 * @return rgb_status_t 当前状态
 */
rgb_status_t rgb_status_get_current(void);

/**
 * @brief 清除当前状态（设置为关闭）
 * 
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_status_clear(void);

/**
 * @brief 强制刷新显示
 * 
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_status_refresh(void);

/**
 * @brief 反初始化RGB状态管理器
 * 
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t rgb_status_deinit(void);

// 预定义的状态设置宏
#define RGB_SET_ERROR()         rgb_status_set(RGB_STATUS_ERROR, RGB_ANIMATION_BLINKING, RGB_PRIORITY_ERROR, 0)
#define RGB_SET_WARNING()       rgb_status_set(RGB_STATUS_WARNING, RGB_ANIMATION_BREATHING, RGB_PRIORITY_WARNING, 0)
#define RGB_SET_NORMAL()        rgb_status_set(RGB_STATUS_NORMAL, RGB_ANIMATION_NONE, RGB_PRIORITY_NORMAL, 0)
#define RGB_SET_CONFIG()        rgb_status_set(RGB_STATUS_CONFIG, RGB_ANIMATION_BREATHING, RGB_PRIORITY_FUNCTION, 0)
#define RGB_SET_RECORDING()     rgb_status_set(RGB_STATUS_RECORDING, RGB_ANIMATION_BREATHING, RGB_PRIORITY_FUNCTION, 0)
#define RGB_SET_PLAYING()       rgb_status_set(RGB_STATUS_PLAYING, RGB_ANIMATION_FADE, RGB_PRIORITY_FUNCTION, 0)
#define RGB_SET_STARTUP()       rgb_status_set(RGB_STATUS_STARTUP, RGB_ANIMATION_RAINBOW, RGB_PRIORITY_FUNCTION, 3000)

#ifdef __cplusplus
}
#endif

#endif // RGB_STATUS_H