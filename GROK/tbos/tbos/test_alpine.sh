#!/bin/bash
# Test Alpine Linux Boot

echo "╔══════════════════════════════════════════════════════════╗"
echo "║           Testing Alpine Linux Boot                     ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

ALPINE_ISO="/home/vboxuser/ssr/tbos/build/alpine/build/alpine/alpine-virt-3.19.1-x86_64.iso"

if [ ! -f "$ALPINE_ISO" ]; then
    echo "❌ Alpine ISO not found at: $ALPINE_ISO"
    exit 1
fi

echo "Alpine ISO found: $(ls -lh "$ALPINE_ISO" | awk '{print $5}')"
echo ""
echo "Starting QEMU..."
echo "Login as 'root' (no password)"
echo ""
echo "To test TernaryBit later, we'll integrate it into Alpine"
echo "Press Ctrl+A then X to exit QEMU"
echo ""
echo "════════════════════════════════════════════════════════════"
echo ""

sleep 2

# Boot Alpine
qemu-system-x86_64 \
    -m 512 \
    -cdrom "$ALPINE_ISO" \
    -nographic \
    -serial mon:stdio
