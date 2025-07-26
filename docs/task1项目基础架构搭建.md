# 任务1: 项目基础架构搭建   （已完成）

- [ ] 1. 项目基础架构搭建
  - 创建ESP-IDF项目结构和基础配置
  - 设置硬件抽象层和组件架构
  - 配置编译环境和调试工具
  - _需求: 1.1, 1.6_
  - _对应主程序: app_main()函数和系统引导阶段_

  **开发前提条件：**
  - ✅ ESP-IDF v5.4.1已安装在D:/Espressif/frameworks/esp-idf-v5.4.1/
  - ✅ ESP32-C6开发板已准备并可正常连接
  - ✅ USB数据线（用于烧录和调试）
  - ✅ 串口调试工具（如PuTTY或ESP-IDF Monitor）
  - ✅ 代码编辑器或IDE（如VS Code + ESP-IDF插件）
  - ✅ Git版本控制工具
  - ✅ 项目需求文档和设计文档已确认

  **必需准备材料：**
  - 📋 项目目录结构规划文档
  - 📋 组件依赖关系图
  - 📋 编译配置模板文件
  - 📋 调试配置文件模板
  - 📋 代码规范和命名约定文档
  - 📋 版本控制策略文档

  **具体实现内容（基于主程序设计）：**
  ```c
  // 1. 创建main/main.c - 主程序入口
  void app_main(void) {
      // NVS Flash初始化
      esp_err_t ret = nvs_flash_init();
      if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
          ESP_ERROR_CHECK(nvs_flash_erase());
          ret = nvs_flash_init();
      }
      ESP_ERROR_CHECK(ret);
      
      // 日志系统初始化
      esp_log_level_set("*", ESP_LOG_INFO);
      ESP_LOGI(TAG, "Choomi Hardware Starting...");
      
      // 看门狗初始化
      esp_task_wdt_init(WDT_TIMEOUT, true);
      
      // 进入硬件初始化阶段
      hardware_init_start();
  }
  
  // 2. 创建components目录结构
  components/
  ├── audio/           # 音频处理组件
  ├── display/         # 显示控制组件
  ├── input/           # 输入处理组件
  ├── network/         # 网络通信组件
  ├── hardware/        # 硬件配置组件
  ├── system/          # 系统管理组件
  └── common/          # 公共定义组件
  
  // 3. 创建include/choomi_common.h - 公共定义
  #define TAG "CHOOMI"
  #define WDT_TIMEOUT 30
  #define MAX_RECORDING_TIME_MS 30000
  #define HEARTBEAT_INTERVAL_MS 30000
  ```

  **详细任务内容：**
  - 创建ESP-IDF v5.4.1项目，设置目标芯片为ESP32-C6
  - 建立模块化组件架构：audio/、display/、input/、network/、hardware/组件
  - 配置CMakeLists.txt和component.mk文件
  - 设置sdkconfig基础配置（CPU频率160MHz、FreeRTOS tick 1000Hz）
  - 创建main.c主程序入口和基础日志系统
  - 配置UART调试接口和日志输出级别

  **验收标准：**
  - 项目能够成功编译并生成.bin固件文件
  - 固件能够正常烧录到ESP32-C6开发板
  - 串口监控能够输出"Choomi Hardware Starting..."启动日志
  - 各组件目录结构完整，包含必要的CMakeLists.txt文件
  - 系统能够正常启动并进入main函数