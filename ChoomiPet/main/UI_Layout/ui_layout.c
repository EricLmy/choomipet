#include "ui_layout.h"

static const char *TAG = "UI_LAYOUT";

// LVGL对象句柄
static lv_obj_t *main_screen = NULL;
static lv_obj_t *status_bar_container = NULL;
static lv_obj_t *pet_display_container = NULL;
static lv_obj_t *info_display_container = NULL;

// 状态栏组件
static lv_obj_t *wifi_icon = NULL;
static lv_obj_t *battery_icon = NULL;
static lv_obj_t *time_label = NULL;
static lv_obj_t *rgb_status_dot = NULL;

// 宠物显示组件
static lv_obj_t *pet_face = NULL;
static lv_obj_t *pet_eyes_left = NULL;
static lv_obj_t *pet_eyes_right = NULL;
static lv_obj_t *pet_mouth = NULL;

// 信息显示组件
static lv_obj_t *info_line1 = NULL;
static lv_obj_t *info_line2 = NULL;
static lv_obj_t *volume_bar = NULL;
static lv_obj_t *volume_label = NULL;

// 全局状态
static ui_state_t g_ui_state = {0};
static layout_region_config_t g_regions[LAYOUT_REGION_MAX];
static SemaphoreHandle_t g_ui_mutex = NULL;

// 内部函数声明
static esp_err_t create_main_layout(void);
static esp_err_t create_status_bar(void);
static esp_err_t create_pet_display(void);
static esp_err_t create_info_display(void);
static void update_pet_face(pet_emotion_t emotion);
static void update_wifi_icon(wifi_signal_level_t level);
static void update_battery_icon(battery_level_t level, bool charging);
static lv_color_t convert_rgb565_to_lv_color(uint16_t rgb565);

esp_err_t ui_layout_init(void)
{
    ESP_LOGI(TAG, "Initializing UI Layout");
    
    // 创建互斥锁
    g_ui_mutex = xSemaphoreCreateMutex();
    if (g_ui_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create UI mutex");
        return ESP_ERR_NO_MEM;
    }
    
    // 初始化区域配置
    g_regions[LAYOUT_REGION_STATUS_BAR] = (layout_region_config_t){
        .x = 0, .y = STATUS_BAR_Y,
        .width = SCREEN_WIDTH, .height = STATUS_BAR_HEIGHT,
        .dirty = true, .visible = true
    };
    
    g_regions[LAYOUT_REGION_PET_DISPLAY] = (layout_region_config_t){
        .x = 0, .y = PET_DISPLAY_Y,
        .width = SCREEN_WIDTH, .height = PET_DISPLAY_HEIGHT,
        .dirty = true, .visible = true
    };
    
    g_regions[LAYOUT_REGION_INFO_DISPLAY] = (layout_region_config_t){
        .x = 0, .y = INFO_DISPLAY_Y,
        .width = SCREEN_WIDTH, .height = INFO_DISPLAY_HEIGHT,
        .dirty = true, .visible = true
    };
    
    // 初始化UI状态
    g_ui_state.current_state = DEVICE_STATE_BOOTING;
    g_ui_state.pet_emotion = PET_EMOTION_IDLE;
    strcpy(g_ui_state.status_bar.time_str, "12:00");
    g_ui_state.status_bar.wifi_signal = WIFI_SIGNAL_NONE;
    g_ui_state.status_bar.battery_level = BATTERY_LEVEL_MEDIUM;
    
    // 创建主界面布局
    esp_err_t ret = create_main_layout();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create main layout");
        return ret;
    }
    
    // 初始化各个组件
    ret = status_bar_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize status bar");
        return ret;
    }
    
    ret = pet_display_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize pet display");
        return ret;
    }
    
    ret = info_display_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize info display");
        return ret;
    }
    
    ESP_LOGI(TAG, "UI Layout initialized successfully");
    return ESP_OK;
}

esp_err_t ui_layout_deinit(void)
{
    if (g_ui_mutex) {
        vSemaphoreDelete(g_ui_mutex);
        g_ui_mutex = NULL;
    }
    
    if (main_screen) {
        lv_obj_del(main_screen);
        main_screen = NULL;
    }
    
    return ESP_OK;
}

static esp_err_t create_main_layout(void)
{
    // 创建主屏幕
    main_screen = lv_obj_create(NULL);
    lv_obj_set_size(main_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(main_screen, lv_color_black(), 0);
    lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);
    
    // 创建状态栏容器
    status_bar_container = lv_obj_create(main_screen);
    lv_obj_set_pos(status_bar_container, 0, STATUS_BAR_Y);
    lv_obj_set_size(status_bar_container, SCREEN_WIDTH, STATUS_BAR_HEIGHT);
    lv_obj_set_style_bg_color(status_bar_container, lv_color_make(0x2C, 0x3E, 0x50), 0);
    lv_obj_set_style_border_width(status_bar_container, 0, 0);
    lv_obj_set_style_pad_all(status_bar_container, 4, 0);
    lv_obj_clear_flag(status_bar_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // 创建宠物显示容器
    pet_display_container = lv_obj_create(main_screen);
    lv_obj_set_pos(pet_display_container, 0, PET_DISPLAY_Y);
    lv_obj_set_size(pet_display_container, SCREEN_WIDTH, PET_DISPLAY_HEIGHT);
    lv_obj_set_style_bg_color(pet_display_container, lv_color_make(0x34, 0x49, 0x5E), 0);
    lv_obj_set_style_border_width(pet_display_container, 0, 0);
    lv_obj_clear_flag(pet_display_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // 创建信息显示容器
    info_display_container = lv_obj_create(main_screen);
    lv_obj_set_pos(info_display_container, 0, INFO_DISPLAY_Y);
    lv_obj_set_size(info_display_container, SCREEN_WIDTH, INFO_DISPLAY_HEIGHT);
    lv_obj_set_style_bg_color(info_display_container, lv_color_make(0x2C, 0x3E, 0x50), 0);
    lv_obj_set_style_border_width(info_display_container, 0, 0);
    lv_obj_set_style_pad_all(info_display_container, MARGIN_HORIZONTAL, 0);
    lv_obj_clear_flag(info_display_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // 加载主屏幕
    lv_scr_load(main_screen);
    
    return ESP_OK;
}

esp_err_t status_bar_init(void)
{
    if (!status_bar_container) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // 创建WiFi图标
    wifi_icon = lv_label_create(status_bar_container);
    lv_obj_set_pos(wifi_icon, MARGIN_HORIZONTAL, 6);
    lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(wifi_icon, lv_color_white(), 0);
    lv_obj_set_style_text_font(wifi_icon, &lv_font_montserrat_14, 0);
    
    // 创建时间标签
    time_label = lv_label_create(status_bar_container);
    lv_obj_set_pos(time_label, SCREEN_WIDTH/2 - 20, 6);
    lv_label_set_text(time_label, "12:00");
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_14, 0);
    
    // 创建电池图标
    battery_icon = lv_label_create(status_bar_container);
    lv_obj_set_pos(battery_icon, SCREEN_WIDTH - 40, 6);
    lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_FULL);
    lv_obj_set_style_text_color(battery_icon, lv_color_white(), 0);
    lv_obj_set_style_text_font(battery_icon, &lv_font_montserrat_14, 0);
    
    // 创建RGB状态指示点
    rgb_status_dot = lv_obj_create(status_bar_container);
    lv_obj_set_size(rgb_status_dot, 8, 8);
    lv_obj_set_pos(rgb_status_dot, SCREEN_WIDTH - 16, 11);
    lv_obj_set_style_bg_color(rgb_status_dot, lv_color_white(), 0);
    lv_obj_set_style_border_width(rgb_status_dot, 0, 0);
    lv_obj_set_style_radius(rgb_status_dot, 4, 0);
    
    return ESP_OK;
}

esp_err_t pet_display_init(void)
{
    if (!pet_display_container) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // 创建宠物脸部容器
    pet_face = lv_obj_create(pet_display_container);
    lv_obj_set_size(pet_face, 120, 120);
    lv_obj_center(pet_face);
    lv_obj_set_style_bg_color(pet_face, lv_color_make(0xFF, 0xE4, 0xB5), 0);
    lv_obj_set_style_border_width(pet_face, 2, 0);
    lv_obj_set_style_border_color(pet_face, lv_color_make(0x8B, 0x45, 0x13), 0);
    lv_obj_set_style_radius(pet_face, 60, 0);
    lv_obj_clear_flag(pet_face, LV_OBJ_FLAG_SCROLLABLE);
    
    // 创建左眼
    pet_eyes_left = lv_obj_create(pet_face);
    lv_obj_set_size(pet_eyes_left, 20, 20);
    lv_obj_set_pos(pet_eyes_left, 25, 35);
    lv_obj_set_style_bg_color(pet_eyes_left, lv_color_black(), 0);
    lv_obj_set_style_border_width(pet_eyes_left, 0, 0);
    lv_obj_set_style_radius(pet_eyes_left, 10, 0);
    
    // 创建右眼
    pet_eyes_right = lv_obj_create(pet_face);
    lv_obj_set_size(pet_eyes_right, 20, 20);
    lv_obj_set_pos(pet_eyes_right, 75, 35);
    lv_obj_set_style_bg_color(pet_eyes_right, lv_color_black(), 0);
    lv_obj_set_style_border_width(pet_eyes_right, 0, 0);
    lv_obj_set_style_radius(pet_eyes_right, 10, 0);
    
    // 创建嘴巴
    pet_mouth = lv_arc_create(pet_face);
    lv_obj_set_size(pet_mouth, 40, 20);
    lv_obj_set_pos(pet_mouth, 40, 70);
    lv_arc_set_angles(pet_mouth, 0, 180);
    lv_obj_set_style_arc_color(pet_mouth, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_arc_width(pet_mouth, 3, LV_PART_MAIN);
    lv_obj_remove_style(pet_mouth, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(pet_mouth, LV_OBJ_FLAG_CLICKABLE);
    
    return ESP_OK;
}

esp_err_t info_display_init(void)
{
    if (!info_display_container) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // 创建第一行文字
    info_line1 = lv_label_create(info_display_container);
    lv_obj_set_pos(info_line1, 0, 10);
    lv_obj_set_width(info_line1, SCREEN_WIDTH - 2 * MARGIN_HORIZONTAL);
    lv_label_set_text(info_line1, "Choomi Starting...");
    lv_obj_set_style_text_color(info_line1, lv_color_white(), 0);
    lv_obj_set_style_text_font(info_line1, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(info_line1, LV_TEXT_ALIGN_CENTER, 0);
    
    // 创建第二行文字
    info_line2 = lv_label_create(info_display_container);
    lv_obj_set_pos(info_line2, 0, 35);
    lv_obj_set_width(info_line2, SCREEN_WIDTH - 2 * MARGIN_HORIZONTAL);
    lv_label_set_text(info_line2, "Initializing Hardware");
    lv_obj_set_style_text_color(info_line2, lv_color_make(0xCC, 0xCC, 0xCC), 0);
    lv_obj_set_style_text_font(info_line2, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(info_line2, LV_TEXT_ALIGN_CENTER, 0);
    
    // 创建音量条（初始隐藏）
    volume_bar = lv_bar_create(info_display_container);
    lv_obj_set_size(volume_bar, SCREEN_WIDTH - 4 * MARGIN_HORIZONTAL, 8);
    lv_obj_set_pos(volume_bar, MARGIN_HORIZONTAL, 60);
    lv_bar_set_range(volume_bar, 0, 100);
    lv_bar_set_value(volume_bar, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(volume_bar, lv_color_make(0x40, 0x40, 0x40), LV_PART_MAIN);
    lv_obj_set_style_bg_color(volume_bar, convert_rgb565_to_lv_color(COLOR_PRIMARY), LV_PART_INDICATOR);
    lv_obj_add_flag(volume_bar, LV_OBJ_FLAG_HIDDEN);
    
    // 创建音量标签（初始隐藏）
    volume_label = lv_label_create(info_display_container);
    lv_obj_set_pos(volume_label, 0, 72);
    lv_obj_set_width(volume_label, SCREEN_WIDTH - 2 * MARGIN_HORIZONTAL);
    lv_label_set_text(volume_label, "Volume: 50%");
    lv_obj_set_style_text_color(volume_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(volume_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(volume_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_flag(volume_label, LV_OBJ_FLAG_HIDDEN);
    
    return ESP_OK;
}

esp_err_t ui_layout_set_region_dirty(layout_region_t region)
{
    if (region >= LAYOUT_REGION_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (xSemaphoreTake(g_ui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        g_regions[region].dirty = true;
        xSemaphoreGive(g_ui_mutex);
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

esp_err_t ui_layout_update_display(void)
{
    // LVGL会自动处理重绘，这里主要用于触发更新
    lv_refr_now(NULL);
    return ESP_OK;
}

esp_err_t ui_layout_get_region_config(layout_region_t region, layout_region_config_t *config)
{
    if (region >= LAYOUT_REGION_MAX || !config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (xSemaphoreTake(g_ui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        *config = g_regions[region];
        xSemaphoreGive(g_ui_mutex);
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

esp_err_t status_bar_set_time(const char *time_str)
{
    if (!time_str || !time_label) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (xSemaphoreTake(g_ui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        strncpy(g_ui_state.status_bar.time_str, time_str, sizeof(g_ui_state.status_bar.time_str) - 1);
        lv_label_set_text(time_label, time_str);
        xSemaphoreGive(g_ui_mutex);
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

esp_err_t status_bar_set_wifi_signal(wifi_signal_level_t level)
{
    if (!wifi_icon) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (xSemaphoreTake(g_ui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        g_ui_state.status_bar.wifi_signal = level;
        update_wifi_icon(level);
        xSemaphoreGive(g_ui_mutex);
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

esp_err_t status_bar_set_battery(battery_level_t level, bool charging)
{
    if (!battery_icon) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (xSemaphoreTake(g_ui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        g_ui_state.status_bar.battery_level = level;
        g_ui_state.status_bar.charging = charging;
        update_battery_icon(level, charging);
        xSemaphoreGive(g_ui_mutex);
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

esp_err_t status_bar_set_rgb_status(uint32_t color, bool active)
{
    if (!rgb_status_dot) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (xSemaphoreTake(g_ui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        g_ui_state.status_bar.rgb_led_color = color;
        g_ui_state.status_bar.rgb_led_active = active;
        
        if (active) {
            lv_color_t lv_color = lv_color_make((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
            lv_obj_set_style_bg_color(rgb_status_dot, lv_color, 0);
            lv_obj_clear_flag(rgb_status_dot, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(rgb_status_dot, LV_OBJ_FLAG_HIDDEN);
        }
        
        xSemaphoreGive(g_ui_mutex);
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

esp_err_t pet_display_set_emotion(pet_emotion_t emotion)
{
    if (xSemaphoreTake(g_ui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        g_ui_state.pet_emotion = emotion;
        update_pet_face(emotion);
        xSemaphoreGive(g_ui_mutex);
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

esp_err_t info_display_show_text(const char *line1, const char *line2)
{
    if (!line1 || !info_line1 || !info_line2) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (xSemaphoreTake(g_ui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // 隐藏音量控件
        lv_obj_add_flag(volume_bar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(volume_label, LV_OBJ_FLAG_HIDDEN);
        
        // 显示文字
        lv_obj_clear_flag(info_line1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_line2, LV_OBJ_FLAG_HIDDEN);
        
        lv_label_set_text(info_line1, line1);
        if (line2) {
            lv_label_set_text(info_line2, line2);
        } else {
            lv_label_set_text(info_line2, "");
        }
        
        xSemaphoreGive(g_ui_mutex);
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

esp_err_t info_display_show_volume(uint8_t volume, bool show_percentage)
{
    if (!volume_bar || !volume_label) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (xSemaphoreTake(g_ui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // 隐藏文字
        lv_obj_add_flag(info_line1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(info_line2, LV_OBJ_FLAG_HIDDEN);
        
        // 显示音量控件
        lv_obj_clear_flag(volume_bar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(volume_label, LV_OBJ_FLAG_HIDDEN);
        
        // 更新音量值
        lv_bar_set_value(volume_bar, volume, LV_ANIM_ON);
        
        if (show_percentage) {
            char volume_text[32];
            snprintf(volume_text, sizeof(volume_text), "Volume: %d%%", volume);
            lv_label_set_text(volume_label, volume_text);
        } else {
            lv_label_set_text(volume_label, "Volume Control");
        }
        
        xSemaphoreGive(g_ui_mutex);
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

// 高级接口实现
esp_err_t ui_layout_show_boot_screen(void)
{
    pet_display_set_emotion(PET_EMOTION_IDLE);
    status_bar_set_wifi_signal(WIFI_SIGNAL_NONE);
    info_display_show_text("Choomi Starting...", "Initializing Hardware");
    return ESP_OK;
}

esp_err_t ui_layout_show_wifi_config_screen(void)
{
    pet_display_set_emotion(PET_EMOTION_CONFUSED);
    status_bar_set_wifi_signal(WIFI_SIGNAL_NONE);
    info_display_show_text("WiFi Disconnected", "Long press to configure");
    return ESP_OK;
}

esp_err_t ui_layout_show_listening_screen(void)
{
    pet_display_set_emotion(PET_EMOTION_LISTENING);
    info_display_show_text("Listening...", "Please speak your question");
    return ESP_OK;
}

esp_err_t ui_layout_show_thinking_screen(void)
{
    pet_display_set_emotion(PET_EMOTION_THINKING);
    info_display_show_text("AI Thinking...", "Analyzing your question");
    return ESP_OK;
}

esp_err_t ui_layout_show_speaking_screen(void)
{
    pet_display_set_emotion(PET_EMOTION_SPEAKING);
    info_display_show_text("Choomi Speaking...", "");
    return ESP_OK;
}

esp_err_t ui_layout_show_error_screen(const char *error_msg)
{
    pet_display_set_emotion(PET_EMOTION_SAD);
    info_display_show_text("System Error!", error_msg ? error_msg : "Please restart device");
    return ESP_OK;
}

esp_err_t ui_layout_show_volume_screen(uint8_t volume)
{
    info_display_show_volume(volume, true);
    return ESP_OK;
}

// 内部辅助函数实现
static void update_wifi_icon(wifi_signal_level_t level)
{
    const char* wifi_symbols[] = {
        "✗",           // WIFI_SIGNAL_NONE
        LV_SYMBOL_WIFI, // WIFI_SIGNAL_WEAK
        LV_SYMBOL_WIFI, // WIFI_SIGNAL_MEDIUM  
        LV_SYMBOL_WIFI  // WIFI_SIGNAL_STRONG
    };
    
    lv_color_t colors[] = {
        lv_color_make(0xFF, 0x00, 0x00), // 红色 - 无信号
        lv_color_make(0xFF, 0xA5, 0x00), // 橙色 - 弱信号
        lv_color_make(0xFF, 0xFF, 0x00), // 黄色 - 中等信号
        lv_color_make(0x00, 0xFF, 0x00)  // 绿色 - 强信号
    };
    
    if (level < sizeof(wifi_symbols)/sizeof(wifi_symbols[0])) {
        lv_label_set_text(wifi_icon, wifi_symbols[level]);
        lv_obj_set_style_text_color(wifi_icon, colors[level], 0);
    }
}

static void update_battery_icon(battery_level_t level, bool charging)
{
    const char* battery_symbols[] = {
        LV_SYMBOL_BATTERY_EMPTY,  // BATTERY_LEVEL_CRITICAL
        LV_SYMBOL_BATTERY_1,      // BATTERY_LEVEL_LOW
        LV_SYMBOL_BATTERY_2,      // BATTERY_LEVEL_MEDIUM
        LV_SYMBOL_BATTERY_FULL    // BATTERY_LEVEL_HIGH
    };
    
    lv_color_t colors[] = {
        lv_color_make(0xFF, 0x00, 0x00), // 红色 - 电量危险
        lv_color_make(0xFF, 0xA5, 0x00), // 橙色 - 电量低
        lv_color_make(0xFF, 0xFF, 0x00), // 黄色 - 电量中等
        lv_color_make(0x00, 0xFF, 0x00)  // 绿色 - 电量充足
    };
    
    if (level < sizeof(battery_symbols)/sizeof(battery_symbols[0])) {
        if (charging) {
            lv_label_set_text(battery_icon, LV_SYMBOL_CHARGE);
            lv_obj_set_style_text_color(battery_icon, lv_color_make(0x00, 0xFF, 0x00), 0);
        } else {
            lv_label_set_text(battery_icon, battery_symbols[level]);
            lv_obj_set_style_text_color(battery_icon, colors[level], 0);
        }
    }
}

static void update_pet_face(pet_emotion_t emotion)
{
    if (!pet_eyes_left || !pet_eyes_right || !pet_mouth) {
        return;
    }
    
    switch (emotion) {
        case PET_EMOTION_HAPPY:
            // 眯眯眼 + 笑脸
            lv_obj_set_height(pet_eyes_left, 8);
            lv_obj_set_height(pet_eyes_right, 8);
            lv_arc_set_angles(pet_mouth, 0, 180);
            break;
            
        case PET_EMOTION_SAD:
            // 正常眼睛 + 倒转嘴巴
            lv_obj_set_height(pet_eyes_left, 20);
            lv_obj_set_height(pet_eyes_right, 20);
            lv_arc_set_angles(pet_mouth, 180, 360);
            break;
            
        case PET_EMOTION_SURPRISED:
            // 大眼睛 + 圆嘴
            lv_obj_set_size(pet_eyes_left, 25, 25);
            lv_obj_set_size(pet_eyes_right, 25, 25);
            lv_arc_set_angles(pet_mouth, 0, 360);
            break;
            
        case PET_EMOTION_THINKING:
            // 一只眼睛眯起来
            lv_obj_set_height(pet_eyes_left, 8);
            lv_obj_set_height(pet_eyes_right, 20);
            lv_arc_set_angles(pet_mouth, 45, 135);
            break;
            
        case PET_EMOTION_LISTENING:
            // 专注的眼神
            lv_obj_set_size(pet_eyes_left, 18, 22);
            lv_obj_set_size(pet_eyes_right, 18, 22);
            lv_arc_set_angles(pet_mouth, 80, 100);
            break;
            
        case PET_EMOTION_SPEAKING:
            // 张嘴说话
            lv_obj_set_height(pet_eyes_left, 20);
            lv_obj_set_height(pet_eyes_right, 20);
            lv_arc_set_angles(pet_mouth, 30, 150);
            break;
            
        case PET_EMOTION_SLEEPING:
            // 闭眼睡觉
            lv_obj_set_height(pet_eyes_left, 4);
            lv_obj_set_height(pet_eyes_right, 4);
            lv_arc_set_angles(pet_mouth, 85, 95);
            break;
            
        case PET_EMOTION_CONFUSED:
            // 困惑表情
            lv_obj_set_height(pet_eyes_left, 20);
            lv_obj_set_height(pet_eyes_right, 15);
            lv_arc_set_angles(pet_mouth, 60, 120);
            break;
            
        default: // PET_EMOTION_IDLE
            // 默认表情
            lv_obj_set_size(pet_eyes_left, 20, 20);
            lv_obj_set_size(pet_eyes_right, 20, 20);
            lv_arc_set_angles(pet_mouth, 45, 135);
            break;
    }
}

static lv_color_t convert_rgb565_to_lv_color(uint16_t rgb565)
{
    uint8_t r = (rgb565 >> 11) & 0x1F;
    uint8_t g = (rgb565 >> 5) & 0x3F;
    uint8_t b = rgb565 & 0x1F;
    
    // 转换到8位
    r = (r * 255) / 31;
    g = (g * 255) / 63;
    b = (b * 255) / 31;
    
    return lv_color_make(r, g, b);
}

// 其他接口函数实现
esp_err_t status_bar_update_data(const status_bar_data_t *data) 
{ 
    if (!data) return ESP_ERR_INVALID_ARG;
    
    if (xSemaphoreTake(g_ui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        g_ui_state.status_bar = *data;
        
        // 更新各个状态栏元素
        status_bar_set_time(data->time_str);
        status_bar_set_wifi_signal(data->wifi_signal);
        status_bar_set_battery(data->battery_level, data->charging);
        status_bar_set_rgb_status(data->rgb_led_color, data->rgb_led_active);
        
        xSemaphoreGive(g_ui_mutex);
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

esp_err_t pet_display_update(uint32_t delta_time_ms) 
{ 
    // 这里可以添加动画更新逻辑
    return ESP_OK; 
}

pet_emotion_t pet_display_get_current_emotion(void) 
{ 
    return g_ui_state.pet_emotion; 
}

bool pet_display_is_animation_playing(void) 
{ 
    return g_ui_state.pet_animation_playing; 
}

esp_err_t info_display_show_progress(const char *label, uint8_t progress) 
{ 
    if (!label) return ESP_ERR_INVALID_ARG;
    
    char progress_text[64];
    snprintf(progress_text, sizeof(progress_text), "%s: %d%%", label, progress);
    return info_display_show_text(progress_text, "");
}

esp_err_t info_display_show_error(const char *error_msg) 
{ 
    return info_display_show_text("Error!", error_msg ? error_msg : "Unknown error"); 
}

esp_err_t info_display_update(uint32_t delta_time_ms) 
{ 
    // 这里可以添加信息显示的动画更新逻辑
    return ESP_OK; 
}

esp_err_t info_display_clear(void) 
{ 
    return info_display_show_text("", ""); 
}

esp_err_t ui_layout_set_device_state(device_state_t state) 
{ 
    if (xSemaphoreTake(g_ui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        g_ui_state.previous_state = g_ui_state.current_state;
        g_ui_state.current_state = state;
        g_ui_state.state_enter_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        xSemaphoreGive(g_ui_mutex);
        return ESP_OK;
    }
    return ESP_ERR_TIMEOUT;
}

esp_err_t ui_layout_update_status_bar(const status_bar_data_t *data) 
{ 
    return status_bar_update_data(data);
}