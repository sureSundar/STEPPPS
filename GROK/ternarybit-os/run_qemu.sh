#!/bin/bash
# TernaryBit OS - QEMU Test Runner
# 🕉️ Swamiye Saranam Aiyappa 🕉️

echo "=== TernaryBit OS - QEMU Test ==="
echo ""

cd "$(dirname "$0")"

# Check if build exists
if [ ! -f "build/tbos_universal.img" ] && [ ! -f "build/tbos_bare_metal.img" ]; then
    echo "Error: build/tbos_universal.img not found!"
    echo "Please run: ./build_universal.sh first"
    exit 1
fi

echo "Starting TernaryBit OS in QEMU..."
echo ""
echo "What to expect:"
echo "  1. BIOS screen (few seconds)"
echo "  2. TBOS bootloader with STEPPPS messages"
echo "  3. Stage 2 protected mode bootloader"
echo "  4. Kernel initialization messages"
echo "  5. Interactive shell: TBOS> _"
echo ""
echo "Try these commands in the shell:"
echo "  help     - Show all commands"
echo "  calc 2+2 - Calculator test"
echo "  steppps  - Framework status"
echo "  about    - OS information"
echo ""
echo "Press Ctrl+Alt+2 for QEMU monitor"
echo "Press Ctrl+Alt+Q to quit QEMU"
echo ""
echo "Launching QEMU..."
echo ""

# Determine disk image to use (prefer universal)
DISK_IMAGE=$(ls build/tbos_universal.img build/tbos_bare_metal.img 2>/dev/null | head -n1)

# Run QEMU with proper format specification
qemu-system-i386 \
    -drive file="${DISK_IMAGE}",format=raw,if=floppy \
    -m 512 \
    -name "TernaryBit OS"

echo ""
echo "QEMU exited."
