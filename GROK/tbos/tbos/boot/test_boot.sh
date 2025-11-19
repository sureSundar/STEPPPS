#!/bin/bash
# Test TernaryBit OS boot in QEMU

echo "╔══════════════════════════════════════════════════════════╗"
echo "║         Testing TernaryBit OS Boot in QEMU              ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

BUILD_DIR="/home/vboxuser/ssr/tbos/build"

if [ ! -f "${BUILD_DIR}/tbos.img" ]; then
    echo "Error: Boot image not found!"
    echo "Please run ./boot/build_bootable.sh first"
    exit 1
fi

echo "Select test mode:"
echo "  1. Text mode (no graphics, can see serial output)"
echo "  2. GUI mode (graphical window)"
echo "  3. Screenshot test (boot and save screenshot)"
echo ""
read -p "Enter choice (1-3): " choice

case $choice in
    1)
        echo ""
        echo "Booting in text mode..."
        echo "Press Ctrl+A then X to exit QEMU"
        echo ""
        sleep 2
        qemu-system-x86_64 \
            -drive file="${BUILD_DIR}/tbos.img",format=raw \
            -serial stdio \
            -nographic
        ;;
    2)
        echo ""
        echo "Booting in GUI mode..."
        echo "Close the window to exit"
        echo ""
        sleep 2
        qemu-system-x86_64 \
            -drive file="${BUILD_DIR}/tbos.img",format=raw \
            -serial stdio
        ;;
    3)
        echo ""
        echo "Booting and capturing screenshot..."
        timeout 5 qemu-system-x86_64 \
            -drive file="${BUILD_DIR}/tbos.img",format=raw \
            -vnc :1 \
            -daemonize \
            -pidfile /tmp/qemu-tbos.pid

        sleep 2

        if command -v vncsnapshot &> /dev/null; then
            vncsnapshot localhost:1 "${BUILD_DIR}/boot_screenshot.jpg"
            echo "✓ Screenshot saved to: ${BUILD_DIR}/boot_screenshot.jpg"
        else
            echo "vncsnapshot not installed"
        fi

        if [ -f /tmp/qemu-tbos.pid ]; then
            kill $(cat /tmp/qemu-tbos.pid) 2>/dev/null
            rm /tmp/qemu-tbos.pid
        fi
        ;;
    *)
        echo "Invalid choice"
        exit 1
        ;;
esac
