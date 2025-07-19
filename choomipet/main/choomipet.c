#include <stdio.h>
#include "ST7789.h"
#include "LVGL_Driver.h"
#include "lvgl.h"

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

    /*
     * 要显示图片，请按照以下步骤操作:
     * 1. 将图片文件 (例如 a.png) 通过 https://lvgl.io/tools/imageconverter 转换为C文件。
     *    - 颜色格式选择: CF_TRUE_COLOR_ALPHA (如果图片带透明通道) or CF_TRUE_COLOR
     *    - 输出格式选择: C array
     * 2. 将生成的C文件 (例如 a.c) 添加到 main 目录下, 并在 CMakeLists.txt 中添加该文件的引用。
     * 3. 在此文件中声明并使用图片:
     *    LV_IMG_DECLARE(a); // 在文件顶部声明
     *    lv_obj_t *img = lv_img_create(lv_scr_act()); // 创建图片对象
     *    lv_img_set_src(img, &a); // 设置图片源
     *    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0); // 设置对齐方式
     */

    // 创建一个LVGL标签
    lv_obj_t *label = lv_label_create(lv_scr_act());
    // 设置标签文本
    lv_label_set_text(label, "Hello world!");
    // 将标签居中对齐
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

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
