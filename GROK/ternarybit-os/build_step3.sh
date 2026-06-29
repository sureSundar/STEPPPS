#!/bin/bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os

echo "=== Building Kernel Step 3: Shell ==="

nasm -f elf32 kernel/kernel_step3.asm -o build/kernel_step3.o
ld -m elf_i386 -T kernel/linker_minimal.ld -o build/kernel_step3.elf build/kernel_step3.o
objcopy -O binary build/kernel_step3.elf build/kernel_step3.bin

echo "Kernel size: $(wc -c < build/kernel_step3.bin) bytes"

dd if=/dev/zero of=build/tbos_step3.img bs=1M count=1 2>/dev/null
dd if=build/boot.bin of=build/tbos_step3.img bs=512 count=1 conv=notrunc 2>/dev/null
dd if=build/stage2.bin of=build/tbos_step3.img bs=512 seek=1 count=8 conv=notrunc 2>/dev/null
dd if=build/kernel_step3.bin of=build/tbos_step3.img bs=512 seek=10 conv=notrunc 2>/dev/null

echo "✅ Build complete: tbos_step3.img"
echo ""
echo "Testing..."
timeout 2 qemu-system-i386 -drive file=build/tbos_step3.img,format=raw,if=floppy -nographic > /tmp/s3.log 2>&1

BOOTS=$(grep -c 'Stage 2 loaded' /tmp/s3.log)
echo "Boot count: $BOOTS"

if [ $BOOTS -eq 1 ]; then
    echo "✅ No boot loop - shell is running!"
    echo ""
    echo "To test interactively:"
    echo "  qemu-system-i386 -drive file=build/tbos_step3.img,format=raw,if=floppy"
    echo ""
    echo "Available commands: help, clear, about"
fi
