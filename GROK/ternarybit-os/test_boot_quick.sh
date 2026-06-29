#!/bin/bash
# Quick Boot Test - TernaryBit OS
# 🕉️ Swamiye Saranam Aiyappa 🕉️

echo "==================================="
echo "  TernaryBit OS - Quick Boot Test"
echo "==================================="
echo ""

cd "$(dirname "$0")"

if [ ! -f "build/tbos_universal.img" ]; then
    echo "❌ Build not found. Run: ./build_universal.sh"
    exit 1
fi

echo "✅ Image found: build/tbos_universal.img"
echo ""
echo "🔍 Boot signature check..."
BOOT_SIG=$(xxd -s 510 -l 2 -p build/boot.bin)
if [ "$BOOT_SIG" = "55aa" ]; then
    echo "  ✅ Valid boot signature (0x55AA)"
else
    echo "  ❌ Invalid signature: $BOOT_SIG"
    exit 1
fi
echo ""

echo "🚀 Booting in QEMU..."
echo ""
echo "Controls:"
echo "  • Type commands in the QEMU window"
echo "  • Try: help, calc 2+2, steppps, about, om"
echo "  • Close window or Ctrl+C to exit"
echo ""
echo "Starting in 2 seconds..."
sleep 2

# Boot with default display (will use GTK, curses, or VNC depending on availability)
exec qemu-system-x86_64 \
    -drive file=build/tbos_universal.img,format=raw \
    -m 512 \
    -cpu qemu64 \
    -name "TernaryBit OS"
