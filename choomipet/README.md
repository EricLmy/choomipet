# Choomi Pet - 1.47寸LCD图片显示项目

## 项目概述
这是一个专为1.47寸172×320分辨率LCD屏幕设计的ESP32-C6图片显示项目。项目使用LVGL图形库实现高质量的图片显示，支持262K彩色显示，并提供完整的图片转换和显示解决方案。

### 硬件规格
- **开发板**: ESP32-C6
- **屏幕**: 1.47寸LCD显示屏
- **分辨率**: 172×320像素
- **颜色深度**: 262K彩色 (18位)
- **接口**: SPI通信

## 硬件要求

- **ESP32-C6 开发板** (推荐使用官方开发板)
- **1.47寸LCD显示屏** (172×320分辨率，262K彩色)
- **SPI连接线缆** (用于屏幕通信)
- **USB-C数据线** (用于程序烧录和调试)

### 硬件连接
| ESP32-C6引脚 | LCD屏幕引脚 | 功能 |
|-------------|------------|------|
| GPIO8       | CS         | 片选 |
| GPIO10      | DC         | 数据/命令选择 |
| GPIO6       | SDA/MOSI   | 数据输入 |
| GPIO7       | SCL/SCK    | 时钟 |
| GPIO9       | RST        | 复位 |
| 3.3V        | VCC        | 电源 |
| GND         | GND        | 地线 |

## 软件要求

- **ESP-IDF v5.4.1** 或更高版本
- **LVGL 图形库** v8.x (已集成在项目中)
- **Python 3.6+** (用于ESP-IDF工具链)
- **Git** (用于代码管理)
- **图片编辑工具** (可选，用于调整图片尺寸)

## 项目结构
```
choomipet/
├── main/
│   ├── choomipet.c              # 主程序文件 - 包含LCD初始化和图片显示逻辑
│   ├── jiumi_img.c              # 图片数据文件 - 存储转换后的图片C数组
│   ├── jiumi_img.h              # 图片头文件 - 图片数据声明
│   ├── lv_conf.h                # LVGL配置文件
│   ├── CMakeLists.txt           # 主程序构建配置
│   └── README_IMAGE_CONVERSION.md  # 图片转换详细指南
├── components/
│   ├── lcd_driver/              # 1.47寸LCD驱动组件 (ST7789)
│   ├── lvgl_driver/             # LVGL驱动组件
│   └── lvgl__lvgl/              # LVGL库组件 v8.x
├── images/
│   └── jiumi.jpg                # 原始图片文件
├── build/                       # 编译输出目录
├── CMakeLists.txt              # 项目构建配置
├── sdkconfig.defaults          # 默认配置
└── TROUBLESHOOTING.md          # 故障排除指南
```

## 核心功能

### 1. LCD初始化 (172×320分辨率)
- **SPI接口配置**: 高速SPI通信，支持DMA传输
- **ST7789驱动初始化**: 专为1.47寸屏幕优化
- **显示参数设置**: 172×320分辨率，262K彩色支持
- **背光控制**: 可调节亮度

### 2. LVGL图形系统集成
- **LVGL v8.x**: 现代化图形库，支持硬件加速
- **双缓冲显示**: 流畅的图像渲染
- **内存优化**: 针对ESP32-C6内存限制优化
- **颜色格式**: 支持RGB565/RGB888格式

### 3. 智能图片显示系统
- **多尺寸支持**: 64×64, 120×120, 172×320等多种尺寸
- **居中对齐**: 自动计算居中位置
- **内存管理**: 高效的图片数据存储
- **格式转换**: 支持JPEG到C数组的完整转换流程

### 4. 用户界面布局
- **顶部标题**: "Choomi Pet Display"
- **中央图片**: jiumi图片居中显示
- **底部信息**: 屏幕规格和项目信息
- **响应式布局**: 适配172×320分辨率

## 快速开始

### 1. 环境准备

确保已安装ESP-IDF v5.4.1开发环境：

**Windows系统 (PowerShell)**:
```powershell
# 激活ESP-IDF环境 (根据实际安装路径调整)
D:/Espressif/frameworks/esp-idf-v5.4.1/export.ps1
```

**Linux/macOS系统**:
```bash
# 激活ESP-IDF环境
. $HOME/esp/esp-idf/export.sh
```

### 2. 编译项目

```bash
# 进入项目目录
cd choomipet

# 配置目标芯片 (首次编译时)
idf.py set-target esp32c6

# 编译项目
idf.py build
```

### 3. 烧录到设备

```bash
# 烧录并监控串口输出 (自动检测端口)
idf.py flash monitor

# 或指定端口烧录 (Windows)
idf.py -p COM4 flash monitor

# 或指定端口烧录 (Linux/macOS)
idf.py -p /dev/ttyUSB0 flash monitor
```

### 4. 预期运行效果

成功烧录后，1.47寸LCD屏幕将显示：
- **顶部**: "Choomi Pet Display" 标题
- **中央**: jiumi测试图片 (当前为64×64像素彩色方块)
- **底部**: "1.47\" LCD 172x320\nJiumi Test Image" 信息

### 5. 串口输出信息

```
I (xxx) main_task: Calling app_main()
I (xxx) LCD: LCD initialized successfully
I (xxx) LVGL: LVGL initialized
Jiumi image loaded and displayed on LCD
```

## 代码说明

### choomipet.c - 主程序文件

主程序实现了完整的LCD初始化、LVGL配置和图片显示功能：

```c
void app_main(void)
{
    // 1. 初始化1.47寸LCD驱动 (ST7789, 172×320分辨率)
    LCD_Init();
    
    // 2. 初始化LVGL图形库 v8.x
    LVGL_Init();
    
    // 3. 创建图片显示组件
    // 在172×320分辨率的1.47寸LCD屏幕上显示64×64像素的测试图片
    lv_obj_t *img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &jiumi_img);
    lv_obj_center(img);  // 将图片居中显示在屏幕上
    
    // 4. 创建底部信息标签
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "1.47\" LCD 172x320\nJiumi Test Image");
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    
    // 5. 创建顶部标题标签
    lv_obj_t *title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "Choomi Pet Display");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    
    printf("Jiumi image loaded and displayed on LCD\n");
    
    // 6. LVGL任务循环 - 处理图形更新
    while (1) {
        lv_timer_handler();  // 处理LVGL定时器和动画
        vTaskDelay(pdMS_TO_TICKS(10));  // 10ms延时，保持流畅显示
    }
}
```

### jiumi_img.c - 图片数据文件

当前包含64×64像素的RGB565格式测试图片：

```c
#include "lvgl.h"

// 64×64像素RGB565格式测试图片数据
// 包含红、绿、蓝、黄四色方块，便于测试显示效果
static const uint8_t jiumi_img_map[] = {
    // RGB565格式数据 (每像素2字节)
    // 总大小: 64 × 64 × 2 = 8,192字节
    // 数据内容: 彩色方块测试图案
};

// 图片描述符 - 定义图片属性
const lv_img_dsc_t jiumi_img = {
    .header.cf = LV_IMG_CF_TRUE_COLOR,  // RGB565颜色格式
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 64,  // 图片宽度: 64像素
    .header.h = 64,  // 图片高度: 64像素
    .data_size = 8192,  // 数据大小: 8KB
    .data = jiumi_img_map,  // 指向图片数据
};
```

### 关键技术特性

1. **内存优化**: 使用RGB565格式减少内存占用 (相比RGB888节省33%)
2. **居中对齐**: 自动计算图片在172×320屏幕上的居中位置
3. **多标签布局**: 顶部标题 + 中央图片 + 底部信息的三层布局
4. **硬件适配**: 专为1.47寸LCD屏幕优化的显示参数

## 图片转换流程

要显示自定义的 `jiumi.jpg` 图片，需要将其转换为LVGL支持的C数组格式。

📖 **[详细转换指南](README_IMAGE_CONVERSION.md)** - 包含完整的转换步骤和优化建议

### 快速转换步骤

1. **图片预处理**
   - 调整 `jiumi.jpg` 尺寸为 120×120 像素 (推荐)
   - 或选择 64×64 (小图标) 或 172×320 (全屏)

2. **在线转换** 
   - 访问 [LVGL图片转换器](https://lvgl.io/tools/imageconverter)
   - 上传调整后的图片
   - 设置参数:
     - **Color format**: `CF_TRUE_COLOR` (RGB565)
     - **Output**: `C array`
     - **Name**: `jiumi_img`

3. **更新代码**
   - 复制生成的数据到 `main/jiumi_img.c`
   - 更新图片描述符的宽度和高度

4. **编译烧录**
   ```bash
   idf.py build
   idf.py -p COM4 flash monitor
   ```

## 预期运行效果

### 当前测试效果
程序运行后，1.47寸LCD屏幕显示：
- **顶部**: "Choomi Pet Display" 标题
- **中央**: 64×64像素彩色方块测试图片
- **底部**: "1.47\" LCD 172x320\nJiumi Test Image"

### 替换真实图片后
- **中央**: 显示转换后的 `jiumi.jpg` 图片
- **布局**: 保持相同的三层布局结构
- **对齐**: 图片自动居中显示

## 技术特性

### 硬件特性
- ✅ **1.47寸LCD**: 172×320分辨率，262K彩色
- ✅ **ESP32-C6**: 高性能RISC-V处理器
- ✅ **SPI通信**: 高速数据传输
- ✅ **ST7789驱动**: 专业LCD驱动芯片

### 软件特性
- ✅ **LVGL v8.x**: 现代化图形库
- ✅ **内存优化**: RGB565格式，节省33%内存
- ✅ **多尺寸支持**: 64×64到172×320像素
- ✅ **居中对齐**: 自动计算显示位置
- ✅ **模块化设计**: 易于扩展和维护

## 故障排除

📋 **[详细故障排除指南](TROUBLESHOOTING.md)** - 解决常见问题

### 快速解决方案

| 问题 | 解决方案 |
|------|----------|
| COM端口占用 | 关闭串口监控程序，重新插拔USB |
| 编译失败 | 检查ESP-IDF环境，确认路径正确 |
| 图片不显示 | 验证图片数据格式和尺寸 |
| 屏幕空白 | 检查硬件连接和电源供应 |

### 常见编译错误
```bash
# 如果出现目标芯片错误
idf.py set-target esp32c6

# 如果出现依赖问题
idf.py clean
idf.py build
```

## 下一步开发计划

### 短期目标 (1-2周)
- [ ] **真实图片显示**: 完成 `jiumi.jpg` 的转换和显示
- [ ] **尺寸优化**: 测试不同尺寸的显示效果
- [ ] **性能优化**: 减少内存占用和提升刷新率

### 中期目标 (1个月)
- [ ] **多图片支持**: 实现图片切换功能
- [ ] **触摸交互**: 添加触摸屏支持
- [ ] **动画效果**: 图片切换动画和过渡效果
- [ ] **配置界面**: 亮度调节、显示设置

### 长期目标 (3个月)
- [ ] **外部存储**: 支持从SD卡加载图片
- [ ] **网络功能**: WiFi连接和在线图片下载
- [ ] **图片处理**: 实时缩放、旋转、滤镜
- [ ] **宠物互动**: 添加虚拟宠物功能

---

## 项目信息

- **项目名称**: Choomi Pet - 1.47寸LCD图片显示
- **硬件平台**: ESP32-C6 + 1.47寸LCD (172×320)
- **开发框架**: ESP-IDF v5.4.1 + LVGL v8.x
- **许可证**: MIT License
- **维护状态**: 积极开发中

**如有问题或建议，欢迎提交Issue或Pull Request！** 🚀

## 许可证
本项目基于ESP-IDF和LVGL开源许可证开发。