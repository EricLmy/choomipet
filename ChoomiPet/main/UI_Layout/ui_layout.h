#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lvgl.h"

// 屏幕尺寸定义
#define SCREEN_WIDTH            172
#define SCREEN_HEIGHT           320

// 区域尺寸定义
#define STATUS_BAR_HEIGHT       30
#define PET_DISPLAY_HEIGHT      200
#define INFO_DISPLAY_HEIGHT     90

// 区域坐标定义
#define STATUS_BAR_Y            0
#define PET_DISPLAY_Y           STATUS_BAR_HEIGHT
#define INFO_DISPLAY_Y          (STATUS_BAR_HEIGHT + PET_DISPLAY_HEIGHT)

// 边距和间距
#define MARGIN_HORIZONTAL       8
#define MARGIN_VERTICAL         4
#define SPACING_SMALL           4
#define SPACING_MEDIUM          8
#define SPACING_LARGE           12

// 颜色定义 (RGB565格式)
#define COLOR_PRIMARY           0x4A90    // 主色调蓝色
#define COLOR_SUCCESS           0x07E0    // 成功绿色
#define COLOR_WARNING           0xFD20    // 警告橙色
#define COLOR_ERROR             0xF800    // 错误红色
#define COLOR_WHITE             0xFFFF    // 白色
#define COLOR_BLACK             0x0000    // 黑色
#define COLOR_GRAY_LIGHT        0xC618    // 浅灰色
#define COLOR_GRAY_DARK         0x4208    // 深灰色

// 字体大小定义
#define FONT_SIZE_SMALL         12
#define FONT_SIZE_MEDIUM        14
#define FONT_SIZE_LARGE         16
#define FONT_SIZE_TIME          14

// 图标尺寸定义
#define ICON_SIZE_STATUS        16
#define ICON_SIZE_FUNCTION      24
#define ICON_SIZE_EMOJI         20

// 布局区域枚举
typedef enum {
    LAYOUT_REGION_STATUS_BAR = 0,
    LAYOUT_REGION_PET_DISPLAY,
    LAYOUT_REGION_INFO_DISPLAY,
    LAYOUT_REGION_MAX
} layout_region_t;

// 区域配置结构
typedef struct {
    uint16_t x, y;          // 区域起始坐标
    uint16_t width, height; // 区域尺寸
    bool dirty;             // 脏标记，需要重绘
    bool visible;           // 可见性
} layout_region_config_t;

// WiFi信号强度等级
typedef enum {
    WIFI_SIGNAL_NONE = 0,
    WIFI_SIGNAL_WEAK,
    WIFI_SIGNAL_MEDIUM,
    WIFI_SIGNAL_STRONG
} wifi_signal_level_t;

// 电池状态
typedef enum {
    BATTERY_LEVEL_CRITICAL = 0, // <15%
    BATTERY_LEVEL_LOW,          // 15-30%
    BATTERY_LEVEL_MEDIUM,       // 30-70%
    BATTERY_LEVEL_HIGH          // >70%
} battery_level_t;

// 状态栏数据结构
typedef struct {
    wifi_signal_level_t wifi_signal;
    battery_level_t battery_level;
    bool charging;
    char time_str[16];          // "HH:MM" 格式
    uint32_t rgb_led_color;     // RGB LED状态颜色
    bool rgb_led_active;
} status_bar_data_t;

// 宠物表情类型
typedef enum {
    PET_EMOTION_IDLE = 0,
    PET_EMOTION_HAPPY,
    PET_EMOTION_SAD,
    PET_EMOTION_ANGRY,
    PET_EMOTION_SURPRISED,
    PET_EMOTION_THINKING,
    PET_EMOTION_LISTENING,
    PET_EMOTION_SPEAKING,
    PET_EMOTION_SLEEPING,
    PET_EMOTION_CONFUSED
} pet_emotion_t;

// 信息显示类型
typedef enum {
    INFO_TYPE_STATUS_TEXT = 0,
    INFO_TYPE_OPERATION_GUIDE,
    INFO_TYPE_VOLUME_INDICATOR,
    INFO_TYPE_PROGRESS_BAR,
    INFO_TYPE_ERROR_MESSAGE
} info_display_type_t;

// 音量指示器配置
typedef struct {
    uint8_t volume_level;       // 0-100
    bool show_percentage;
    uint32_t display_duration_ms;
} volume_indicator_config_t;

// 进度条配置
typedef struct {
    uint8_t progress;           // 0-100
    char label[32];
    uint16_t color;
} progress_bar_config_t;

// 信息显示数据
typedef struct {
    info_display_type_t type;
    union {
        char text[64];                          // 状态文字或操作指引
        volume_indicator_config_t volume;       // 音量指示器
        progress_bar_config_t progress;         // 进度条
    } data;
    uint32_t display_duration_ms;   // 显示持续时间，0表示持续显示
} info_display_data_t;

// 设备状态枚举
typedef enum {
    DEVICE_STATE_BOOTING = 0,
    DEVICE_STATE_WIFI_DISCONNECTED,
    DEVICE_STATE_WIFI_CONFIGURING,
    DEVICE_STATE_WIFI_CONNECTING,
    DEVICE_STATE_SERVER_CONNECTING,
    DEVICE_STATE_ONLINE_READY,
    DEVICE_STATE_LISTENING,
    DEVICE_STATE_PROCESSING,
    DEVICE_STATE_SPEAKING,
    DEVICE_STATE_ERROR,
    DEVICE_STATE_SLEEPING,
    DEVICE_STATE_LOW_BATTERY
} device_state_t;

// 界面状态数据
typedef struct {
    device_state_t current_state;
    device_state_t previous_state;
    uint32_t state_enter_time;
    
    // 状态栏数据
    status_bar_data_t status_bar;
    
    // 宠物显示数据
    pet_emotion_t pet_emotion;
    bool pet_animation_playing;
    
    // 信息显示数据
    info_display_data_t info_display;
    
    // 动画状态
    bool transition_active;
    uint32_t transition_duration;
} ui_state_t;

// 布局管理器接口
esp_err_t ui_layout_init(void);
esp_err_t ui_layout_deinit(void);
esp_err_t ui_layout_set_region_dirty(layout_region_t region);
esp_err_t ui_layout_update_display(void);
esp_err_t ui_layout_get_region_config(layout_region_t region, layout_region_config_t *config);

// 状态栏组件接口
esp_err_t status_bar_init(void);
esp_err_t status_bar_update_data(const status_bar_data_t *data);
esp_err_t status_bar_set_time(const char *time_str);
esp_err_t status_bar_set_wifi_signal(wifi_signal_level_t level);
esp_err_t status_bar_set_battery(battery_level_t level, bool charging);
esp_err_t status_bar_set_rgb_status(uint32_t color, bool active);

// 宠物显示组件接口
esp_err_t pet_display_init(void);
esp_err_t pet_display_set_emotion(pet_emotion_t emotion);
esp_err_t pet_display_update(uint32_t delta_time_ms);
pet_emotion_t pet_display_get_current_emotion(void);
bool pet_display_is_animation_playing(void);

// 信息显示组件接口
esp_err_t info_display_init(void);
esp_err_t info_display_show_text(const char *line1, const char *line2);
esp_err_t info_display_show_volume(uint8_t volume, bool show_percentage);
esp_err_t info_display_show_progress(const char *label, uint8_t progress);
esp_err_t info_display_show_error(const char *error_msg);
esp_err_t info_display_update(uint32_t delta_time_ms);
esp_err_t info_display_clear(void);

// 高级接口
esp_err_t ui_layout_set_device_state(device_state_t state);
esp_err_t ui_layout_update_status_bar(const status_bar_data_t *data);
esp_err_t ui_layout_show_boot_screen(void);
esp_err_t ui_layout_show_wifi_config_screen(void);
esp_err_t ui_layout_show_listening_screen(void);
esp_err_t ui_layout_show_thinking_screen(void);
esp_err_t ui_layout_show_speaking_screen(void);
esp_err_t ui_layout_show_error_screen(const char *error_msg);
esp_err_t ui_layout_show_volume_screen(uint8_t volume);