#ifndef COMMON_H
#define COMMON_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// 通用工具函数

// 字符串工具
char* common_trim_string(char* str);
bool common_string_is_empty(const char* str);
int common_string_compare_ignore_case(const char* str1, const char* str2);

// 数值转换工具
esp_err_t common_string_to_int(const char* str, int* result);
esp_err_t common_int_to_string(int value, char* buffer, size_t buffer_size);

// 时间工具
uint32_t common_get_timestamp_ms(void);
void common_delay_ms(uint32_t ms);

// 内存工具
void* common_malloc_check(size_t size);
void common_free_safe(void** ptr);

// 错误处理工具
const char* common_error_to_string(esp_err_t error);
void common_log_error(const char* tag, const char* function, esp_err_t error);

// 数据校验工具
bool common_is_valid_ip(const char* ip_str);
bool common_is_valid_mac(const char* mac_str);

#endif // COMMON_H