@echo off
rem Compila com MinGW g++ (ou use CMake / Visual Studio com o CMakeLists.txt)
cd /d "%~dp0"
g++ -std=c++17 -O2 -Wall -mwindows src\*.cpp -o DemonSlayer.exe -lgdi32 -luser32 -lwinmm -static
if errorlevel 1 (echo Falha na compilacao & exit /b 1)
echo OK: DemonSlayer.exe
