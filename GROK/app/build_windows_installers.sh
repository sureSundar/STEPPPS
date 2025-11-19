#!/bin/bash
# TBOS Universal Detector - Windows Installer Build Script
# 🕉️ Swamiye Saranam Aiyappa 🕉️
#
# This script creates Windows installers (.exe via NSIS)
# and provides MSI build instructions

set -e

echo "╔══════════════════════════════════════════════════════════╗"
echo "║  TBOS Windows Installer Builder                         ║"
echo "║         🕉️ Swamiye Saranam Aiyappa 🕉️                   ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

# First, build the portable executables if not already built
if [ ! -f "build/windows/tbos_detector_x64.exe" ]; then
    echo "[INFO] Portable executables not found. Building them first..."
    ./build_windows_portable.sh
    echo ""
fi

# Check for NSIS
if command -v makensis &> /dev/null; then
    echo "[1/3] Building NSIS Installer (.exe)..."
    echo ""

    # Build NSIS installer
    makensis installer_windows.nsi

    if [ $? -eq 0 ]; then
        echo ""
        echo "    ✅ NSIS installer created successfully!"
        NSIS_SIZE=$(du -h dist/windows/TBOS_Detector_Setup_v1.0.0.exe | cut -f1)
        echo "       Size: $NSIS_SIZE"
        echo "       Location: dist/windows/TBOS_Detector_Setup_v1.0.0.exe"
    else
        echo "    ❌ NSIS installer build failed!"
    fi
else
    echo "[1/3] NSIS not available (skipping .exe installer)"
    echo "      Install NSIS: sudo apt-get install nsis"
fi

echo ""
echo "[2/3] Checking for WiX/wixl (MSI builder)..."

# Check for wixl (Linux MSI builder)
if command -v wixl &> /dev/null; then
    echo "    Found wixl! Building MSI installer..."

    wixl -o dist/windows/TBOS_Detector_Setup_v1.0.0.msi tbos_detector.wxs

    if [ $? -eq 0 ]; then
        echo "    ✅ MSI installer created successfully!"
        MSI_SIZE=$(du -h dist/windows/TBOS_Detector_Setup_v1.0.0.msi | cut -f1)
        echo "       Size: $MSI_SIZE"
        echo "       Location: dist/windows/TBOS_Detector_Setup_v1.0.0.msi"
    else
        echo "    ⚠️  MSI build had errors (may still work)"
    fi
else
    echo "    ℹ️  wixl not available on Linux"
    echo "    Install: sudo apt-get install wixl"
    echo ""
    echo "    To build MSI on Windows:"
    echo "    1. Install WiX Toolset: https://wixtoolset.org/"
    echo "    2. Run: candle tbos_detector.wxs"
    echo "    3. Run: light -ext WixUIExtension tbos_detector.wixobj"
fi

echo ""
echo "[3/3] Creating distribution summary..."

# Create a distribution info file
cat > dist/windows/DISTRIBUTION_INFO.txt << 'EOF'
TBOS Universal Detector - Windows Distribution Files
=====================================================
🕉️ Swamiye Saranam Aiyappa 🕉️

Available Distributions:

1. PORTABLE EXECUTABLES (No Installation Required)
   ------------------------------------------------
   - tbos_detector_x64.exe (64-bit Windows)
   - tbos_detector_x86.exe (32-bit Windows)
   - Just copy and run - works from USB, Downloads, anywhere!

   ZIP Packages:
   - tbos_detector_windows_x64_v1.0.0.zip (64-bit only)
   - tbos_detector_windows_x86_v1.0.0.zip (32-bit only)
   - tbos_detector_windows_universal_v1.0.0.zip (both versions)

2. NSIS INSTALLER (Recommended)
   -----------------------------
   File: TBOS_Detector_Setup_v1.0.0.exe

   Features:
   - Professional Windows installer
   - Adds to Start Menu
   - Creates Desktop shortcut
   - Adds to System PATH
   - Easy uninstallation
   - Works on Windows 7, 8, 10, 11

   Installation:
   - Double-click the .exe file
   - Follow the installation wizard
   - Choose installation directory
   - Installer handles everything

3. MSI INSTALLER (Enterprise)
   ---------------------------
   File: TBOS_Detector_Setup_v1.0.0.msi (if available)

   Features:
   - Microsoft Installer format
   - Group Policy deployment
   - Silent installation support
   - Enterprise management compatible

   Installation:
   - Double-click the .msi file
   - Or: msiexec /i TBOS_Detector_Setup_v1.0.0.msi

   Silent Install:
   - msiexec /i TBOS_Detector_Setup_v1.0.0.msi /quiet

USAGE AFTER INSTALLATION:
--------------------------

Method 1: Start Menu
- Click Start → TBOS Detector → TBOS Detector

Method 2: Desktop Shortcut
- Double-click "TBOS Detector" on desktop

Method 3: Command Line (if added to PATH)
- Open Command Prompt
- Type: tbos_detector_x64 --detect

Method 4: Portable (no installation)
- Just run the .exe file directly

COMPARISON:
-----------

Portable .exe:
  ✅ No installation needed
  ✅ Works from USB drive
  ✅ Fastest to use
  ✅ Smallest download
  ❌ No Start Menu integration
  ❌ Not in system PATH

NSIS Installer:
  ✅ Professional installation
  ✅ Start Menu shortcuts
  ✅ Desktop shortcut
  ✅ Added to PATH
  ✅ Easy uninstall
  ✅ Standard Windows experience
  ⚠️  Requires admin privileges

MSI Installer:
  ✅ Enterprise deployment
  ✅ Group Policy support
  ✅ Silent installation
  ✅ Corporate IT friendly
  ⚠️  Requires admin privileges
  ⚠️  May need .NET Framework

RECOMMENDATION:
---------------
- Home users: Use portable .exe OR NSIS installer
- IT departments: Use MSI installer
- USB/portable use: Use portable .exe
- First-time users: Use NSIS installer

🕉️ Swamiye Saranam Aiyappa 🕉️
EOF

echo "    ✅ Distribution info created"

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║  Build Summary                                           ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Distribution Directory: dist/windows/"
echo ""
echo "Available Files:"
ls -lh dist/windows/ | grep -E "\.(exe|msi|zip)$" | awk '{print "  " $9 " (" $5 ")"}'
echo ""
echo "📦 READY FOR DISTRIBUTION!"
echo ""
echo "To distribute:"
echo "1. Upload to GitHub Releases"
echo "2. Share via download link"
echo "3. Deploy to software repositories"
echo "4. Distribute on USB drives"
echo ""
echo "🕉️ Swamiye Saranam Aiyappa 🕉️"
