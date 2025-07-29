#include "rgb_status.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <math.h>
#include <string.h>

static const char *TAG = "RGB_STATUS";

// 全局状态管理器
static rgb_status_manager_t g_rgb_manager = {0};

// 状态颜色映射表
static const ws2812b_color_t status_colors[RGB_STATUS_MAX] = {
    {255, 0, 0},    // RGB_STATUS_ERROR - 红色
    {255, 255, 0},  // RGB_STATUS_WARNING - 黄色
    {0, 255, 0},    // RGB_STATUS_NORMAL - 绿色
    {0, 0, 255},    // RGB_STATUS_CONFIG - 蓝色
    {128, 0, 128},  // RGB_STATUS_RECORDING - 紫色
    {0, 255, 255},  // RGB_STATUS_PLAYING - 青色
    {255, 255, 255}, // RGB_STATUS_STARTUP - 白色
    {0, 0, 0}       // RGB_STATUS_OFF - 黑色
};

// 状态优先级映射表
static const rgb_priority_t __attribute__((unused)) status_priorities[RGB_STATUS_MAX] = {
    RGB_PRIORITY_ERROR,     // RGB_STATUS_ERROR
    RGB_PRIORITY_WARNING,   // RGB_STATUS_WARNING
    RGB_PRIORITY_NORMAL,    // RGB_STATUS_NORMAL
    RGB_PRIORITY_FUNCTION,  // RGB_STATUS_CONFIG
    RGB_PRIORITY_FUNCTION,  // RGB_STATUS_RECORDING
    RGB_PRIORITY_FUNCTION,  // RGB_STATUS_PLAYING
    RGB_PRIORITY_FUNCTION,  // RGB_STATUS_STARTUP
    RGB_PRIORITY_NORMAL     // RGB_STATUS_OFF
};

// 动画定时器回调函数声明
static void animation_timer_callback(TimerHandle_t xTimer);

// 应用亮度调整
static ws2812b_color_t apply_brightness(ws2812b_color_t color, uint8_t brightness) {
    ws2812b_color_t result;
    result.r = (color.r * brightness) / 255;
    result.g = (color.g * brightness) / 255;
    result.b = (color.b * brightness) / 255;
    return result;
}

// 呼吸效果计算
static uint8_t calculate_breathing_brightness(uint32_t frame) {
    // 使用正弦波实现呼吸效果，周期约2秒（60帧）
    float phase = (frame % 60) * 2.0f * M_PI / 60.0f;
    float brightness_factor = (sin(phase) + 1.0f) / 2.0f; // 0-1范围
    return (uint8_t)(brightness_factor * 255);
}

// 闪烁效果计算
static uint8_t calculate_blinking_brightness(uint32_t frame) {
    // 每15帧切换一次（约0.5秒）
    return ((frame / 15) % 2) ? 255 : 0;
}

// 渐变效果计算
static uint8_t calculate_fade_brightness(uint32_t frame) {
    // 线性渐变，周期约3秒（90帧）
    uint32_t cycle_frame = frame % 180; // 完整周期180帧
    if (cycle_frame < 90) {
        return (cycle_frame * 255) / 90; // 渐亮
    } else {
        return ((180 - cycle_frame) * 255) / 90; // 渐暗
    }
}

// 彩虹效果计算
static ws2812b_color_t calculate_rainbow_color(uint32_t frame) {
    // HSV到RGB转换实现彩虹效果
    float hue = (frame % 360) * 1.0f; // 色相0-360度
    float saturation = 1.0f;
    float value = 1.0f;
    
    float c = value * saturation;
    float x = c * (1 - fabs(fmod(hue / 60.0f, 2) - 1));
    float m = value - c;
    
    float r, g, b;
    if (hue < 60) {
        r = c; g = x; b = 0;
    } else if (hue < 120) {
        r = x; g = c; b = 0;
    } else if (hue < 180) {
        r = 0; g = c; b = x;
    } else if (hue < 240) {
        r = 0; g = x; b = c;
    } else if (hue < 300) {
        r = x; g = 0; b = c;
    } else {
        r = c; g = 0; b = x;
    }
    
    ws2812b_color_t color;
    color.r = (uint8_t)((r + m) * 255);
    color.g = (uint8_t)((g + m) * 255);
    color.b = (uint8_t)((b + m) * 255);
    
    return color;
}

// 淡入淡出效果计算
static uint8_t calculate_fade_in_out_brightness(uint32_t frame) {
    // 淡入淡出效果，周期约4秒（120帧）
    uint32_t cycle_frame = frame % 240; // 完整周期240帧
    if (cycle_frame < 60) {
        return (cycle_frame * 255) / 60; // 淡入
    } else if (cycle_frame < 180) {
        return 255; // 保持亮
    } else {
        return ((240 - cycle_frame) * 255) / 60; // 淡出
    }
}

// 更新LED显示
static esp_err_t update_led_display(void) {
    if (!g_rgb_manager.is_initialized || !g_rgb_manager.ws2812b_config) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ws2812b_color_t base_color = status_colors[g_rgb_manager.current_config.status];
    ws2812b_color_t final_color = base_color;
    uint8_t brightness = g_rgb_manager.current_config.brightness;
    
    // 应用动画效果
    switch (g_rgb_manager.current_config.animation) {
        case RGB_ANIMATION_BREATHING:
            brightness = (brightness * calculate_breathing_brightness(g_rgb_manager.animation_frame)) / 255;
            break;
            
        case RGB_ANIMATION_BLINKING:
            brightness = (brightness * calculate_blinking_brightness(g_rgb_manager.animation_frame)) / 255;
            break;
            
        case RGB_ANIMATION_FADE:
            brightness = (brightness * calculate_fade_brightness(g_rgb_manager.animation_frame)) / 255;
            break;
            
        case RGB_ANIMATION_RAINBOW:
            final_color = calculate_rainbow_color(g_rgb_manager.animation_frame);
            break;
            
        case RGB_ANIMATION_FADE_IN_OUT:
            brightness = (brightness * calculate_fade_in_out_brightness(g_rgb_manager.animation_frame)) / 255;
            break;
            
        case RGB_ANIMATION_NONE:
        default:
            // 静态显示，不改变亮度
            break;
    }
    
    // 应用全局亮度
    brightness = (brightness * g_rgb_manager.global_brightness) / 255;
    final_color = apply_brightness(final_color, brightness);
    
    // 更新LED
    esp_err_t ret = ws2812b_set_all_color(g_rgb_manager.ws2812b_config, final_color);
    if (ret == ESP_OK) {
        ret = ws2812b_refresh(g_rgb_manager.ws2812b_config);
    }
    
    return ret;
}

// 动画定时器回调函数
static void animation_timer_callback(TimerHandle_t xTimer) {
    if (!g_rgb_manager.is_animation_running) {
        return;
    }
    
    g_rgb_manager.animation_frame++;
    
    // 检查动画持续时间
    if (g_rgb_manager.current_config.duration_ms > 0) {
        uint32_t elapsed_ms = g_rgb_manager.animation_frame * 33; // 30fps
        if (elapsed_ms >= g_rgb_manager.current_config.duration_ms) {
            // 动画结束，恢复到正常状态
            rgb_status_set(RGB_STATUS_NORMAL, RGB_ANIMATION_NONE, RGB_PRIORITY_NORMAL, 0);
            return;
        }
    }
    
    update_led_display();
}

esp_err_t rgb_status_init(ws2812b_config_t *ws2812b_config) {
    if (!ws2812b_config) {
        ESP_LOGE(TAG, "WS2812B config is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (g_rgb_manager.is_initialized) {
        ESP_LOGW(TAG, "RGB status manager already initialized");
        return ESP_OK;
    }
    
    // 初始化管理器
    memset(&g_rgb_manager, 0, sizeof(rgb_status_manager_t));
    g_rgb_manager.ws2812b_config = ws2812b_config;
    g_rgb_manager.global_brightness = 128; // 默认50%亮度
    
    // 设置初始状态
    g_rgb_manager.current_config.status = RGB_STATUS_OFF;
    g_rgb_manager.current_config.animation = RGB_ANIMATION_NONE;
    g_rgb_manager.current_config.priority = RGB_PRIORITY_NORMAL;
    g_rgb_manager.current_config.brightness = 255;
    g_rgb_manager.current_config.duration_ms = 0;
    g_rgb_manager.current_config.auto_brightness = false;
    
    // 创建动画定时器（30fps）
    g_rgb_manager.animation_timer = xTimerCreate(
        "rgb_animation",
        pdMS_TO_TICKS(33), // 33ms = 30fps
        pdTRUE,            // 自动重载
        NULL,              // 定时器ID
        animation_timer_callback
    );
    
    if (!g_rgb_manager.animation_timer) {
        ESP_LOGE(TAG, "Failed to create animation timer");
        return ESP_ERR_NO_MEM;
    }
    
    g_rgb_manager.is_initialized = true;
    
    // 清除LED显示
    ws2812b_clear(ws2812b_config);
    ws2812b_refresh(ws2812b_config);
    
    ESP_LOGI(TAG, "RGB status manager initialized");
    return ESP_OK;
}

esp_err_t rgb_status_set(rgb_status_t status, rgb_animation_t animation, rgb_priority_t priority, uint32_t duration_ms) {
    if (!g_rgb_manager.is_initialized) {
        ESP_LOGE(TAG, "RGB status manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (status >= RGB_STATUS_MAX || animation >= RGB_ANIMATION_MAX) {
        ESP_LOGE(TAG, "Invalid status or animation");
        return ESP_ERR_INVALID_ARG;
    }
    
    // 检查优先级，只有更高或相等优先级才能覆盖当前状态
    if (priority < g_rgb_manager.current_config.priority) {
        ESP_LOGD(TAG, "Status priority too low, ignored");
        return ESP_OK;
    }
    
    // 停止当前动画
    if (g_rgb_manager.is_animation_running) {
        xTimerStop(g_rgb_manager.animation_timer, 0);
        g_rgb_manager.is_animation_running = false;
    }
    
    // 更新配置
    g_rgb_manager.current_config.status = status;
    g_rgb_manager.current_config.animation = animation;
    g_rgb_manager.current_config.priority = priority;
    g_rgb_manager.current_config.brightness = 255;
    g_rgb_manager.current_config.duration_ms = duration_ms;
    g_rgb_manager.animation_frame = 0;
    
    // 启动动画（如果需要）
    if (animation != RGB_ANIMATION_NONE) {
        g_rgb_manager.is_animation_running = true;
        xTimerStart(g_rgb_manager.animation_timer, 0);
    }
    
    // 立即更新显示
    esp_err_t ret = update_led_display();
    
    ESP_LOGD(TAG, "Status set to %d with animation %d, priority %d", status, animation, priority);
    return ret;
}

esp_err_t rgb_status_set_brightness(uint8_t brightness) {
    if (!g_rgb_manager.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    g_rgb_manager.global_brightness = brightness;
    return update_led_display();
}

esp_err_t rgb_status_set_auto_brightness(bool enable) {
    if (!g_rgb_manager.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    g_rgb_manager.current_config.auto_brightness = enable;
    
    // TODO: 实现环境光传感器读取和自动亮度调节
    // 这里可以添加光传感器的读取逻辑
    
    ESP_LOGD(TAG, "Auto brightness %s", enable ? "enabled" : "disabled");
    return ESP_OK;
}

rgb_status_t rgb_status_get_current(void) {
    if (!g_rgb_manager.is_initialized) {
        return RGB_STATUS_OFF;
    }
    
    return g_rgb_manager.current_config.status;
}

esp_err_t rgb_status_clear(void) {
    return rgb_status_set(RGB_STATUS_OFF, RGB_ANIMATION_NONE, RGB_PRIORITY_NORMAL, 0);
}

esp_err_t rgb_status_refresh(void) {
    if (!g_rgb_manager.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    return update_led_display();
}

esp_err_t rgb_status_deinit(void) {
    if (!g_rgb_manager.is_initialized) {
        return ESP_OK;
    }
    
    // 停止动画定时器
    if (g_rgb_manager.animation_timer) {
        xTimerStop(g_rgb_manager.animation_timer, 0);
        xTimerDelete(g_rgb_manager.animation_timer, 0);
    }
    
    // 清除LED显示
    if (g_rgb_manager.ws2812b_config) {
        ws2812b_clear(g_rgb_manager.ws2812b_config);
        ws2812b_refresh(g_rgb_manager.ws2812b_config);
    }
    
    // 重置管理器
    memset(&g_rgb_manager, 0, sizeof(rgb_status_manager_t));
    
    ESP_LOGI(TAG, "RGB status manager deinitialized");
    return ESP_OK;
}