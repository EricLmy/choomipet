#include "ui_layout_demo.h"
#include "ui_layout.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "UI_DEMO";

// 演示任务句柄
static TaskHandle_t demo_task_handle = NULL;

// 演示状态
static int demo_step = 0;
static uint32_t demo_start_time = 0;

// 演示任务
static void ui_layout_demo_task(void *pvParameters)
{
    ESP_LOGI(TAG, "UI Layout Demo Task Started");
    
    demo_start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    while (1) {
        uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        uint32_t elapsed_time = current_time - demo_start_time;
        
        switch (demo_step) {
            case 0: // 启动画面 (0-3秒)
                if (elapsed_time < 3000) {
                    ui_layout_show_boot_screen();
                    status_bar_set_time("12:00");
                } else {
                    demo_step++;
                    demo_start_time = current_time;
                }
                break;
                
            case 1: // WiFi未连接 (3-6秒)
                if (elapsed_time < 3000) {
                    ui_layout_show_wifi_config_screen();
                    status_bar_set_wifi_signal(WIFI_SIGNAL_NONE);
                } else {
                    demo_step++;
                    demo_start_time = current_time;
                }
                break;
                
            case 2: // WiFi连接中 (6-8秒)
                if (elapsed_time < 2000) {
                    pet_display_set_emotion(PET_EMOTION_THINKING);
                    info_display_show_text("Connecting WiFi...", "Please wait...");
                    status_bar_set_wifi_signal(WIFI_SIGNAL_WEAK);
                } else {
                    demo_step++;
                    demo_start_time = current_time;
                }
                break;
                
            case 3: // WiFi连接成功 (8-10秒)
                if (elapsed_time < 2000) {
                    pet_display_set_emotion(PET_EMOTION_HAPPY);
                    info_display_show_text("WiFi Connected!", "Connecting to server...");
                    status_bar_set_wifi_signal(WIFI_SIGNAL_STRONG);
                } else {
                    demo_step++;
                    demo_start_time = current_time;
                }
                break;
                
            case 4: // 在线就绪 (10-13秒)
                if (elapsed_time < 3000) {
                    pet_display_set_emotion(PET_EMOTION_IDLE);
                    info_display_show_text("I'm Choomi!", "How can I help you?");
                    status_bar_set_rgb_status(0x00FF00, true); // 绿色RGB指示
                } else {
                    demo_step++;
                    demo_start_time = current_time;
                }
                break;
                
            case 5: // 正在倾听 (13-16秒)
                if (elapsed_time < 3000) {
                    ui_layout_show_listening_screen();
                    status_bar_set_rgb_status(0x00FFFF, true); // 青色RGB指示
                } else {
                    demo_step++;
                    demo_start_time = current_time;
                }
                break;
                
            case 6: // AI思考中 (16-19秒)
                if (elapsed_time < 3000) {
                    ui_layout_show_thinking_screen();
                    status_bar_set_rgb_status(0x0000FF, true); // 蓝色RGB指示
                } else {
                    demo_step++;
                    demo_start_time = current_time;
                }
                break;
                
            case 7: // AI回答中 (19-22秒)
                if (elapsed_time < 3000) {
                    ui_layout_show_speaking_screen();
                    status_bar_set_rgb_status(0xFF00FF, true); // 紫色RGB指示
                } else {
                    demo_step++;
                    demo_start_time = current_time;
                }
                break;
                
            case 8: // 音量调节演示 (22-25秒)
                if (elapsed_time < 3000) {
                    static uint8_t volume = 50;
                    static int volume_direction = 1;
                    
                    volume += volume_direction * 2;
                    if (volume >= 100) {
                        volume = 100;
                        volume_direction = -1;
                    } else if (volume <= 0) {
                        volume = 0;
                        volume_direction = 1;
                    }
                    
                    ui_layout_show_volume_screen(volume);
                    vTaskDelay(pdMS_TO_TICKS(100)); // 更快的更新频率
                } else {
                    demo_step++;
                    demo_start_time = current_time;
                }
                break;
                
            case 9: // 电池状态演示 (25-28秒)
                if (elapsed_time < 3000) {
                    static int battery_demo_step = 0;
                    uint32_t battery_elapsed = elapsed_time % 1000;
                    
                    if (battery_elapsed < 250) {
                        status_bar_set_battery(BATTERY_LEVEL_HIGH, false);
                        info_display_show_text("Battery Full", "100%");
                    } else if (battery_elapsed < 500) {
                        status_bar_set_battery(BATTERY_LEVEL_MEDIUM, false);
                        info_display_show_text("Battery Medium", "60%");
                    } else if (battery_elapsed < 750) {
                        status_bar_set_battery(BATTERY_LEVEL_LOW, false);
                        info_display_show_text("Battery Low", "25%");
                    } else {
                        status_bar_set_battery(BATTERY_LEVEL_CRITICAL, true);
                        info_display_show_text("Battery Critical!", "Charging...");
                    }
                } else {
                    demo_step++;
                    demo_start_time = current_time;
                }
                break;
                
            case 10: // 错误状态演示 (28-31秒)
                if (elapsed_time < 3000) {
                    ui_layout_show_error_screen("Network Connection Failed");
                    status_bar_set_rgb_status(0xFF0000, true); // 红色RGB指示
                    status_bar_set_wifi_signal(WIFI_SIGNAL_NONE);
                } else {
                    demo_step++;
                    demo_start_time = current_time;
                }
                break;
                
            case 11: // 睡眠状态演示 (31-34秒)
                if (elapsed_time < 3000) {
                    pet_display_set_emotion(PET_EMOTION_SLEEPING);
                    info_display_show_text("Choomi Sleeping...", "Touch screen to wake up");
                    status_bar_set_rgb_status(0x000080, true); // 暗蓝色RGB指示
                } else {
                    demo_step = 0; // 重新开始演示
                    demo_start_time = current_time;
                }
                break;
                
            default:
                demo_step = 0;
                demo_start_time = current_time;
                break;
        }
        
        // 更新时间显示
        char time_str[16];
        uint32_t minutes = (current_time / 60000) % 60;
        uint32_t hours = (12 + (current_time / 3600000)) % 24;
        snprintf(time_str, sizeof(time_str), "%02d:%02d", (int)hours, (int)minutes);
        status_bar_set_time(time_str);
        
        vTaskDelay(pdMS_TO_TICKS(50)); // 20fps更新频率
    }
}

esp_err_t ui_layout_demo_start(void)
{
    ESP_LOGI(TAG, "Starting UI Layout Demo");
    
    // 初始化UI布局系统
    esp_err_t ret = ui_layout_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize UI layout: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 创建演示任务
    BaseType_t task_ret = xTaskCreate(
        ui_layout_demo_task,
        "ui_demo_task",
        4096,
        NULL,
        5,
        &demo_task_handle
    );
    
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create demo task");
        return ESP_ERR_NO_MEM;
    }
    
    ESP_LOGI(TAG, "UI Layout Demo started successfully");
    return ESP_OK;
}

esp_err_t ui_layout_demo_stop(void)
{
    if (demo_task_handle) {
        vTaskDelete(demo_task_handle);
        demo_task_handle = NULL;
    }
    
    ui_layout_deinit();
    
    ESP_LOGI(TAG, "UI Layout Demo stopped");
    return ESP_OK;
}

void ui_layout_demo_next_step(void)
{
    demo_step++;
    demo_start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    ESP_LOGI(TAG, "Demo step advanced to: %d", demo_step);
}

int ui_layout_demo_get_current_step(void)
{
    return demo_step;
}