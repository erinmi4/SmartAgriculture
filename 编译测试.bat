@echo off
echo 正在编译STM32项目...
echo.

cd /d "d:\Intership_stm32\SmartAgriculture\USER\stm32f407project"

REM 检查Keil是否在PATH中
where keil >nul 2>&1
if %errorlevel% neq 0 (
    echo 警告: 找不到Keil编译器，请确保已安装Keil MDK-ARM
    echo 请手动打开Keil MDK，加载 stm32f407project.uvprojx 项目文件
    echo.
    echo 然后点击编译按钮进行编译
    pause
    exit /b 1
)

REM 尝试使用命令行编译
echo 尝试使用命令行编译...
keil -j0 -o ".\Objects\build.log" "stm32f407project.uvprojx"

if %errorlevel% equ 0 (
    echo 编译成功！
) else (
    echo 编译失败，请检查错误信息
    type ".\Objects\build.log"
)

echo.
echo 编译完成。现在应该包含以下MQ2文件：
echo - mq2.c ^(包含所有MQ2函数实现^)
echo - mq2.h ^(包含所有MQ2函数声明^)
echo.
echo 如果仍有链接错误，请：
echo 1. 打开Keil MDK
echo 2. 加载 stm32f407project.uvprojx
echo 3. 检查Project面板中是否包含 HARDWARE 组下的 mq2.c 和 mq2.h
echo 4. 右键项目 -^> Options -^> C/C++ -^> Include Paths 确认包含了 ..\..\HARDWARE\MQ
echo 5. 重新编译

pause
