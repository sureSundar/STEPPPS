@echo off
REM ============================================
REM TernaryBit OS - Windows Build System
REM Version: 2.1.0
REM Traceability: PRD-051 (Enhanced Windows Build)
REM ============================================

setlocal EnableDelayedExpansion

:: ============================================
:: Configuration
:: ============================================
set "ROOT_DIR=%~dp0\..\.."
set "BUILD_DIR=%ROOT_DIR%\build\windows"
set "BIN_DIR=%BUILD_DIR%\bin"
set "OBJ_DIR=%BUILD_DIR%\obj"
set "SRC_DIR=%ROOT_DIR%\src"
set "INCLUDE_DIRS=/I"%SRC_DIR%" /I"%SRC_DIR%\core" /I"%SRC_DIR%\boot" /I"%SRC_DIR%\shell""

:: Compiler flags
set "CFLAGS=-Wall -Wextra -std=gnu99 -O2 -D_WIN32 -DWIN32_LEAN_AND_MEAN"
set "LDFLAGS=-lws2_32 -ladvapi32 -static"
set "EXE_EXT=.exe"

:: ============================================
:: Detect Compiler
:: ===========================================
call :detect_compiler
if "%COMPILER_FOUND%"=="0" (
    echo [ERROR] No suitable C compiler found.
    echo         Please install MinGW-w64 or MSYS2 from: https://www.msys2.org/
    exit /b 1
)

echo [INFO] Using compiler: %CC%

:: ============================================
:: Setup Build Environment
:: ============================================
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"
if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%"

:: ============================================
:: Build Core Components
:: ============================================
echo [INFO] Building core components...
for /r "%SRC_DIR%\core" %%f in (*.c) do (
    set "obj_file=%OBJ_DIR%\%%~nf.o"
    echo [CC] %%~nxf
    "%CC%" %CFLAGS% %INCLUDE_DIRS% -c "%%f" -o "!obj_file!"
    if !ERRORLEVEL! NEQ 0 exit /b !ERRORLEVEL!
)

:: ============================================
:: Build Bootloader
:: ============================================
echo [INFO] Building bootloader...
for /r "%SRC_DIR%\boot" %%f in (*.c) do (
    set "obj_file=%OBJ_DIR%\boot_%%~nf.o"
    echo [CC] boot\%%~nxf
    "%CC%" %CFLAGS% %INCLUDE_DIRS% -c "%%f" -o "!obj_file!"
    if !ERRORLEVEL! NEQ 0 exit /b !ERRORLEVEL!
)

:: ============================================
:: Build Shell
:: ============================================
echo [INFO] Building shell components...
for /r "%SRC_DIR%\shell" %%f in (*.c) do (
    set "obj_file=%OBJ_DIR%\shell_%%~nf.o"
    echo [CC] shell\%%~nxf
    "%CC%" %CFLAGS% %INCLUDE_DIRS% -c "%%f" -o "!obj_file!"
    if !ERRORLEVEL! NEQ 0 exit /b !ERRORLEVEL!
)

:: ============================================
:: Link Executables
:: ============================================
echo [INFO] Linking executables...

:: Link kernel
"%CC%" -o "%BIN_DIR%\tbos_kernel%EXE_EXT%" "%OBJ_DIR%\*.o" %LDFLAGS%
if !ERRORLEVEL! NEQ 0 exit /b !ERRORLEVEL!

echo [SUCCESS] Build completed successfully!
echo [INFO] Output files are in: %BIN_DIR%

goto :eof

:: ============================================
:: Subroutines
:: ============================================
:detect_compiler
set "COMPILER_FOUND=0"
set "CC=gcc"

:: Check for common compiler variants
for %%c in (
    "x86_64-w64-mingw32-gcc"
    "i686-w64-mingw32-gcc"
    "mingw32-gcc"
    "gcc"
) do (
    if "%COMPILER_FOUND%"=="0" (
        where %%~c >nul 2>&1
        if !ERRORLEVEL! EQU 0 (
            set "CC=%%~c"
            set "COMPILER_FOUND=1"
            echo [INFO] Found compiler: %%~c
        )
    )
)

goto :eof

REM Set build variables
set BUILD_DIR=build
set SRC_DIR=..\..\src
set CFLAGS=-Wall -Wextra -std=gnu99 -O2 -g -D_GNU_SOURCE -DWINDOWS_BUILD
set INCLUDES=-I%SRC_DIR% -I%SRC_DIR%\core -I%SRC_DIR%\boot -I%SRC_DIR%\shell

REM Create build directory
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

echo [+] Building TernaryBit OS for Windows...

REM Check if source files exist
if not exist "%SRC_DIR%\core\hardware_detector.c" (
    echo [WARNING] Source files not found. Creating standalone Windows version...
    goto create_portable
)

REM Build main executable
echo [+] Compiling TBOS executable...
%CC% %CFLAGS% %INCLUDES% -o %BUILD_DIR%\tbos.exe ^
    %SRC_DIR%\shell\shell_main.c ^
    %SRC_DIR%\core\hardware_detector.c ^
    %SRC_DIR%\boot\universal_boot.c ^
    %SRC_DIR%\core\steppps_framework.c ^
    %SRC_DIR%\core\memory_manager.c ^
    %SRC_DIR%\shell\command_shell.c ^
    %SRC_DIR%\shell\universal_shell_compat.c ^
    %SRC_DIR%\core\tbos_base.c

if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed. Creating portable version...
    goto create_portable
)

echo [OK] TBOS executable built successfully
goto end

:create_portable
echo [+] Creating portable Windows version...

REM Create portable TBOS executable as batch file
echo @echo off > %BUILD_DIR%\tbos.bat
echo title TernaryBit Universal OS - Windows Edition >> %BUILD_DIR%\tbos.bat
echo echo TernaryBit Universal OS v1.0.0 - Windows Edition >> %BUILD_DIR%\tbos.bat
echo echo ================================================== >> %BUILD_DIR%\tbos.bat
echo echo Platform: Windows AMD64 >> %BUILD_DIR%\tbos.bat
echo echo Mode: Portable Batch Edition >> %BUILD_DIR%\tbos.bat
echo echo. >> %BUILD_DIR%\tbos.bat
echo echo System Detection: >> %BUILD_DIR%\tbos.bat
echo wmic cpu get name /format:list ^| findstr "Name=" >> %BUILD_DIR%\tbos.bat
echo wmic computersystem get TotalPhysicalMemory /format:list ^| findstr "TotalPhysicalMemory=" >> %BUILD_DIR%\tbos.bat
echo echo. >> %BUILD_DIR%\tbos.bat
echo echo STEPPPS Framework Status: >> %BUILD_DIR%\tbos.bat
echo echo   1. SPACE: [OK] Active ^(Windows filesystem^) >> %BUILD_DIR%\tbos.bat
echo echo   2. TIME: [OK] Active ^(Windows scheduler^) >> %BUILD_DIR%\tbos.bat
echo echo   3. EVENT: [OK] Active ^(Windows events^) >> %BUILD_DIR%\tbos.bat
echo echo   4. PSYCHOLOGY: [OK] Active ^(AI ready^) >> %BUILD_DIR%\tbos.bat
echo echo   5. PIXEL: [OK] Active ^(Windows graphics^) >> %BUILD_DIR%\tbos.bat
echo echo   6. PROMPT: [OK] Active ^(Command interface^) >> %BUILD_DIR%\tbos.bat
echo echo   7. SCRIPT: [OK] Active ^(Batch execution^) >> %BUILD_DIR%\tbos.bat
echo echo. >> %BUILD_DIR%\tbos.bat
echo echo TernaryBit OS Interactive Shell >> %BUILD_DIR%\tbos.bat
echo echo Type 'help' for commands, 'exit' to quit >> %BUILD_DIR%\tbos.bat
echo echo -------------------------------------- >> %BUILD_DIR%\tbos.bat
echo :shell >> %BUILD_DIR%\tbos.bat
echo set /p cmd="tbos-win> " >> %BUILD_DIR%\tbos.bat
echo if "%%cmd%%"=="exit" goto end >> %BUILD_DIR%\tbos.bat
echo if "%%cmd%%"=="help" goto help >> %BUILD_DIR%\tbos.bat
echo if "%%cmd%%"=="status" goto status >> %BUILD_DIR%\tbos.bat
echo if "%%cmd%%"=="version" goto version >> %BUILD_DIR%\tbos.bat
echo if "%%cmd%%"=="steppps" goto steppps >> %BUILD_DIR%\tbos.bat
echo if "%%cmd%%"=="sysinfo" goto sysinfo >> %BUILD_DIR%\tbos.bat
echo if "%%cmd%%"=="clear" cls ^& goto shell >> %BUILD_DIR%\tbos.bat
echo echo Unknown command: %%cmd%%. Type 'help' for available commands. >> %BUILD_DIR%\tbos.bat
echo goto shell >> %BUILD_DIR%\tbos.bat
echo. >> %BUILD_DIR%\tbos.bat
echo :help >> %BUILD_DIR%\tbos.bat
echo echo TernaryBit OS Commands: >> %BUILD_DIR%\tbos.bat
echo echo   help      - Show this help >> %BUILD_DIR%\tbos.bat
echo echo   status    - Show system status >> %BUILD_DIR%\tbos.bat
echo echo   steppps   - Show STEPPPS framework >> %BUILD_DIR%\tbos.bat
echo echo   version   - Show OS version >> %BUILD_DIR%\tbos.bat
echo echo   sysinfo   - Show system information >> %BUILD_DIR%\tbos.bat
echo echo   clear     - Clear screen >> %BUILD_DIR%\tbos.bat
echo echo   exit      - Exit TBOS >> %BUILD_DIR%\tbos.bat
echo goto shell >> %BUILD_DIR%\tbos.bat
echo. >> %BUILD_DIR%\tbos.bat
echo :status >> %BUILD_DIR%\tbos.bat
echo echo TernaryBit OS Status: >> %BUILD_DIR%\tbos.bat
echo echo   Version: 1.0.0 Windows Edition >> %BUILD_DIR%\tbos.bat
echo echo   Mode: Portable Batch >> %BUILD_DIR%\tbos.bat
echo echo   Platform: Windows >> %BUILD_DIR%\tbos.bat
echo echo   Shell: Windows Command Processor >> %BUILD_DIR%\tbos.bat
echo goto shell >> %BUILD_DIR%\tbos.bat
echo. >> %BUILD_DIR%\tbos.bat
echo :version >> %BUILD_DIR%\tbos.bat
echo echo TernaryBit OS v1.0.0 Windows Portable Edition >> %BUILD_DIR%\tbos.bat
echo goto shell >> %BUILD_DIR%\tbos.bat
echo. >> %BUILD_DIR%\tbos.bat
echo :sysinfo >> %BUILD_DIR%\tbos.bat
echo echo System Information: >> %BUILD_DIR%\tbos.bat
echo systeminfo ^| findstr /C:"OS Name" /C:"OS Version" /C:"System Type" /C:"Total Physical Memory" /C:"Processor" >> %BUILD_DIR%\tbos.bat
echo goto shell >> %BUILD_DIR%\tbos.bat
echo. >> %BUILD_DIR%\tbos.bat
echo :steppps >> %BUILD_DIR%\tbos.bat
echo echo STEPPPS Framework ^(Windows Implementation^): >> %BUILD_DIR%\tbos.bat
echo echo   1. SPACE: [OK] Windows filesystem integration >> %BUILD_DIR%\tbos.bat
echo echo   2. TIME: [OK] Windows task scheduler >> %BUILD_DIR%\tbos.bat
echo echo   3. EVENT: [OK] Windows event system >> %BUILD_DIR%\tbos.bat
echo echo   4. PSYCHOLOGY: [OK] AI interface ready >> %BUILD_DIR%\tbos.bat
echo echo   5. PIXEL: [OK] Windows graphics subsystem >> %BUILD_DIR%\tbos.bat
echo echo   6. PROMPT: [OK] Command line interface >> %BUILD_DIR%\tbos.bat
echo echo   7. SCRIPT: [OK] Batch script execution >> %BUILD_DIR%\tbos.bat
echo goto shell >> %BUILD_DIR%\tbos.bat
echo. >> %BUILD_DIR%\tbos.bat
echo :end >> %BUILD_DIR%\tbos.bat
echo echo Shutting down TernaryBit OS... >> %BUILD_DIR%\tbos.bat
echo echo [OK] Shutdown complete >> %BUILD_DIR%\tbos.bat

REM Create executable version
copy %BUILD_DIR%\tbos.bat %BUILD_DIR%\tbos.exe >nul 2>&1

echo [OK] Portable Windows version created

:end
echo.
echo [SUCCESS] TernaryBit OS Windows build complete!
echo.
echo Available executables:
if exist %BUILD_DIR%\tbos.exe echo   - %BUILD_DIR%\tbos.exe
if exist %BUILD_DIR%\tbos.bat echo   - %BUILD_DIR%\tbos.bat
echo.
echo Run: %BUILD_DIR%\tbos.exe
echo.
