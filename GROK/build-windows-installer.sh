#!/bin/bash
# TBOS Windows Self-Extracting Installer Builder
# 🕉️ Swamiye Saranam Aiyappa 🕉️

set -e

VERSION="1.0"
ARCH="x64"
INSTALLER_NAME="TBOS-Detector-Windows-${ARCH}-v${VERSION}-Setup.exe"

echo "╔══════════════════════════════════════════════════════════╗"
echo "║  Building TBOS Windows Self-Extracting Installer        ║"
echo "║     🕉️ Swamiye Saranam Aiyappa 🕉️                       ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build/windows-installer"
PAYLOAD_DIR="${BUILD_DIR}/payload"

# Clean and create build directory
rm -rf "${BUILD_DIR}"
mkdir -p "${PAYLOAD_DIR}"
mkdir -p "${SCRIPT_DIR}/dist"

echo "[1/6] Checking dependencies..."
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "Installing MinGW cross-compiler..."
    sudo apt install -y mingw-w64
fi

if ! command -v makensis &> /dev/null; then
    echo "Installing NSIS..."
    sudo apt install -y nsis
fi
echo "✅ Dependencies ready"

echo "[2/6] Cross-compiling for Windows..."
cd "${SCRIPT_DIR}/app"

x86_64-w64-mingw32-gcc \
    -I./include \
    -o "${PAYLOAD_DIR}/tbos_detector.exe" \
    src/main.c \
    src/hardware_windows.c \
    src/device_classifier.c \
    src/print_utils.c \
    -ladvapi32 -static -s

echo "✅ Windows binary created"

echo "[3/6] Preparing payload..."
mkdir -p "${PAYLOAD_DIR}/docs"

# Copy documentation
cp ../README.md "${PAYLOAD_DIR}/docs/" 2>/dev/null || true
cp README_WINDOWS_SIMPLE.txt "${PAYLOAD_DIR}/docs/" 2>/dev/null || true

# Create launcher batch file
cat > "${PAYLOAD_DIR}/TBOS-Detector.bat" << 'BAT_EOF'
@echo off
echo ╔══════════════════════════════════════════════════════════╗
echo ║  TBOS Universal Hardware Detector                       ║
echo ║     🕉 Swamiye Saranam Aiyappa 🕉                       ║
echo ╚══════════════════════════════════════════════════════════╝
echo.

"%~dp0tbos_detector.exe" --detect

echo.
pause
BAT_EOF

# Create README
cat > "${PAYLOAD_DIR}/README.txt" << 'README_EOF'
╔══════════════════════════════════════════════════════════╗
║  TBOS Universal Hardware Detector - Windows            ║
║     🕉 Swamiye Saranam Aiyappa 🕉                       ║
╚══════════════════════════════════════════════════════════╝

HOW TO USE:
1. Double-click: TBOS-Detector.bat
2. Or run from Command Prompt: tbos_detector.exe --detect

FILES:
- tbos_detector.exe   - Main application
- TBOS-Detector.bat   - Easy launcher
- docs\               - Documentation

Version 1.0
README_EOF

echo "✅ Payload prepared"

echo "[4/6] Creating NSIS installer script..."
cat > "${BUILD_DIR}/installer.nsi" << 'NSI_EOF'
!define APP_NAME "TBOS Detector"
!define APP_VERSION "1.0"
!define APP_PUBLISHER "TBOS Project"
!define APP_URL "https://github.com/tbos-project/tbos"
!define APP_EXE "tbos_detector.exe"

Name "${APP_NAME} v${APP_VERSION}"
OutFile "INSTALLER_OUTPUT_PATH"
InstallDir "$PROGRAMFILES64\TBOS Detector"
RequestExecutionLevel admin

; Modern UI
!include "MUI2.nsh"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; Installer Section
Section "Install"
    SetOutPath "$INSTDIR"

    File "payload\tbos_detector.exe"
    File "payload\TBOS-Detector.bat"
    File "payload\README.txt"

    CreateDirectory "$INSTDIR\docs"
    SetOutPath "$INSTDIR\docs"
    File /nonfatal /r "payload\docs\*.*"

    SetOutPath "$INSTDIR"

    ; Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ; Create shortcuts
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortcut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "--detect"
    CreateShortcut "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortcut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "--detect"

    ; Add to PATH
    EnVar::SetHKCU
    EnVar::AddValue "PATH" "$INSTDIR"

    ; Registry
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayVersion" "${APP_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "Publisher" "${APP_PUBLISHER}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "URLInfoAbout" "${APP_URL}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayIcon" "$INSTDIR\${APP_EXE}"

    ; Success message
    MessageBox MB_OK "🕉 Swamiye Saranam Aiyappa 🕉$\n$\nTBOS Detector installed successfully!$\n$\nRun from: Start Menu → TBOS Detector"
SectionEnd

; Uninstaller Section
Section "Uninstall"
    Delete "$INSTDIR\tbos_detector.exe"
    Delete "$INSTDIR\TBOS-Detector.bat"
    Delete "$INSTDIR\README.txt"
    Delete "$INSTDIR\Uninstall.exe"

    RMDir /r "$INSTDIR\docs"
    RMDir "$INSTDIR"

    Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
    Delete "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk"
    RMDir "$SMPROGRAMS\${APP_NAME}"
    Delete "$DESKTOP\${APP_NAME}.lnk"

    ; Remove from PATH
    EnVar::SetHKCU
    EnVar::DeleteValue "PATH" "$INSTDIR"

    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
SectionEnd
NSI_EOF

# Replace output path
sed -i "s|INSTALLER_OUTPUT_PATH|${SCRIPT_DIR}/dist/${INSTALLER_NAME}|g" "${BUILD_DIR}/installer.nsi"

echo "✅ NSIS script created"

echo "[5/6] Compiling installer..."
cd "${BUILD_DIR}"
makensis installer.nsi

echo "✅ Installer compiled"

echo "[6/6] Verifying installer..."
if [ -f "${SCRIPT_DIR}/dist/${INSTALLER_NAME}" ]; then
    echo "✅ Installer verified"
else
    echo "❌ Installer creation failed!"
    exit 1
fi

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║  ✅ BUILD COMPLETE                                       ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Installer: dist/${INSTALLER_NAME}"
echo "Size: $(du -h "${SCRIPT_DIR}/dist/${INSTALLER_NAME}" | cut -f1)"
echo ""
echo "To install on Windows:"
echo "  1. Copy to Windows machine"
echo "  2. Double-click the .exe"
echo "  3. Follow the wizard"
echo ""
echo "🕉️ Swamiye Saranam Aiyappa 🕉️"
