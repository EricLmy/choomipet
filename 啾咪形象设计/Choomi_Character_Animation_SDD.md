# 啾咪宠物形象与开机动画系统 - 系统设计文档 (SDD)

## 文档基本信息

- **项目名称**: Choomi Pet - 啾咪宠物形象与开机动画系统
- **文档版本**: v1.0
- **创建日期**: 2025年1月21日
- **作者**: 项目负责人
- **评审状态**: 待评审
- **关联文档**: Choomi_Character_Design_PRD.md

## 1. 系统架构设计

### 1.1 整体架构图

```
┌─────────────────────────────────────────────────────────────┐
│                    Choomi Pet System                        │
├─────────────────────────────────────────────────────────────┤
│  Application Layer (应用层)                                 │
│  ┌─────────────────┐  ┌─────────────────┐                  │
│  │  Choomi Manager │  │ Animation Engine │                  │
│  │  - State Control│  │ - Frame Control  │                  │
│  │  - Event Handle │  │ - Timing Control │                  │
│  └─────────────────┘  └─────────────────┘                  │
├─────────────────────────────────────────────────────────────┤
│  Graphics Layer (图形层)                                    │
│  ┌─────────────────┐  ┌─────────────────┐                  │
│  │   LVGL Core     │  │  Image Manager  │                  │
│  │  - Widget Tree  │  │ - Resource Load  │                  │
│  │  - Event System │  │ - Memory Manage  │                  │
│  └─────────────────┘  └─────────────────┘                  │
├─────────────────────────────────────────────────────────────┤
│  Hardware Layer (硬件层)                                    │
│  ┌─────────────────┐  ┌─────────────────┐                  │
│  │   LCD Driver    │  │   SPI Driver    │                  │
│  │  - ST7789 Ctrl  │  │ - Data Transfer  │                  │
│  │  - Display Buf  │  │ - Clock Control  │                  │
│  └─────────────────┘  └─────────────────┘                  │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 模块依赖关系

```
Choomi Manager ──┐
                 ├──→ LVGL Core ──→ LCD Driver ──→ SPI Driver
Animation Engine ─┘
                 ┌──→ Image Manager ──→ Flash Storage
                 └──→ Timer Service ──→ ESP32 Timer
```

### 1.3 数据流架构

```
[Flash Storage] → [Image Manager] → [LVGL Core] → [LCD Driver] → [Display]
       ↑               ↑               ↑             ↑
   Image Data    Resource Cache   Frame Buffer   SPI Commands
```

## 2. 技术方案说明

### 2.1 技术选型

| 组件 | 技术选择 | 版本 | 选择理由 |
|------|----------|------|----------|
| 图形库 | LVGL | v8.3+ | 轻量级、功能完整、ESP32优化好 |
| 动画引擎 | LVGL Animation | v8.3+ | 与LVGL深度集成、性能优秀 |
| 图片格式 | RGB565 | - | 内存效率高、硬件支持好 |
| 存储方式 | Flash Embedded | - | 启动速度快、无外部依赖 |
| 定时器 | ESP-IDF Timer | v5.4.1 | 精度高、资源占用少 |

### 2.2 框架设计

#### 2.2.1 Choomi状态机

```c
typedef enum {
    CHOOMI_STATE_BOOT_ANIMATION,    // 开机动画状态
    CHOOMI_STATE_NORMAL,            // 正常显示状态
    CHOOMI_STATE_HAPPY,             // 开心状态
    CHOOMI_STATE_THINKING,          // 思考状态
    CHOOMI_STATE_SLEEPING,          // 睡眠状态
    CHOOMI_STATE_TRANSITION,        // 状态转换中
    CHOOMI_STATE_ERROR              // 错误状态
} choomi_state_t;
```

#### 2.2.2 动画系统架构

```c
// 动画类型定义
typedef enum {
    ANIM_TYPE_BOOT_SEQUENCE,        // 开机动画序列
    ANIM_TYPE_STATE_TRANSITION,     // 状态转换动画
    ANIM_TYPE_IDLE_BREATHING,       // 待机呼吸动画
    ANIM_TYPE_INTERACTION           // 交互动画
} choomi_anim_type_t;

// 动画控制器
typedef struct {
    choomi_anim_type_t type;        // 动画类型
    lv_obj_t* container;            // 动画容器
    lv_anim_t* anim_list;          // 动画列表
    uint8_t anim_count;            // 动画数量
    uint32_t duration;             // 总时长(ms)
    bool is_playing;               // 播放状态
    bool is_loop;                  // 是否循环
    void (*on_complete)(void);     // 完成回调
} choomi_animation_controller_t;
```

## 3. 详细设计

### 3.1 核心流程图

#### 3.1.1 系统启动流程

```
[系统上电] → [硬件初始化] → [LVGL初始化] → [资源加载] → [开机动画] → [主界面]
     ↓             ↓             ↓            ↓           ↓          ↓
  ESP32启动    LCD/SPI初始化   图形系统就绪   图片解码    动画播放    正常运行
```

#### 3.1.2 动画播放流程

```
[动画开始] → [加载帧数据] → [创建动画对象] → [启动定时器] → [帧更新循环] → [动画结束]
     ↓            ↓             ↓             ↓            ↓           ↓
  状态检查     内存分配      LVGL动画配置   定时触发     渲染显示    资源释放
```

### 3.2 时序图

#### 3.2.1 开机动画时序

```
User         System       Image_Mgr    LVGL_Core    LCD_Driver
 │              │             │            │            │
 │──Power On──→ │             │            │            │
 │              │──Init──────→│            │            │
 │              │             │──Load─────→│            │
 │              │             │            │──Display──→│
 │              │◄──Ready────│            │            │
 │              │──Start_Anim────────────→│            │
 │              │             │            │──Render───→│
 │◄──Animation──│             │            │            │
 │   Playing    │             │            │            │
 │              │──Complete──────────────→│            │
 │◄──Main_UI───│             │            │            │
```

### 3.3 类图设计

```
┌─────────────────────────┐
│     ChoomiBoot          │
├─────────────────────────┤
│ - state: choomi_state_t │
│ - anim_ctrl: *controller│
│ - image_mgr: *manager   │
├─────────────────────────┤
│ + init()                │
│ + start_boot_anim()     │
│ + set_state()           │
│ + update()              │
└─────────────────────────┘
            │
            ▼
┌─────────────────────────┐
│   AnimationController   │
├─────────────────────────┤
│ - type: anim_type_t     │
│ - frames: **frame_data  │
│ - current_frame: uint8  │
│ - is_playing: bool      │
├─────────────────────────┤
│ + play()                │
│ + pause()               │
│ + stop()                │
│ + next_frame()          │
└─────────────────────────┘
            │
            ▼
┌─────────────────────────┐
│     ImageManager       │
├─────────────────────────┤
│ - cache: *image_cache   │
│ - total_size: uint32    │
│ - used_size: uint32     │
├─────────────────────────┤
│ + load_image()          │
│ + cache_image()         │
│ + free_image()          │
│ + get_image()           │
└─────────────────────────┘
```

## 4. 数据库设计

### 4.1 图片资源表结构

```c
// 图片资源描述符
typedef struct {
    uint16_t id;                    // 资源ID
    char name[32];                  // 资源名称
    uint16_t width;                 // 图片宽度
    uint16_t height;                // 图片高度
    uint32_t data_size;             // 数据大小
    lv_img_cf_t color_format;       // 颜色格式
    const uint8_t* data;            // 图片数据指针
    uint32_t checksum;              // 数据校验和
} choomi_image_desc_t;

// 图片资源表
static const choomi_image_desc_t choomi_images[] = {
    {0x0001, "choomi_normal",    120, 120, 28800, LV_IMG_CF_TRUE_COLOR, choomi_normal_data, 0x12345678},
    {0x0002, "choomi_happy",     120, 120, 28800, LV_IMG_CF_TRUE_COLOR, choomi_happy_data,  0x23456789},
    {0x0003, "choomi_thinking",  120, 120, 28800, LV_IMG_CF_TRUE_COLOR, choomi_think_data,  0x3456789A},
    {0x0004, "choomi_sleeping",  120, 120, 28800, LV_IMG_CF_TRUE_COLOR, choomi_sleep_data,  0x456789AB},
    // 动画帧数据
    {0x0101, "boot_frame_01",    172, 320, 110080, LV_IMG_CF_TRUE_COLOR, boot_frame_01_data, 0x56789ABC},
    {0x0102, "boot_frame_02",    172, 320, 110080, LV_IMG_CF_TRUE_COLOR, boot_frame_02_data, 0x6789ABCD},
    // ...
};
```

### 4.2 动画序列表结构

```c
// 动画帧描述符
typedef struct {
    uint16_t image_id;              // 对应图片ID
    uint16_t duration_ms;           // 帧持续时间(毫秒)
    lv_coord_t pos_x;               // X坐标
    lv_coord_t pos_y;               // Y坐标
    uint8_t opacity;                // 透明度(0-255)
    lv_anim_path_t* path;           // 动画路径
} choomi_anim_frame_t;

// 动画序列描述符
typedef struct {
    uint16_t sequence_id;           // 序列ID
    char name[32];                  // 序列名称
    uint8_t frame_count;            // 帧数量
    uint32_t total_duration;        // 总时长
    bool is_loop;                   // 是否循环
    const choomi_anim_frame_t* frames; // 帧数据
} choomi_anim_sequence_t;

// 开机动画序列定义
static const choomi_anim_frame_t boot_animation_frames[] = {
    // 阶段1: 系统启动
    {0x0101, 1000, 86, 160, 255, &lv_anim_path_ease_in},
    // 阶段2: 像素汇聚
    {0x0102, 500,  86, 160, 200, &lv_anim_path_ease_in_out},
    {0x0103, 500,  86, 160, 255, &lv_anim_path_ease_in_out},
    // 阶段3: 形象显现
    {0x0001, 1000, 86, 160, 255, &lv_anim_path_ease_out},
    // 阶段4: 欢迎互动
    {0x0002, 1000, 86, 160, 255, &lv_anim_path_linear},
};

static const choomi_anim_sequence_t boot_sequence = {
    .sequence_id = 0x0001,
    .name = "boot_animation",
    .frame_count = 5,
    .total_duration = 4000,
    .is_loop = false,
    .frames = boot_animation_frames
};
```

## 5. 接口规范

### 5.1 核心API接口

#### 5.1.1 Choomi管理器接口

```c
/**
 * @brief 初始化Choomi系统
 * @return esp_err_t 初始化结果
 */
esp_err_t choomi_init(void);

/**
 * @brief 启动开机动画
 * @return esp_err_t 启动结果
 */
esp_err_t choomi_start_boot_animation(void);

/**
 * @brief 设置Choomi状态
 * @param state 目标状态
 * @return esp_err_t 设置结果
 */
esp_err_t choomi_set_state(choomi_state_t state);

/**
 * @brief 获取当前状态
 * @return choomi_state_t 当前状态
 */
choomi_state_t choomi_get_state(void);

/**
 * @brief 系统更新函数(需要在主循环中调用)
 */
void choomi_update(void);

/**
 * @brief 注册状态变化回调
 * @param callback 回调函数
 */
void choomi_register_state_callback(void (*callback)(choomi_state_t old_state, choomi_state_t new_state));
```

#### 5.1.2 动画控制器接口

```c
/**
 * @brief 创建动画控制器
 * @param type 动画类型
 * @param container 容器对象
 * @return choomi_animation_controller_t* 控制器指针
 */
choomi_animation_controller_t* choomi_anim_create(choomi_anim_type_t type, lv_obj_t* container);

/**
 * @brief 播放动画序列
 * @param controller 控制器指针
 * @param sequence_id 序列ID
 * @return esp_err_t 播放结果
 */
esp_err_t choomi_anim_play_sequence(choomi_animation_controller_t* controller, uint16_t sequence_id);

/**
 * @brief 暂停动画
 * @param controller 控制器指针
 */
void choomi_anim_pause(choomi_animation_controller_t* controller);

/**
 * @brief 恢复动画
 * @param controller 控制器指针
 */
void choomi_anim_resume(choomi_animation_controller_t* controller);

/**
 * @brief 停止动画
 * @param controller 控制器指针
 */
void choomi_anim_stop(choomi_animation_controller_t* controller);

/**
 * @brief 销毁动画控制器
 * @param controller 控制器指针
 */
void choomi_anim_destroy(choomi_animation_controller_t* controller);
```

#### 5.1.3 图片管理器接口

```c
/**
 * @brief 初始化图片管理器
 * @param cache_size 缓存大小(字节)
 * @return esp_err_t 初始化结果
 */
esp_err_t choomi_image_manager_init(uint32_t cache_size);

/**
 * @brief 加载图片资源
 * @param image_id 图片ID
 * @return const lv_img_dsc_t* 图片描述符指针
 */
const lv_img_dsc_t* choomi_image_load(uint16_t image_id);

/**
 * @brief 预加载图片到缓存
 * @param image_id 图片ID
 * @return esp_err_t 加载结果
 */
esp_err_t choomi_image_preload(uint16_t image_id);

/**
 * @brief 释放图片缓存
 * @param image_id 图片ID
 */
void choomi_image_unload(uint16_t image_id);

/**
 * @brief 清空所有缓存
 */
void choomi_image_clear_cache(void);

/**
 * @brief 获取缓存使用情况
 * @param used_size 已使用大小指针
 * @param total_size 总大小指针
 */
void choomi_image_get_cache_info(uint32_t* used_size, uint32_t* total_size);
```

### 5.2 事件系统接口

```c
// 事件类型定义
typedef enum {
    CHOOMI_EVENT_BOOT_ANIM_START,   // 开机动画开始
    CHOOMI_EVENT_BOOT_ANIM_END,     // 开机动画结束
    CHOOMI_EVENT_STATE_CHANGED,     // 状态改变
    CHOOMI_EVENT_ANIMATION_COMPLETE,// 动画完成
    CHOOMI_EVENT_ERROR              // 错误事件
} choomi_event_type_t;

// 事件数据结构
typedef struct {
    choomi_event_type_t type;       // 事件类型
    void* data;                     // 事件数据
    uint32_t timestamp;             // 时间戳
} choomi_event_t;

// 事件回调函数类型
typedef void (*choomi_event_callback_t)(const choomi_event_t* event);

/**
 * @brief 注册事件监听器
 * @param event_type 事件类型
 * @param callback 回调函数
 * @return esp_err_t 注册结果
 */
esp_err_t choomi_event_register(choomi_event_type_t event_type, choomi_event_callback_t callback);

/**
 * @brief 发送事件
 * @param event 事件指针
 */
void choomi_event_send(const choomi_event_t* event);
```

## 6. 安全方案

### 6.1 内存安全

#### 6.1.1 内存管理策略
- **静态分配**: 图片数据使用静态数组，避免动态分配
- **缓存限制**: 设置最大缓存大小，防止内存溢出
- **引用计数**: 实现图片资源引用计数，自动释放
- **内存检查**: 定期检查内存使用情况，及时报警

#### 6.1.2 缓冲区保护
```c
// 安全的图片数据访问
static inline bool is_valid_image_id(uint16_t image_id) {
    return (image_id > 0 && image_id < CHOOMI_MAX_IMAGE_COUNT);
}

static inline bool is_valid_frame_index(uint8_t frame_index, uint8_t total_frames) {
    return (frame_index < total_frames);
}

// 边界检查宏
#define CHOOMI_CHECK_BOUNDS(ptr, size, max_size) \
    do { \
        if ((ptr) == NULL || (size) > (max_size)) { \
            ESP_LOGE(TAG, "Buffer bounds check failed"); \
            return ESP_ERR_INVALID_ARG; \
        } \
    } while(0)
```

### 6.2 数据完整性

#### 6.2.1 校验和验证
```c
/**
 * @brief 计算图片数据校验和
 * @param data 数据指针
 * @param size 数据大小
 * @return uint32_t 校验和
 */
uint32_t choomi_calculate_checksum(const uint8_t* data, uint32_t size);

/**
 * @brief 验证图片数据完整性
 * @param image_desc 图片描述符
 * @return bool 验证结果
 */
bool choomi_verify_image_integrity(const choomi_image_desc_t* image_desc);
```

#### 6.2.2 错误恢复机制
```c
// 错误恢复策略
typedef enum {
    RECOVERY_STRATEGY_RETRY,        // 重试
    RECOVERY_STRATEGY_FALLBACK,     // 降级
    RECOVERY_STRATEGY_RESET,        // 重置
    RECOVERY_STRATEGY_IGNORE        // 忽略
} recovery_strategy_t;

/**
 * @brief 错误恢复处理
 * @param error_code 错误码
 * @param strategy 恢复策略
 * @return esp_err_t 处理结果
 */
esp_err_t choomi_handle_error(esp_err_t error_code, recovery_strategy_t strategy);
```

## 7. 性能设计

### 7.1 内存优化

#### 7.1.1 内存分配策略
```c
// 内存池配置
#define CHOOMI_IMAGE_CACHE_SIZE     (100 * 1024)  // 100KB图片缓存
#define CHOOMI_ANIM_BUFFER_SIZE     (20 * 1024)   // 20KB动画缓冲
#define CHOOMI_WORK_BUFFER_SIZE     (10 * 1024)   // 10KB工作缓冲

// 内存使用监控
typedef struct {
    uint32_t total_allocated;       // 总分配内存
    uint32_t peak_usage;           // 峰值使用
    uint32_t current_usage;        // 当前使用
    uint32_t cache_hits;           // 缓存命中
    uint32_t cache_misses;         // 缓存未命中
} choomi_memory_stats_t;

extern choomi_memory_stats_t g_choomi_memory_stats;
```

#### 7.1.2 图片压缩优化
```c
// 图片压缩配置
typedef struct {
    bool enable_compression;        // 启用压缩
    uint8_t quality_level;         // 质量等级(1-10)
    bool use_dithering;            // 使用抖动
    lv_img_cf_t target_format;     // 目标格式
} choomi_compression_config_t;

/**
 * @brief 压缩图片数据
 * @param src_data 源数据
 * @param src_size 源大小
 * @param dst_data 目标数据
 * @param dst_size 目标大小
 * @param config 压缩配置
 * @return esp_err_t 压缩结果
 */
esp_err_t choomi_compress_image(const uint8_t* src_data, uint32_t src_size,
                                uint8_t* dst_data, uint32_t* dst_size,
                                const choomi_compression_config_t* config);
```

### 7.2 渲染性能优化

#### 7.2.1 帧率控制
```c
// 帧率控制配置
#define CHOOMI_TARGET_FPS           15            // 目标帧率
#define CHOOMI_FRAME_TIME_MS        (1000/CHOOMI_TARGET_FPS)  // 帧时间

// 性能监控
typedef struct {
    uint32_t frame_count;          // 帧计数
    uint32_t render_time_us;       // 渲染时间(微秒)
    uint32_t avg_fps;              // 平均帧率
    uint32_t min_frame_time;       // 最小帧时间
    uint32_t max_frame_time;       // 最大帧时间
} choomi_performance_stats_t;

/**
 * @brief 更新性能统计
 */
void choomi_update_performance_stats(void);

/**
 * @brief 获取性能统计
 * @return const choomi_performance_stats_t* 统计数据指针
 */
const choomi_performance_stats_t* choomi_get_performance_stats(void);
```

#### 7.2.2 渲染优化策略
```c
// 渲染优化配置
typedef struct {
    bool enable_partial_refresh;   // 启用部分刷新
    bool enable_double_buffer;     // 启用双缓冲
    bool enable_dma_transfer;      // 启用DMA传输
    uint8_t refresh_rate;          // 刷新率
} choomi_render_config_t;

/**
 * @brief 配置渲染优化
 * @param config 配置参数
 * @return esp_err_t 配置结果
 */
esp_err_t choomi_configure_rendering(const choomi_render_config_t* config);
```

## 8. 容灾备份方案

### 8.1 故障检测

```c
// 故障类型定义
typedef enum {
    CHOOMI_FAULT_MEMORY_OVERFLOW,   // 内存溢出
    CHOOMI_FAULT_IMAGE_CORRUPT,     // 图片损坏
    CHOOMI_FAULT_ANIMATION_STUCK,   // 动画卡死
    CHOOMI_FAULT_DISPLAY_ERROR,     // 显示错误
    CHOOMI_FAULT_SYSTEM_HANG        // 系统挂起
} choomi_fault_type_t;

// 故障检测器
typedef struct {
    choomi_fault_type_t type;       // 故障类型
    bool (*detect_func)(void);      // 检测函数
    void (*handle_func)(void);      // 处理函数
    uint32_t check_interval_ms;     // 检查间隔
    uint32_t last_check_time;       // 上次检查时间
} choomi_fault_detector_t;

/**
 * @brief 注册故障检测器
 * @param detector 检测器配置
 * @return esp_err_t 注册结果
 */
esp_err_t choomi_register_fault_detector(const choomi_fault_detector_t* detector);

/**
 * @brief 运行故障检测
 */
void choomi_run_fault_detection(void);
```

### 8.2 自动恢复机制

```c
// 恢复操作定义
typedef struct {
    char name[32];                  // 恢复操作名称
    esp_err_t (*recovery_func)(void); // 恢复函数
    uint32_t timeout_ms;           // 超时时间
    uint8_t max_retry_count;       // 最大重试次数
    uint8_t current_retry;         // 当前重试次数
} choomi_recovery_action_t;

/**
 * @brief 执行恢复操作
 * @param action 恢复操作
 * @return esp_err_t 执行结果
 */
esp_err_t choomi_execute_recovery(choomi_recovery_action_t* action);

/**
 * @brief 系统重置恢复
 * @return esp_err_t 重置结果
 */
esp_err_t choomi_system_reset_recovery(void);

/**
 * @brief 图片缓存重建
 * @return esp_err_t 重建结果
 */
esp_err_t choomi_rebuild_image_cache(void);
```

### 8.3 备份数据管理

```c
// 备份配置
typedef struct {
    bool enable_backup;             // 启用备份
    uint32_t backup_interval_ms;    // 备份间隔
    uint8_t max_backup_count;       // 最大备份数
    const char* backup_partition;   // 备份分区
} choomi_backup_config_t;

/**
 * @brief 创建系统备份
 * @return esp_err_t 备份结果
 */
esp_err_t choomi_create_backup(void);

/**
 * @brief 从备份恢复
 * @param backup_index 备份索引
 * @return esp_err_t 恢复结果
 */
esp_err_t choomi_restore_from_backup(uint8_t backup_index);
```

## 9. 部署架构

### 9.1 Flash分区规划

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32-C6 Flash Layout                   │
├─────────────────────────────────────────────────────────────┤
│ 0x0000   - 0x8000   │ Bootloader (32KB)                    │
│ 0x8000   - 0x9000   │ Partition Table (4KB)                │
│ 0x9000   - 0x10000  │ NVS (28KB)                           │
│ 0x10000  - 0x200000 │ Application (1984KB)                 │
│ 0x200000 - 0x280000 │ Choomi Images (512KB)                │
│ 0x280000 - 0x300000 │ Animation Data (512KB)               │
│ 0x300000 - 0x380000 │ Backup Data (512KB)                  │
│ 0x380000 - 0x400000 │ User Data (512KB)                    │
└─────────────────────────────────────────────────────────────┘
```

### 9.2 分区表配置

```csv
# Name,   Type, SubType, Offset,  Size,     Flags
nvs,      data, nvs,     0x9000,  0x7000,
phy_init, data, phy,     0x10000, 0x1000,
factory,  app,  factory, 0x11000, 0x1EF000,
choomi_img, data, 0x40,  0x200000, 0x80000,
anim_data,  data, 0x41,  0x280000, 0x80000,
backup,     data, 0x42,  0x300000, 0x80000,
user_data,  data, 0x43,  0x380000, 0x80000,
```

### 9.3 编译配置

```cmake
# CMakeLists.txt 配置
set(CHOOMI_ENABLE_ANIMATION ON)
set(CHOOMI_ENABLE_COMPRESSION ON)
set(CHOOMI_ENABLE_BACKUP ON)
set(CHOOMI_DEBUG_LEVEL 2)

# 编译选项
target_compile_definitions(choomi_system PRIVATE
    CHOOMI_VERSION_MAJOR=1
    CHOOMI_VERSION_MINOR=0
    CHOOMI_VERSION_PATCH=0
    CHOOMI_MAX_IMAGE_COUNT=50
    CHOOMI_MAX_ANIMATION_FRAMES=20
    CHOOMI_CACHE_SIZE=102400
)

# 链接库
target_link_libraries(choomi_system
    lvgl
    esp_lcd
    esp_timer
    nvs_flash
)
```

## 10. 测试策略

### 10.1 单元测试

```c
// 测试用例示例
void test_choomi_image_manager(void) {
    // 测试图片加载
    const lv_img_dsc_t* img = choomi_image_load(0x0001);
    TEST_ASSERT_NOT_NULL(img);
    TEST_ASSERT_EQUAL(120, img->header.w);
    TEST_ASSERT_EQUAL(120, img->header.h);
    
    // 测试缓存管理
    uint32_t used, total;
    choomi_image_get_cache_info(&used, &total);
    TEST_ASSERT_GREATER_THAN(0, used);
    TEST_ASSERT_LESS_OR_EQUAL(total, used);
    
    // 测试资源释放
    choomi_image_unload(0x0001);
    choomi_image_get_cache_info(&used, &total);
    TEST_ASSERT_EQUAL(0, used);
}

void test_choomi_animation_controller(void) {
    // 测试动画创建
    lv_obj_t* container = lv_obj_create(lv_scr_act());
    choomi_animation_controller_t* ctrl = choomi_anim_create(ANIM_TYPE_BOOT_SEQUENCE, container);
    TEST_ASSERT_NOT_NULL(ctrl);
    
    // 测试动画播放
    esp_err_t ret = choomi_anim_play_sequence(ctrl, 0x0001);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_TRUE(ctrl->is_playing);
    
    // 测试动画控制
    choomi_anim_pause(ctrl);
    TEST_ASSERT_FALSE(ctrl->is_playing);
    
    choomi_anim_resume(ctrl);
    TEST_ASSERT_TRUE(ctrl->is_playing);
    
    // 清理资源
    choomi_anim_destroy(ctrl);
    lv_obj_del(container);
}
```

### 10.2 集成测试

```c
// 集成测试场景
void test_boot_animation_integration(void) {
    // 模拟系统启动
    esp_err_t ret = choomi_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    
    // 启动开机动画
    ret = choomi_start_boot_animation();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    
    // 等待动画完成
    uint32_t start_time = esp_timer_get_time() / 1000;
    while (choomi_get_state() == CHOOMI_STATE_BOOT_ANIMATION) {
        choomi_update();
        vTaskDelay(pdMS_TO_TICKS(10));
        
        // 超时检查
        uint32_t current_time = esp_timer_get_time() / 1000;
        if (current_time - start_time > 6000) { // 6秒超时
            TEST_FAIL_MESSAGE("Boot animation timeout");
        }
    }
    
    // 验证最终状态
    TEST_ASSERT_EQUAL(CHOOMI_STATE_NORMAL, choomi_get_state());
}
```

### 10.3 性能测试

```c
// 性能基准测试
void test_animation_performance(void) {
    // 测试帧率稳定性
    choomi_start_boot_animation();
    
    uint32_t frame_count = 0;
    uint32_t start_time = esp_timer_get_time();
    
    while (choomi_get_state() == CHOOMI_STATE_BOOT_ANIMATION) {
        choomi_update();
        frame_count++;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    uint32_t end_time = esp_timer_get_time();
    uint32_t duration_ms = (end_time - start_time) / 1000;
    float actual_fps = (float)frame_count * 1000.0f / duration_ms;
    
    // 验证帧率在合理范围内
    TEST_ASSERT_GREATER_THAN(10.0f, actual_fps);
    TEST_ASSERT_LESS_THAN(20.0f, actual_fps);
    
    // 验证内存使用
    const choomi_performance_stats_t* stats = choomi_get_performance_stats();
    TEST_ASSERT_LESS_THAN(150 * 1024, stats->peak_memory_usage); // 150KB限制
}
```

---

## 附录

### A. 错误码定义

```c
// Choomi系统错误码
#define CHOOMI_ERR_BASE                 0x50000
#define CHOOMI_ERR_INVALID_STATE        (CHOOMI_ERR_BASE + 1)
#define CHOOMI_ERR_INVALID_IMAGE_ID     (CHOOMI_ERR_BASE + 2)
#define CHOOMI_ERR_MEMORY_INSUFFICIENT  (CHOOMI_ERR_BASE + 3)
#define CHOOMI_ERR_ANIMATION_FAILED     (CHOOMI_ERR_BASE + 4)
#define CHOOMI_ERR_IMAGE_CORRUPT        (CHOOMI_ERR_BASE + 5)
#define CHOOMI_ERR_CACHE_FULL           (CHOOMI_ERR_BASE + 6)
#define CHOOMI_ERR_TIMEOUT              (CHOOMI_ERR_BASE + 7)
```

### B. 配置参数

```c
// 系统配置参数
#define CHOOMI_CONFIG_MAX_IMAGES        50
#define CHOOMI_CONFIG_MAX_ANIMATIONS    10
#define CHOOMI_CONFIG_CACHE_SIZE        (100 * 1024)
#define CHOOMI_CONFIG_MAX_FRAME_TIME    100  // ms
#define CHOOMI_CONFIG_WATCHDOG_TIMEOUT  5000 // ms
```

### C. 调试工具

```c
// 调试宏定义
#ifdef CHOOMI_DEBUG
#define CHOOMI_LOG_D(tag, format, ...) ESP_LOGD(tag, format, ##__VA_ARGS__)
#define CHOOMI_LOG_I(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
#define CHOOMI_LOG_W(tag, format, ...) ESP_LOGW(tag, format, ##__VA_ARGS__)
#define CHOOMI_LOG_E(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#else
#define CHOOMI_LOG_D(tag, format, ...)
#define CHOOMI_LOG_I(tag, format, ...)
#define CHOOMI_LOG_W(tag, format, ...)
#define CHOOMI_LOG_E(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#endif

// 性能分析宏
#define CHOOMI_PERF_START(name) \
    uint32_t perf_start_##name = esp_timer_get_time()

#define CHOOMI_PERF_END(name) \
    do { \
        uint32_t perf_end_##name = esp_timer_get_time(); \
        CHOOMI_LOG_D("PERF", #name " took %lu us", perf_end_##name - perf_start_##name); \
    } while(0)
```

---

**文档状态**: 待评审  
**下一步行动**: 开始详细的代码实现工作