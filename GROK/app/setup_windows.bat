@echo off
REM TBOS - Automatic Windows Setup (MinGW Installer)
REM 🕉 Swamiye Saranam Aiyappa 🕉

title TBOS Windows Setup

echo ╔══════════════════════════════════════════════════════════╗
echo ║  TBOS Universal Detector - Windows Setup                ║
echo ║     🕉 Swamiye Saranam Aiyappa 🕉                       ║
echo ╚══════════════════════════════════════════════════════════╝
echo.

echo [SETUP] Checking for compiler...
echo.

REM Check if compiler exists
where gcc >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo ✓ GCC/MinGW is already installed!
    echo.
    goto :build
)

where cl >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo ✓ Microsoft Visual C++ is already installed!
    echo.
    goto :build
)

REM No compiler found - need to install MinGW
echo ✗ No compiler found!
echo.
echo We need to install MinGW (GCC compiler for Windows)
echo.
echo Option 1 - AUTOMATIC (Recommended):
echo   I will download and setup MinGW for you
echo.
echo Option 2 - MANUAL:
echo   You download and install from https://www.mingw-w64.org/
echo.

set /p choice="Choose option (1 or 2): "

if "%choice%"=="2" goto :manual
if "%choice%"=="1" goto :auto

echo Invalid choice. Defaulting to automatic...
goto :auto

:auto
echo.
echo [DOWNLOAD] Downloading MinGW...
echo.

REM Create temp directory
if not exist "%TEMP%\tbos_setup" mkdir "%TEMP%\tbos_setup"
cd /d "%TEMP%\tbos_setup"

REM Download MinGW using PowerShell
echo Downloading MinGW-w64 (this may take a few minutes)...
powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://github.com/niXman/mingw-builds-binaries/releases/download/13.2.0-rt_v11-rev1/x86_64-13.2.0-release-posix-seh-ucrt-rt_v11-rev1.7z' -OutFile 'mingw.7z'}"

if not exist mingw.7z (
    echo.
    echo [ERROR] Download failed!
    echo Please download manually from: https://github.com/niXman/mingw-builds-binaries/releases
    echo.
    pause
    exit /b 1
)

echo.
echo [INSTALL] Installing MinGW to C:\MinGW...
echo.

REM Extract (requires 7-Zip or built-in Windows extraction)
powershell -Command "& {Expand-Archive -Force -Path mingw.7z -DestinationPath C:\MinGW}"

if not exist C:\MinGW\bin\gcc.exe (
    echo.
    echo [ERROR] Installation failed!
    echo Please install manually
    echo.
    pause
    exit /b 1
)

echo.
echo [PATH] Adding MinGW to system PATH...
echo.

REM Add to PATH for current session
set PATH=%PATH%;C:\MinGW\bin

REM Add to PATH permanently (requires admin)
setx PATH "%PATH%;C:\MinGW\bin" >nul 2>&1

echo ✓ MinGW installed successfully!
echo.
echo NOTE: You may need to restart your command prompt
echo       for PATH changes to take effect everywhere
echo.

goto :build

:manual
echo.
echo MANUAL INSTALLATION STEPS:
echo.
echo 1. Go to: https://www.mingw-w64.org/downloads/
echo 2. Download: MingW-W64-builds
echo 3. Run installer and install to C:\MinGW
echo 4. Add C:\MinGW\bin to your PATH
echo 5. Restart Command Prompt
echo 6. Run this script again
echo.
pause
exit /b 0

:build
echo ═══════════════════════════════════════════════════════════
echo  SETUP COMPLETE! Now building TBOS detector...
echo ═══════════════════════════════════════════════════════════
echo.

REM Return to app directory
cd /d "%~dp0"

REM Run build script
call build_windows.bat

exit /b 0
