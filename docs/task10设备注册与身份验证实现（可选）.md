# 任务10: 设备注册与身份验证实现（可选）

- [ ] 10. 设备注册与身份验证实现（可选）
  - [ ] 10.1 实现设备唯一ID生成和管理
    - 编写基于MAC地址的设备ID生成代码
    - 实现设备信息收集和唯一码计算
    - 添加设备ID的加密存储和验证
    - 创建设备标识的安全管理机制
    - _需求: 9.1, 9.3, 9.5_

    **开发前提条件：**
    - ✅ 任务9设备状态管理系统已完成
    - ✅ NVS加密存储功能已配置
    - ✅ 加密算法库已集成

    **必需准备材料：**
    - 📋 设备ID生成算法规范
    - 📋 加密存储安全策略
    - 📋 设备注册协议文档
    - 📋 身份验证流程图
    - 📋 安全密钥管理方案

    **具体实现内容：**
    ```c
    // components/system/device_auth.h
    typedef struct {
        char device_id[64];
        char device_secret[128];
        char access_token[128];
        char refresh_token[128];
        uint64_t token_expire_time;
        bool registered;
    } device_auth_t;

    esp_err_t device_auth_init(auth_config_t *config);
    esp_err_t device_auth_generate_id(char *device_id, size_t size);
    
    // components/system/device_auth.c
    esp_err_t device_auth_generate_id(char *device_id, size_t size) {
        uint8_t mac[6];
        esp_wifi_get_mac(WIFI_IF_STA, mac);
        
        // 获取芯片ID
        uint32_t chip_id = 0;
        for (int i = 0; i < 17; i = i + 8) {
            chip_id |= ((esp_efuse_get_field_value(ESP_EFUSE_MAC_FACTORY_CRC) >> i) & 0xff) << (16 - i);
        }
        
        // 生成设备唯一码
        char temp_id[128];
        snprintf(temp_id, sizeof(temp_id), "%02X%02X%02X%02X%02X%02X-%08X-%s-%s",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                 chip_id, "CHOOMI", "V1.0");
        
        // SHA256哈希
        uint8_t hash[32];
        mbedtls_sha256((uint8_t*)temp_id, strlen(temp_id), hash, 0);
        
        // 格式化为标准设备ID
        snprintf(device_id, size, "CHOOMI-%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                 hash[0], hash[1], hash[2], hash[3],
                 hash[4], hash[5], hash[6], hash[7],
                 hash[8], hash[9], hash[10], hash[11],
                 hash[12], hash[13], hash[14], hash[15]);
        
        return ESP_OK;
    }
    ```

    **详细任务内容：**
    - 创建device_auth.h定义设备认证接口和数据结构
    - 实现device_auth.c包含设备ID生成和管理函数
    - 实现设备ID生成：基于MAC地址+设备型号+生产批次+随机种子的SHA256哈希
    - 创建设备信息收集：获取MAC地址、芯片ID、固件版本等硬件信息
    - 实现设备ID格式化：生成"CHOOMI-XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"格式
    - 添加AES-256加密存储：将设备ID和密钥安全存储到NVS
    - 实现设备ID验证：启动时验证存储的设备ID有效性
    - 创建设备标识管理：支持设备ID查询、更新、重置等操作

    **验收标准：**
    - 设备ID生成算法稳定，相同硬件生成相同ID
    - 设备ID格式符合规范，长度和格式正确
    - 设备信息收集完整，包含所有必要的硬件标识
    - AES加密存储安全可靠，重启后能够正确解密读取
    - 设备ID验证机制有效，能够检测ID篡改或损坏
    - 设备标识管理功能完整，支持各种操作场景

  - [ ] 10.2 实现设备注册和服务器通信
    - 编写设备注册请求和响应处理代码
    - 实现HTTPS安全通信和证书验证
    - 添加注册状态管理和错误处理
    - 创建注册流程的用户界面和提示
    - _需求: 9.2, 9.4, 9.9_

    **开发前提条件：**
    - ✅ 任务10.1 设备唯一ID生成和管理已完成
    - ✅ HTTPS客户端库已集成
    - ✅ 服务器注册接口已确认

    **必需准备材料：**
    - 📋 设备注册API文档
    - 📋 HTTPS通信安全规范
    - 📋 证书验证流程
    - 📋 注册界面设计稿
    - 📋 错误处理策略

    **详细任务内容：**
    - 创建device_register.h定义注册接口和状态枚举
    - 实现device_register.c包含设备注册和通信函数
    - 实现HTTPS客户端：支持TLS 1.2/1.3，证书链验证
    - 创建注册请求：包含设备ID、硬件信息、固件版本等
    - 实现注册响应处理：解析服务器返回的注册状态和访问令牌
    - 添加注册状态管理：未注册、注册中、已注册、注册失败等状态
    - 实现注册重试机制：指数退避算法，最大重试3次
    - 创建注册界面：LCD显示注册进度，RGB LED指示注册状态

    **验收标准：**
    - HTTPS通信安全可靠，支持证书验证和加密传输
    - 注册请求格式正确，包含所有必要的设备信息
    - 注册响应处理准确，能够正确解析服务器返回数据
    - 注册状态管理完整，状态转换逻辑正确
    - 注册重试机制智能，避免频繁请求造成服务器压力
    - 注册界面友好，用户能够清楚了解注册进度和状态

  - [ ] 10.3 实现身份验证和令牌管理
    - 编写访问令牌的获取和刷新代码
    - 实现消息签名验证和安全检查
    - 添加令牌过期处理和自动更新
    - 创建身份验证失败的安全响应
    - _需求: 9.6, 9.7, 9.8, 9.10_

    **开发前提条件：**
    - ✅ 任务10.2 设备注册和服务器通信已完成
    - ✅ JWT令牌处理库已集成
    - ✅ 消息签名算法已确认

    **必需准备材料：**
    - 📋 JWT令牌格式规范
    - 📋 消息签名算法文档
    - 📋 令牌刷新机制设计
    - 📋 安全响应策略
    - 📋 身份验证测试用例

    **详细任务内容：**
    - 创建token_manager.h定义令牌管理接口和安全机制
    - 实现token_manager.c包含令牌获取、刷新和验证函数
    - 实现访问令牌获取：注册成功后从服务器获取JWT令牌
    - 创建令牌刷新机制：令牌过期前自动刷新，支持refresh token
    - 实现消息签名：使用HMAC-SHA256对WebSocket消息进行签名
    - 添加签名验证：验证服务器消息的数字签名，防止伪造攻击
    - 实现令牌过期处理：检测令牌过期，自动重新认证
    - 创建安全响应：认证失败时记录日志，拒绝执行指令，通知用户

    **验收标准：**
    - 访问令牌获取成功，格式符合JWT标准
    - 令牌刷新机制可靠，能够在过期前自动更新
    - 消息签名算法正确，签名验证通过率100%
    - 签名验证机制有效，能够检测和拒绝伪造消息
    - 令牌过期处理及时，不会因过期令牌导致通信中断
    - 安全响应机制完善，认证失败时能够正确处理和提示