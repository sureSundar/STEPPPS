#!/bin/bash
# Boot the REAL TernaryBit OS

clear
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║   TernaryBit OS - Booting REAL Operating System             ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""
echo "Booting from: test_disk.img"
echo ""
echo "Once booted, you can:"
echo "  - Type commands at the shell prompt"
echo "  - Press Ctrl+A then X to exit QEMU"
echo ""
read -p "Press Enter to boot the OS..."

cd /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os
qemu-system-i386 -drive file=test_disk.img,format=raw,if=floppy -serial mon:stdio
