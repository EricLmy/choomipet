#include <stdio.h>
#include "ST7789.h"
#include "LVGL_Driver.h"
#include "lvgl.h"

// 声明外部图片资源
LV_IMG_DECLARE(jiumi_img);

/**
 * @brief 应用主任务
 *
 * @param pvParameter 任务参数
 */
void app_task(void *pvParameter)
{
    printf("Hello world!\n");

    // 初始化LCD
    LCD_Init();
    // 初始化LVGL
    LVGL_Init();

    // Create an image widget and set the image source
    // 在172×320分辨率的1.47寸LCD屏幕上显示64×64像素的测试图片
    lv_obj_t *img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &jiumi_img);
    lv_obj_center(img);  // 将图片居中显示在屏幕上
    
    // Create a label below the image with screen info
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "1.47\" LCD 172x320\nJiumi Test Image");
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    
    // Create a title label at the top
    lv_obj_t *title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "Choomi Pet Display");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    
    printf("Jiumi image loaded and displayed on LCD\n");

    // 主循环
    while (1) {
        // LVGL任务处理器
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler();
    }
}

/**
 * @brief 应用主入口
 *
 */
void app_main(void)
{
    // 创建应用任务
    xTaskCreate(app_task, "app_task", 8192, NULL, 5, NULL);
}
