# 任务2: 硬件初始化和GPIO配置 -已完成

- [x] 2. 硬件初始化和GPIO配置
  - [x] 2.1 实现GPIO引脚配置模块
    - 编写gpio_config.c实现所有引脚的初始化配置
    - 创建引脚映射表和配置验证功能
    - 实现引脚状态检测和错误处理
    - _需求: 1.1, 1.2, 1.3, 1.4_
    - _对应主程序: hardware_init_start()中的GPIO配置阶段_

    **开发前提条件：**
    - ✅ 任务1项目基础架构已完成
    - ✅ ESP32-C6引脚分配表已确认
    - ✅ 硬件连接原理图已准备
    - ✅ 万用表或示波器（用于引脚测试）

    **必需准备材料：**
    - 📋 ESP32-C6引脚功能对照表
    - 📋 硬件连接示意图
    - 📋 GPIO配置参数表
    - 📋 引脚测试用例清单
    - 📋 错误码定义文档

    **具体实现内容（基于主程序设计）：**
    ```c
    // components/hardware/gpio_config.h
    #define I2S_BCLK_PIN    4
    #define I2S_WS_PIN      5
    #define I2S_DIN_PIN     18
    #define I2S_DOUT_PIN    19
    #define SPI_CLK_PIN     7
    #define SPI_MOSI_PIN    6
    #define LCD_CS_PIN      14
    #define LCD_DC_PIN      15
    #define LCD_RST_PIN     21
    #define LCD_BL_PIN      22
    #define BTN_VOL_UP_PIN  1
    #define BTN_VOL_DOWN_PIN 2
    #define BTN_FUNC_PIN    3
    #define RGB_LED_PIN     8
    #define BAT_ADC_PIN     9

    esp_err_t gpio_config_all(void);
    esp_err_t gpio_test_all_pins(void);
    
    // components/hardware/gpio_config.c
    esp_err_t gpio_config_all(void) {
        // 配置I2S引脚
        gpio_config_t i2s_config = {
            .pin_bit_mask = (1ULL << I2S_BCLK_PIN) | (1ULL << I2S_WS_PIN) | 
                           (1ULL << I2S_DOUT_PIN),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        ESP_ERROR_CHECK(gpio_config(&i2s_config));
        
        // 配置按键引脚（内部上拉，中断使能）
        gpio_config_t btn_config = {
            .pin_bit_mask = (1ULL << BTN_VOL_UP_PIN) | (1ULL << BTN_VOL_DOWN_PIN) | 
                           (1ULL << BTN_FUNC_PIN),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_NEGEDGE
        };
        ESP_ERROR_CHECK(gpio_config(&btn_config));
        
        // 配置RGB LED引脚
        gpio_config_t rgb_config = {
            .pin_bit_mask = (1ULL << RGB_LED_PIN),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        ESP_ERROR_CHECK(gpio_config(&rgb_config));
        
        ESP_LOGI(TAG, "GPIO initialization completed");
        return ESP_OK;
    }
    ```

    **详细任务内容：**
    - 创建gpio_config.h定义所有GPIO引脚宏定义和配置结构体
    - 实现gpio_config.c包含引脚初始化函数gpio_init_all()
    - 配置I2S引脚：GPIO4(BCLK)、GPIO5(WS)、GPIO18(DIN)、GPIO19(DOUT)
    - 配置SPI引脚：GPIO6(MOSI)、GPIO7(CLK)、GPIO14(CS)、GPIO15(DC)、GPIO21(RST)、GPIO22(BL)
    - 配置按键引脚：GPIO1、GPIO2、GPIO3（内部上拉，下降沿中断）
    - 配置RGB LED引脚：GPIO8（输出模式）
    - 配置ADC引脚：GPIO9（电池电压检测）
    - 实现引脚状态读取和验证函数

    **验收标准：**
    - 所有GPIO引脚能够正确初始化为指定模式（输入/输出/复用）
    - 引脚映射表与硬件设计文档完全一致
    - 按键引脚能够正确触发中断（通过万用表测试）
    - SPI和I2S引脚能够输出正确的时钟和数据信号
    - GPIO状态检测函数能够正确返回引脚电平状态
    - 系统启动时输出"GPIO initialization completed"日志

  - [x] 2.2 实现硬件初始化管理器
    - 编写hardware_init.c统一管理所有硬件模块初始化
    - 实现初始化顺序控制和依赖管理
    - 添加硬件自检功能和故障检测
    - _需求: 1.1, 1.5_
    - _对应主程序: hardware_init_start()函数和硬件自检阶段_

    **开发前提条件：**
    - ✅ 任务2.1 GPIO引脚配置模块已完成
    - ✅ 硬件模块依赖关系图已准备
    - ✅ 自检测试用例已设计

    **必需准备材料：**
    - 📋 硬件初始化流程图
    - 📋 模块依赖关系表
    - 📋 自检测试清单
    - 📋 故障代码定义表
    - 📋 恢复策略文档

    **具体实现内容（基于主程序设计）：**
    ```c
    // components/hardware/hardware_init.h
    typedef enum {
        HW_INIT_STAGE_GPIO,
        HW_INIT_STAGE_CLOCK,
        HW_INIT_STAGE_SPI,
        HW_INIT_STAGE_I2S,
        HW_INIT_STAGE_ADC,
        HW_INIT_STAGE_SELF_TEST,
        HW_INIT_STAGE_COMPLETE
    } hw_init_stage_t;

    esp_err_t hardware_init_start(void);
    esp_err_t hardware_self_test(void);
    
    // components/hardware/hardware_init.c
    esp_err_t hardware_init_start(void) {
        // 2.1 RGB LED优先初始化（用于状态指示）
        rgb_led_set_color(RGB_COLOR_WHITE);  // 白色表示启动中
        
        // 2.2 LCD初始化并显示启动画面
        ESP_ERROR_CHECK(lcd_init(&lcd_config));
        lcd_display_boot_screen();
        
        // 2.3 GPIO引脚配置
        ESP_ERROR_CHECK(gpio_config_all());
        
        // 2.4 I2S音频系统初始化
        ESP_ERROR_CHECK(i2s_driver_init());
        
        // 2.5 按键系统初始化
        ESP_ERROR_CHECK(button_init(button_configs, BTN_MAX));
        
        // 2.6 电源管理初始化
        ESP_ERROR_CHECK(power_manager_init());
        
        // 2.7 NVS存储初始化
        ESP_ERROR_CHECK(nvs_init());
        
        // 2.8 硬件自检
        if (hardware_self_test() != ESP_OK) {
            rgb_led_set_color(RGB_COLOR_RED);
            lcd_display_error_screen("Hardware Test Failed");
            return ESP_FAIL;
        }
        
        // 2.9 进入任务创建阶段
        return create_system_tasks();
    }

    esp_err_t hardware_self_test(void) {
        ESP_LOGI(TAG, "Starting hardware self-test...");
        
        // 测试SPI通信（LCD响应）
        if (!lcd_test_communication()) {
            ESP_LOGE(TAG, "LCD communication test failed");
            return ESP_FAIL;
        }
        
        // 测试I2S时钟输出
        if (!i2s_test_clock_output()) {
            ESP_LOGE(TAG, "I2S clock test failed");
            return ESP_FAIL;
        }
        
        // 测试按键响应
        if (!button_test_response()) {
            ESP_LOGE(TAG, "Button response test failed");
            return ESP_FAIL;
        }
        
        // 测试电池电压检测
        if (!battery_test_voltage_detection()) {
            ESP_LOGE(TAG, "Battery voltage test failed");
            return ESP_FAIL;
        }
        
        ESP_LOGI(TAG, "Hardware self-test completed successfully");
        return ESP_OK;
    }
    ```

    **详细任务内容：**
    - 创建hardware_init.h定义初始化状态枚举和错误码
    - 实现hardware_init.c包含硬件初始化管理器
    - 设计初始化顺序：GPIO → 时钟 → SPI → I2S → ADC → 外设检测
    - 实现硬件自检功能：检测SPI通信、I2S时钟、按键响应、电池电压
    - 添加初始化失败的错误处理和恢复机制
    - 实现硬件状态监控和故障检测
    - 创建硬件初始化状态的LCD显示和RGB LED指示

    **验收标准：**
    - 硬件初始化按正确顺序执行，无依赖冲突
    - 自检功能能够检测出硬件连接异常（如LCD未连接、麦克风故障）
    - 初始化失败时能够显示具体错误信息和错误码
    - 系统能够从非致命硬件错误中恢复并继续运行
    - 硬件状态能够通过串口日志和LCD界面查看
    - 初始化完成时RGB LED显示绿色，失败时显示红色