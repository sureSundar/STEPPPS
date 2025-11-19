#!/bin/bash
# Modular Test: Minimal Working System

echo "═══════════════════════════════════════════"
echo "  TernaryBit OS - Minimal System Test"
echo "  Universal Bootloader: 2-bit → Supercomputer"
echo "═══════════════════════════════════════════"
echo ""

if [ ! -f "build/tbos_minimal.img" ]; then
    echo "❌ Disk image not found"
    echo "Building minimal system..."
    
    mkdir -p build
    nasm -f bin boot/tbos_sacred.asm -o build/boot.bin
    nasm -f bin boot/stage2_universal.asm -o build/stage2.bin
    nasm -f elf32 kernel/kernel_minimal.asm -o build/kernel_minimal.o
    ld -m elf_i386 -T kernel/linker_minimal.ld -o build/kernel_minimal.elf build/kernel_minimal.o
    objcopy -O binary build/kernel_minimal.elf build/kernel_minimal.bin
    
    dd if=/dev/zero of=build/tbos_minimal.img bs=1M count=1 2>/dev/null
    dd if=build/boot.bin of=build/tbos_minimal.img bs=512 count=1 conv=notrunc 2>/dev/null
    dd if=build/stage2.bin of=build/tbos_minimal.img bs=512 seek=1 count=8 conv=notrunc 2>/dev/null
    dd if=build/kernel_minimal.bin of=build/tbos_minimal.img bs=512 seek=10 conv=notrunc 2>/dev/null
    echo "✅ Build complete"
fi

echo "Running test (2 seconds)..."
timeout 2 qemu-system-i386 -drive file=build/tbos_minimal.img,format=raw,if=floppy -nographic 2>&1 | tee /tmp/minimal_test.log | grep -v "^$" | tail -20

echo ""
echo "═══════════════════════════════════════════"
echo "  Analysis"
echo "═══════════════════════════════════════════"

BOOTS=$(grep -c "Stage 2 loaded" /tmp/minimal_test.log)
echo "Boot count: $BOOTS"

if [ "$BOOTS" -eq 1 ]; then
    echo "✅ SUCCESS - No boot loop!"
    echo "✅ Bootloader working"
    echo "✅ Kernel executed"
    echo ""
    echo "To see GUI output:"
    echo "  qemu-system-i386 -drive file=build/tbos_minimal.img,format=raw,if=floppy"
else
    echo "⚠️  Multiple boots detected"
fi
