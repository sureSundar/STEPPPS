#!/bin/bash

# Simple build script for TBOS bootloader
# Usage: ./build_simple.sh

set -e  # Exit on error

# Create build directory if it doesn't exist
mkdir -p build

# Build first stage bootloader
echo "Building first stage bootloader..."
nasm -f bin -o build/boot.bin boot/boot.asm

# Verify boot sector size
BOOT_SIZE=$(wc -c < build/boot.bin)
if [ "$BOOT_SIZE" -ne 512 ]; then
    echo "Error: Boot sector must be exactly 512 bytes (got $BOOT_SIZE)"
    exit 1
fi

# Create a simple disk image
echo "Creating disk image..."
dd if=/dev/zero of=build/tbos.img bs=512 count=2880  # 1.44MB floppy
dd if=build/boot.bin of=build/tbos.img conv=notrunc

echo "\nBuild complete! Disk image: build/tbos.img"
echo "To test with QEMU: qemu-system-i386 -fda build/tbos.img"
