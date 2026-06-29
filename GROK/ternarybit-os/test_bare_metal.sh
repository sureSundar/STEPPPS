#!/bin/bash
# TernaryBit OS - Bare Metal Boot Test Script
# 🕉️ Swamiye Saranam Aiyappa 🕉️

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "========================================"
echo "  TernaryBit OS Bare Metal Boot Test"
echo "========================================"
echo ""

# Build if needed
if [[ ! -f "build/tbos_universal.img" ]]; then
    echo "Building TernaryBit OS..."
    ./build_universal.sh
    echo ""
fi

echo "Image: build/tbos_universal.img"
echo "Size: $(stat -c%s build/tbos_universal.img) bytes"
echo ""
echo "Boot sequence:"
echo "  1. BIOS loads MBR (sector 0)"
echo "  2. Stage 1 bootloader loads Stage 2"
echo "  3. Stage 2 enters protected mode"
echo "  4. Kernel initializes and displays message"
echo ""
echo "Press Ctrl+C to exit QEMU"
echo "----------------------------------------"
echo ""

# Run QEMU
qemu-system-i386 \
    -drive file=build/tbos_universal.img,format=raw,if=floppy \
    -nographic \
    -no-reboot

echo ""
echo "QEMU exited."
