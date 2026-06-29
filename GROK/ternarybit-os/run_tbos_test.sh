#!/bin/bash
# TBOS Full System Test
# 🕉️ Complete Boot Test 🕉️

echo "=========================================="
echo "  TernaryBit OS - Full System Boot Test"
echo "  Bootloader → Kernel → Shell"
echo "=========================================="
echo
echo "Starting QEMU..."
echo "Watch for:"
echo "  1. Boot messages from bootloader"
echo "  2. Kernel initialization"
echo "  3. Shell prompt (TBOS>)"
echo
echo "Press Ctrl-A then X to exit QEMU"
echo
echo "Starting in 3 seconds..."
sleep 3

# Run QEMU with serial output
qemu-system-i386 \
    -drive format=raw,file=build/tbos.img \
    -m 128M \
    -serial stdio \
    -display none

echo
echo "=========================================="
echo "  TBOS Boot Test Complete"
echo "=========================================="
