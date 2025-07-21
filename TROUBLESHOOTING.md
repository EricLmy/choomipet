# ESP32C6 图片显示项目故障排除指南

## 当前问题：COM4端口被占用

### 问题描述
烧录时出现错误：`Could not open COM4, the port is busy or doesn't exist.`

### 解决方案

#### 方案1：关闭占用端口的程序
1. 打开任务管理器（Ctrl+Shift+Esc）
2. 查找可能占用串口的程序：
   - Arduino IDE
   - PlatformIO
   - 串口调试助手
   - 其他ESP-IDF监控进程
3. 结束这些程序的进程
4. 重新尝试烧录

#### 方案2：重新插拔USB线
1. 拔掉ESP32C6的USB线
2. 等待5秒
3. 重新插入USB线
4. 等待设备重新识别
5. 重新尝试烧录

#### 方案3：检查其他可用端口
运行以下命令查看所有可用串口：
```powershell
Get-WmiObject -Class Win32_PnPEntity | Where-Object { $_.Caption -match "COM" } | Select-Object Caption, DeviceID
```

如果ESP32C6出现在其他COM端口（如COM3、COM5等），使用该端口进行烧录：
```bash
idf.py -p COM3 flash monitor
```

### 烧录命令
解决端口问题后，使用以下命令烧录和监控：

```bash
# 激活ESP-IDF环境并烧录
& "D:/Espressif/frameworks/esp-idf-v5.4.1/export.ps1"
idf.py -p COM4 flash monitor
```

### 预期结果
烧录成功后，串口监控应显示：
1. ESP32C6启动信息
2. LCD初始化成功
3. LVGL初始化成功
4. "Jiumi image loaded and displayed on LCD" 消息
5. LCD屏幕应显示：
   - 一个16x16像素的彩色方块图片（红、绿、蓝、黄色块组成）
   - 底部显示"Jiumi Image Display"文字

### 图片显示验证
如果文字显示正常但图片仍然不显示，可能的原因：
1. **图片数据格式问题**：当前使用的是测试图片数据，需要将实际的jiumi.jpg转换为C数组
2. **颜色格式不匹配**：可能需要调整LVGL的颜色格式设置
3. **图片尺寸问题**：16x16像素可能太小，在LCD上不明显

### 下一步优化建议
1. 使用LVGL官方图片转换工具将jiumi.jpg转换为正确的C数组格式
2. 增大测试图片尺寸（如64x64像素）
3. 调整图片在屏幕上的位置，确保完全可见
4. 添加更多调试信息，确认图片加载状态

### 联系支持
如果问题仍然存在，请提供：
1. 完整的错误日志
2. 设备管理器中的COM端口信息
3. 当前运行的相关程序列表