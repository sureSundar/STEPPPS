#!/bin/bash
# Simple boot test - shows boot output

echo "════════════════════════════════════════════════════════"
echo "  TernaryBit OS - Simple Boot Test"
echo "════════════════════════════════════════════════════════"
echo ""
echo "Starting QEMU with boot image..."
echo "You should see the boot messages appear below."
echo ""
echo "To exit: Press Ctrl+C"
echo ""
echo "════════════════════════════════════════════════════════"
echo ""

sleep 2

# Boot with serial console output
qemu-system-x86_64 \
    -drive file=build/tbos.img,format=raw \
    -display none \
    -serial stdio \
    -m 16
