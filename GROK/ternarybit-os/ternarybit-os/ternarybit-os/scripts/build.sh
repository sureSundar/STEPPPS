#!/bin/bash
# TernaryBit OS Build Script (Simplified)

set -e

echo "==================================="
echo "Building TernaryBit OS v2.0 (Interactive)"
echo "==================================="

BUILD_DIR="build"
BOOT_DIR="boot"

# Create build directory
mkdir -p $BUILD_DIR

echo "Step 1: Building bootloader..."

# Assemble minimal boot sector (debug)
nasm -f bin $BOOT_DIR/boot_minimal.asm -o $BUILD_DIR/boot.bin

# Assemble minimal stage 2 (debug)
nasm -f bin $BOOT_DIR/stage2_minimal.asm -o $BUILD_DIR/stage2.bin

# Check boot sector size
BOOT_SIZE=$(stat -c%s "$BUILD_DIR/boot.bin")
if [ $BOOT_SIZE -ne 512 ]; then
    echo "Error: Boot sector must be exactly 512 bytes (got $BOOT_SIZE)"
    exit 1
fi

echo "✓ Bootloader built successfully"

echo "Step 2: Creating disk image..."

# Create empty disk image (1.44MB floppy)
dd if=/dev/zero of=$BUILD_DIR/om.img bs=512 count=2880 2>/dev/null

# Write boot sector
dd if=$BUILD_DIR/boot.bin of=$BUILD_DIR/om.img bs=512 count=1 conv=notrunc 2>/dev/null

# Write stage 2 bootloader (sectors 2-5)
dd if=$BUILD_DIR/stage2.bin of=$BUILD_DIR/om.img bs=512 seek=1 count=4 conv=notrunc 2>/dev/null

echo "✓ Disk image created: $BUILD_DIR/om.img"

echo "Step 3: Build verification..."

echo "  Boot sector: $(stat -c%s $BUILD_DIR/boot.bin) bytes"
echo "  Stage 2:     $(stat -c%s $BUILD_DIR/stage2.bin) bytes"
echo "  Disk image:  $(stat -c%s $BUILD_DIR/om.img) bytes"

echo ""
echo "🎉 TernaryBit OS built successfully!"
echo ""
echo "To test:"
echo "  qemu-system-x86_64 -fda build/om.img"