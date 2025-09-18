#!/bin/bash
# TernaryBit OS Test Script

echo "==============================="
echo "Testing TernaryBit OS in QEMU"
echo "==============================="

BUILD_DIR="build"
IMAGE="$BUILD_DIR/ternarybit.img"

if [ ! -f "$IMAGE" ]; then
    echo "Error: Disk image not found. Run ./scripts/build.sh first"
    exit 1
fi

echo "Starting TernaryBit OS in QEMU..."
echo "Press Ctrl+Alt+2 for QEMU monitor"
echo "Press Ctrl+Alt+1 to return to OS"
echo "Press Ctrl+A then X to quit QEMU"
echo ""

# Run QEMU with our disk image
qemu-system-x86_64 \
    -fda "$IMAGE" \
    -boot a \
    -m 128M \
    -display curses \
    -no-reboot \
    -monitor stdio