#!/bin/bash

# TernaryBit OS - Complete Build Using Latest Git Version
# Uses the dharma-aligned GDT fix with proper STEPPPS kernel

echo "🕉️ Building Complete TBOS using Latest Git Version..."
echo "✅ Using dharma-aligned GDT fix from git"

# Create build directory
mkdir -p build

# Step 1: Build the simple STEPPPS kernel
echo "🔨 Building STEPPPS Consciousness Kernel..."
gcc -m32 -c -ffreestanding -nostdlib -fno-pie -o build/kernel.o kernel/simple_steppps_kernel.c
ld -m elf_i386 -Ttext 0x10000 --oformat binary -o build/kernel.bin build/kernel.o

# Step 2: Build stage2 with GDT fix
echo "🔨 Building Stage2 with dharma-aligned GDT..."
nasm -f bin boot/stage2_simple.asm -o build/stage2.bin

# Step 3: Build compact bootloader
echo "🔨 Building Compact Bootloader..."
nasm -f bin boot/boot_compact.asm -o build/boot.bin

# Step 4: Create complete disk image
echo "🔨 Creating Complete Disk Image..."
dd if=/dev/zero of=build/tbos_git_complete.img bs=512 count=64 2>/dev/null

# Write bootloader (sector 0)
dd if=build/boot.bin of=build/tbos_git_complete.img bs=512 seek=0 count=1 conv=notrunc 2>/dev/null

# Write stage2 (sectors 1-4)
dd if=build/stage2.bin of=build/tbos_git_complete.img bs=512 seek=1 count=4 conv=notrunc 2>/dev/null

# Write kernel (sectors 6-13)
dd if=build/kernel.bin of=build/tbos_git_complete.img bs=512 seek=6 count=8 conv=notrunc 2>/dev/null

echo "✅ Complete TBOS built successfully!"
echo "🎯 Output: build/tbos_git_complete.img"
echo "🧪 Testing the complete system..."

# Test the complete system
qemu-system-i386 -fda build/tbos_git_complete.img -nographic -serial mon:stdio