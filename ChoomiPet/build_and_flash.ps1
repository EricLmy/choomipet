# ChoomiPet项目编译烧录脚本
# 适用于Windows PowerShell环境

param(
    [string]$Action = "all",  # 可选: build, flash, monitor, all, clean
    [string]$Port = "COM4"    # 默认串口，可根据实际情况修改
)

# 脚本配置
$PROJECT_PATH = "ChoomiPet"
$TARGET_CHIP = "esp32c6"

# 动态获取IDF路径
if (-not $env:IDF_PATH) {
    $IDF_PATH = "$env:USERPROFILE/esp/esp-idf"
} else {
    $IDF_PATH = $env:IDF_PATH
}

# 添加权限检查
function Test-Administrator {
    $user = [Security.Principal.WindowsIdentity]::GetCurrent();
    (New-Object Security.Principal.WindowsPrincipal $user).IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)
}

# 颜色输出函数
function Write-ColorOutput($ForegroundColor) {
    $fc = $host.UI.RawUI.ForegroundColor
    $host.UI.RawUI.ForegroundColor = $ForegroundColor
    if ($args) {
        Write-Output $args
    }
    $host.UI.RawUI.ForegroundColor = $fc
}

function Write-Success($message) {
    Write-ColorOutput Green "✅ $message"
}

function Write-Info($message) {
    Write-ColorOutput Cyan "ℹ️  $message"
}

function Write-Warning($message) {
    Write-ColorOutput Yellow "⚠️  $message"
}

function Write-Error($message) {
    Write-ColorOutput Red "❌ $message"
}

# 检查ESP-IDF环境
function Test-ESPIDFEnvironment {
    Write-Info "检查ESP-IDF环境..."
    
    if (-not (Test-Path $IDF_PATH)) {
        Write-Error "ESP-IDF路径不存在: $IDF_PATH"
        return $false
    }
    
    if (-not (Test-Path "$IDF_PATH/export.ps1")) {
        Write-Error "ESP-IDF export.ps1脚本不存在"
        return $false
    }
    
    Write-Success "ESP-IDF环境检查通过"
    return $true
}

# 设置ESP-IDF环境
function Set-ESPIDFEnvironment {
    Write-Info "设置ESP-IDF环境变量..."
    
    try {
        # 执行ESP-IDF环境设置脚本
        & "$IDF_PATH/export.ps1"
        
        # 验证idf.py是否可用
        $idfVersion = & idf.py --version 2>$null
        if ($LASTEXITCODE -eq 0) {
            Write-Success "ESP-IDF环境设置成功: $idfVersion"
            return $true
        } else {
            Write-Error "idf.py命令不可用"
            return $false
        }
    }
    catch {
        Write-Error "设置ESP-IDF环境失败: $($_.Exception.Message)"
        return $false
    }
}

# 设置目标芯片
function Set-TargetChip {
    Write-Info "设置目标芯片为 $TARGET_CHIP..."
    
    try {
        & idf.py set-target $TARGET_CHIP
        if ($LASTEXITCODE -eq 0) {
            Write-Success "目标芯片设置成功"
            return $true
        } else {
            Write-Error "设置目标芯片失败"
            return $false
        }
    }
    catch {
        Write-Error "设置目标芯片异常: $($_.Exception.Message)"
        return $false
    }
}

# 清理编译缓存
function Clear-BuildCache {
    Write-Info "清理编译缓存..."
    
    try {
        & idf.py fullclean
        if ($LASTEXITCODE -eq 0) {
            Write-Success "编译缓存清理完成"
            return $true
        } else {
            Write-Warning "清理编译缓存时出现警告"
            return $true
        }
    }
    catch {
        Write-Error "清理编译缓存失败: $($_.Exception.Message)"
        return $false
    }
}
}

# 编译项目
function Build-Project {
    Write-Info "开始编译项目..."
    
    try {
        & idf.py build
        if ($LASTEXITCODE -eq 0) {
            Write-Success "项目编译成功"
            return $true
        } else {
            Write-Error "项目编译失败"
            return $false
        }
    }
    catch {
        Write-Error "编译过程异常: $($_.Exception.Message)"
        return $false
    }
}

# 烧录固件
function Flash-Firmware {
    param([string]$SerialPort)
    
    Write-Info "烧录固件到设备 (端口: $SerialPort)..."
    
    try {
        & idf.py -p $SerialPort flash
        if ($LASTEXITCODE -eq 0) {
            Write-Success "固件烧录成功"
            return $true
        } else {
            Write-Error "固件烧录失败"
            return $false
        }
    }
    catch {
        Write-Error "烧录过程异常: $($_.Exception.Message)"
        return $false
    }
}

# 监控串口输出
function Start-Monitor {
    param([string]$SerialPort)
    
    Write-Info "开始监控串口输出 (端口: $SerialPort)..."
    Write-Info "按 Ctrl+] 退出监控"
    
    try {
        & idf.py -p $SerialPort monitor
    }
    catch {
        Write-Error "监控过程异常: $($_.Exception.Message)"
    }
}

# 显示项目信息
function Show-ProjectInfo {
    Write-Info "=== ChoomiPet项目信息 ==="
    Write-Info "项目路径: $(Get-Location)/$PROJECT_PATH"
    Write-Info "ESP-IDF路径: $IDF_PATH"
    Write-Info "目标芯片: $TARGET_CHIP"
    Write-Info "串口: $Port"
    Write-Info "=========================="
}

# 主函数
function Main {
    # 管理员权限检查
    if (-not (Test-Administrator)) {
        Write-Warning "建议使用管理员权限运行脚本(右键使用管理员身份运行)"
        Start-Sleep -Seconds 2
    }

    # 获取脚本真实路径
    $SCRIPT_ROOT = $PSScriptRoot
    Set-Location $SCRIPT_ROOT

    Write-Info "ChoomiPet项目编译烧录脚本启动"
    Write-Info "操作模式: $Action"
    
    # 检查项目目录
    if (-not (Test-Path "$SCRIPT_ROOT/$PROJECT_PATH")) {
        Write-Error "项目目录不存在: $SCRIPT_ROOT/$PROJECT_PATH"
        exit 1
    }
    
    # 进入项目目录
    Push-Location "$PROJECT_PATH"
    
    try {
        # 显示项目信息
        Show-ProjectInfo
        
        # 检查和设置ESP-IDF环境
        if (-not (Test-ESPIDFEnvironment)) {
            exit 1
        }
        
        if (-not (Set-ESPIDFEnvironment)) {
            Write-Error "环境设置失败，请检查:
            1. IDF_PATH是否正确(当前: $IDF_PATH)
            2. 是否已安装ESP-IDF依赖项
            3. Python环境是否配置正确"
            exit 1
        }
        
        # 根据操作模式执行相应操作
        switch ($Action.ToLower()) {
            "clean" {
                Clear-BuildCache
            }
            "build" {
                Set-TargetChip
                Build-Project
            }
            "flash" {
                Flash-Firmware -SerialPort $Port
            }
            "monitor" {
                Start-Monitor -SerialPort $Port
            }
            "all" {
                Set-TargetChip
                if (Build-Project) {
                    if (Flash-Firmware -SerialPort $Port) {
                        Write-Info "等待设备重启..."
                        Start-Sleep -Seconds 3
                        Start-Monitor -SerialPort $Port
                    }
                }
            }
            default {
                Write-Error "未知操作: $Action"
                Write-Info "可用操作: build, flash, monitor, all, clean"
                exit 1
            }
        }
        
        Write-Success "脚本执行完成"
    }
    finally {
        # 返回原目录
        Pop-Location
    }
}
}

# 脚本使用说明
function Show-Usage {
    Write-Info @"
ChoomiPet项目编译烧录脚本使用说明:

基本用法:
  .\build_and_flash.ps1                    # 编译+烧录+监控 (默认)
  .\build_and_flash.ps1 -Action build     # 仅编译
  .\build_and_flash.ps1 -Action flash     # 仅烧录
  .\build_and_flash.ps1 -Action monitor   # 仅监控
  .\build_and_flash.ps1 -Action clean     # 清理编译缓存

指定串口:
  .\build_and_flash.ps1 -Port COM5        # 使用COM5端口

组合使用:
  .\build_and_flash.ps1 -Action all -Port COM5

注意事项:
1. 确保ESP32-C6开发板已连接到电脑
2. 确认串口号是否正确 (可通过设备管理器查看)
3. 首次运行可能需要管理员权限
4. 监控模式下按 Ctrl+] 退出
"@
}

# 检查是否需要显示帮助
if ($args -contains "-h" -or $args -contains "--help" -or $args -contains "help") {
    Show-Usage
    exit 0
}

# 执行主函数
Main