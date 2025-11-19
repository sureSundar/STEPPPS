#!/bin/bash
# TBOS Universal Installer Builder
# Builds self-extracting installers for ALL platforms
# 🕉️ Swamiye Saranam Aiyappa 🕉️

set -e

VERSION="1.0"

echo "╔══════════════════════════════════════════════════════════╗"
echo "║  TBOS Universal Installer Builder                       ║"
echo "║     🕉️ Swamiye Saranam Aiyappa 🕉️                       ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Building installers for all platforms..."
echo "Version: ${VERSION}"
echo ""

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DIST_DIR="${SCRIPT_DIR}/dist"

# Clean and create dist directory
rm -rf "${DIST_DIR}"
mkdir -p "${DIST_DIR}"

echo "═══════════════════════════════════════════════════════════"
echo " Platform 1/4: Linux"
echo "═══════════════════════════════════════════════════════════"
echo ""

if [ -f "${SCRIPT_DIR}/build-linux-installer.sh" ]; then
    bash "${SCRIPT_DIR}/build-linux-installer.sh"
    echo "✅ Linux installer built"
else
    echo "❌ Linux installer script not found"
fi

echo ""
echo "═══════════════════════════════════════════════════════════"
echo " Platform 2/4: Windows"
echo "═══════════════════════════════════════════════════════════"
echo ""

if [ -f "${SCRIPT_DIR}/build-windows-installer.sh" ]; then
    bash "${SCRIPT_DIR}/build-windows-installer.sh"
    echo "✅ Windows installer built"
else
    echo "❌ Windows installer script not found"
fi

echo ""
echo "═══════════════════════════════════════════════════════════"
echo " Platform 3/4: macOS"
echo "═══════════════════════════════════════════════════════════"
echo ""

if [ -f "${SCRIPT_DIR}/build-macos-installer.sh" ]; then
    bash "${SCRIPT_DIR}/build-macos-installer.sh"
    echo "✅ macOS installer built"
else
    echo "❌ macOS installer script not found"
fi

echo ""
echo "═══════════════════════════════════════════════════════════"
echo " Platform 4/4: Android"
echo "═══════════════════════════════════════════════════════════"
echo ""

if [ -f "${SCRIPT_DIR}/build-android-apk.sh" ]; then
    bash "${SCRIPT_DIR}/build-android-apk.sh"
    echo "✅ Android APK built"
else
    echo "❌ Android APK script not found"
fi

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║  ✅ ALL INSTALLERS BUILT                                 ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Distribution packages created in: dist/"
echo ""
echo "═══════════════════════════════════════════════════════════"
echo " Package Summary:"
echo "═══════════════════════════════════════════════════════════"

cd "${DIST_DIR}"
for file in *; do
    if [ -f "$file" ]; then
        SIZE=$(du -h "$file" | cut -f1)
        printf "  %-50s %10s\n" "$file" "$SIZE"
    fi
done

echo ""
echo "═══════════════════════════════════════════════════════════"
echo " Installation Instructions:"
echo "═══════════════════════════════════════════════════════════"
echo ""
echo "🐧 Linux:"
echo "   sudo ./TBOS-Detector-Linux-*.run"
echo ""
echo "🪟 Windows:"
echo "   Double-click: TBOS-Detector-Windows-*.exe"
echo ""
echo "🍎 macOS:"
echo "   Double-click: TBOS-Detector-macOS-*.pkg"
echo "   Or: sudo ./TBOS-Detector-macOS-*.sh"
echo ""
echo "🤖 Android:"
echo "   adb install TBOS-Detector-Android-*.apk"
echo "   Or copy to device and tap to install"
echo ""
echo "═══════════════════════════════════════════════════════════"
echo ""
echo "🕉️ Swamiye Saranam Aiyappa 🕉️"
echo ""
echo "TBOS: Universal Hardware Detection - Package Once, Run Everywhere!"
