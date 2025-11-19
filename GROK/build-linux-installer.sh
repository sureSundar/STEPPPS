#!/bin/bash
# TBOS Linux Self-Extracting Installer Builder
# 🕉️ Swamiye Saranam Aiyappa 🕉️

set -e

VERSION="1.0"
ARCH="x86_64"
INSTALLER_NAME="TBOS-Detector-Linux-${ARCH}-v${VERSION}.run"

echo "╔══════════════════════════════════════════════════════════╗"
echo "║  Building TBOS Linux Self-Extracting Installer          ║"
echo "║     🕉️ Swamiye Saranam Aiyappa 🕉️                       ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build/linux-installer"
PAYLOAD_DIR="${BUILD_DIR}/payload"

# Clean and create build directory
rm -rf "${BUILD_DIR}"
mkdir -p "${PAYLOAD_DIR}"

echo "[1/5] Building application..."
cd "${SCRIPT_DIR}/app"
make clean
make
echo "✅ Application built"

echo "[2/5] Preparing payload..."
mkdir -p "${PAYLOAD_DIR}/bin"
mkdir -p "${PAYLOAD_DIR}/share/doc"
mkdir -p "${PAYLOAD_DIR}/share/applications"

# Copy binary
cp tbos_detector "${PAYLOAD_DIR}/bin/"
chmod 755 "${PAYLOAD_DIR}/bin/tbos_detector"

# Copy documentation
cp ../README.md "${PAYLOAD_DIR}/share/doc/" 2>/dev/null || true
cp ../LICENSE "${PAYLOAD_DIR}/share/doc/" 2>/dev/null || true

# Create desktop entry
cat > "${PAYLOAD_DIR}/share/applications/tbos-detector.desktop" << 'DESKTOP_EOF'
[Desktop Entry]
Name=TBOS Detector
Comment=Universal Hardware Detection
Exec=/opt/tbos/bin/tbos_detector --detect
Icon=utilities-system-monitor
Terminal=true
Type=Application
Categories=System;Utility;
DESKTOP_EOF

echo "✅ Payload prepared"

echo "[3/5] Creating installation script..."
cat > "${BUILD_DIR}/install.sh" << 'INSTALL_EOF'
#!/bin/bash
# TBOS Installation Script

set -e

echo "🕉️ Swamiye Saranam Aiyappa 🕉️"
echo "Installing TBOS Detector..."
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "This installer needs root privileges."
    echo "Please run: sudo ./INSTALLER_NAME"
    exit 1
fi

INSTALL_PREFIX="/opt/tbos"
BIN_LINK="/usr/local/bin/tbos-detector"

echo "Installation directory: $INSTALL_PREFIX"
echo ""

# Create installation directory
mkdir -p "$INSTALL_PREFIX"

# Extract payload (this script assumes payload is appended)
ARCHIVE_LINE=$(awk '/^__PAYLOAD_BEGIN__/ {print NR + 1; exit 0; }' "$0")
tail -n+${ARCHIVE_LINE} "$0" | tar xzf - -C "$INSTALL_PREFIX"

# Create symlink
ln -sf "$INSTALL_PREFIX/bin/tbos_detector" "$BIN_LINK"

# Install desktop entry
if [ -d "/usr/share/applications" ]; then
    cp "$INSTALL_PREFIX/share/applications/tbos-detector.desktop" \
       "/usr/share/applications/"
fi

echo ""
echo "✅ TBOS Detector installed successfully!"
echo ""
echo "Run with: tbos-detector --detect"
echo "Or from menu: Applications → System → TBOS Detector"
echo ""
echo "🕉️ Swamiye Saranam Aiyappa 🕉️"

exit 0

__PAYLOAD_BEGIN__
INSTALL_EOF

echo "✅ Installation script created"

echo "[4/5] Creating payload archive..."
cd "${PAYLOAD_DIR}"
tar czf "${BUILD_DIR}/payload.tar.gz" .
echo "✅ Payload archived"

echo "[5/5] Building self-extracting installer..."
cd "${BUILD_DIR}"

# Replace INSTALLER_NAME placeholder
sed -i "s/INSTALLER_NAME/${INSTALLER_NAME}/g" install.sh

# Combine installer script and payload
cat install.sh > "${SCRIPT_DIR}/dist/${INSTALLER_NAME}"
cat payload.tar.gz >> "${SCRIPT_DIR}/dist/${INSTALLER_NAME}"

# Make executable
chmod +x "${SCRIPT_DIR}/dist/${INSTALLER_NAME}"

echo "✅ Self-extracting installer created"
echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║  ✅ BUILD COMPLETE                                       ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Installer: dist/${INSTALLER_NAME}"
echo "Size: $(du -h "${SCRIPT_DIR}/dist/${INSTALLER_NAME}" | cut -f1)"
echo ""
echo "To install:"
echo "  sudo ./dist/${INSTALLER_NAME}"
echo ""
echo "🕉️ Swamiye Saranam Aiyappa 🕉️"
