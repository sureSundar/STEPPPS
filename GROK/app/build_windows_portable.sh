#!/bin/bash
# TBOS Universal Detector - Windows Portable Build Script
# 🕉️ Swamiye Saranam Aiyappa 🕉️
#
# This script cross-compiles TBOS detector for Windows from Linux
# Creates portable .exe files that work without installation

set -e

echo "╔══════════════════════════════════════════════════════════╗"
echo "║  TBOS Universal Detector - Windows Portable Build       ║"
echo "║         🕉️ Swamiye Saranam Aiyappa 🕉️                   ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

# Configuration
SRC_DIR="src"
BUILD_DIR="build/windows"
DIST_DIR="dist/windows"
VERSION="1.0.0"

# Source files
SOURCES="src/main.c src/device_classifier.c src/print_utils.c src/hardware_windows.c"

# Create directories
mkdir -p "$BUILD_DIR"
mkdir -p "$DIST_DIR"

echo "[1/5] Cleaning previous builds..."
rm -f "$BUILD_DIR"/*.exe "$BUILD_DIR"/*.o
rm -f "$DIST_DIR"/*.exe "$DIST_DIR"/*.zip

echo "[2/5] Building 64-bit portable executable..."
x86_64-w64-mingw32-gcc \
    -O2 \
    -static \
    -static-libgcc \
    $SOURCES \
    -o "$BUILD_DIR/tbos_detector_x64.exe" \
    -ladvapi32 \
    -DVERSION=\"$VERSION\"

if [ $? -eq 0 ]; then
    echo "    ✅ 64-bit build successful: $(du -h "$BUILD_DIR/tbos_detector_x64.exe" | cut -f1)"
else
    echo "    ❌ 64-bit build failed!"
    exit 1
fi

echo "[3/5] Building 32-bit portable executable..."
i686-w64-mingw32-gcc \
    -O2 \
    -static \
    -static-libgcc \
    $SOURCES \
    -o "$BUILD_DIR/tbos_detector_x86.exe" \
    -ladvapi32 \
    -DVERSION=\"$VERSION\"

if [ $? -eq 0 ]; then
    echo "    ✅ 32-bit build successful: $(du -h "$BUILD_DIR/tbos_detector_x86.exe" | cut -f1)"
else
    echo "    ❌ 32-bit build failed!"
    exit 1
fi

echo "[4/5] Stripping debug symbols for smaller size..."
x86_64-w64-mingw32-strip "$BUILD_DIR/tbos_detector_x64.exe"
i686-w64-mingw32-strip "$BUILD_DIR/tbos_detector_x86.exe"
echo "    ✅ Stripped executables:"
echo "       64-bit: $(du -h "$BUILD_DIR/tbos_detector_x64.exe" | cut -f1)"
echo "       32-bit: $(du -h "$BUILD_DIR/tbos_detector_x86.exe" | cut -f1)"

echo "[5/5] Creating distribution packages..."

# Copy executables to dist
cp "$BUILD_DIR/tbos_detector_x64.exe" "$DIST_DIR/"
cp "$BUILD_DIR/tbos_detector_x86.exe" "$DIST_DIR/"

# Create README for Windows users
cat > "$DIST_DIR/README_WINDOWS.txt" << 'EOF'
TBOS Universal Hardware Detector - Windows Edition
===================================================
🕉️ Swamiye Saranam Aiyappa 🕉️

PORTABLE EXECUTABLES - NO INSTALLATION REQUIRED

Quick Start:
------------
1. Double-click tbos_detector_x64.exe (for 64-bit Windows)
   OR tbos_detector_x86.exe (for 32-bit Windows)

2. Use Command Prompt for better output:
   - Press Windows+R
   - Type: cmd
   - Navigate to this folder
   - Run: tbos_detector_x64.exe --detect

Usage:
------
tbos_detector_x64.exe --detect    Show hardware detection report
tbos_detector_x64.exe --json      Output in JSON format
tbos_detector_x64.exe --help      Show help

System Requirements:
-------------------
- Windows 7 or later (64-bit: use x64.exe, 32-bit: use x86.exe)
- No additional software required
- Runs from any location (USB drive, Downloads folder, etc.)

What This Tool Does:
--------------------
- Detects your CPU (vendor, cores, speed)
- Detects your RAM (total amount)
- Classifies your device type (Desktop, Workstation, Server, etc.)
- Recommends optimal Linux distribution for your hardware
- Outputs standardized JSON for server integration

Privacy:
--------
This tool only reads hardware information locally.
No data is sent anywhere unless you explicitly use it with a server.

Version: 1.0.0
Date: 2025-11-03
EOF

# Create batch file for easy testing
cat > "$DIST_DIR/RUN_DETECTOR.bat" << 'EOF'
@echo off
echo ╔══════════════════════════════════════════════════════════╗
echo ║  TBOS Universal Hardware Detector                       ║
echo ║         🕉 Swamiye Saranam Aiyappa 🕉                   ║
echo ╚══════════════════════════════════════════════════════════╝
echo.

REM Detect architecture
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set EXE=tbos_detector_x64.exe
    echo [INFO] Detected 64-bit Windows
) else (
    set EXE=tbos_detector_x86.exe
    echo [INFO] Detected 32-bit Windows
)

echo [INFO] Running hardware detection...
echo.

%EXE% --detect

echo.
echo ╔══════════════════════════════════════════════════════════╗
echo ║  Detection Complete                                      ║
echo ╚══════════════════════════════════════════════════════════╝
echo.
pause
EOF

# Create batch file for JSON output
cat > "$DIST_DIR/GET_JSON.bat" << 'EOF'
@echo off
REM Detect architecture and run JSON output
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    tbos_detector_x64.exe --json > hardware_profile.json
) else (
    tbos_detector_x86.exe --json > hardware_profile.json
)
echo Hardware profile saved to: hardware_profile.json
pause
EOF

# Create ZIP packages
cd "$DIST_DIR"
echo "    Creating ZIP packages..."

# 64-bit package
zip -q "tbos_detector_windows_x64_v${VERSION}.zip" \
    tbos_detector_x64.exe \
    README_WINDOWS.txt \
    RUN_DETECTOR.bat \
    GET_JSON.bat

# 32-bit package
zip -q "tbos_detector_windows_x86_v${VERSION}.zip" \
    tbos_detector_x86.exe \
    README_WINDOWS.txt \
    RUN_DETECTOR.bat \
    GET_JSON.bat

# Universal package with both
zip -q "tbos_detector_windows_universal_v${VERSION}.zip" \
    tbos_detector_x64.exe \
    tbos_detector_x86.exe \
    README_WINDOWS.txt \
    RUN_DETECTOR.bat \
    GET_JSON.bat

cd - > /dev/null

echo "    ✅ Created distribution packages:"
echo "       - tbos_detector_windows_x64_v${VERSION}.zip (64-bit only)"
echo "       - tbos_detector_windows_x86_v${VERSION}.zip (32-bit only)"
echo "       - tbos_detector_windows_universal_v${VERSION}.zip (both)"

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║  Build Complete! ✅                                      ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Distribution files are in: $DIST_DIR/"
echo ""
echo "To test on Windows:"
echo "1. Copy the ZIP file to a Windows machine"
echo "2. Extract the ZIP"
echo "3. Double-click RUN_DETECTOR.bat"
echo ""
echo "Or share the portable .exe files directly - no installation needed!"
echo ""
echo "🕉️ Swamiye Saranam Aiyappa 🕉️"
