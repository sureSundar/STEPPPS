#!/bin/bash
# TernaryBit OS - Day 1 Build Script
# Builds complete bootloader → kernel → shell with Day 1 features
# 🕉️ Swamiye Saranam Aiyappa 🕉️

set -e  # Exit on error

echo "=== TernaryBit OS Day 1 Build ==="
echo ""

# Create build directory
mkdir -p build

echo "[1/7] Assembling Stage 1 bootloader (MBR)..."
nasm -f bin boot/tbos_sacred.asm -o build/boot.bin
echo "  ✓ boot.bin created (512 bytes)"

echo "[2/7] Assembling Stage 2 bootloader (Universal - Protected Mode)..."
nasm -f bin boot/stage2_universal.asm -o build/stage2.bin
echo "  ✓ stage2.bin created (~4KB)"

echo "[3/7] Assembling kernel entry point..."
nasm -f elf32 kernel/kernel_sacred.asm -o build/kernel_entry.o
echo "  ✓ kernel_entry.o"

echo "[4/7] Assembling ISR stubs..."
nasm -f elf32 kernel/isr.asm -o build/isr.o
echo "  ✓ isr.o"

echo "[5/7] Compiling kernel C code with Day 1 features..."
gcc -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
    -O2 -Wall -Wextra \
    -c kernel/kernel_main.c -o build/kernel_main.o
echo "  ✓ kernel_main.o"

gcc -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
    -O2 -Wall -Wextra \
    -c kernel/interrupt.c -o build/interrupt.o
echo "  ✓ interrupt.o"

gcc -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
    -O2 -Wall -Wextra \
    -c drivers/keyboard/keyboard.c -o build/keyboard.o
echo "  ✓ keyboard.o"

gcc -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
    -O2 -Wall -Wextra \
    -c kernel/shell.c -o build/shell.o
echo "  ✓ shell.o (with Day 1 karma & consciousness)"

echo "[6/7] Linking kernel..."
ld -m elf_i386 -T kernel/linker_sacred.ld -o build/kernel.elf \
    build/kernel_entry.o \
    build/kernel_main.o \
    build/interrupt.o \
    build/isr.o \
    build/keyboard.o \
    build/shell.o

echo "  ✓ kernel.elf created"

echo "[6.5/7] Converting kernel to flat binary..."
objcopy -O binary build/kernel.elf build/kernel.bin

KERNEL_SIZE=$(wc -c < build/kernel.bin)
echo "  ✓ kernel.bin created ($KERNEL_SIZE bytes)"

echo "[7/7] Creating bootable disk image..."
# Create 10MB disk image
dd if=/dev/zero of=build/tbos_day1.img bs=1M count=10 status=none

# Write Stage 1 to sector 0 (MBR)
dd if=build/boot.bin of=build/tbos_day1.img conv=notrunc status=none

# Write Stage 2 to sector 1 (8 sectors = 4KB)
dd if=build/stage2.bin of=build/tbos_day1.img seek=1 conv=notrunc status=none

# Write kernel to sector 10 (50 sectors reserved)
dd if=build/kernel.bin of=build/tbos_day1.img seek=10 conv=notrunc status=none

echo "  ✓ tbos_day1.img created (10MB)"

echo ""
echo "=== Build Complete! ===="
echo ""
echo "Day 1 Features Integrated:"
echo "  ✓ Karma tracking (starts at 100)"
echo "  ✓ Consciousness levels (AWAKENING)"
echo "  ✓ karma command"
echo "  ✓ consciousness command"
echo "  ✓ Command execution tracking"
echo ""
echo "To test in QEMU:"
echo "  qemu-system-i386 -fda build/tbos_day1.img"
echo ""
