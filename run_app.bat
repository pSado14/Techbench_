@echo off
set PATH=E:\Qt\Tools\CMake_64\bin;E:\Qt\Tools\Ninja;E:\Qt\Tools\mingw1310_64\bin;E:\Qt\6.9.2\mingw_64\bin;%PATH%
echo Uygulama baslatiliyor...
set PATH=E:\Qt\6.9.2\mingw_64\bin;%PATH%
cd /d "e:\Qt Projects\proje1\Techbench\build_cli"
Techbench.exe > app_log.txt 2>&1
pause
