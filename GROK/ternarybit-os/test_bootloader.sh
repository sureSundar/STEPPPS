#!/bin/bash
# TernaryBit OS - Modular Test Script
cd "$(dirname "$0")"

echo "🕉️  TernaryBit OS - Universal Bootloader Test"
echo ""

timeout 2 qemu-system-i386 \
    -drive file=build/tbos_bare_metal.img,format=raw,if=floppy \
    -nographic > /tmp/boot_test.log 2>&1

echo "=== Boot Sequence Check ==="
grep -q "Stage 2 loaded" /tmp/boot_test.log && echo "✅ Stage 1: MBR OK" || echo "❌ Stage 1 failed"
grep -q "Kernel loaded" /tmp/boot_test.log && echo "✅ Stage 2: Disk read OK" || echo "❌ Disk read failed"
grep -q "protected mode" /tmp/boot_test.log && echo "✅ Stage 2: Protected mode OK" || echo "❌ PM failed"

BOOT_COUNT=$(grep -c "Stage 2 loaded" /tmp/boot_test.log)
if [ "$BOOT_COUNT" -gt 1 ]; then
    echo "⚠️  Boot loop detected ($BOOT_COUNT reboots)"
else
    echo "✅ No boot loop"
fi

echo ""
echo "To see GUI: qemu-system-i386 -drive file=build/tbos_bare_metal.img,format=raw,if=floppy"
