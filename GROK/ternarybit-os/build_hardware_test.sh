#!/bin/bash
# Build and test universal hardware detection bootloader
# 🕉️ Swamiye Saranam Aiyappa 🕉️

set -e  # Exit on error

echo "╔══════════════════════════════════════════════════════════╗"
echo "║  TBOS Universal Bootloader - Hardware Detection Test    ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo

# Create build directory
mkdir -p build

echo "[1/4] Assembling Stage 1 bootloader..."
nasm -f bin boot/tbos_sacred.asm -o build/boot.bin

echo "[2/4] Assembling Stage 2 with hardware detection..."
nasm -f bin boot/stage2_hardware_test.asm -o build/stage2.bin

echo "[3/4] Creating bootable disk image..."
# Create 10MB disk image
dd if=/dev/zero of=build/tbos_hardware_test.img bs=512 count=20480 2>/dev/null

# Write stage 1 to sector 0
dd if=build/boot.bin of=build/tbos_hardware_test.img bs=512 count=1 conv=notrunc 2>/dev/null

# Write stage 2 starting at sector 2 (skip MBR)
dd if=build/stage2.bin of=build/tbos_hardware_test.img bs=512 seek=1 conv=notrunc 2>/dev/null

echo "[4/4] Build complete!"
echo
echo "Output: build/tbos_hardware_test.img"
echo
echo "╔══════════════════════════════════════════════════════════╗"
echo "║  To test in QEMU, run:                                   ║"
echo "║  qemu-system-i386 -fda build/tbos_hardware_test.img      ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo

# Ask if user wants to run test
read -p "Run test in QEMU now? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]
then
    echo "Launching QEMU..."
    echo "Press Ctrl+C to exit QEMU"
    sleep 2
    qemu-system-i386 -fda build/tbos_hardware_test.img
fi
