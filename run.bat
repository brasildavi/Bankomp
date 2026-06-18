@echo off
set BANKOMP_KEY=IME_SE9_SECRET_KEY_2026

if not exist build mkdir build
cd build

cmake ..
cmake --build . --config Release

if %errorlevel% equ 0 (
    cls
    inline_execution
    Release\bankomp.exe
) else (
    echo.
    echo [ERRO] Falha na compilacao do projeto via CMake.
    echo.
)
cd ..
pause