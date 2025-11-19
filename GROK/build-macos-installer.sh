#!/bin/bash
# TBOS macOS Self-Extracting Installer Builder
# 🕉️ Swamiye Saranam Aiyappa 🕉️

set -e

VERSION="1.0"
ARCH="universal"
INSTALLER_NAME="TBOS-Detector-macOS-${ARCH}-v${VERSION}.pkg"

echo "╔══════════════════════════════════════════════════════════╗"
echo "║  Building TBOS macOS Self-Extracting Installer          ║"
echo "║     🕉️ Swamiye Saranam Aiyappa 🕉️                       ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build/macos-installer"
PAYLOAD_DIR="${BUILD_DIR}/payload"
ROOT_DIR="${BUILD_DIR}/root"

# Clean and create build directory
rm -rf "${BUILD_DIR}"
mkdir -p "${PAYLOAD_DIR}"
mkdir -p "${ROOT_DIR}/usr/local/bin"
mkdir -p "${ROOT_DIR}/Applications"
mkdir -p "${SCRIPT_DIR}/dist"

echo "[1/5] Building application..."
cd "${SCRIPT_DIR}/app"
make clean
make
echo "✅ Application built"

echo "[2/5] Creating application bundle..."
APP_BUNDLE="${ROOT_DIR}/Applications/TBOS Detector.app"
mkdir -p "${APP_BUNDLE}/Contents/MacOS"
mkdir -p "${APP_BUNDLE}/Contents/Resources"

# Copy binary
cp tbos_detector "${APP_BUNDLE}/Contents/MacOS/"
chmod 755 "${APP_BUNDLE}/Contents/MacOS/tbos_detector"

# Create wrapper script
cat > "${APP_BUNDLE}/Contents/MacOS/TBOS Detector" << 'WRAPPER_EOF'
#!/bin/bash
# TBOS Detector Launcher

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

osascript -e 'tell application "Terminal"
    activate
    do script "'"${DIR}/tbos_detector"' --detect; echo; echo Press any key to close...; read -n 1; exit"
end tell'
WRAPPER_EOF
chmod +x "${APP_BUNDLE}/Contents/MacOS/TBOS Detector"

# Create Info.plist
cat > "${APP_BUNDLE}/Contents/Info.plist" << 'PLIST_EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>
    <string>TBOS Detector</string>
    <key>CFBundleDisplayName</key>
    <string>TBOS Detector</string>
    <key>CFBundleIdentifier</key>
    <string>com.tbos.detector</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleExecutable</key>
    <string>TBOS Detector</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.13</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>NSHumanReadableCopyright</key>
    <string>🕉️ Swamiye Saranam Aiyappa 🕉️</string>
</dict>
</plist>
PLIST_EOF

# Create symlink for command line
ln -s "/Applications/TBOS Detector.app/Contents/MacOS/tbos_detector" \
      "${ROOT_DIR}/usr/local/bin/tbos-detector"

echo "✅ Application bundle created"

echo "[3/5] Creating package scripts..."
SCRIPTS_DIR="${BUILD_DIR}/scripts"
mkdir -p "${SCRIPTS_DIR}"

# Postinstall script
cat > "${SCRIPTS_DIR}/postinstall" << 'POST_EOF'
#!/bin/bash
# Post-installation script

echo "🕉️ Swamiye Saranam Aiyappa 🕉️"
echo "TBOS Detector installed successfully!"
echo ""
echo "You can now run:"
echo "  - From Applications folder"
echo "  - From Terminal: tbos-detector --detect"
echo ""

exit 0
POST_EOF
chmod +x "${SCRIPTS_DIR}/postinstall"

echo "✅ Package scripts created"

echo "[4/5] Building package..."

# Check if we're on macOS
if [[ "$OSTYPE" == "darwin"* ]]; then
    # On macOS, use pkgbuild
    pkgbuild --root "${ROOT_DIR}" \
             --scripts "${SCRIPTS_DIR}" \
             --identifier "com.tbos.detector" \
             --version "${VERSION}" \
             --install-location "/" \
             "${SCRIPT_DIR}/dist/${INSTALLER_NAME}"

    echo "✅ Package created with pkgbuild"
else
    # On Linux, create self-extracting installer
    echo "Running on Linux - creating self-extracting script instead..."

    SELF_EXTRACT="${SCRIPT_DIR}/dist/TBOS-Detector-macOS-${ARCH}-v${VERSION}.sh"

    cat > "${SELF_EXTRACT}" << 'EXTRACT_EOF'
#!/bin/bash
# TBOS Detector for macOS - Self-Extracting Installer
# 🕉️ Swamiye Saranam Aiyappa 🕉️

set -e

echo "╔══════════════════════════════════════════════════════════╗"
echo "║  TBOS Detector for macOS                                ║"
echo "║     🕉️ Swamiye Saranam Aiyappa 🕉️                       ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

# Check if running on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "❌ This installer is for macOS only!"
    exit 1
fi

# Check for sudo
if [ "$EUID" -ne 0 ]; then
    echo "This installer needs administrator privileges."
    echo "Please run: sudo ./$(basename $0)"
    exit 1
fi

echo "Installing TBOS Detector..."

# Extract payload
ARCHIVE_LINE=$(awk '/^__PAYLOAD_BEGIN__/ {print NR + 1; exit 0; }' "$0")
TEMP_DIR=$(mktemp -d)
tail -n+${ARCHIVE_LINE} "$0" | tar xzf - -C "$TEMP_DIR"

# Install files
cp -R "$TEMP_DIR/Applications/TBOS Detector.app" "/Applications/"
ln -sf "/Applications/TBOS Detector.app/Contents/MacOS/tbos_detector" \
       "/usr/local/bin/tbos-detector"

# Cleanup
rm -rf "$TEMP_DIR"

echo ""
echo "✅ TBOS Detector installed successfully!"
echo ""
echo "Run from:"
echo "  - Applications folder"
echo "  - Terminal: tbos-detector --detect"
echo ""
echo "🕉️ Swamiye Saranam Aiyappa 🕉️"

exit 0

__PAYLOAD_BEGIN__
EXTRACT_EOF

    # Create payload
    cd "${ROOT_DIR}"
    tar czf "${BUILD_DIR}/payload.tar.gz" Applications usr

    # Combine
    cat "${SELF_EXTRACT}" > "${SELF_EXTRACT}.tmp"
    cat "${BUILD_DIR}/payload.tar.gz" >> "${SELF_EXTRACT}.tmp"
    mv "${SELF_EXTRACT}.tmp" "${SELF_EXTRACT}"
    chmod +x "${SELF_EXTRACT}"

    echo "✅ Self-extracting installer created"
fi

echo "[5/5] Verifying..."
if [ -f "${SCRIPT_DIR}/dist/${INSTALLER_NAME}" ] || [ -f "${SELF_EXTRACT}" ]; then
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

if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Installer: dist/${INSTALLER_NAME}"
    echo "Size: $(du -h "${SCRIPT_DIR}/dist/${INSTALLER_NAME}" | cut -f1)"
    echo ""
    echo "To install:"
    echo "  Double-click the .pkg file"
else
    echo "Installer: dist/TBOS-Detector-macOS-${ARCH}-v${VERSION}.sh"
    echo "Size: $(du -h "${SELF_EXTRACT}" | cut -f1)"
    echo ""
    echo "To install on macOS:"
    echo "  sudo ./dist/TBOS-Detector-macOS-${ARCH}-v${VERSION}.sh"
fi

echo ""
echo "🕉️ Swamiye Saranam Aiyappa 🕉️"
