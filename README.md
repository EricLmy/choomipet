# Choomi Pet - ESP32-C6 Hardware Project

## 项目简介

Choomi Pet是一个基于ESP32-C6的智能硬件项目，具备音频录制播放、LCD显示、按键输入、WiFi网络连接和RGB状态指示等功能。

## 硬件要求

- ESP32-C6开发板
- I2S音频模块（麦克风和扬声器）
- SPI LCD显示屏
- RGB LED
- 按键开关（录音、播放、菜单）
- USB数据线

## 开发环境

- ESP-IDF v5.4.1
- 目标芯片：ESP32-C6
- 编译工具链：Xtensa GCC

## 项目结构

```
ChoomiPet/
├── main/                   # 主程序
│   ├── main.c             # 程序入口
│   ├── choomi_common.h    # 公共定义
│   └── CMakeLists.txt     # 主程序构建配置
├── components/            # 组件目录
│   ├── audio/            # 音频处理组件
│   ├── display/          # 显示控制组件
│   ├── input/            # 输入处理组件
│   ├── network/          # 网络通信组件
│   ├── hardware/         # 硬件配置组件
│   ├── system/           # 系统管理组件
│   └── common/           # 公共工具组件
├── docs/                 # 文档目录
├── CMakeLists.txt        # 项目构建配置
├── sdkconfig.defaults    # 默认配置
└── README.md            # 项目说明
```

## 编译和烧录

1. 设置ESP-IDF环境：
```bash
. $HOME/esp/esp-idf/export.sh
```

2. 配置项目：
```bash
idf.py set-target esp32c6
idf.py menuconfig
```

3. 编译项目：
```bash
idf.py build
```

4. 烧录固件：
```bash
idf.py -p PORT flash
```

5. 监控串口输出：
```bash
idf.py -p PORT monitor
```

## 功能特性

- ✅ 模块化组件架构
- ✅ 硬件抽象层
- ✅ 系统状态管理
- ✅ 日志系统
- ✅ 看门狗保护
- 🔄 I2S音频录制播放（待实现）
- 🔄 SPI LCD显示控制（待实现）
- 🔄 GPIO按键输入处理（待实现）
- 🔄 WiFi网络连接（待实现）
- 🔄 WebSocket通信（待实现）
- 🔄 RGB状态指示（待实现）

## GPIO引脚定义

| 功能 | GPIO | 说明 |
|------|------|------|
| RGB LED R | GPIO8 | 红色LED |
| RGB LED G | GPIO9 | 绿色LED |
| RGB LED B | GPIO10 | 蓝色LED |
| 录音按键 | GPIO0 | 录音功能按键 |
| 播放按键 | GPIO1 | 播放功能按键 |
| 菜单按键 | GPIO2 | 菜单功能按键 |
| LCD CS | GPIO5 | LCD片选信号 |
| LCD DC | GPIO6 | LCD数据/命令选择 |
| LCD RST | GPIO7 | LCD复位信号 |
| I2S BCLK | GPIO15 | I2S位时钟 |
| I2S WS | GPIO16 | I2S字选择 |
| I2S DIN | GPIO17 | I2S数据输入 |
| I2S DOUT | GPIO18 | I2S数据输出 |

## 系统配置

- CPU频率：160MHz
- FreeRTOS Tick频率：1000Hz
- 看门狗超时：30秒
- 串口波特率：115200
- Flash大小：4MB

## 开发计划

项目按照以下任务顺序进行开发：

1. ✅ 项目基础架构搭建
2. 🔄 硬件初始化和GPIO配置
3. 🔄 I2S音频系统实现
4. 🔄 SPI-LCD显示系统实现
5. 🔄 按键输入系统实现
6. 🔄 WiFi网络连接实现
7. 🔄 WebSocket通信系统实现
8. 🔄 RGB状态指示灯实现
9. 🔄 设备状态管理系统实现
10. 🔄 设备注册与身份验证实现（可选）
11. 🔄 系统集成和性能优化
12. 🔄 测试验证和质量保证

## 许可证

本项目采用MIT许可证。