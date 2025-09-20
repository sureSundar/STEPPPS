#!/bin/bash

# Build script for optimized TBOS bootloader
# Usage: ./build_fast_boot.sh

set -e  # Exit on error

# Configuration
BUILD_DIR="build_fast"
BOOT_ASM="../boot/boot_fast.asm"
BOOT_BIN="$BUILD_DIR/boot.bin"
BOOT_ELF="$BUILD_DIR/boot.elf"
BOOT_MAP="$BUILD_DIR/boot.map"
SECOND_STAGE="../boot/tbos_bootstrap_fast.c"
SECOND_STAGE_ELF="$BUILD_DIR/stage2.elf"
SECOND_STAGE_BIN="$BUILD_DIR/stage2.bin"

# Create build directory
mkdir -p "$BUILD_DIR"

# Build first stage (boot sector)
echo "[1/3] Building first stage bootloader..."
nasm -f bin -o "$BOOT_BIN" -l "$BUILD_DIR/boot.lst" "$BOOT_ASM"

# Verify boot sector size (must be exactly 512 bytes)
BOOT_SIZE=$(wc -c < "$BOOT_BIN")
if [ "$BOOT_SIZE" -ne 512 ]; then
    echo "Error: Boot sector must be exactly 512 bytes (got $BOOT_SIZE)"
    exit 1
fi

# Build second stage bootloader
echo "[2/3] Building second stage bootloader..."
# Use optimizations: -O3 for speed, -fno-stack-protector for smaller code
# -ffreestanding and -fno-builtin for bare-metal environment
gcc -m32 -O3 -fno-stack-protector -ffreestanding -fno-builtin \
    -I../include -nostdlib -nostartfiles -nodefaultlibs \
    -Wl,-Ttext=0x8000 -Wl,--oformat=binary \
    -o "$SECOND_STAGE_BIN" "$SECOND_STAGE"

# Create disk image
echo "[3/3] Creating disk image..."
dd if=/dev/zero of="$BUILD_DIR/tbos.img" bs=512 count=2880  # 1.44MB floppy
# Copy boot sector
dd if="$BOOT_BIN" of="$BUILD_DIR/tbos.img" conv=notrunc
# Copy second stage (starts at sector 2)
dd if="$SECOND_STAGE_BIN" of="$BUILD_DIR/tbos.img" bs=512 seek=2 conv=notrunc

echo "\nBuild complete! Disk image: $BUILD_DIR/tbos.img"
echo "To test with QEMU: qemu-system-i386 -fda $BUILD_DIR/tbos.img"

# Display bootloader sizes
echo -e "\nBootloader Sizes:"
echo "- First stage: $(wc -c < "$BOOT_BIN") bytes"
echo "- Second stage: $(wc -c < "$SECOND_STAGE_BIN") bytes"
