#!/bin/bash
# TernaryBit OS Universal Installer

INSTALL_DIR="/usr/local/bin"
USER_DIR="$HOME/.tbos"

echo "🕉️  Installing TernaryBit OS Universal..."

# Create user directory
mkdir -p "$USER_DIR"

# Copy files
if [ -w "$INSTALL_DIR" ]; then
    echo "Installing to system directory: $INSTALL_DIR"
    cp tbos_universal "$INSTALL_DIR/" 2>/dev/null || true
    cp tbos_launcher.sh "$INSTALL_DIR/tbos" 2>/dev/null || true
else
    echo "Installing to user directory: $USER_DIR"
    cp tbos_universal "$USER_DIR/"
    cp tbos_launcher.sh "$USER_DIR/tbos.sh"
    echo "Add $USER_DIR to your PATH to use 'tbos' command"
fi

# Copy Java implementation
cp TernaryBitOS.java "$USER_DIR/"

echo "✅ TernaryBit OS Universal installed!"
echo "Run with: tbos (if in PATH) or ./tbos_launcher.sh"
echo "🙏 Om Namah Shivaya"
