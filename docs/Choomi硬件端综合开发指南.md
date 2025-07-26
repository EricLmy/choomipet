# Choomi硬件端综合开发指南

## 📖 概述

本文档整合了实现路线图、主程序流程、代码模板和执行检查清单，为Choomi硬件端开发提供完整的指导。通过严格按照本指南执行，开发者可以高效、准确地实现一个完整的Choomi硬件端系统。

## 🎯 核心设计原则

### 1. 主程序驱动开发
- 所有任务都严格对应主程序中的具体函数
- 按照代码执行顺序进行开发
- 确保每个实现都与主程序设计完全一致

### 2. 分层架构设计
```
应用层 ──── 主控制任务 (任务11)
  │
管理层 ──── 系统监控 (任务9) + 设备认证 (任务10)
  │
通信层 ──── WiFi管理 (任务6) + WebSocket (任务7)
  │
驱动层 ──── 音频 (任务3) + 显示 (任务4) + 按键 (任务5) + RGB (任务8)
  │
硬件层 ──── 系统引导 (任务1) + 硬件初始化 (任务2)
```

### 3. 双核任务分配
- **Core 0**: 网络通信和系统管理任务
- **Core 1**: 实时处理和用户交互任务

## 🚀 快速开始指南

### 第一步：环境准备
```bash
# 1. 安装ESP-IDF v5.4.1
git clone -b v5.4.1 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf && ./install.sh

# 2. 设置环境变量
source export.sh

# 3. 创建项目
idf.py create-project choomi-hardware
cd choomi-hardware

# 4. 设置目标芯片
idf.py set-target esp32c6
```

### 第二步：项目结构创建
```
choomi-hardware/
├── main/
│   ├── main.c                 # 系统入口 (任务1)
│   ├── main_control.c         # 主控制任务 (任务11)
│   └── CMakeLists.txt
├── components/
│   ├── hardware/              # 硬件配置 (任务2)
│   ├── audio/                 # 音频处理 (任务3)
│   ├── display/               # 显示控制 (任务4,8)
│   ├── input/                 # 输入处理 (任务5)
│   ├── network/               # 网络通信 (任务6,7)
│   ├── system/                # 系统管理 (任务9,10)
│   └── common/                # 公共定义
├── test/                      # 测试框架 (任务12)
├── CMakeLists.txt
└── sdkconfig
```

## 📋 任务实现路线图

### 🎯 主程序流程与任务对应关系

| 执行顺序 | 主程序函数 | 对应任务 | 实现内容 | 开发时间 |
|---------|-----------|----------|----------|----------|
| 1 | `app_main()` | 任务1 | 系统引导和基础架构 | 1天 |
| 2 | `hardware_init_start()` | 任务2 | 硬件初始化管理 | 1天 |
| 3 | `rgb_led_init()` | 任务8 | RGB状态指示灯 | 2天 |
| 4 | `audio_processing_task()` | 任务3 | I2S音频系统 | 3天 |
| 5 | `display_update_task()` | 任务4 | LCD显示系统 | 3天 |
| 6 | `button_handler_task()` | 任务5 | 按键输入系统 | 2天 |
| 7 | `wifi_manager_task()` | 任务6 | WiFi网络连接 | 3天 |
| 8 | `websocket_client_task()` | 任务7 | WebSocket通信 | 3天 |
| 9 | `system_monitor_task()` | 任务9 | 设备状态管理 | 2天 |
| 10 | `main_control_task()` | 任务11 | 系统集成 | 3天 |
| 11 | `device_auth_*()` | 任务10 | 设备认证(可选) | 2天 |
| 12 | `run_all_tests()` | 任务12 | 测试验证 | 5天 |

## 🔧 核心代码模板

### 系统入口模板 (任务1)
```c
#include "choomi_common.h"

static const char *TAG = "CHOOMI_MAIN";

void app_main(void) {
    // NVS Flash初始化
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // 日志系统初始化
    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_LOGI(TAG, "Choomi Hardware Starting...");
    
    // 看门狗初始化
    esp_task_wdt_init(WDT_TIMEOUT, true);
    
    // 进入硬件初始化阶段
    ESP_ERROR_CHECK(hardware_init_start());
}
```

### 硬件初始化模板 (任务2)
```c
esp_err_t hardware_init_start(void) {
    ESP_LOGI(TAG, "Starting hardware initialization...");
    
    // RGB LED优先初始化（用于状态指示）
    ESP_ERROR_CHECK(rgb_led_init());
    rgb_led_set_color(RGB_COLOR_WHITE);
    
    // LCD初始化并显示启动画面
    ESP_ERROR_CHECK(lcd_init(&lcd_config));
    lcd_display_boot_screen();
    
    // GPIO引脚配置
    ESP_ERROR_CHECK(gpio_config_all());
    
    // I2S音频系统初始化
    ESP_ERROR_CHECK(i2s_driver_init());
    
    // 按键系统初始化
    ESP_ERROR_CHECK(button_init(button_configs, BTN_MAX));
    
    // 电源管理初始化
    ESP_ERROR_CHECK(power_manager_init());
    
    // NVS存储初始化
    ESP_ERROR_CHECK(nvs_init());
    
    // 硬件自检
    if (hardware_self_test() != ESP_OK) {
        rgb_led_set_color(RGB_COLOR_RED);
        lcd_display_error_screen("Hardware Test Failed");
        return ESP_FAIL;
    }
    
    // 进入任务创建阶段
    return create_system_tasks();
}
```

### 任务创建模板 (任务11)
```c
esp_err_t create_system_tasks(void) {
    // 创建核心任务（高优先级，Core1）
    xTaskCreatePinnedToCore(main_control_task, "main_ctrl", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(audio_processing_task, "audio_proc", 8192, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(display_update_task, "display", 4096, NULL, 3, NULL, 1);
    
    // 创建网络任务（中优先级，Core0）
    xTaskCreatePinnedToCore(wifi_manager_task, "wifi_mgr", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(websocket_client_task, "ws_client", 6144, NULL, 2, NULL, 0);
    
    // 创建系统管理任务（低优先级，Core0）
    xTaskCreatePinnedToCore(system_monitor_task, "sys_monitor", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(button_handler_task, "button", 2048, NULL, 1, NULL, 0);
    
    // 创建事件队列和信号量
    create_system_queues();
    
    ESP_LOGI(TAG, "All system tasks created successfully");
    return ESP_OK;
}
```

## 📊 系统状态机设计

### 状态定义
```c
typedef enum {
    SYSTEM_STATE_BOOTING,           // 系统启动中
    SYSTEM_STATE_HARDWARE_INIT,     // 硬件初始化
    SYSTEM_STATE_WIFI_CONFIG,       // WiFi配置模式
    SYSTEM_STATE_WIFI_CONNECTING,   // WiFi连接中
    SYSTEM_STATE_WIFI_CONNECTED,    // WiFi已连接
    SYSTEM_STATE_SERVER_CONNECTING, // 服务器连接中
    SYSTEM_STATE_ONLINE_READY,      // 在线就绪
    SYSTEM_STATE_LISTENING,         // 录音中
    SYSTEM_STATE_PROCESSING,        // 处理中
    SYSTEM_STATE_SPEAKING,          // 播放中
    SYSTEM_STATE_IDLE,              // 空闲状态
    SYSTEM_STATE_SLEEP,             // 休眠状态
    SYSTEM_STATE_ERROR              // 错误状态
} system_state_t;
```

### 状态机实现
```c
void system_state_machine(void) {
    static system_state_t current_state = SYSTEM_STATE_BOOTING;
    static system_state_t previous_state = SYSTEM_STATE_BOOTING;
    
    // 状态变化检测
    if (current_state != previous_state) {
        ESP_LOGI(TAG, "State change: %s -> %s", 
                 state_to_string(previous_state), 
                 state_to_string(current_state));
        
        // 状态进入处理
        state_enter_handler(current_state);
        
        // 更新显示和LED状态
        update_status_display(current_state);
        update_rgb_led_status(current_state);
        
        previous_state = current_state;
    }
    
    // 状态处理逻辑
    switch (current_state) {
        case SYSTEM_STATE_ONLINE_READY:
            current_state = handle_online_ready_state();
            break;
        case SYSTEM_STATE_LISTENING:
            current_state = handle_listening_state();
            break;
        case SYSTEM_STATE_PROCESSING:
            current_state = handle_processing_state();
            break;
        case SYSTEM_STATE_SPEAKING:
            current_state = handle_speaking_state();
            break;
        default:
            break;
    }
}
```

## 🎯 关键实现要点

### 1. RGB LED状态指示 (任务8)
```c
// 状态颜色映射
typedef struct {
    system_state_t state;
    rgb_color_t color;
    rgb_effect_t effect;
} state_led_mapping_t;

static const state_led_mapping_t led_mappings[] = {
    {SYSTEM_STATE_BOOTING, RGB_COLOR_WHITE, RGB_EFFECT_SOLID},
    {SYSTEM_STATE_WIFI_CONNECTING, RGB_COLOR_BLUE, RGB_EFFECT_BLINKING},
    {SYSTEM_STATE_ONLINE_READY, RGB_COLOR_GREEN, RGB_EFFECT_BREATHING},
    {SYSTEM_STATE_LISTENING, RGB_COLOR_PURPLE, RGB_EFFECT_BREATHING},
    {SYSTEM_STATE_PROCESSING, RGB_COLOR_BLUE, RGB_EFFECT_BLINKING},
    {SYSTEM_STATE_SPEAKING, RGB_COLOR_CYAN, RGB_EFFECT_BREATHING},
    {SYSTEM_STATE_ERROR, RGB_COLOR_RED, RGB_EFFECT_BLINKING}
};
```

### 2. 音频处理流程 (任务3)
```c
void audio_processing_task(void *pvParameters) {
    while (1) {
        // 录音处理
        if (is_recording) {
            static uint8_t audio_buffer[AUDIO_BUFFER_SIZE];
            size_t bytes_read = mic_read_data(audio_buffer, sizeof(audio_buffer), 10);
            
            if (bytes_read > 0) {
                // 更新波形显示
                update_audio_waveform_display(audio_buffer, bytes_read);
                // 存储音频数据
                store_audio_data(audio_buffer, bytes_read);
            }
        }
        
        // 播放处理
        if (is_playing) {
            if (!speaker_is_playing()) {
                // 播放完成，返回就绪状态
                system_state = SYSTEM_STATE_ONLINE_READY;
                is_playing = false;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

### 3. 按键事件处理 (任务5)
```c
void handle_button_press_event(system_event_t *event) {
    button_id_t button_id = *(button_id_t*)event->data;
    
    switch (button_id) {
        case BTN_VOL_UP:
            audio_volume_up();
            lcd_display_volume_indicator(get_current_volume());
            break;
            
        case BTN_VOL_DOWN:
            audio_volume_down();
            lcd_display_volume_indicator(get_current_volume());
            break;
            
        case BTN_FUNC:
            if (system_state == SYSTEM_STATE_ONLINE_READY) {
                start_audio_recording();
            } else if (system_state == SYSTEM_STATE_LISTENING) {
                stop_audio_recording();
            }
            break;
    }
}
```

## ✅ 质量控制检查清单

### 每个任务完成标准
- [ ] **功能实现完整** - 所有要求的功能都已实现
- [ ] **代码质量达标** - 通过代码审查和质量检查
- [ ] **测试验证通过** - 单元测试、集成测试、性能测试全部通过
- [ ] **文档更新完成** - 相关文档已更新
- [ ] **与主程序对应** - 实现与主程序设计完全一致

### 关键性能指标
| 指标 | 目标值 | 验证方法 |
|------|--------|----------|
| 系统启动时间 | < 30秒 | 计时测试 |
| 音频端到端延迟 | < 100ms | 示波器测量 |
| 按键响应时间 | < 50ms | 中断计时 |
| 界面响应时间 | < 200ms | 用户体验测试 |
| WiFi连接时间 | < 10秒 | 网络测试 |
| WebSocket连接时间 | < 5秒 | 通信测试 |
| 24小时稳定运行 | 无崩溃 | 长期测试 |

### 代码质量标准
- [ ] 符合ESP-IDF编码规范
- [ ] 所有函数都有注释说明
- [ ] 错误处理完整
- [ ] 日志输出级别合适
- [ ] 内存使用合理
- [ ] 无内存泄漏

## 🔄 开发流程建议

### 第一阶段：基础架构 (1-2天)
1. 完成任务1：建立项目基础架构
2. 完成任务2：实现硬件初始化管理
3. 验证系统能够正常启动

### 第二阶段：核心驱动 (3-10天)
1. 完成任务8：RGB LED状态指示
2. 完成任务3：I2S音频系统
3. 完成任务4：LCD显示系统
4. 完成任务5：按键输入系统
5. 验证所有硬件功能正常

### 第三阶段：网络通信 (11-16天)
1. 完成任务6：WiFi网络连接
2. 完成任务7：WebSocket通信
3. 验证网络通信稳定

### 第四阶段：系统集成 (17-22天)
1. 完成任务9：设备状态管理
2. 完成任务11：系统集成和状态机
3. 验证系统完整运行

### 第五阶段：质量保证 (23-30天)
1. 完成任务10：设备认证(可选)
2. 完成任务12：测试验证
3. 系统优化和文档完善

## 🎉 成功标准

项目成功的标志是：
1. **功能完整性** - 所有设计功能都已实现
2. **性能达标** - 所有性能指标都达到要求
3. **稳定可靠** - 系统能够长期稳定运行
4. **用户体验** - 操作流畅，响应及时
5. **代码质量** - 代码规范，易于维护

通过严格按照本综合开发指南执行，开发者将能够高效、准确地实现一个完整、高质量的Choomi硬件端系统！

---

**注意**: 本指南是基于主程序流程设计的完整实现指导，每个任务的实现都必须严格对应主程序中的具体函数和流程。只有这样，才能确保最终得到一个完整、正确的系统。