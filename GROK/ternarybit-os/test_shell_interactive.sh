#!/bin/bash
echo "=== Testing Step 3 Shell with Keyboard Input ==="
echo ""
echo "This will boot the shell and you can type commands."
echo "Try typing: help, clear, about"
echo ""
echo "Press Ctrl+C to exit"
echo ""
read -p "Press Enter to start..."

qemu-system-i386 -drive file=build/tbos_step3.img,format=raw,if=floppy
