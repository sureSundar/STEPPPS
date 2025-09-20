#!/bin/bash
# Quick test of TernaryBit Linux bootloader

set -e

echo "Testing TernaryBit Linux bootloader..."

# Build the bootloader
nasm -f bin boot/boot_compact.asm -o build/boot.bin
nasm -f bin boot/stage2_linux.asm -o build/stage2_linux.bin

# Create a minimal test image
dd if=/dev/zero of=build/test_linux.img bs=512 count=2048 2>/dev/null

# Write bootloader
dd if=build/boot.bin of=build/test_linux.img bs=512 count=1 conv=notrunc 2>/dev/null
dd if=build/stage2_linux.bin of=build/test_linux.img bs=512 seek=1 conv=notrunc 2>/dev/null

# Create dummy kernel at sector 9
echo "Dummy Linux kernel" | dd of=build/test_linux.img bs=512 seek=9 conv=notrunc 2>/dev/null

echo "Test image created: build/test_linux.img"
echo "Run with: qemu-system-i386 -drive format=raw,file=build/test_linux.img -m 512M"