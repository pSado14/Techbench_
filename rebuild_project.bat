@echo off
echo Setting up environment...
set PATH=E:\Qt\Tools\CMake_64\bin;E:\Qt\Tools\Ninja;E:\Qt\Tools\mingw1310_64\bin;E:\Qt\6.9.2\mingw_64\bin;%PATH%

echo Cleaning build directory...
if exist build_cli (
    rmdir /s /q build_cli
)
mkdir build_cli
cd build_cli

echo Configuring with CMake...
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="E:\Qt\6.9.2\mingw_64" ..
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    exit /b %errorlevel%
)

echo Building...
cmake --build .
if %errorlevel% neq 0 (
    echo Build failed!
    exit /b %errorlevel%
)

echo Build successful!
echo Executable is in build_cli\Techbench.exe
