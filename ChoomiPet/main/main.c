/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "ST7789.h"
#include "SD_SPI.h"
#include "RGB.h"
#include "Wireless.h"
#include "LVGL_Example.h"
#include "ui_layout_demo.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "Choomi Pet Hardware Starting...");
    
    // 初始化硬件组件
    Wireless_Init();
    Flash_Searching();
    RGB_Init();
    RGB_Example();
    SD_Init();                              // SD must be initialized behind the LCD
    LCD_Init();
    BK_Light(50);                          // 设置背光亮度为50%
    LVGL_Init();                           // 初始化LVGL

    ESP_LOGI(TAG, "Hardware initialization completed");

/********************* UI Layout Demo *********************/
    // 启动UI布局演示
    esp_err_t ret = ui_layout_demo_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start UI layout demo: %s", esp_err_to_name(ret));
        // 如果UI布局演示失败，回退到原始演示
        Lvgl_Example1();
    } else {
        ESP_LOGI(TAG, "UI Layout Demo started successfully");
    }

    // 可选的其他LVGL演示（注释掉，因为我们使用自定义UI）
    // lv_demo_widgets();
    // lv_demo_keypad_encoder();
    // lv_demo_benchmark();
    // lv_demo_stress();
    // lv_demo_music();

    ESP_LOGI(TAG, "Entering main loop");
    
    while (1) {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
    }
}
