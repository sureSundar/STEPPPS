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
echo "[BUILD] Assembling stage 2..."
nasm -f bin boot/stage2_sacred.asm -o build/stage2.bin

# Compile kernel
echo "[BUILD] Compiling kernel..."
# Assemble kernel entry
nasm -f elf32 kernel/kernel_sacred.asm -o build/kernel_entry.o

# Compile C files
gcc -m32 -c kernel/kernel_simple.c -o build/kernel.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/memory.c -o build/memory.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/interrupt.c -o build/interrupt.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/timer.c -o build/timer.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2
gcc -m32 -c kernel/steppps.c -o build/steppps.o -ffreestanding -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -O2

# Link kernel
echo "[BUILD] Linking kernel..."
ld -m elf_i386 -T kernel/linker_sacred.ld -o build/kernel.bin build/kernel_entry.o build/kernel.o build/memory.o build/interrupt.o build/timer.o build/steppps.o

# Create disk image
echo "[BUILD] Creating disk image..."
dd if=/dev/zero of=build/tbos.img bs=1M count=10 2>/dev/null
dd if=build/boot.bin of=build/tbos.img conv=notrunc 2>/dev/null
dd if=build/stage2.bin of=build/tbos.img seek=1 conv=notrunc 2>/dev/null
dd if=build/kernel.bin of=build/tbos.img seek=10 conv=notrunc 2>/dev/null

echo "[BUILD] Build complete!"
echo ""
echo "To test: qemu-system-i386 -fda build/tbos.img"
echo ""
echo "🕉️ Sprint 11 Complete - Pathinettam Padi 🕉️"