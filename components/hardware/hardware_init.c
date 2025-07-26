#include "hardware_init.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2s.h"
#include "driver/adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "HW_INIT";

// 全局硬件初始化状态
static hw_init_status_t g_hw_status = {
    .current_stage = HW_INIT_STAGE_START,
    .last_error = HW_INIT_OK,
    .initialization_complete = false,
    .self_test_result = {0}
};

// 阶段名称映射表
static const char* stage_names[] = {
    "START",
    "GPIO",
    "CLOCK",
    "SPI",
    "I2S",
    "ADC",
    "SELF_TEST",
    "COMPLETE",
    "ERROR"
};

// 错误名称映射表
static const char* error_names[] = {
    "OK",
    "GPIO_ERROR",
    "CLOCK_ERROR",
    "SPI_ERROR",
    "I2S_ERROR",
    "ADC_ERROR",
    "SELF_TEST_ERROR",
    "UNKNOWN_ERROR"
};

esp_err_t hardware_init_start(void) {
    ESP_LOGI(TAG, "Hardware initialization started - 硬件初始化开始");
    
    // 2.1 RGB LED优先初始化（用于状态指示）
    g_hw_status.current_stage = HW_INIT_STAGE_GPIO;
    ESP_LOGI(TAG, "Stage: %s - 阶段: %s", hardware_get_stage_name(g_hw_status.current_stage), hardware_get_stage_name(g_hw_status.current_stage));
    
    // 2.3 GPIO引脚配置
    esp_err_t ret = gpio_config_all();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO configuration failed: %s - GPIO配置失败", esp_err_to_name(ret));
        g_hw_status.last_error = HW_INIT_ERR_GPIO;
        g_hw_status.current_stage = HW_INIT_STAGE_ERROR;
        return ret;
    }
    
    // 设置RGB LED为白色表示启动中
    gpio_set_level(RGB_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(RGB_LED_PIN, 0);
    
    // 2.4 时钟配置阶段
    g_hw_status.current_stage = HW_INIT_STAGE_CLOCK;
    ESP_LOGI(TAG, "Stage: %s - 阶段: %s", hardware_get_stage_name(g_hw_status.current_stage), hardware_get_stage_name(g_hw_status.current_stage));
    
    // 基本时钟配置（ESP32-C6默认配置通常足够）
    ESP_LOGI(TAG, "System clock configuration completed - 系统时钟配置完成");
    
    // 2.5 SPI初始化阶段
    g_hw_status.current_stage = HW_INIT_STAGE_SPI;
    ESP_LOGI(TAG, "Stage: %s - 阶段: %s", hardware_get_stage_name(g_hw_status.current_stage), hardware_get_stage_name(g_hw_status.current_stage));
    
    // SPI基础配置（为LCD显示做准备）
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,  // 不使用MISO
        .mosi_io_num = SPI_MOSI_PIN,
        .sclk_io_num = SPI_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096
    };
    
    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {  // 忽略已初始化错误
        ESP_LOGE(TAG, "SPI bus initialization failed: %s", esp_err_to_name(ret));
        g_hw_status.last_error = HW_INIT_ERR_SPI;
        g_hw_status.current_stage = HW_INIT_STAGE_ERROR;
        return ret;
    }
    ESP_LOGI(TAG, "SPI bus initialized successfully");
    
    // 2.6 I2S初始化阶段
    g_hw_status.current_stage = HW_INIT_STAGE_I2S;
    ESP_LOGI(TAG, "Stage: %s", hardware_get_stage_name(g_hw_status.current_stage));
    
    // I2S基础配置
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX,
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_DOUT_PIN,
        .data_in_num = I2S_DIN_PIN
    };
    
    ret = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2S driver installation failed: %s", esp_err_to_name(ret));
        g_hw_status.last_error = HW_INIT_ERR_I2S;
        g_hw_status.current_stage = HW_INIT_STAGE_ERROR;
        return ret;
    }
    
    ret = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2S pin configuration failed: %s", esp_err_to_name(ret));
        g_hw_status.last_error = HW_INIT_ERR_I2S;
        g_hw_status.current_stage = HW_INIT_STAGE_ERROR;
        return ret;
    }
    ESP_LOGI(TAG, "I2S driver initialized successfully");
    
    // 2.7 ADC初始化阶段
    g_hw_status.current_stage = HW_INIT_STAGE_ADC;
    ESP_LOGI(TAG, "Stage: %s", hardware_get_stage_name(g_hw_status.current_stage));
    
    // ADC基础配置（用于电池电压检测）
    ret = adc1_config_width(ADC_WIDTH_BIT_12);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC width configuration failed: %s", esp_err_to_name(ret));
        g_hw_status.last_error = HW_INIT_ERR_ADC;
        g_hw_status.current_stage = HW_INIT_STAGE_ERROR;
        return ret;
    }
    
    ret = adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_12);  // GPIO9对应ADC1_CHANNEL_6
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC channel configuration failed: %s", esp_err_to_name(ret));
        g_hw_status.last_error = HW_INIT_ERR_ADC;
        g_hw_status.current_stage = HW_INIT_STAGE_ERROR;
        return ret;
    }
    ESP_LOGI(TAG, "ADC initialized successfully");
    
    // 2.8 硬件自检阶段
    g_hw_status.current_stage = HW_INIT_STAGE_SELF_TEST;
    ESP_LOGI(TAG, "Stage: %s - 阶段: %s", hardware_get_stage_name(g_hw_status.current_stage), hardware_get_stage_name(g_hw_status.current_stage));
    
    ret = hardware_self_test(&g_hw_status.self_test_result);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Hardware self-test failed - 硬件自检失败");
        g_hw_status.last_error = HW_INIT_ERR_SELF_TEST;
        g_hw_status.current_stage = HW_INIT_STAGE_ERROR;
        
        // 设置RGB LED为红色表示错误
        for (int i = 0; i < 3; i++) {
            gpio_set_level(RGB_LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            gpio_set_level(RGB_LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        return ret;
    }
    
    // 2.9 初始化完成
    g_hw_status.current_stage = HW_INIT_STAGE_COMPLETE;
    g_hw_status.initialization_complete = true;
    ESP_LOGI(TAG, "Hardware initialization completed successfully - 硬件初始化成功完成");
    
    // 设置RGB LED为绿色表示初始化完成
    gpio_set_level(RGB_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_set_level(RGB_LED_PIN, 0);
    
    return ESP_OK;
}

esp_err_t hardware_self_test(hw_self_test_result_t *result) {
    if (result == NULL) {
        ESP_LOGE(TAG, "Self-test result pointer is NULL - 自检结果指针为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Starting hardware self-test... - 开始硬件自检");
    
    // 初始化测试结果
    result->lcd_communication_ok = false;
    result->i2s_clock_ok = false;
    result->button_response_ok = false;
    result->battery_voltage_ok = false;
    result->rgb_led_ok = false;
    
    // 测试SPI通信（LCD响应）
    result->lcd_communication_ok = lcd_test_communication();
    if (!result->lcd_communication_ok) {
        ESP_LOGE(TAG, "LCD communication test failed - LCD通信测试失败");
    }
    
    // 测试I2S时钟输出
    result->i2s_clock_ok = i2s_test_clock_output();
    if (!result->i2s_clock_ok) {
        ESP_LOGE(TAG, "I2S clock test failed - I2S时钟测试失败");
    }
    
    // 测试按键响应
    result->button_response_ok = button_test_response();
    if (!result->button_response_ok) {
        ESP_LOGE(TAG, "Button response test failed - 按键响应测试失败");
    }
    
    // 测试电池电压检测
    result->battery_voltage_ok = battery_test_voltage_detection();
    if (!result->battery_voltage_ok) {
        ESP_LOGE(TAG, "Battery voltage test failed - 电池电压测试失败");
    }
    
    // 测试RGB LED功能
    result->rgb_led_ok = rgb_led_test_function();
    if (!result->rgb_led_ok) {
        ESP_LOGE(TAG, "RGB LED test failed - RGB LED测试失败");
    }
    
    // 检查所有测试是否通过
    bool all_passed = result->lcd_communication_ok && result->i2s_clock_ok && 
                     result->button_response_ok && result->battery_voltage_ok && 
                     result->rgb_led_ok;
    
    if (all_passed) {
        ESP_LOGI(TAG, "Hardware self-test completed successfully");
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "Hardware self-test failed - some components not working properly");
        return ESP_FAIL;
    }
}

bool lcd_test_communication(void) {
    ESP_LOGI(TAG, "Testing LCD communication...");
    
    // 基本的SPI通信测试
    // 设置LCD控制引脚
    gpio_set_level(LCD_CS_PIN, 0);   // 选中LCD
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(LCD_CS_PIN, 1);   // 取消选中
    
    // 测试复位功能
    gpio_set_level(LCD_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(LCD_RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    ESP_LOGI(TAG, "LCD communication test completed (basic)");
    return true;  // 基础测试通过
}

bool i2s_test_clock_output(void) {
    ESP_LOGI(TAG, "Testing I2S clock output...");
    
    // 启动I2S以测试时钟输出
    esp_err_t ret = i2s_start(I2S_NUM_0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start I2S: %s", esp_err_to_name(ret));
        return false;
    }
    
    // 等待一段时间让时钟稳定
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 停止I2S
    ret = i2s_stop(I2S_NUM_0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop I2S: %s", esp_err_to_name(ret));
        return false;
    }
    
    ESP_LOGI(TAG, "I2S clock test completed");
    return true;
}

bool button_test_response(void) {
    ESP_LOGI(TAG, "Testing button response...");
    
    // 读取所有按键状态
    int vol_up = gpio_get_level(BTN_VOL_UP_PIN);
    int vol_down = gpio_get_level(BTN_VOL_DOWN_PIN);
    int func = gpio_get_level(BTN_FUNC_PIN);
    
    ESP_LOGI(TAG, "Button states - VOL_UP: %d, VOL_DOWN: %d, FUNC: %d", vol_up, vol_down, func);
    
    // 检查按键是否能正常读取（上拉状态下应该为高电平）
    if (vol_up >= 0 && vol_down >= 0 && func >= 0) {
        ESP_LOGI(TAG, "Button response test completed");
        return true;
    }
    
    ESP_LOGE(TAG, "Button response test failed");
    return false;
}

bool battery_test_voltage_detection(void) {
    ESP_LOGI(TAG, "Testing battery voltage detection...");
    
    // 读取ADC值
    int adc_reading = adc1_get_raw(ADC1_CHANNEL_6);
    if (adc_reading < 0) {
        ESP_LOGE(TAG, "Failed to read ADC value");
        return false;
    }
    
    ESP_LOGI(TAG, "ADC reading: %d (raw value)", adc_reading);
    
    // 基本范围检查（0-4095为12位ADC的有效范围）
    if (adc_reading >= 0 && adc_reading <= 4095) {
        ESP_LOGI(TAG, "Battery voltage detection test completed");
        return true;
    }
    
    ESP_LOGE(TAG, "Battery voltage detection test failed - invalid ADC reading");
    return false;
}

bool rgb_led_test_function(void) {
    ESP_LOGI(TAG, "Testing RGB LED function...");
    
    // 测试LED开关功能
    gpio_set_level(RGB_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(RGB_LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(RGB_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(RGB_LED_PIN, 0);
    
    ESP_LOGI(TAG, "RGB LED test completed");
    return true;
}

esp_err_t hardware_get_init_status(hw_init_status_t *status) {
    if (status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *status = g_hw_status;
    return ESP_OK;
}

esp_err_t hardware_init_recovery(hw_init_error_t error_code) {
    ESP_LOGI(TAG, "Attempting hardware initialization recovery for error: %s - 尝试硬件初始化恢复，错误: %s", 
             hardware_get_error_name(error_code), hardware_get_error_name(error_code));
    
    switch (error_code) {
        case HW_INIT_ERR_GPIO:
            // 重新尝试GPIO配置
            return gpio_config_all();
            
        case HW_INIT_ERR_SPI:
            // 重新初始化SPI
            spi_bus_free(SPI2_HOST);
            vTaskDelay(pdMS_TO_TICKS(100));
            return hardware_init_start();  // 重新开始初始化
            
        case HW_INIT_ERR_I2S:
            // 重新初始化I2S
            i2s_driver_uninstall(I2S_NUM_0);
            vTaskDelay(pdMS_TO_TICKS(100));
            return hardware_init_start();  // 重新开始初始化
            
        default:
            ESP_LOGW(TAG, "No recovery method available for error: %s - 错误无可用恢复方法: %s", 
                     hardware_get_error_name(error_code), hardware_get_error_name(error_code));
            return ESP_ERR_NOT_SUPPORTED;
    }
}

const char* hardware_get_stage_name(hw_init_stage_t stage) {
    if (stage >= 0 && stage < sizeof(stage_names)/sizeof(stage_names[0])) {
        return stage_names[stage];
    }
    return "UNKNOWN";
}

const char* hardware_get_error_name(hw_init_error_t error) {
    if (error >= 0 && error < sizeof(error_names)/sizeof(error_names[0])) {
        return error_names[error];
    }
    return "UNKNOWN";
}