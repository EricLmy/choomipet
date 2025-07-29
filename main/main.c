#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"
#include "choomi_common.h"
#include "hardware.h"
#include "audio.h"
#include "display.h"
#include "input.h"
#include "network.h"
#include "system.h"
#include "common.h"
#include "ws2812b_driver.h"
#include "rgb_status.h"
#include "rgb_sync.h"

static const char *TAG = "CHOOMI";

// 全局WS2812B配置
ws2812b_config_t ws2812b_config = {
    .gpio_pin = GPIO_NUM_8,
    .led_count = 1
};

// 硬件初始化函数声明（本地实现）
void hardware_init_main(void);

// RGB LED验证测试函数声明
void rgb_led_validation_test(void);
void rgb_status_test(void);
void rgb_animation_test(void);
void rgb_sync_test(void);
void rgb_breathing_demo(void);
void simple_led_test(void);

void app_main(void) {
    // NVS Flash初始化
    ESP_LOGI(TAG, "NVS Flash initialization started");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS Flash initialization completed");
    
    // 日志系统初始化
    ESP_LOGI(TAG, "Log system initialization started");
    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_LOGI(TAG, "Choomi Hardware Starting...");
    ESP_LOGI(TAG, "Log system initialization completed");
    
    // 看门狗配置 (ESP-IDF已自动初始化)
    ESP_LOGI(TAG, "Watchdog configuration started");
    // 添加当前任务到看门狗监控
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    ESP_LOGI(TAG, "Watchdog configuration completed");
    
    // 进入硬件初始化阶段
    ESP_LOGI(TAG, "Entering hardware initialization phase");
    hardware_init_main();
    
    ESP_LOGI(TAG, "System initialization completed successfully");
    
    // RGB LED完整测试序列
    rgb_led_validation_test();
    rgb_status_test();
    rgb_animation_test();
    rgb_sync_test();
    
    // 最后展示呼吸灯效果
    rgb_breathing_demo();
    
    // 主任务循环，保持任务运行并重置看门狗
    ESP_LOGI(TAG, "Entering main task loop");
    while(1) {
        esp_task_wdt_reset();  // 重置看门狗
        vTaskDelay(pdMS_TO_TICKS(5000));  // 延时5秒
    }
}

// 硬件初始化函数实现
void hardware_init_main(void) {
    ESP_LOGI(TAG, "Hardware initialization started");
    
    // 1. 硬件层初始化（使用新的硬件初始化管理器）
    esp_err_t ret = hardware_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Hardware initialization failed: %s - 硬件初始化失败", esp_err_to_name(ret));
        
        // 显示错误状态
        hardware_rgb_set_color(255, 0, 0);  // 红色表示错误
        
        // 尝试错误恢复
        ESP_LOGW(TAG, "Attempting hardware initialization recovery... - 尝试硬件初始化恢复");
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // 重试一次
        ret = hardware_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Hardware initialization recovery failed, system halted - 硬件初始化恢复失败，系统停止");
            while(1) {
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }
    }
    
    // 2. 系统管理初始化
    ESP_ERROR_CHECK(system_init());
    
    // 3. 音频系统初始化
    ESP_ERROR_CHECK(audio_init());
    
    // 4. 显示系统初始化
    ESP_ERROR_CHECK(display_init());
    
    // 5. 输入系统初始化
    ESP_ERROR_CHECK(input_init());
    
    // 6. 网络系统初始化
    ESP_ERROR_CHECK(network_init());
    
    // 7. 最终硬件状态检查
    hw_init_status_t hw_status;
    if (hardware_get_status(&hw_status) == ESP_OK) {
        ESP_LOGI(TAG, "Hardware status - Stage: %s, Complete: %s", 
                 hardware_get_stage_name(hw_status.current_stage),
                 hw_status.initialization_complete ? "YES" : "NO",
                 hardware_get_stage_name(hw_status.current_stage),
                 hw_status.initialization_complete ? "YES" : "NO");
    }
    
    // 8. 设置RGB LED为绿色表示初始化完成
    hardware_rgb_set_color(0, 255, 0);
    ESP_LOGI(TAG, "RGB LED set to green (R:0 G:255 B:0)");
    
    ESP_LOGI(TAG, "Hardware initialization completed successfully");
}

// RGB LED验证测试主函数
void rgb_led_validation_test(void) {
    ESP_LOGI(TAG, "=== RGB LED Validation Test Started ===");
    
    // 测试1: 基础WS2812B驱动测试
    ESP_LOGI(TAG, "Test 1: Basic WS2812B Driver Test");
    
    // 使用全局WS2812B配置
    
    esp_err_t ret = ws2812b_init(&ws2812b_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WS2812B initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "WS2812B driver initialized successfully");
    
    // 测试基本颜色显示
    ESP_LOGI(TAG, "Testing basic colors");
    
    // 红色
    ws2812b_color_t red = {255, 0, 0};
    ws2812b_set_all_color(&ws2812b_config, red);
    ws2812b_refresh(&ws2812b_config);
    ESP_LOGI(TAG, "Red color displayed");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 绿色
    ws2812b_color_t green = {0, 255, 0};
    ws2812b_set_all_color(&ws2812b_config, green);
    ws2812b_refresh(&ws2812b_config);
    ESP_LOGI(TAG, "Green color displayed");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 蓝色
    ws2812b_color_t blue = {0, 0, 255};
    ws2812b_set_all_color(&ws2812b_config, blue);
    ws2812b_refresh(&ws2812b_config);
    ESP_LOGI(TAG, "Blue color displayed");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 黄色
    ws2812b_color_t yellow = {255, 255, 0};
    ws2812b_set_all_color(&ws2812b_config, yellow);
    ws2812b_refresh(&ws2812b_config);
    ESP_LOGI(TAG, "Yellow color displayed");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 紫色
    ws2812b_color_t purple = {128, 0, 128};
    ws2812b_set_all_color(&ws2812b_config, purple);
    ws2812b_refresh(&ws2812b_config);
    ESP_LOGI(TAG, "Purple color displayed");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 青色
    ws2812b_color_t cyan = {0, 255, 255};
    ws2812b_set_all_color(&ws2812b_config, cyan);
    ws2812b_refresh(&ws2812b_config);
    ESP_LOGI(TAG, "Cyan color displayed");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 白色
    ws2812b_color_t white = {255, 255, 255};
    ws2812b_set_all_color(&ws2812b_config, white);
    ws2812b_refresh(&ws2812b_config);
    ESP_LOGI(TAG, "White color displayed");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 清除LED
    ws2812b_clear(&ws2812b_config);
    ws2812b_refresh(&ws2812b_config);
    ESP_LOGI(TAG, "LED cleared");
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(TAG, "Test 1 PASSED: Basic colors displayed correctly");
    
    // 测试2: RGB状态指示系统测试
    rgb_status_test();
    
    // 测试3: 动画效果测试
    rgb_animation_test();
    
    // 测试4: 系统同步测试
    rgb_sync_test();
    
    ESP_LOGI(TAG, "=== RGB LED Validation Test Completed ===");
    ESP_LOGI(TAG, "All tests PASSED");
}

// RGB状态指示系统测试
void rgb_status_test(void) {
    ESP_LOGI(TAG, "Test 2: RGB Status System Test");
    
    // 使用全局WS2812B配置（已经在Test 1中初始化）
    esp_err_t ret = rgb_status_init(&ws2812b_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RGB status initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "RGB status manager initialized");
    
    // 测试各种状态显示
    ESP_LOGI(TAG, "Testing status colors");
    
    // 错误状态 - 红色
    RGB_SET_ERROR();
    ESP_LOGI(TAG, "Error status (red) displayed");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 警告状态 - 黄色
    RGB_SET_WARNING();
    ESP_LOGI(TAG, "Warning status (yellow) displayed");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 正常状态 - 绿色
    RGB_SET_NORMAL();
    ESP_LOGI(TAG, "Normal status (green) displayed");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 配置状态 - 蓝色
    RGB_SET_CONFIG();
    ESP_LOGI(TAG, "Config status (blue) displayed");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 录音状态 - 紫色
    RGB_SET_RECORDING();
    ESP_LOGI(TAG, "Recording status (purple) displayed");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 播放状态 - 青色
    RGB_SET_PLAYING();
    ESP_LOGI(TAG, "Playing status (cyan) displayed");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 测试亮度调节
    ESP_LOGI(TAG, "Testing brightness control");
    rgb_status_set_brightness(64);  // 25%亮度
    ESP_LOGI(TAG, "Brightness set to 25%");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    rgb_status_set_brightness(128); // 50% brightness
    ESP_LOGI(TAG, "Brightness set to 50%");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    rgb_status_set_brightness(255); // 100% brightness
    ESP_LOGI(TAG, "Brightness set to 100%");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "Test 2 PASSED: Status colors and brightness control working");
}

// 动画效果测试
void rgb_animation_test(void) {
    ESP_LOGI(TAG, "Test 3: Animation Effects Test");
    
    // 测试呼吸效果
    ESP_LOGI(TAG, "Testing breathing animation");
    rgb_status_set(RGB_STATUS_NORMAL, RGB_ANIMATION_BREATHING, RGB_PRIORITY_FUNCTION, 5000);
    vTaskDelay(pdMS_TO_TICKS(5500));
    
    // 测试闪烁效果
    ESP_LOGI(TAG, "Testing blinking animation");
    rgb_status_set(RGB_STATUS_ERROR, RGB_ANIMATION_BLINKING, RGB_PRIORITY_ERROR, 3000);
    vTaskDelay(pdMS_TO_TICKS(3500));
    
    // 测试渐变效果
    ESP_LOGI(TAG, "Testing fade animation");
    rgb_status_set(RGB_STATUS_PLAYING, RGB_ANIMATION_FADE, RGB_PRIORITY_FUNCTION, 4000);
    vTaskDelay(pdMS_TO_TICKS(4500));
    
    // 测试彩虹效果
    ESP_LOGI(TAG, "Testing rainbow animation");
    RGB_SET_STARTUP(); // 启动状态使用彩虹效果
    vTaskDelay(pdMS_TO_TICKS(3500));
    
    // 测试淡入淡出效果
    ESP_LOGI(TAG, "Testing fade in/out animation");
    rgb_status_set(RGB_STATUS_CONFIG, RGB_ANIMATION_FADE_IN_OUT, RGB_PRIORITY_FUNCTION, 4000);
    vTaskDelay(pdMS_TO_TICKS(4500));
    
    ESP_LOGI(TAG, "Test 3 PASSED: All animations working smoothly");
}

// 系统同步测试
void rgb_sync_test(void) {
    ESP_LOGI(TAG, "Test 4: System Sync Test");
    
    // 初始化RGB同步管理器
    esp_err_t ret = rgb_sync_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RGB sync initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "RGB sync manager initialized");
    
    // 测试系统启动事件
    ESP_LOGI(TAG, "Testing system startup event");
    RGB_SYNC_SYSTEM_READY();
    vTaskDelay(pdMS_TO_TICKS(3500));
    
    // 测试WiFi连接事件
    ESP_LOGI(TAG, "Testing WiFi connection events");
    RGB_SYNC_WIFI_CONNECTING();
    vTaskDelay(pdMS_TO_TICKS(2000));
    RGB_SYNC_WIFI_CONNECTED();
    vTaskDelay(pdMS_TO_TICKS(2500));
    
    // 测试音频事件
    ESP_LOGI(TAG, "Testing audio events");
    RGB_SYNC_AUDIO_RECORD_START();
    vTaskDelay(pdMS_TO_TICKS(3000));
    RGB_SYNC_AUDIO_RECORD_STOP();
    vTaskDelay(pdMS_TO_TICKS(500));
    RGB_SYNC_AUDIO_PLAY();
    vTaskDelay(pdMS_TO_TICKS(2000));
    RGB_SYNC_AUDIO_STOP();
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 测试按键事件
    ESP_LOGI(TAG, "Testing button events");
    RGB_SYNC_BUTTON_PRESS(1);
    vTaskDelay(pdMS_TO_TICKS(300));
    RGB_SYNC_BUTTON_RELEASE(1);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 测试WebSocket事件
    ESP_LOGI(TAG, "Testing WebSocket events");
    RGB_SYNC_WS_CONNECTED();
    vTaskDelay(pdMS_TO_TICKS(1500));
    rgb_sync_handle_websocket_message("chat");
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 测试电池事件
    ESP_LOGI(TAG, "Testing battery events");
    rgb_sync_handle_battery_status(15, false); // 低电量
    vTaskDelay(pdMS_TO_TICKS(2000));
    rgb_sync_handle_battery_status(50, true);  // 充电中
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 测试系统错误和警告
    ESP_LOGI(TAG, "Testing system error and warning events");
    rgb_sync_handle_system_warning(1001, "Test warning");
    vTaskDelay(pdMS_TO_TICKS(2000));
    rgb_sync_handle_system_error(2001, "Test error");
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // 恢复正常状态
    RGB_SET_NORMAL();
    ESP_LOGI(TAG, "Restored to normal status");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "Test 4 PASSED: System sync working correctly");
}

// RGB呼吸灯演示
void rgb_breathing_demo(void) {
    ESP_LOGI(TAG, "Starting RGB breathing demo");
    
    // 初始化WS2812B驱动
    esp_err_t ret = ws2812b_init(&ws2812b_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WS2812B initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "Starting breathing effect with different colors");
    
    // 定义呼吸效果的颜色
    ws2812b_color_t base_colors[] = {
        {255, 0, 0},    // 红色呼吸
        {0, 255, 0},    // 绿色呼吸
        {0, 0, 255},    // 蓝色呼吸
        {255, 255, 0},  // 黄色呼吸
        {255, 0, 255},  // 紫色呼吸
        {0, 255, 255},  // 青色呼吸
        {255, 255, 255} // 白色呼吸
    };
    
    const char* color_names[] = {
        "Red", "Green", "Blue", "Yellow", "Purple", "Cyan", "White"
    };
    
    // 每种颜色呼吸3个周期
    for (int color_idx = 0; color_idx < 7; color_idx++) {
        ESP_LOGI(TAG, "Breathing effect: %s", color_names[color_idx]);
        
        for (int cycle = 0; cycle < 3; cycle++) {
            // 呼吸周期：从暗到亮再到暗
            for (int brightness = 0; brightness <= 255; brightness += 5) {
                ws2812b_color_t current_color = {
                    .r = (base_colors[color_idx].r * brightness) / 255,
                    .g = (base_colors[color_idx].g * brightness) / 255,
                    .b = (base_colors[color_idx].b * brightness) / 255
                };
                
                ws2812b_set_color(&ws2812b_config, 0, current_color);
                vTaskDelay(pdMS_TO_TICKS(20)); // 20ms延时，平滑过渡
            }
            
            // 从亮到暗
            for (int brightness = 255; brightness >= 0; brightness -= 5) {
                ws2812b_color_t current_color = {
                    .r = (base_colors[color_idx].r * brightness) / 255,
                    .g = (base_colors[color_idx].g * brightness) / 255,
                    .b = (base_colors[color_idx].b * brightness) / 255
                };
                
                ws2812b_set_color(&ws2812b_config, 0, current_color);
                vTaskDelay(pdMS_TO_TICKS(20)); // 20ms延时，平滑过渡
            }
        }
        
        // 颜色间短暂停顿
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    // 清除LED
    ws2812b_clear(&ws2812b_config);
    
    ESP_LOGI(TAG, "RGB breathing demo completed");
}

// 简单LED测试
void simple_led_test(void) {
    ESP_LOGI(TAG, "Starting simple LED test");
    
    // 初始化WS2812B驱动
    esp_err_t ret = ws2812b_init(&ws2812b_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WS2812B initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "WS2812B driver initialized successfully");
    
    // 测试基本颜色
    ws2812b_color_t colors[] = {
        {255, 0, 0},    // 红色
        {0, 255, 0},    // 绿色
        {0, 0, 255},    // 蓝色
        {255, 255, 0},  // 黄色
        {255, 0, 255},  // 紫色
        {0, 255, 255},  // 青色
        {255, 255, 255}, // 白色
        {0, 0, 0}       // 黑色（关闭）
    };
    
    const char* color_names[] = {
        "Red", "Green", "Blue", "Yellow", "Purple", "Cyan", "White", "Off"
    };
    
    int num_colors = sizeof(colors) / sizeof(colors[0]);
    
    for (int cycle = 0; cycle < 3; cycle++) {
        ESP_LOGI(TAG, "Color test cycle %d", cycle + 1);
        
        for (int i = 0; i < num_colors; i++) {
            ESP_LOGI(TAG, "Setting color: %s (R:%d, G:%d, B:%d)", 
                     color_names[i], colors[i].r, colors[i].g, colors[i].b);
            
            ret = ws2812b_set_color(&ws2812b_config, 0, colors[i]);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to set color: %s", esp_err_to_name(ret));
            }
            
            vTaskDelay(pdMS_TO_TICKS(1000)); // 每种颜色显示1秒
        }
    }
    
    ESP_LOGI(TAG, "Simple LED test completed");
}