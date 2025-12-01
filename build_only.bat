@echo off
set PATH=E:\Qt\Tools\CMake_64\bin;E:\Qt\Tools\Ninja;E:\Qt\Tools\mingw1310_64\bin;E:\Qt\6.9.2\mingw_64\bin;%PATH%
cd build_cli
cmake --build .
