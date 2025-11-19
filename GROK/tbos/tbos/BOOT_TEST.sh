#!/bin/bash
# TernaryBit OS - Easy Boot Test

echo "╔══════════════════════════════════════════════════════════╗"
echo "║          TernaryBit OS - Boot Test                      ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

if [ ! -f build/tbos.img ]; then
    echo "❌ Error: build/tbos.img not found!"
    echo ""
    echo "Please run: ./boot/build_bootable.sh first"
    exit 1
fi

echo "Boot image found: build/tbos.img"
echo ""
echo "Starting QEMU boot test..."
echo "You should see:"
echo "  1. TernaryBit OS Bootloader"
echo "  2. Loading Stage 2..."
echo "  3. Stage 2 Loaded"
echo "  4. Entering Protected Mode..."
echo ""
echo "Press Ctrl+C to exit when done"
echo ""
echo "════════════════════════════════════════════════════════════"
echo ""

sleep 2

# Boot the image
qemu-system-x86_64 \
    -drive file=build/tbos.img,format=raw \
    -m 16 \
    -nographic

echo ""
echo "════════════════════════════════════════════════════════════"
echo "Boot test completed!"
echo "════════════════════════════════════════════════════════════"
