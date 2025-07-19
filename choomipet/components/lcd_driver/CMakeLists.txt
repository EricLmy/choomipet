idf_component_register(SRCS "ST7789.c" "Vernon_ST7789T/Vernon_ST7789T.c"
                    INCLUDE_DIRS "." "Vernon_ST7789T"
                    REQUIRES esp_lcd esp_timer lvgl lvgl_driver)