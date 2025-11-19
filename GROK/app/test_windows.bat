@echo off
REM TBOS Universal Detector - Windows Test Script
REM 🕉 Swamiye Saranam Aiyappa 🕉

echo ╔══════════════════════════════════════════════════════════╗
echo ║  TBOS Universal Detector - Windows Test                 ║
echo ║     🕉 Swamiye Saranam Aiyappa 🕉                       ║
echo ╚══════════════════════════════════════════════════════════╝
echo.

if not exist tbos_detector.exe (
    echo [ERROR] tbos_detector.exe not found!
    echo Please run build_windows.bat first
    echo.
    pause
    exit /b 1
)

echo [TEST 1] Full hardware detection report...
echo.
echo ═══════════════════════════════════════════════════════════
tbos_detector.exe --detect
echo ═══════════════════════════════════════════════════════════
echo.

echo [TEST 2] JSON output...
echo.
tbos_detector.exe --json > hardware_report.json
type hardware_report.json
echo.

echo [SUCCESS] ✅ Both tests complete!
echo.
echo Results saved to: hardware_report.json
echo.
pause
