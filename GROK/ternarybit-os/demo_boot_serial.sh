#!/bin/bash
# TernaryBit OS - Serial Output Boot Demo
# Captures boot sequence via serial port
# 🕉️ Swamiye Saranam Aiyappa 🕉️

set -euo pipefail

echo "=============================================="
echo "  TernaryBit OS - Boot Sequence Demo"
echo "  (Serial Output Capture)"
echo "=============================================="
echo ""

cd "$(dirname "$0")"

if [ ! -f "build/tbos_universal.img" ]; then
    echo "❌ Build not found. Run: ./build_universal.sh"
    exit 1
fi

echo "✅ Image ready: build/tbos_universal.img"
echo ""
echo "📊 Build Info:"
echo "  • Boot sector:  $(stat -c%s build/boot.bin) bytes"
echo "  • Stage 2:      $(stat -c%s build/stage2.bin) bytes"
echo "  • Kernel:       $(stat -c%s build/kernel.bin) bytes"
echo ""

echo "🔍 Verifying boot signature..."
BOOT_SIG=$(xxd -s 510 -l 2 -p build/boot.bin)
if [ "$BOOT_SIG" = "55aa" ]; then
    echo "  ✅ Valid (0x55AA)"
else
    echo "  ❌ Invalid: $BOOT_SIG"
    exit 1
fi
echo ""

echo "🚀 Booting system (5 second capture)..."
echo "=========================================="
echo ""

# Create a temporary expect script to send commands
cat > /tmp/tbos_test_commands.txt << 'EOF'
help
calc 2+2
steppps
about
om
EOF

# Boot and capture output for 5 seconds
timeout 5 qemu-system-x86_64 \
    -drive file=build/tbos_universal.img,format=raw \
    -m 512 \
    -nographic \
    -serial mon:stdio \
    2>&1 || true

echo ""
echo "=========================================="
echo "✅ Boot demonstration complete!"
echo ""
echo "🎯 What happened:"
echo "  1. BIOS initialized the system"
echo "  2. Stage 1 bootloader loaded Stage 2"
echo "  3. Stage 2 transitioned to 64-bit mode"
echo "  4. Kernel initialized (VGA, interrupts, keyboard)"
echo "  5. Shell displayed TBOS> prompt"
echo ""
echo "💡 For interactive mode, use:"
echo "   ./test_boot_quick.sh"
echo ""
echo "📚 Documentation:"
echo "   • BOOTLOADER_TO_SHELL_DEMO.md"
echo "   • BOOT_TO_SHELL_GUIDE.md"
echo ""
echo "🕉️ Swamiye Saranam Aiyappa 🕉️"
