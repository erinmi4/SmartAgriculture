@echo off
chcp 65001 >nul
echo.
echo ========================================
echo     STM32项目大文件检查工具
echo     作者: Mika (202205570105)
echo ========================================
echo.

echo 正在检查大于50MB的文件...
echo.

powershell.exe -ExecutionPolicy Bypass -File "check_large_files.ps1"

echo.
echo 按任意键退出...
pause >nul
