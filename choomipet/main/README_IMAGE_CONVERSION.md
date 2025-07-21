# 图片显示说明文档

## 概述
本项目已配置为在LCD屏幕上显示jiumi.jpg图片。当前使用的是示例图片数据，要显示真实的jiumi.jpg图片，需要按照以下步骤进行转换。

## 图片转换步骤

### 1. 访问LVGL图片转换工具
打开浏览器访问：https://lvgl.io/tools/imageconverter

### 2. 上传图片文件
- 点击"Choose file"按钮
- 选择 `e:/workplace/Choomi/src/ChoomiPet/images/jiumi.jpg` 文件
- 上传完成后会显示图片预览

### 3. 配置转换参数
**颜色格式选择：**
- 如果图片有透明通道：选择 `CF_TRUE_COLOR_ALPHA`
- 如果图片无透明通道：选择 `CF_TRUE_COLOR`
- 对于JPG格式，通常选择 `CF_TRUE_COLOR`

**输出格式选择：**
- 选择 `C array`

**其他设置：**
- Dither: 可选择 `Ordered` 或 `Floyd-Steinberg` 提高图片质量
- 如果LCD屏幕较小，可以调整输出尺寸

### 4. 下载转换结果
- 点击 `Convert` 按钮开始转换
- 转换完成后下载生成的C文件

### 5. 替换示例数据
将下载的C文件内容替换到 `jiumi_img.c` 文件中：

```c
// 替换 jiumi_img_map 数组数据
const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_IMG_JIUMI uint8_t jiumi_img_map[] = {
    // 这里粘贴转换工具生成的图片数据
};

// 更新图片描述符中的宽度和高度
const lv_img_dsc_t jiumi_img = {
    .header.cf = LV_IMG_CF_TRUE_COLOR,  // 根据选择的颜色格式调整
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 实际图片宽度,  // 替换为真实宽度
    .header.h = 实际图片高度,  // 替换为真实高度
    .data_size = sizeof(jiumi_img_map),
    .data = jiumi_img_map,
};
```

## 当前代码结构

### 文件说明
- `choomipet.c`: 主程序文件，包含图片显示逻辑
- `jiumi_img.c`: 图片数据文件，包含转换后的图片数组
- `CMakeLists.txt`: 构建配置文件，已添加jiumi_img.c

### 显示逻辑
```c
// 声明图片资源
LV_IMG_DECLARE(jiumi_img);

// 创建图片对象
lv_obj_t *img = lv_img_create(lv_scr_act());
// 设置图片源
lv_img_set_src(img, &jiumi_img);
// 居中对齐
lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
```

## 编译和运行

1. 确保ESP-IDF环境已正确配置
2. 在项目根目录执行：
   ```bash
   idf.py build
   idf.py flash monitor
   ```

## 注意事项

1. **内存限制**: 大尺寸图片会占用较多内存，建议根据ESP32-C6的内存容量调整图片尺寸
2. **颜色格式**: 选择合适的颜色格式可以平衡显示质量和内存使用
3. **LCD分辨率**: 确保图片尺寸适合LCD屏幕分辨率
4. **编译时间**: 大图片数据会增加编译时间

## 故障排除

- 如果显示异常，检查颜色格式是否正确
- 如果编译失败，检查图片数据大小是否超出内存限制
- 如果图片不显示，确认图片声明和引用是否正确

## 优化建议

1. 对于动画效果，可以准备多张图片实现帧动画
2. 使用压缩格式可以减少内存占用
3. 考虑使用外部存储（如SD卡）存储大图片文件