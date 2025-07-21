# 图片转换指南 - 针对1.47寸172×320 LCD屏幕

本文档详细说明如何将 `jiumi.jpg` 图片转换为适合1.47寸172×320分辨率LCD屏幕的LVGL C数组格式。

## 屏幕规格
- **屏幕尺寸**: 1.47寸
- **分辨率**: 172×320像素
- **颜色深度**: 262K彩色 (18位色彩)
- **推荐图片格式**: RGB565 (16位) 或 RGB888 (24位)

## 步骤 1: 图片预处理

### 调整图片尺寸
建议将 `jiumi.jpg` 调整为以下尺寸之一：
- **全屏显示**: 172×320像素
- **居中显示**: 120×120像素 (推荐)
- **小图标**: 64×64像素

可以使用以下工具调整图片尺寸：
- Photoshop
- GIMP (免费)
- 在线工具: https://www.iloveimg.com/resize-image

## 步骤 2: 使用 LVGL 图片转换工具

### 在线转换工具（推荐）

1. 访问 LVGL 在线图片转换器：https://lvgl.io/tools/imageconverter
2. 上传调整尺寸后的 `jiumi.jpg` 文件
3. 设置转换参数：
   - **Color format**: `CF_TRUE_COLOR` (RGB565, 16位) - 推荐用于节省内存
   - **Output format**: `C array`
   - **Name**: `jiumi_img`
   - **Dither**: 启用 (改善色彩过渡)
4. 点击 "Convert" 按钮
5. 下载生成的 C 代码文件

### 本地转换工具

```bash
# 安装 lv_img_conv 工具
npm install -g lv_img_conv

# 转换为RGB565格式 (推荐)
lv_img_conv images/jiumi_resized.jpg -f -c CF_TRUE_COLOR -t c_array > jiumi_img_data.c

# 或转换为RGB888格式 (更高质量但占用更多内存)
lv_img_conv images/jiumi_resized.jpg -f -c CF_TRUE_COLOR_ALPHA -t c_array > jiumi_img_data.c
```

## 步骤 3: 更新 jiumi_img.c 文件

### 替换图片数据

1. 打开生成的 C 代码文件
2. 复制图片数据数组（通常名为 `jiumi_img_map[]`）
3. 打开 `main/jiumi_img.c` 文件
4. 完全替换现有内容

### 针对172×320屏幕的示例代码

```c
#include "lvgl.h"

// 图片数据数组（从LVGL转换工具生成）
// 示例：120×120像素，RGB565格式
static const uint8_t jiumi_img_map[] = {
    // 这里粘贴从转换工具生成的实际数据
    // 数据格式: 每个像素2字节 (RGB565)
    // 总大小: 120 × 120 × 2 = 28,800字节
};

// 图片描述符
const lv_img_dsc_t jiumi_img = {
    .header.cf = LV_IMG_CF_TRUE_COLOR,  // RGB565格式
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 120,  // 图片宽度
    .header.h = 120,  // 图片高度
    .data_size = sizeof(jiumi_img_map),
    .data = jiumi_img_map,
};
```

## 步骤 4: 内存优化建议

### 对于ESP32-C6的内存限制
- **SRAM**: ~512KB
- **推荐图片大小**: 不超过100KB
- **计算公式**: 宽度 × 高度 × 字节数/像素
  - RGB565: 2字节/像素
  - RGB888: 3字节/像素
  - ARGB8888: 4字节/像素

### 尺寸建议
| 图片尺寸 | RGB565内存占用 | 适用场景 |
|----------|----------------|----------|
| 64×64    | 8KB           | 小图标   |
| 120×120  | 28.8KB        | 中等图片 |
| 172×200  | 68.8KB        | 大图片   |
| 172×320  | 110KB         | 全屏背景 |

## 步骤 5: 编译和测试

```bash
# 编译项目
idf.py build

# 烧录到设备 (确保COM端口可用)
idf.py -p COM4 flash monitor
```

## 显示效果预期

成功烧录后，LCD屏幕应显示：
- 顶部: "Choomi Pet Display" 标题
- 中央: jiumi图片 (居中显示)
- 底部: "1.47\" LCD 172x320\nJiumi Test Image" 信息

## 故障排除

### 图片不显示
1. **检查数据格式**: 确保使用正确的颜色格式 (CF_TRUE_COLOR)
2. **检查尺寸**: 验证图片描述符中的宽度和高度
3. **内存检查**: 确保图片数据不超过可用内存
4. **编译错误**: 检查数组语法是否正确

### 显示异常
1. **颜色错误**: 尝试不同的颜色格式
2. **图片变形**: 检查宽高比是否正确
3. **部分显示**: 可能是内存不足，尝试减小图片尺寸

### COM端口问题
参考 `TROUBLESHOOTING.md` 文件解决端口占用问题。

## 高级选项

### 使用外部Flash存储
对于大图片，可以考虑将数据存储在外部Flash中：

```c
// 将图片数据标记为存储在Flash中
static const uint8_t jiumi_img_map[] __attribute__((section(".rodata"))) = {
    // 图片数据
};
```

### 动态加载
也可以实现从SD卡或网络动态加载图片的功能。