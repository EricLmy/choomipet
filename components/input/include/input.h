#ifndef INPUT_H
#define INPUT_H

#include "esp_err.h"
#include <stdbool.h>

// 按键事件类型
typedef enum {
    INPUT_EVENT_PRESS,
    INPUT_EVENT_RELEASE,
    INPUT_EVENT_LONG_PRESS
} input_event_type_t;

// 按键ID
typedef enum {
    INPUT_KEY_RECORD,
    INPUT_KEY_PLAY,
    INPUT_KEY_MENU
} input_key_id_t;

// 按键事件结构
typedef struct {
    input_key_id_t key_id;
    input_event_type_t event_type;
    uint32_t timestamp;
} input_event_t;

// 按键事件回调函数类型
typedef void (*input_event_callback_t)(input_event_t *event);

// 输入系统初始化
esp_err_t input_init(void);

// 输入系统去初始化
esp_err_t input_deinit(void);

// 注册按键事件回调
esp_err_t input_register_callback(input_event_callback_t callback);

// 获取按键状态
bool input_get_key_state(input_key_id_t key_id);

#endif // INPUT_H