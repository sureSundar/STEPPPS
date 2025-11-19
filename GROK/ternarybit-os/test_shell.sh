#!/bin/bash
clear
echo "=========================================="
echo "  TernaryBit OS - Shell Test"
echo "=========================================="
echo ""
echo "Building kernel..."
./build_step3.sh > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    echo "Starting QEMU..."
    echo ""
    echo "Commands to try:"
    echo "  - Type 'help' and press Enter"
    echo "  - Type 'about' and press Enter"
    echo "  - Type 'clear' and press Enter"
    echo ""
    echo "Press Ctrl+C in QEMU window to exit"
    echo ""
    read -p "Press Enter to start the shell..."
    
    qemu-system-i386 -drive file=build/tbos_step3.img,format=raw,if=floppy
else
    echo "❌ Build failed"
fi
