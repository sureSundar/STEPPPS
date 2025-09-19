@echo off
REM TernaryBit OS - Windows Launcher
REM Universal Operating System - Calculator to Supercomputer

echo.
echo 🚀 TernaryBit OS - Windows Launcher
echo ===================================
echo Universal OS: Calculator to Supercomputer
echo Architecture Support: 8-bit to 64-bit
echo Platform: Windows
echo.

REM Check Java availability
java -version >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ Java not found. Please install Java 8 or higher.
    echo Download from: https://www.oracle.com/java/technologies/downloads/
    pause
    exit /b 1
)

REM Display Java version
echo ☕ Java Version:
java -version 2>&1 | findstr "version"

echo.
echo 🎯 Starting TernaryBit OS on Windows...
echo Hardware Detection: Automatic
echo Optimization: Adaptive
echo Press Ctrl+C to exit
echo.

REM Run TernaryBit OS
java -jar TernaryBitOS.jar

echo.
echo ✅ TernaryBit OS Windows Session Complete
pause