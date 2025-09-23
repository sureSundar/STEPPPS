#!/bin/bash
# TBOS Build Script - Sprint 11
# 🕉️ Swamiye Saranam Aiyappa 🕉️

echo "======================================"
echo "    TBOS Sacred Build System"
echo "    Swamiye Saranam Aiyappa"
echo "======================================"

# Create build directory
mkdir -p build

# Assemble boot sector
echo "[BUILD] Assembling boot sector..."
nasm -f bin boot/tbos_sacred.asm -o build/boot.bin

# Assemble stage 2
echo "[BUILD] Using stable stage2..."
cp build/stage2_linux.bin build/stage2.bin

# Compile kernel
echo "[BUILD] Compiling kernel..."
# Assemble kernel entry
nasm -f elf32 kernel/kernel_sacred.asm -o build/kernel_entry.o

# Compile C files
gcc -m32 -c kernel/kernel.c -o build/kernel.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/shell.c -o build/shell.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c drivers/keyboard/keyboard.c -o build/keyboard.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/memory.c -o build/memory.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/interrupt.c -o build/interrupt.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/timer.c -o build/timer.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/steppps.c -o build/steppps.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c fs/pxfs_core.c -o build/pxfs.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/ternary_engine.c -o build/ternary.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/aito_sequence.c -o build/aito.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/process.c -o build/process.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/scheduler.c -o build/scheduler.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/drivers.c -o build/drivers.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/integration.c -o build/integration.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/gui.c -o build/gui.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/network.c -o build/network.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/audio.c -o build/audio.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c fs/pxfs_advanced.c -o build/pxfs_advanced.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/security.c -o build/security.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/performance.c -o build/performance.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/testing.c -o build/testing.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/resonance.c -o build/resonance.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/processor_analysis.c -o build/processor_analysis.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2

# Link kernel
echo "[BUILD] Linking kernel..."
ld -m elf_i386 -T kernel/linker_sacred.ld -o build/kernel.bin build/kernel_entry.o build/kernel.o build/shell.o build/keyboard.o build/memory.o build/interrupt.o build/timer.o build/steppps.o build/pxfs.o build/ternary.o build/aito.o build/process.o build/scheduler.o build/drivers.o build/integration.o build/gui.o build/network.o build/audio.o build/pxfs_advanced.o build/security.o build/performance.o build/testing.o build/resonance.o build/processor_analysis.o

# Build Alpine GUI Integration
echo "[BUILD] Building Alpine GUI integration..."
if [ ! -f "deploy/alpine/out/x86_64/initramfs.gz" ]; then
    echo "[ALPINE] Building Alpine initramfs with GUI..."
    cd deploy/alpine
    ./build-x86_64.sh
    cd ../..
else
    echo "[ALPINE] Using existing Alpine initramfs"
fi

# Create disk image with Alpine
echo "[BUILD] Creating sacred disk image with Alpine GUI..."
dd if=/dev/zero of=build/tbos.img bs=1M count=50 2>/dev/null
dd if=build/boot.bin of=build/tbos.img conv=notrunc 2>/dev/null
dd if=build/stage2.bin of=build/tbos.img seek=1 conv=notrunc 2>/dev/null
dd if=build/kernel.bin of=build/tbos.img seek=10 conv=notrunc 2>/dev/null

# Add Alpine initramfs if available
if [ -f "deploy/alpine/out/x86_64/initramfs.gz" ]; then
    echo "[ALPINE] Adding Alpine GUI to disk image..."
    dd if=deploy/alpine/out/x86_64/initramfs.gz of=build/tbos.img seek=500 conv=notrunc 2>/dev/null
fi

echo "[BUILD] Build complete!"
echo ""
echo "To test: qemu-system-i386 -fda build/tbos.img"
echo ""
echo "🕉️ Sprint 11 Complete - Pathinettam Padi 🕉️"