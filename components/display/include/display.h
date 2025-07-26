#ifndef DISPLAY_H
#define DISPLAY_H

#include "esp_err.h"

// 显示系统初始化
esp_err_t display_init(void);

// 显示系统去初始化
esp_err_t display_deinit(void);

// 清屏
esp_err_t display_clear(void);

// 显示文本
esp_err_t display_text(const char *text, int x, int y);

// 显示图像
esp_err_t display_image(const uint8_t *image_data, int width, int height, int x, int y);

// 设置背光亮度
esp_err_t display_set_brightness(uint8_t brightness);

#endif // DISPLAY_H