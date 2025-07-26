#include "common.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>
#include <ctype.h>

static const char *TAG = "COMMON";

char* common_trim_string(char* str) {
    if (str == NULL) return NULL;
    
    // 去除前导空格
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return str;
    
    // 去除尾随空格
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    end[1] = '\0';
    return str;
}

bool common_string_is_empty(const char* str) {
    return (str == NULL || *str == '\0');
}

int common_string_compare_ignore_case(const char* str1, const char* str2) {
    if (str1 == NULL && str2 == NULL) return 0;
    if (str1 == NULL) return -1;
    if (str2 == NULL) return 1;
    
    while (*str1 && *str2) {
        int diff = tolower((unsigned char)*str1) - tolower((unsigned char)*str2);
        if (diff != 0) return diff;
        str1++;
        str2++;
    }
    
    return tolower((unsigned char)*str1) - tolower((unsigned char)*str2);
}

esp_err_t common_string_to_int(const char* str, int* result) {
    if (str == NULL || result == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    char* endptr;
    long val = strtol(str, &endptr, 10);
    
    if (endptr == str || *endptr != '\0') {
        return ESP_ERR_INVALID_ARG;
    }
    
    *result = (int)val;
    return ESP_OK;
}

esp_err_t common_int_to_string(int value, char* buffer, size_t buffer_size) {
    if (buffer == NULL || buffer_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    int ret = snprintf(buffer, buffer_size, "%d", value);
    if (ret < 0 || ret >= buffer_size) {
        return ESP_ERR_INVALID_SIZE;
    }
    
    return ESP_OK;
}

uint32_t common_get_timestamp_ms(void) {
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void common_delay_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void* common_malloc_check(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        ESP_LOGE(TAG, "Memory allocation failed for size %d", size);
    }
    return ptr;
}

void common_free_safe(void** ptr) {
    if (ptr != NULL && *ptr != NULL) {
        free(*ptr);
        *ptr = NULL;
    }
}

const char* common_error_to_string(esp_err_t error) {
    switch (error) {
        case ESP_OK: return "Success";
        case ESP_ERR_INVALID_ARG: return "Invalid argument";
        case ESP_ERR_INVALID_STATE: return "Invalid state";
        case ESP_ERR_INVALID_SIZE: return "Invalid size";
        case ESP_ERR_NOT_FOUND: return "Not found";
        case ESP_ERR_NO_MEM: return "Out of memory";
        case ESP_ERR_TIMEOUT: return "Timeout";
        default: return "Unknown error";
    }
}

void common_log_error(const char* tag, const char* function, esp_err_t error) {
    ESP_LOGE(tag, "Error in %s: %s (0x%x)", function, common_error_to_string(error), error);
}

bool common_is_valid_ip(const char* ip_str) {
    if (ip_str == NULL) return false;
    
    // 简单的IP地址格式检查
    int dots = 0;
    int digits = 0;
    
    for (const char* p = ip_str; *p; p++) {
        if (*p == '.') {
            if (digits == 0 || digits > 3) return false;
            dots++;
            digits = 0;
        } else if (isdigit((unsigned char)*p)) {
            digits++;
        } else {
            return false;
        }
    }
    
    return (dots == 3 && digits > 0 && digits <= 3);
}

bool common_is_valid_mac(const char* mac_str) {
    if (mac_str == NULL) return false;
    
    // 简单的MAC地址格式检查 (XX:XX:XX:XX:XX:XX)
    int colons = 0;
    int hex_digits = 0;
    
    for (const char* p = mac_str; *p; p++) {
        if (*p == ':') {
            if (hex_digits != 2) return false;
            colons++;
            hex_digits = 0;
        } else if (isxdigit((unsigned char)*p)) {
            hex_digits++;
        } else {
            return false;
        }
    }
    
    return (colons == 5 && hex_digits == 2);
}