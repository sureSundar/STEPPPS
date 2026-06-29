#!/bin/bash
# Build MINIMAL working kernel
set -e

echo "=== Building Minimal Working Kernel ==="

mkdir -p build

echo "[1/4] Assembling kernel entry..."
nasm -f elf32 kernel/kernel_sacred.asm -o build/kernel_entry.o

echo "[2/4] Compiling minimal kernel..."
gcc -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
    -O2 -Wall -Wextra \
    -c kernel/kernel_minimal_working.c -o build/kernel_minimal.o

echo "[3/4] Linking..."
ld -m elf_i386 -T kernel/linker_sacred.ld -o build/kernel.elf \
    build/kernel_entry.o \
    build/kernel_minimal.o

echo "[4/4] Creating binary..."
objcopy -O binary build/kernel.elf build/kernel.bin

echo ""
echo "✅ Minimal kernel built: build/kernel.bin"
echo ""
echo "Now rebuild disk image with:"
echo "  ./build_universal.sh"
