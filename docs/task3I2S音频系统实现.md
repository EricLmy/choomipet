# 任务3: I2S音频系统实现

- [ ] 3. I2S音频系统实现
  - [ ] 3.1 实现I2S驱动基础框架
    - 编写I2S控制器配置和初始化代码
    - 实现I2S时钟配置和同步机制
    - 创建音频数据缓冲区管理
    - _需求: 4.2, 5.2_
    - _对应主程序: audio_processing_task()中的音频处理流程_

    **开发前提条件：**
    - ✅ 任务2硬件初始化和GPIO配置已完成
    - ✅ INMP441麦克风模块已连接到开发板
    - ✅ MAX98357A功放模块已连接到开发板
    - ✅ 示波器或逻辑分析仪（用于I2S信号测试）

    **必需准备材料：**
    - 📋 I2S协议规范文档
    - 📋 INMP441数据手册
    - 📋 MAX98357A数据手册
    - 📋 音频缓冲区设计方案
    - 📋 I2S时钟配置参数表
    - 📋 DMA配置参数表

    **具体实现内容（基于主程序设计）：**
    ```c
    // components/audio/i2s_driver.h
    #define AUDIO_SAMPLE_RATE    16000
    #define AUDIO_BITS_PER_SAMPLE 16
    #define AUDIO_BUFFER_SIZE    1024
    #define I2S_NUM_0           I2S_NUM_0  // 麦克风
    #define I2S_NUM_1           I2S_NUM_1  // 扬声器

    typedef struct {
        i2s_port_t port;
        uint32_t sample_rate;
        uint8_t bits_per_sample;
        i2s_channel_fmt_t channel_format;
        size_t buffer_size;
        QueueHandle_t audio_queue;
    } i2s_config_choomi_t;

    esp_err_t i2s_driver_init(void);
    
    // components/audio/i2s_driver.c
    esp_err_t i2s_driver_init(void) {
        // I2S0配置（麦克风输入）
        i2s_config_t i2s_config_mic = {
            .mode = I2S_MODE_MASTER | I2S_MODE_RX,
            .sample_rate = AUDIO_SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
            .dma_buf_count = 4,
            .dma_buf_len = 1024,
            .use_apll = false,
            .tx_desc_auto_clear = false,
            .fixed_mclk = 0
        };

        i2s_pin_config_t pin_config_mic = {
            .bck_io_num = I2S_BCLK_PIN,
            .ws_io_num = I2S_WS_PIN,
            .data_out_num = I2S_PIN_NO_CHANGE,
            .data_in_num = I2S_DIN_PIN
        };

        ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &i2s_config_mic, 0, NULL));
        ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_0, &pin_config_mic));

        // I2S1配置（扬声器输出）
        i2s_config_t i2s_config_spk = {
            .mode = I2S_MODE_MASTER | I2S_MODE_TX,
            .sample_rate = AUDIO_SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
            .dma_buf_count = 4,
            .dma_buf_len = 1024,
            .use_apll = false,
            .tx_desc_auto_clear = true,
            .fixed_mclk = 0
        };

        i2s_pin_config_t pin_config_spk = {
            .bck_io_num = I2S_BCLK_PIN,
            .ws_io_num = I2S_WS_PIN,
            .data_out_num = I2S_DOUT_PIN,
            .data_in_num = I2S_PIN_NO_CHANGE
        };

        ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_1, &i2s_config_spk, 0, NULL));
        ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_1, &pin_config_spk));

        ESP_LOGI(TAG, "I2S driver initialized");
        return ESP_OK;
    }
    ```

    **详细任务内容：**
    - 创建i2s_driver.h定义I2S配置结构体和接口函数
    - 实现i2s_driver.c包含I2S初始化和配置函数
    - 配置I2S0用于麦克风输入，I2S1用于扬声器输出
    - 设置I2S时钟：16kHz采样率，16bit位深，单声道
    - 实现音频缓冲区管理：环形缓冲区，支持DMA传输
    - 添加I2S错误处理和状态监控
    - 创建音频数据格式转换函数（PCM/WAV）

    **验收标准：**
    - I2S控制器能够正确初始化并输出16kHz时钟信号
    - 音频缓冲区能够稳定工作，无数据丢失或溢出
    - I2S DMA传输正常，CPU占用率<20%
    - 时钟同步准确，音频采样率误差<1%
    - 错误处理机制能够检测并恢复I2S通信异常
    - 系统启动时输出"I2S driver initialized"日志

  - [ ] 3.2 实现INMP441麦克风驱动
    - 编写麦克风初始化和配置代码
    - 实现16kHz采样率的音频采集功能
    - 添加音频数据格式转换和缓存机制
    - 创建录音控制接口（开始/停止/暂停）
    - _需求: 4.1, 4.2, 4.3, 4.4_

    **开发前提条件：**
    - ✅ 任务3.1 I2S驱动基础框架已完成
    - ✅ INMP441麦克风模块已正确连接并测试
    - ✅ 音频测试设备（扬声器或耳机）

    **必需准备材料：**
    - 📋 INMP441详细技术规格书
    - 📋 音频采集测试用例
    - 📋 音频格式转换算法文档
    - 📋 录音控制状态机设计
    - 📋 音频质量测试标准

    **详细任务内容：**
    - 创建mic_driver.h定义麦克风配置和控制接口
    - 实现mic_driver.c包含INMP441初始化和控制函数
    - 配置I2S接收模式：GPIO18(DIN)、GPIO4(BCLK)、GPIO5(WS)
    - 实现录音控制：mic_start_recording()、mic_stop_recording()
    - 添加音频数据预处理：增益控制、噪音抑制
    - 实现录音状态监控和音量检测
    - 创建录音数据的WebSocket上传接口

    **验收标准：**
    - 麦克风能够正常采集16kHz/16bit音频数据
    - 录音功能响应时间<100ms，停止录音立即生效
    - 音频质量清晰，信噪比≥60dB
    - 录音过程中LCD显示实时音量指示
    - 录音数据能够正确格式化并上传到服务器
    - 麦克风故障时能够检测并提示用户

  - [ ] 3.3 实现MAX98357A扬声器驱动
    - 编写功放模块初始化和配置代码
    - 实现音频播放功能和音量控制
    - 添加播放队列管理和优先级控制
    - 创建播放状态监控和错误恢复
    - _需求: 5.1, 5.2, 5.3, 5.4_

    **开发前提条件：**
    - ✅ 任务3.1 I2S驱动基础框架已完成
    - ✅ MAX98357A功放模块已正确连接并测试
    - ✅ 8Ω/3W扬声器已连接到功放输出
    - ✅ 音频测试文件（WAV/PCM格式）

    **必需准备材料：**
    - 📋 MAX98357A详细技术规格书
    - 📋 音频播放测试用例
    - 📋 音量控制算法文档
    - 📋 播放队列设计方案
    - 📋 音频格式支持列表

    **详细任务内容：**
    - 创建speaker_driver.h定义扬声器配置和播放接口
    - 实现speaker_driver.c包含MAX98357A初始化和播放函数
    - 配置I2S发送模式：GPIO19(DOUT)、GPIO4(BCLK)、GPIO5(WS)
    - 实现音量控制：256级数字音量调节
    - 添加播放队列：支持多音频文件排队播放
    - 实现播放优先级：系统提示音>语音回复>背景音乐
    - 创建播放状态监控和错误恢复机制

    **验收标准：**
    - 扬声器能够正常播放16kHz/16bit音频数据
    - 音量调节平滑，256级调节无明显跳跃
    - 播放队列工作正常，支持播放/暂停/停止控制
    - 音频播放延迟<50ms，音质清晰无失真
    - 播放过程中LCD显示音量条和播放状态
    - 扬声器故障时能够检测并提示用户

  - [ ] 3.4 实现音频系统集成测试
    - 编写音频采集播放的端到端测试
    - 验证音频延迟和同步性能
    - 测试音频质量和噪音抑制效果
    - _需求: 4.3, 4.7, 5.7_

    **开发前提条件：**
    - ✅ 任务3.2 INMP441麦克风驱动已完成
    - ✅ 任务3.3 MAX98357A扬声器驱动已完成
    - ✅ 音频测试设备（信号发生器、频谱分析仪）
    - ✅ 标准测试音频文件

    **必需准备材料：**
    - 📋 音频测试标准和规范
    - 📋 端到端测试用例设计
    - 📋 音频质量评估标准
    - 📋 性能基准测试方案
    - 📋 长期稳定性测试计划

    **详细任务内容：**
    - 创建audio_test.c包含音频系统测试用例
    - 实现回音测试：录音后立即播放，测试延迟
    - 添加音频质量测试：频率响应、失真度测量
    - 实现长时间稳定性测试：连续录音播放24小时
    - 添加异常情况测试：I2S错误、缓冲区溢出等
    - 创建音频性能报告和优化建议
    - 实现自动化测试脚本和结果分析

    **验收标准：**
    - 端到端音频延迟<100ms（录音到播放）
    - 音频质量测试通过：THD+N<1%，频响20Hz-8kHz
    - 长时间测试稳定：24小时无音频中断或质量下降
    - 异常恢复测试通过：能够从各种错误状态自动恢复
    - 测试报告完整：包含性能指标和优化建议
    - 自动化测试覆盖率≥90%