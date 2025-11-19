#!/bin/bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os

echo "=== Building Kernel Step 3A: Echo Test (Simplified) ==="

nasm -f elf32 kernel/kernel_step3a.asm -o build/kernel_step3a.o
ld -m elf_i386 -T kernel/linker_minimal.ld -o build/kernel_step3a.elf build/kernel_step3a.o
objcopy -O binary build/kernel_step3a.elf build/kernel_step3a.bin

echo "Kernel size: $(wc -c < build/kernel_step3a.bin) bytes"

dd if=/dev/zero of=build/tbos_step3a.img bs=1M count=1 2>/dev/null
dd if=build/boot.bin of=build/tbos_step3a.img bs=512 count=1 conv=notrunc 2>/dev/null
dd if=build/stage2.bin of=build/tbos_step3a.img bs=512 seek=1 count=8 conv=notrunc 2>/dev/null
dd if=build/kernel_step3a.bin of=build/tbos_step3a.img bs=512 seek=10 conv=notrunc 2>/dev/null

echo "✅ Build complete: tbos_step3a.img"
echo ""
echo "Testing for boot loop..."
timeout 2 qemu-system-i386 -drive file=build/tbos_step3a.img,format=raw,if=floppy -nographic > /tmp/s3a.log 2>&1

BOOTS=$(grep -c 'Stage 2 loaded' /tmp/s3a.log)
echo "Boot count: $BOOTS"

if [ $BOOTS -eq 1 ]; then
    echo "✅ No boot loop - echo test is stable!"
    echo ""
    echo "To test interactively:"
    echo "  qemu-system-i386 -drive file=build/tbos_step3a.img,format=raw,if=floppy"
else
    echo "❌ Still boot looping ($BOOTS boots)"
    echo "Check /tmp/s3a.log for details"
fi
