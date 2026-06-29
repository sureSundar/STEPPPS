#!/bin/bash
echo "Starting TernaryBit OS Shell..."
echo ""
echo "This will open a QEMU window."
echo "Click in the window and start typing!"
echo ""
sleep 1

qemu-system-i386 -drive file=build/tbos_shell.img,format=raw,if=floppy
