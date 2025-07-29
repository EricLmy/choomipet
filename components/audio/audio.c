#include "audio.h"
#include "esp_log.h"
#include "driver/i2s.h"
#include "gpio_config.h"

static const char *TAG = "AUDIO";
static bool audio_initialized = false;

esp_err_t audio_init(void) {
    ESP_LOGI(TAG, "Audio system initializing... - 音频系统初始化中");
    
    if (audio_initialized) {
        ESP_LOGW(TAG, "Audio system already initialized - 音频系统已初始化");
        return ESP_OK;
    }
    
    // 音频系统初始化在hardware_init.c中完成
    // 这里只需要标记为已初始化
    audio_initialized = true;
    ESP_LOGI(TAG, "Audio system initialized - 音频系统初始化完成");
    return ESP_OK;
}

esp_err_t audio_deinit(void) {
    ESP_LOGI(TAG, "Audio system deinitializing... - 音频系统去初始化中");
    
    if (!audio_initialized) {
        ESP_LOGW(TAG, "Audio system not initialized - 音频系统未初始化");
        return ESP_OK;
    }
    
    // 卸载I2S驱动
    esp_err_t ret = i2s_driver_uninstall(I2S_NUM_0);  // 麦克风
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to uninstall microphone I2S driver: %s", esp_err_to_name(ret));
    }
    
    ret = i2s_driver_uninstall(I2S_NUM_1);  // 扬声器
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to uninstall speaker I2S driver: %s", esp_err_to_name(ret));
    }
    
    audio_initialized = false;
    ESP_LOGI(TAG, "Audio system deinitialized - 音频系统去初始化完成");
    return ESP_OK;
}

esp_err_t audio_start_recording(void) {
    ESP_LOGI(TAG, "Starting audio recording... - 开始录音");
    
    if (!audio_initialized) {
        ESP_LOGE(TAG, "Audio system not initialized - 音频系统未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    // 启动麦克风I2S接收
    esp_err_t ret = i2s_start(I2S_NUM_0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start microphone I2S: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Audio recording started - 录音已开始");
    return ESP_OK;
}

esp_err_t audio_stop_recording(void) {
    ESP_LOGI(TAG, "Stopping audio recording... - 停止录音");
    
    if (!audio_initialized) {
        ESP_LOGE(TAG, "Audio system not initialized - 音频系统未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    // 停止麦克风I2S接收
    esp_err_t ret = i2s_stop(I2S_NUM_0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop microphone I2S: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Audio recording stopped - 录音已停止");
    return ESP_OK;
}

esp_err_t audio_start_playback(void) {
    ESP_LOGI(TAG, "Starting audio playback... - 开始播放");
    
    if (!audio_initialized) {
        ESP_LOGE(TAG, "Audio system not initialized - 音频系统未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    // 启动扬声器I2S发送
    esp_err_t ret = i2s_start(I2S_NUM_1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start speaker I2S: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Audio playback started - 播放已开始");
    return ESP_OK;
}

esp_err_t audio_stop_playback(void) {
    ESP_LOGI(TAG, "Stopping audio playback... - 停止播放");
    
    if (!audio_initialized) {
        ESP_LOGE(TAG, "Audio system not initialized - 音频系统未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    // 停止扬声器I2S发送
    esp_err_t ret = i2s_stop(I2S_NUM_1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop speaker I2S: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Audio playback stopped - 播放已停止");
    return ESP_OK;
}