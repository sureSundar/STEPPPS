#!/bin/bash
# TernaryBit OS Bootloader Build Script
# 🕉️ Swamiye Saranam Aiyappa 🕉️
# Builds all bootloader components

set -e  # Exit on error

echo "======================================"
echo "  TBOS Bootloader Build System"
echo "  🕉️ Swamiye Saranam Aiyappa 🕉️"
echo "======================================"
echo ""

# Set paths
BOOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$BOOT_DIR/../build"

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Change to boot directory for relative includes
cd "$BOOT_DIR"

echo "[1/4] Building Stage 1 Bootloader (tbos_sacred.asm)..."
nasm -f bin tbos_sacred.asm -o "$BUILD_DIR/boot.bin"
if [ $? -eq 0 ]; then
    SIZE=$(stat -c%s "$BUILD_DIR/boot.bin")
    echo "      ✓ boot.bin created ($SIZE bytes, must be 512)"
    if [ "$SIZE" -ne 512 ]; then
        echo "      ⚠ WARNING: Boot sector should be exactly 512 bytes!"
    fi
else
    echo "      ✗ FAILED to build boot.bin"
    exit 1
fi

echo ""
echo "[2/4] Building Stage 2 Bootloader (stage2_linux.asm)..."
nasm -f bin stage2_linux.asm -o "$BUILD_DIR/stage2_linux.bin"
if [ $? -eq 0 ]; then
    SIZE=$(stat -c%s "$BUILD_DIR/stage2_linux.bin")
    echo "      ✓ stage2_linux.bin created ($SIZE bytes)"
else
    echo "      ✗ FAILED to build stage2_linux.bin"
    exit 1
fi

echo ""
echo "[3/4] Building Alternative Stage 2 (stage2_protected.asm)..."
nasm -f bin stage2_protected.asm -o "$BUILD_DIR/stage2_protected.bin"
if [ $? -eq 0 ]; then
    SIZE=$(stat -c%s "$BUILD_DIR/stage2_protected.bin")
    echo "      ✓ stage2_protected.bin created ($SIZE bytes)"
else
    echo "      ✗ FAILED to build stage2_protected.bin"
    exit 1
fi

echo ""
echo "[4/4] Selecting active Stage 2 bootloader..."
# Copy the preferred stage2 to stage2.bin
cp "$BUILD_DIR/stage2_linux.bin" "$BUILD_DIR/stage2.bin"
echo "      ✓ Using stage2_linux.bin as active stage2"

echo ""
echo "======================================"
echo "  Bootloader Build Complete!"
echo "======================================"
echo ""
echo "Generated files in $BUILD_DIR:"
ls -lh "$BUILD_DIR"/boot.bin "$BUILD_DIR"/stage2*.bin 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
echo ""
echo "To test the bootloader:"
echo "  1. Run: cd .. && ./build.sh"
echo "  2. Test: qemu-system-i386 -drive format=raw,file=build/tbos.img"
echo ""
echo "🕉️ Build Complete - Swamiye Saranam Aiyappa 🕉️"
