#ifndef AUDIO_H
#define AUDIO_H

#include "esp_err.h"

// 音频系统初始化
esp_err_t audio_init(void);

// 音频系统去初始化
esp_err_t audio_deinit(void);

// 开始录音
esp_err_t audio_start_recording(void);

// 停止录音
esp_err_t audio_stop_recording(void);

// 开始播放
esp_err_t audio_start_playback(void);

// 停止播放
esp_err_t audio_stop_playback(void);

#endif // AUDIO_H