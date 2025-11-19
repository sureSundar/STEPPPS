# Testing TBOS on Windows
## 🕉️ Swamiye Saranam Aiyappa 🕉️

**You have a shared folder!** This makes testing on Windows super easy.

---

## 📁 Shared Folder Setup

Your files are at:
- **Linux**: `/media/sf_vboxshare/lab/STEPPPS/GROK/`
- **Windows**: `\\vboxsvr\vboxshare\lab\STEPPPS\GROK\` or `Z:\lab\STEPPPS\GROK\`

---

## 🪟 OPTION 1: Test with MinGW (RECOMMENDED)

### Step 1: Install MinGW (if not installed)

Download from: https://www.mingw-w64.org/

Or use MSYS2: https://www.msys2.org/

### Step 2: Open MinGW Terminal

```bash
# Navigate to shared folder
cd /z/lab/STEPPPS/GROK/app
# or
cd /mnt/vboxshare/lab/STEPPPS/GROK/app
```

### Step 3: Build

```bash
gcc -o tbos_detector.exe src/main.c src/device_classifier.c src/print_utils.c src/hardware_windows.c -ladvapi32
```

### Step 4: Test

```bash
./tbos_detector.exe --detect
./tbos_detector.exe --json
```

---

## 🪟 OPTION 2: Test with Visual Studio

### Step 1: Open Developer Command Prompt

Start Menu → Visual Studio → Developer Command Prompt

### Step 2: Navigate to folder

```cmd
cd Z:\lab\STEPPPS\GROK\app
```

### Step 3: Build

```cmd
cl /O2 src\main.c src\device_classifier.c src\print_utils.c src\hardware_windows.c /Fe:tbos_detector.exe advapi32.lib
```

### Step 4: Test

```cmd
tbos_detector.exe --detect
tbos_detector.exe --json
```

---

## 🪟 OPTION 3: Quick Test (Pre-compiled)

If you just want to test quickly, I can create a batch file!

### Step 1: Create `build_windows.bat`

Open Notepad and create this file in `GROK\app\`:

```batch
@echo off
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
    goto :test
)

REM Try MSVC
where cl >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo Using Microsoft Visual C++...
    cl /O2 src\main.c src\device_classifier.c src\print_utils.c src\hardware_windows.c /Fe:tbos_detector.exe advapi32.lib
    goto :test
)

echo [ERROR] No compiler found!
echo Please install MinGW or Visual Studio
pause
exit /b 1

:test
echo.
echo [BUILD] ✅ Build successful!
echo.
echo [TEST] Running hardware detection...
echo.
tbos_detector.exe --detect
echo.
echo [TEST] ✅ Test complete!
echo.
pause
```

### Step 2: Double-click `build_windows.bat`

It will automatically:
1. Detect your compiler (MinGW or MSVC)
2. Build the detector
3. Run the test
4. Show you the results

---

## 🪟 OPTION 4: PowerShell Script

Create `build.ps1`:

```powershell
Write-Host "╔══════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║  TBOS Universal Detector - Windows Build                ║" -ForegroundColor Cyan
Write-Host "╚══════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

Write-Host "[BUILD] Compiling TBOS detector..." -ForegroundColor Yellow

# Try to build
$result = & gcc -o tbos_detector.exe src\main.c src\device_classifier.c src\print_utils.c src\hardware_windows.c -ladvapi32 2>&1

if ($LASTEXITCODE -eq 0) {
    Write-Host "[BUILD] ✅ Build successful!" -ForegroundColor Green
    Write-Host ""
    Write-Host "[TEST] Running hardware detection..." -ForegroundColor Yellow
    Write-Host ""
    & .\tbos_detector.exe --detect
    Write-Host ""
    Write-Host "[TEST] ✅ Test complete!" -ForegroundColor Green
} else {
    Write-Host "[BUILD] ❌ Build failed!" -ForegroundColor Red
    Write-Host $result
}

Write-Host ""
Read-Host "Press Enter to exit"
```

Run in PowerShell:
```powershell
cd Z:\lab\STEPPPS\GROK\app
.\build.ps1
```

---

## 📊 What You Should See

### Expected Output on Windows:

```
╔══════════════════════════════════════════════════════════╗
║     TBOS Universal Hardware Detection Report            ║
║         🕉 Swamiye Saranam Aiyappa 🕉                    ║
╚══════════════════════════════════════════════════════════╝

=== SYSTEM INFORMATION ===
  Operating System: Windows 10.0 Build 19045
  Architecture: x86_64

=== CPU INFORMATION ===
  Vendor: GenuineIntel (or AuthenticAMD)
  Cores: 4 (or whatever your CPU has)
  Speed: 2400 MHz (your actual CPU speed)

=== MEMORY INFORMATION ===
  Total RAM: XXXXX KB
  Total RAM: XXXX MB
  Total RAM: X.XX GB

=== DEVICE CLASSIFICATION ===
  Device Class: DESKTOP (or WORKSTATION, etc.)
  Class ID: 3 (or 4, etc.)

=== RECOMMENDED OS ===
  Recommended: Ubuntu Desktop / Alpine Linux Desktop
  Reason: Good balance of features and performance

╔══════════════════════════════════════════════════════════╗
║  Detection Complete - Universal Compatibility Verified   ║
╚══════════════════════════════════════════════════════════╝
```

---

## 🐛 Troubleshooting

### Problem: "gcc is not recognized"

**Solution**: Install MinGW or add it to PATH
```cmd
set PATH=%PATH%;C:\MinGW\bin
```

### Problem: "cl is not recognized"

**Solution**: Use Developer Command Prompt for Visual Studio, not regular CMD

### Problem: "Cannot find advapi32.lib"

**Solution**: You're using MSVC. The library is `advapi32.lib` not `-ladvapi32`

### Problem: Build succeeds but crashes

**Solution**: Make sure you're using the Windows implementation:
```bash
# Check that this file exists and is being compiled:
src/hardware_windows.c
```

---

## ✅ Success Criteria

After running the test, you should see:
1. ✅ No build errors
2. ✅ Program runs without crashing
3. ✅ Detects your CPU correctly
4. ✅ Detects your RAM correctly
5. ✅ Classifies your device correctly
6. ✅ JSON output is valid (test with `--json` flag)

---

## 📸 Send Results

After testing, please share:
1. Your Windows version
2. Your CPU type
3. Your RAM amount
4. The detected classification
5. Any errors you encountered

---

## 🎯 Quick Test Checklist

```
[ ] MinGW or Visual Studio installed
[ ] Navigated to shared folder
[ ] Built successfully
[ ] Ran --detect test
[ ] Saw correct CPU info
[ ] Saw correct RAM info
[ ] Saw device classification
[ ] Ran --json test
[ ] JSON output valid
```

---

🕉 **Swamiye Saranam Aiyappa** 🕉

**Test it on Windows and let me know the results!**
