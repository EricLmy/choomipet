#include "ws2812b_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"
#include <string.h>

static const char *TAG = "WS2812B";

// LED颜色缓冲区
static ws2812b_color_t *led_buffer = NULL;
static ws2812b_config_t *global_config = NULL;

// RMT编码器结构体
typedef struct {
    rmt_encoder_t base;
    rmt_encoder_t *bytes_encoder;
    rmt_encoder_t *copy_encoder;
    int state;
    rmt_symbol_word_t reset_code;
} rmt_led_strip_encoder_t;

// WS2812B编码器状态
enum {
    RMT_LED_STRIP_ENCODER_STATE_RESET = 0,
    RMT_LED_STRIP_ENCODER_STATE_DATA,
    RMT_LED_STRIP_ENCODER_STATE_DONE
};

// 编码函数
static size_t rmt_encode_led_strip(rmt_encoder_t *encoder, rmt_channel_handle_t channel,
                                   const void *primary_data, size_t data_size,
                                   rmt_encode_state_t *ret_state)
{
    rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
    rmt_encoder_handle_t bytes_encoder = led_encoder->bytes_encoder;
    rmt_encoder_handle_t copy_encoder = led_encoder->copy_encoder;
    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    rmt_encode_state_t state = RMT_ENCODING_RESET;
    size_t encoded_symbols = 0;
    
    switch (led_encoder->state) {
    case RMT_LED_STRIP_ENCODER_STATE_RESET:
        encoded_symbols += copy_encoder->encode(copy_encoder, channel, &led_encoder->reset_code,
                                                sizeof(led_encoder->reset_code), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            led_encoder->state = RMT_LED_STRIP_ENCODER_STATE_DATA;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
        // fall-through
    case RMT_LED_STRIP_ENCODER_STATE_DATA:
        encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            led_encoder->state = RMT_LED_STRIP_ENCODER_STATE_DONE;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
        // fall-through
    case RMT_LED_STRIP_ENCODER_STATE_DONE:
        state |= RMT_ENCODING_COMPLETE;
        break;
    }
out:
    *ret_state = state;
    return encoded_symbols;
}

// 编码器重置函数
static esp_err_t rmt_led_strip_encoder_reset(rmt_encoder_t *encoder)
{
    rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
    rmt_encoder_reset(led_encoder->bytes_encoder);
    rmt_encoder_reset(led_encoder->copy_encoder);
    led_encoder->state = RMT_LED_STRIP_ENCODER_STATE_RESET;
    return ESP_OK;
}

// 编码器删除函数
static esp_err_t rmt_led_strip_encoder_del(rmt_encoder_t *encoder)
{
    rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
    rmt_del_encoder(led_encoder->bytes_encoder);
    rmt_del_encoder(led_encoder->copy_encoder);
    free(led_encoder);
    return ESP_OK;
}

// 创建WS2812B编码器
static esp_err_t rmt_new_led_strip_encoder(rmt_encoder_handle_t *ret_encoder)
{
    esp_err_t ret = ESP_OK;
    rmt_led_strip_encoder_t *led_encoder = NULL;
    
    led_encoder = calloc(1, sizeof(rmt_led_strip_encoder_t));
    if (!led_encoder) {
        ESP_LOGE(TAG, "No memory for LED strip encoder");
        return ESP_ERR_NO_MEM;
    }
    
    led_encoder->base.encode = rmt_encode_led_strip;
    led_encoder->base.del = rmt_led_strip_encoder_del;
    led_encoder->base.reset = rmt_led_strip_encoder_reset;
    
    // 创建字节编码器
    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = {
            .level0 = 1,
            .duration0 = WS2812B_T0H_TICKS,
            .level1 = 0,
            .duration1 = WS2812B_T0L_TICKS,
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = WS2812B_T1H_TICKS,
            .level1 = 0,
            .duration1 = WS2812B_T1L_TICKS,
        },
        .flags.msb_first = 1
    };
    ret = rmt_new_bytes_encoder(&bytes_encoder_config, &led_encoder->bytes_encoder);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Create bytes encoder failed");
        goto err;
    }
    
    // 创建复制编码器
    rmt_copy_encoder_config_t copy_encoder_config = {};
    ret = rmt_new_copy_encoder(&copy_encoder_config, &led_encoder->copy_encoder);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Create copy encoder failed");
        goto err;
    }
    
    // 设置复位码
    led_encoder->reset_code = (rmt_symbol_word_t) {
        .level0 = 0,
        .duration0 = WS2812B_RESET_TICKS,
        .level1 = 0,
        .duration1 = WS2812B_RESET_TICKS,
    };
    
    *ret_encoder = &led_encoder->base;
    return ESP_OK;
    
err:
    if (led_encoder) {
        if (led_encoder->bytes_encoder) {
            rmt_del_encoder(led_encoder->bytes_encoder);
        }
        if (led_encoder->copy_encoder) {
            rmt_del_encoder(led_encoder->copy_encoder);
        }
        free(led_encoder);
    }
    return ret;
}

esp_err_t ws2812b_init(ws2812b_config_t *config)
{
    esp_err_t ret = ESP_OK;
    
    if (!config) {
        ESP_LOGE(TAG, "Config is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    // 分配LED缓冲区
    led_buffer = calloc(config->led_count, sizeof(ws2812b_color_t));
    if (!led_buffer) {
        ESP_LOGE(TAG, "No memory for LED buffer");
        return ESP_ERR_NO_MEM;
    }
    
    // 配置RMT发送通道
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = config->gpio_pin,
        .mem_block_symbols = 64,
        .resolution_hz = WS2812B_RMT_RESOLUTION,
        .trans_queue_depth = 4,
    };
    ret = rmt_new_tx_channel(&tx_chan_config, &config->rmt_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Create RMT TX channel failed");
        goto err;
    }
    
    // 创建LED编码器
    ret = rmt_new_led_strip_encoder(&config->rmt_encoder);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Create LED encoder failed");
        goto err;
    }
    
    // 启用RMT通道
    ret = rmt_enable(config->rmt_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Enable RMT channel failed");
        goto err;
    }
    
    global_config = config;
    ESP_LOGI(TAG, "WS2812B driver initialized on GPIO%d with %d LEDs", config->gpio_pin, config->led_count);
    return ESP_OK;
    
err:
    if (led_buffer) {
        free(led_buffer);
        led_buffer = NULL;
    }
    if (config->rmt_channel) {
        rmt_del_channel(config->rmt_channel);
    }
    if (config->rmt_encoder) {
        rmt_del_encoder(config->rmt_encoder);
    }
    return ret;
}

esp_err_t ws2812b_set_color(ws2812b_config_t *config, uint16_t led_index, ws2812b_color_t color)
{
    if (!config || !led_buffer) {
        ESP_LOGE(TAG, "WS2812B not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (led_index >= config->led_count) {
        ESP_LOGE(TAG, "LED index out of range");
        return ESP_ERR_INVALID_ARG;
    }
    
    led_buffer[led_index] = color;
    return ESP_OK;
}

esp_err_t ws2812b_set_all_color(ws2812b_config_t *config, ws2812b_color_t color)
{
    if (!config || !led_buffer) {
        ESP_LOGE(TAG, "WS2812B not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    for (uint16_t i = 0; i < config->led_count; i++) {
        led_buffer[i] = color;
    }
    
    return ESP_OK;
}

esp_err_t ws2812b_clear(ws2812b_config_t *config)
{
    ws2812b_color_t black = {0, 0, 0};
    return ws2812b_set_all_color(config, black);
}

esp_err_t ws2812b_refresh(ws2812b_config_t *config)
{
    if (!config || !led_buffer) {
        ESP_LOGE(TAG, "WS2812B not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    // 准备发送数据（GRB格式）
    uint8_t *pixel_data = malloc(config->led_count * 3);
    if (!pixel_data) {
        ESP_LOGE(TAG, "No memory for pixel data");
        return ESP_ERR_NO_MEM;
    }
    
    for (uint16_t i = 0; i < config->led_count; i++) {
        pixel_data[i * 3 + 0] = led_buffer[i].r;  // Red (交换到第一位)
        pixel_data[i * 3 + 1] = led_buffer[i].g;  // Green (交换到第二位)
        pixel_data[i * 3 + 2] = led_buffer[i].b;  // Blue
    }
    
    // 发送数据
    rmt_transmit_config_t tx_config = {
        .loop_count = 0,
    };
    
    esp_err_t ret = rmt_transmit(config->rmt_channel, config->rmt_encoder, pixel_data, config->led_count * 3, &tx_config);
    
    free(pixel_data);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RMT transmit failed");
        return ret;
    }
    
    // 等待传输完成
    ret = rmt_tx_wait_all_done(config->rmt_channel, 100);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RMT transmit timeout");
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t ws2812b_deinit(ws2812b_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (config->rmt_channel) {
        rmt_disable(config->rmt_channel);
        rmt_del_channel(config->rmt_channel);
    }
    
    if (config->rmt_encoder) {
        rmt_del_encoder(config->rmt_encoder);
    }
    
    if (led_buffer) {
        free(led_buffer);
        led_buffer = NULL;
    }
    
    global_config = NULL;
    ESP_LOGI(TAG, "WS2812B driver deinitialized");
    return ESP_OK;
}