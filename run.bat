@echo off
set BANKOMP_KEY=IME_SE9_SECRET_KEY_2026
setlocal enabledelayedexpansion
set SOURCES=main.cpp
for %%f in (Control\*.cpp Interface\*.cpp Model\*.cpp Storage\*.cpp) do (
    set SOURCES=!SOURCES! %%f
)
g++ -std=c++17 !SOURCES! -o bankomp.exe
if %errorlevel% equ 0 (
    cls
    bankomp.exe
) else (
    echo.
    echo [ERRO] Falha na compilacao do projeto.
    echo.
)
pause