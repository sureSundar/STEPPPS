@echo off
echo 🕉️  TernaryBit OS - Sanatana Dharma Computing
echo Platform: Windows
echo.

REM Try Java first
java -version >nul 2>&1
if %errorlevel% == 0 (
    echo ☕ Launching Java Universal TBOS...
    java TernaryBitOS
) else if exist tbos_universal.exe (
    echo ⚡ Launching Native Universal TBOS...
    tbos_universal.exe
) else (
    echo ❌ No compatible TBOS implementation found
    echo Please install Java or use native binary
    pause
)
