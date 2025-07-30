@echo off
REM ChoomiPet项目编译烧录批处理脚本
REM 适用于Windows CMD环境

setlocal enabledelayedexpansion

REM 脚本配置
set IDF_PATH=D:\Espressif\frameworks\esp-idf-v5.4.1
set PROJECT_PATH=ChoomiPet
set TARGET_CHIP=esp32c6
set DEFAULT_PORT=COM4

REM 解析命令行参数
set ACTION=%1
set SERIAL_PORT=%2

if "%ACTION%"=="" set ACTION=all
if "%SERIAL_PORT%"=="" set SERIAL_PORT=%DEFAULT_PORT%

echo.
echo ========================================
echo ChoomiPet项目编译烧录脚本
echo ========================================
echo 操作模式: %ACTION%
echo 串口: %SERIAL_PORT%
echo 项目路径: %PROJECT_PATH%
echo ESP-IDF路径: %IDF_PATH%
echo 目标芯片: %TARGET_CHIP%
echo ========================================
echo.

REM 检查项目目录
if not exist "%PROJECT_PATH%" (
    echo [错误] 项目目录不存在: %PROJECT_PATH%
    pause
    exit /b 1
)

REM 检查ESP-IDF环境
if not exist "%IDF_PATH%" (
    echo [错误] ESP-IDF路径不存在: %IDF_PATH%
    pause
    exit /b 1
)

if not exist "%IDF_PATH%\export.bat" (
    echo [错误] ESP-IDF export.bat脚本不存在
    pause
    exit /b 1
)

REM 进入项目目录
cd /d "%PROJECT_PATH%"

REM 设置ESP-IDF环境
echo [信息] 设置ESP-IDF环境...
call "%IDF_PATH%\export.bat"

REM 验证idf.py是否可用
idf.py --version >nul 2>&1
if errorlevel 1 (
    echo [错误] idf.py命令不可用，请检查ESP-IDF环境设置
    pause
    exit /b 1
)

echo [成功] ESP-IDF环境设置完成

REM 根据操作模式执行相应操作
if /i "%ACTION%"=="clean" goto :clean
if /i "%ACTION%"=="build" goto :build
if /i "%ACTION%"=="flash" goto :flash
if /i "%ACTION%"=="monitor" goto :monitor
if /i "%ACTION%"=="all" goto :all

echo [错误] 未知操作: %ACTION%
echo 可用操作: build, flash, monitor, all, clean
pause
exit /b 1

:clean
echo [信息] 清理编译缓存...
idf.py fullclean
if errorlevel 1 (
    echo [警告] 清理编译缓存时出现警告
) else (
    echo [成功] 编译缓存清理完成
)
goto :end

:build
echo [信息] 设置目标芯片为 %TARGET_CHIP%...
idf.py set-target %TARGET_CHIP%
if errorlevel 1 (
    echo [错误] 设置目标芯片失败
    pause
    exit /b 1
)

echo [信息] 开始编译项目...
idf.py build
if errorlevel 1 (
    echo [错误] 项目编译失败
    pause
    exit /b 1
)
echo [成功] 项目编译成功
goto :end

:flash
echo [信息] 烧录固件到设备 (端口: %SERIAL_PORT%)...
idf.py -p %SERIAL_PORT% flash
if errorlevel 1 (
    echo [错误] 固件烧录失败
    pause
    exit /b 1
)
echo [成功] 固件烧录成功
goto :end

:monitor
echo [信息] 开始监控串口输出 (端口: %SERIAL_PORT%)...
echo [信息] 按 Ctrl+] 退出监控
idf.py -p %SERIAL_PORT% monitor
goto :end

:all
echo [信息] 执行完整流程: 编译 + 烧录 + 监控
echo.

REM 设置目标芯片
echo [信息] 设置目标芯片为 %TARGET_CHIP%...
idf.py set-target %TARGET_CHIP%
if errorlevel 1 (
    echo [错误] 设置目标芯片失败
    pause
    exit /b 1
)

REM 编译项目
echo [信息] 开始编译项目...
idf.py build
if errorlevel 1 (
    echo [错误] 项目编译失败
    pause
    exit /b 1
)
echo [成功] 项目编译成功

REM 烧录固件
echo [信息] 烧录固件到设备 (端口: %SERIAL_PORT%)...
idf.py -p %SERIAL_PORT% flash
if errorlevel 1 (
    echo [错误] 固件烧录失败
    pause
    exit /b 1
)
echo [成功] 固件烧录成功

REM 等待设备重启
echo [信息] 等待设备重启...
timeout /t 3 /nobreak >nul

REM 开始监控
echo [信息] 开始监控串口输出 (端口: %SERIAL_PORT%)...
echo [信息] 按 Ctrl+] 退出监控
idf.py -p %SERIAL_PORT% monitor

:end
echo.
echo [完成] 脚本执行完成
cd /d "%~dp0"
pause