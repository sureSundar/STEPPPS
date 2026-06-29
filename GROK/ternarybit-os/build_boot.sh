#!/bin/bash

# Simple bootloader build script
# Usage: ./build_boot.sh

set -e  # Exit on error

# Create build directory
BUILD_DIR="build"
mkdir -p "$BUILD_DIR"

# Build first stage bootloader
echo "[1/3] Building timed bootloader..."
nasm -f bin boot/timed_boot.asm -o "$BUILD_DIR/boot.bin"

# Verify boot sector size
BOOT_SIZE=$(wc -c < "$BUILD_DIR/boot.bin")
if [ "$BOOT_SIZE" -ne 512 ]; then
    echo "Error: Boot sector must be exactly 512 bytes (got $BOOT_SIZE)"
    exit 1
fi

# Create disk image
echo "[2/3] Creating disk image..."
dd if=/dev/zero of="$BUILD_DIR/tbos.img" bs=512 count=2880  # 1.44MB floppy
dd if="$BUILD_DIR/boot.bin" of="$BUILD_DIR/tbos.img" conv=notrunc

echo "[3/3] Build complete!"
echo "To test: qemu-system-i386 -fda $BUILD_DIR/tbos.img"
