# ChoomiPet项目编译烧录脚本
# 适用于Windows PowerShell环境

param(
    [string]$Action = "all",  # 可选: build, flash, monitor, all, clean
    [string]$Port = "COM4"    # 默认串口，可根据实际情况修改
)

# 脚本配置
$PROJECT_PATH = "main"
$TARGET_CHIP = "esp32c6"
$IDF_PATH = "D:/Espressif/frameworks/esp-idf-v5.4.1"
$env:IDF_PATH = $IDF_PATH

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

# 清理编译缓存
function Clear-BuildCache {
    Write-Info "清理编译缓存..."
    & idf.py fullclean
    if ($LASTEXITCODE -eq 0) {
        Write-Success "编译缓存清理完成"
    } else {
        Write-Warning "清理编译缓存时出现警告"
    }
}

# 编译项目
function Build-Project {
    Write-Info "开始编译项目..."
    & idf.py build
    if ($LASTEXITCODE -eq 0) {
        Write-Success "项目编译成功"
    } else {
        Write-Error "项目编译失败"
    }
}

# 烧录固件
function Flash-Firmware {
    param([string]$SerialPort)
    Write-Info "烧录固件到设备 (端口: $SerialPort)..."
    Write-Info "执行命令: idf.py -p $SerialPort flash"
    
    # 检查串口是否存在
    if (-not (Test-Path "$SerialPort")) {
        Write-Error "串口 $SerialPort 不存在，请检查设备连接"
        return
    }
    
    # 显示详细命令执行过程
    $output = & idf.py -p $SerialPort flash 2>&1 | Out-String
    Write-Info "命令输出:
$output"
    
    if ($LASTEXITCODE -eq 0) {
        Write-Success "固件烧录成功"
    } else {
        Write-Error "固件烧录失败"
    }
}

# 监控串口输出
function Start-Monitor {
    param([string]$SerialPort)
    Write-Info "开始监控串口输出 (端口: $SerialPort)..."
    Write-Info "按 Ctrl+] 退出监控"
    & idf.py -p $SerialPort monitor
}

# 主函数
function Main {
    # 获取脚本真实路径
    $SCRIPT_ROOT = $PSScriptRoot
    Set-Location $SCRIPT_ROOT

    Write-Info "ChoomiPet项目编译烧录脚本启动"
    Write-Info "操作模式: $Action"
    
    # 进入项目目录
    Push-Location "$PROJECT_PATH"
    
    try {
        # 根据操作模式执行相应操作
        switch ($Action.ToLower()) {
            "clean" { Clear-BuildCache }
            "build" { Build-Project }
            "flash" { Flash-Firmware -SerialPort $Port }
            "monitor" { Start-Monitor -SerialPort $Port }
            "all" { 
                Build-Project
                if ($LASTEXITCODE -eq 0) {
                    Flash-Firmware -SerialPort $Port
                    if ($LASTEXITCODE -eq 0) {
                        Write-Info "等待设备重启..."
                        Start-Sleep -Seconds 3
                        Start-Monitor -SerialPort $Port
                    }
                }
            }
            default {
                Write-Error "未知操作: $Action"
                Write-Info "可用操作: build, flash, monitor, all, clean"
            }
        }
    }
    finally {
        # 返回原目录
        Pop-Location
    }
}

# 执行主函数
Main