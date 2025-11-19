@echo off
REM TBOS Windows Distribution Test Script
REM Run this from Windows to test all distributions
REM 🕉 Swamiye Saranam Aiyappa 🕉

echo ╔══════════════════════════════════════════════════════════╗
echo ║  TBOS Windows Distribution Test                         ║
echo ║         🕉 Swamiye Saranam Aiyappa 🕉                   ║
echo ╚══════════════════════════════════════════════════════════╝
echo.

REM Detect if running from shared folder
if exist "dist\windows\" (
    set DIST_DIR=dist\windows
) else if exist "\\vboxsvr\vboxshare\lab\STEPPPS\GROK\app\dist\windows\" (
    set DIST_DIR=\\vboxsvr\vboxshare\lab\STEPPPS\GROK\app\dist\windows
) else (
    echo [ERROR] Cannot find distribution directory!
    echo Please run this script from the app folder or shared folder.
    pause
    exit /b 1
)

echo [INFO] Distribution directory: %DIST_DIR%
echo.

REM Detect architecture
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set EXE=tbos_detector_x64.exe
    set ARCH=64-bit
) else (
    set EXE=tbos_detector_x86.exe
    set ARCH=32-bit
)

echo ╔══════════════════════════════════════════════════════════╗
echo ║  Test 1: Portable Executable                            ║
echo ╚══════════════════════════════════════════════════════════╝
echo.
echo [INFO] Testing %ARCH% portable executable: %EXE%
echo.

if not exist "%DIST_DIR%\%EXE%" (
    echo [ERROR] Executable not found: %DIST_DIR%\%EXE%
    pause
    exit /b 1
)

echo [TEST] Running: %EXE% --detect
echo ────────────────────────────────────────────────────────────
"%DIST_DIR%\%EXE%" --detect
set TEST1_RESULT=%ERRORLEVEL%
echo ────────────────────────────────────────────────────────────
echo.

if %TEST1_RESULT%==0 (
    echo [RESULT] ✅ Test 1 PASSED - Portable executable works!
) else (
    echo [RESULT] ❌ Test 1 FAILED - Error code: %TEST1_RESULT%
)
echo.
echo.

echo ╔══════════════════════════════════════════════════════════╗
echo ║  Test 2: JSON Output                                    ║
echo ╚══════════════════════════════════════════════════════════╝
echo.
echo [TEST] Running: %EXE% --json
echo ────────────────────────────────────────────────────────────
"%DIST_DIR%\%EXE%" --json
set TEST2_RESULT=%ERRORLEVEL%
echo ────────────────────────────────────────────────────────────
echo.

if %TEST2_RESULT%==0 (
    echo [RESULT] ✅ Test 2 PASSED - JSON output works!
) else (
    echo [RESULT] ❌ Test 2 FAILED - Error code: %TEST2_RESULT%
)
echo.
echo.

echo ╔══════════════════════════════════════════════════════════╗
echo ║  Test 3: Batch Script Helper                            ║
echo ╚══════════════════════════════════════════════════════════╝
echo.
echo [INFO] Testing RUN_DETECTOR.bat helper script
echo.

if exist "%DIST_DIR%\RUN_DETECTOR.bat" (
    echo [RESULT] ✅ RUN_DETECTOR.bat exists
) else (
    echo [RESULT] ❌ RUN_DETECTOR.bat not found
)

if exist "%DIST_DIR%\GET_JSON.bat" (
    echo [RESULT] ✅ GET_JSON.bat exists
) else (
    echo [RESULT] ❌ GET_JSON.bat not found
)
echo.
echo.

echo ╔══════════════════════════════════════════════════════════╗
echo ║  Test 4: Distribution Files                             ║
echo ╚══════════════════════════════════════════════════════════╝
echo.
echo [INFO] Checking distribution files...
echo.

if exist "%DIST_DIR%\README_WINDOWS.txt" (
    echo ✅ README_WINDOWS.txt
) else (
    echo ❌ README_WINDOWS.txt MISSING
)

if exist "%DIST_DIR%\tbos_detector_x64.exe" (
    echo ✅ tbos_detector_x64.exe
) else (
    echo ❌ tbos_detector_x64.exe MISSING
)

if exist "%DIST_DIR%\tbos_detector_x86.exe" (
    echo ✅ tbos_detector_x86.exe
) else (
    echo ❌ tbos_detector_x86.exe MISSING
)

if exist "%DIST_DIR%\TBOS_Detector_Setup_v1.0.0.exe" (
    echo ✅ TBOS_Detector_Setup_v1.0.0.exe (NSIS Installer)
) else (
    echo ⚠️  TBOS_Detector_Setup_v1.0.0.exe not found (optional)
)

if exist "%DIST_DIR%\tbos_detector_windows_universal_v1.0.0.zip" (
    echo ✅ tbos_detector_windows_universal_v1.0.0.zip
) else (
    echo ❌ Universal ZIP MISSING
)

echo.
echo.

echo ╔══════════════════════════════════════════════════════════╗
echo ║  Test Summary                                            ║
echo ╚══════════════════════════════════════════════════════════╝
echo.

if %TEST1_RESULT%==0 (
    if %TEST2_RESULT%==0 (
        echo ╔══════════════════════════════════════════════════════════╗
        echo ║  ✅ ALL TESTS PASSED!                                    ║
        echo ║  TBOS Windows distribution is working correctly!        ║
        echo ╚══════════════════════════════════════════════════════════╝
        echo.
        echo Your Windows distribution is ready to use!
        echo.
        echo Next steps:
        echo 1. Test the NSIS installer: TBOS_Detector_Setup_v1.0.0.exe
        echo 2. Share the ZIP files for portable use
        echo 3. Deploy to your Windows machines
    ) else (
        echo ⚠️  Some tests failed. Please review the output above.
    )
) else (
    echo ⚠️  Some tests failed. Please review the output above.
)

echo.
echo 🕉 Swamiye Saranam Aiyappa 🕉
echo.
pause
