@echo off
echo Setting up full environment...
set PATH=E:\Qt\Tools\CMake_64\bin;E:\Qt\Tools\Ninja;E:\Qt\Tools\mingw1310_64\bin;E:\Qt\6.9.2\mingw_64\bin;%PATH%

echo Current PATH:
echo %PATH% > path_debug.txt

echo Launching Techbench...
cd build_cli
Techbench.exe > app_output.txt 2>&1
if %errorlevel% neq 0 (
    echo App crashed with error level %errorlevel%
    echo App crashed with error level %errorlevel% >> app_output.txt
) else (
    echo App exited normally.
)
pause
