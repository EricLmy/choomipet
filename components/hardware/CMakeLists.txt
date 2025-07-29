idf_component_register(SRCS "ws2812b_driver.c" "rgb_status.c" "rgb_sync.c" "hardware.c" "gpio_config.c" "hardware_init.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "driver" "esp_common" "freertos" "esp_timer" "esp_event" "led_strip")