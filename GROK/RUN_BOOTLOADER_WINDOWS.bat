@echo off
REM TBOS Bootloader - Windows Interactive Test Script
REM 🕉️ Swamiye Saranam Aiyappa 🕉️

cls

echo ╔══════════════════════════════════════════════════════════╗
echo ║  TBOS Universal Bootloader - Windows Test              ║
echo ║     🕉️ Swamiye Saranam Aiyappa 🕉️                       ║
echo ╚══════════════════════════════════════════════════════════╝
echo.

cd /d "%~dp0\ternarybit-os"

REM Check if QEMU is installed
where qemu-system-i386 >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] QEMU not found!
    echo.
    echo Please install QEMU for Windows:
    echo   https://www.qemu.org/download/#windows
    echo.
    echo Or use Chocolatey:
    echo   choco install qemu
    echo.
    pause
    exit /b 1
)

REM Check if bootloader image exists
if not exist "build\tbos_hardware_test.img" (
    echo [INFO] Bootloader not built yet. Building now...
    echo.
    call build_hardware_test.sh
    echo.
)

echo ═══════════════════════════════════════════════════════════
echo  Choose how to run TBOS bootloader on Windows:
echo ═══════════════════════════════════════════════════════════
echo.
echo   1) Graphical Window (see the boot process visually)
echo   2) Text Mode (serial console, like a server)
echo   3) With more RAM (256 MB - see different classification)
echo   4) Cancel
echo.
set /p choice="Your choice (1-4): "

if "%choice%"=="1" goto graphical
if "%choice%"=="2" goto textmode
if "%choice%"=="3" goto moreram
if "%choice%"=="4" goto cancel
goto invalid

:graphical
    echo.
    echo [RUNNING] Opening QEMU graphical window on Windows...
    echo.
    echo You will see:
    echo   - BIOS boot sequence
    echo   - TBOS banner
    echo   - Hardware detection
    echo   - Classification results
    echo.
    echo Press Ctrl+Alt+2 to switch to QEMU monitor
    echo Type 'quit' in monitor to exit
    echo.
    pause

    qemu-system-i386 -fda build\tbos_hardware_test.img -m 128M
    goto end

:textmode
    echo.
    echo [RUNNING] Text mode (serial console) on Windows...
    echo.
    echo Press Ctrl+C to exit QEMU
    echo.
    pause

    qemu-system-i386 -fda build\tbos_hardware_test.img -nographic -m 128M
    goto end

:moreram
    echo.
    echo [RUNNING] With 256 MB RAM (different classification)...
    echo.
    echo This should classify as SERVER or WORKSTATION instead of CLUSTER
    echo.
    pause

    qemu-system-i386 -fda build\tbos_hardware_test.img -m 256M
    goto end

:cancel
    echo.
    echo Cancelled.
    exit /b 0

:invalid
    echo.
    echo Invalid choice. Exiting.
    exit /b 1

:end
echo.
echo ═══════════════════════════════════════════════════════════
echo  QEMU exited. Test complete on Windows!
echo ═══════════════════════════════════════════════════════════
pause
