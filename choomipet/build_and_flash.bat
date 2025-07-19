@echo off
REM 设置 ESP-IDF 环境
call D:\Espressif\frameworks\esp-idf-v5.4.1\export.bat

REM 切换到项目目录
cd /d %~dp0

REM 构建项目
echo "Building project..."
idf.py build

if %errorlevel% neq 0 (
    echo "Build failed!"
    exit /b %errorlevel%
)

REM 烧录项目
echo "Flashing project..."
idf.py -p COM4 flash monitor

if %errorlevel% neq 0 (
    echo "Flash failed!"
    exit /b %errorlevel%
)

echo "Done!"