@echo off
REM TBOS Universal Detector - Windows Build Script
REM 🕉 Swamiye Saranam Aiyappa 🕉

echo ╔══════════════════════════════════════════════════════════╗
echo ║  TBOS Universal Detector - Windows Build                ║
echo ║     🕉 Swamiye Saranam Aiyappa 🕉                       ║
echo ╚══════════════════════════════════════════════════════════╝
echo.

echo [BUILD] Compiling TBOS detector for Windows...
echo.

REM Try MinGW first
where gcc >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo Using MinGW GCC...
    gcc -o tbos_detector.exe src\main.c src\device_classifier.c src\print_utils.c src\hardware_windows.c -ladvapi32
    if %ERRORLEVEL% == 0 goto :test
    echo Build failed with MinGW
    goto :error
)

REM Try MSVC
where cl >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo Using Microsoft Visual C++...
    cl /nologo /O2 src\main.c src\device_classifier.c src\print_utils.c src\hardware_windows.c /Fe:tbos_detector.exe advapi32.lib
    if %ERRORLEVEL% == 0 goto :test
    echo Build failed with MSVC
    goto :error
)

echo [ERROR] No compiler found!
echo.
echo Please install one of the following:
echo   - MinGW: https://www.mingw-w64.org/
echo   - Visual Studio: https://visualstudio.microsoft.com/
echo.
pause
exit /b 1

:test
echo.
echo [BUILD] ✅ Build successful!
echo.
echo [TEST] Running hardware detection...
echo.
echo ═══════════════════════════════════════════════════════════
echo.
tbos_detector.exe --detect
echo.
echo ═══════════════════════════════════════════════════════════
echo.
echo [TEST] ✅ Test complete!
echo.
echo To see JSON output, run:
echo   tbos_detector.exe --json
echo.
pause
exit /b 0

:error
echo.
echo [BUILD] ❌ Build failed!
echo Check the error messages above
echo.
pause
exit /b 1
