#!/bin/bash

# Build script for timed TBOS bootloader
# Usage: ./build_timed.sh

set -e  # Exit on error

# Configuration
BUILD_DIR="build_timed"
BOOT_ASM="boot/boot_timed.asm"
BOOT_BIN="$BUILD_DIR/boot.bin"
SECOND_STAGE_SRC="boot/tbos_bootstrap_fast.c"
SECOND_STAGE_ELF="$BUILD_DIR/stage2.elf"
SECOND_STAGE_BIN="$BUILD_DIR/stage2.bin"
DISK_IMG="$BUILD_DIR/tbos_timed.img"

# Create build directory
mkdir -p "$BUILD_DIR"

# Build first stage (boot sector)
echo "[1/3] Building timed bootloader..."
nasm -f bin -o "$BOOT_BIN" "$BOOT_ASM"

# Verify boot sector size
BOOT_SIZE=$(wc -c < "$BOOT_BIN")
if [ "$BOOT_SIZE" -ne 512 ]; then
    echo "Error: Boot sector must be exactly 512 bytes (got $BOOT_SIZE)"
    exit 1
fi

# Build second stage bootloader
echo "[2/3] Building second stage..."
# Assemble startup code
nasm -f elf32 -o "$BUILD_DIR/startup.o" boot/startup.S

# Compile second stage with minimal dependencies
gcc -m32 -O3 -fno-stack-protector -ffreestanding -fno-builtin \
    -march=i386 -m32 -mfpmath=387 -mno-sse -mno-sse2 \
    -Iinclude -I. -nostdlib -nostartfiles -nodefaultlibs -fno-pie \
    -c -o "$BUILD_DIR/stage2.o" "$SECOND_STAGE_SRC"

# Link everything together
ld -m elf_i386 -Ttext 0x8000 -Tdata 0x9000 -Tbss 0xA000 \
    --oformat binary -o "$SECOND_STAGE_BIN" \
    "$BUILD_DIR/startup.o" "$BUILD_DIR/stage2.o"

# Create disk image
echo "[3/3] Creating disk image..."
dd if=/dev/zero of="$DISK_IMG" bs=512 count=2880  # 1.44MB floppy
# Copy boot sector
dd if="$BOOT_BIN" of="$DISK_IMG" conv=notrunc
# Copy second stage (starts at sector 2)
dd if="$SECOND_STAGE_BIN" of="$DISK_IMG" bs=512 seek=1 conv=notrunc

echo "\nBuild complete! Disk image: $DISK_IMG"
echo "To test with QEMU: qemu-system-i386 -fda $DISK_IMG"

# Display bootloader sizes
echo -e "\nBootloader Sizes:"
echo "- First stage: $(wc -c < "$BOOT_BIN") bytes"
echo "- Second stage: $(wc -c < "$SECOND_STAGE_BIN") bytes"
