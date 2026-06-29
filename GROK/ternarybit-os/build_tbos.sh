#!/bin/bash

# TBOS Build Script
# Builds the bootloader and kernel, then creates a bootable disk image

set -e  # Exit on error

# Configuration
BUILD_DIR="build"
BOOTLOADER_SRC="boot/tbos_boot.asm"
BOOTLOADER_BIN="$BUILD_DIR/boot.bin"
KERNEL_ENTRY_ASM="kernel/entry.asm"
KERNEL_ENTRY_OBJ="$BUILD_DIR/entry.o"
KERNEL_MAIN_C="kernel/simple_kernel.c"
KERNEL_MAIN_OBJ="$BUILD_DIR/kernel.o"
KERNEL_ELF="$BUILD_DIR/kernel.elf"
KERNEL_BIN="$BUILD_DIR/kernel.bin"
LINKER_SCRIPT="kernel/linker.ld"
DISK_IMG="$BUILD_DIR/tbos.img"

# Create build directory
echo "[1/7] Setting up build environment..."
mkdir -p "$BUILD_DIR"

# Build bootloader
echo "[2/7] Building bootloader..."
nasm -f bin "$BOOTLOADER_SRC" -o "$BOOTLOADER_BIN"

# Verify boot sector size
BOOT_SIZE=$(wc -c < "$BOOTLOADER_BIN")
if [ "$BOOT_SIZE" -ne 512 ]; then
    echo "Error: Boot sector must be exactly 512 bytes (got $BOOT_SIZE)"
    exit 1
fi

# Assemble kernel entry point
echo "[3/7] Assembling kernel entry point..."
nasm -f elf32 "$KERNEL_ENTRY_ASM" -o "$KERNEL_ENTRY_OBJ"

# Compile kernel C code
echo "[4/7] Compiling kernel..."
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c "$KERNEL_MAIN_C" -o "$KERNEL_MAIN_OBJ" -I./include

# Link kernel
echo "[5/7] Linking kernel..."
ld -m elf_i386 -T "$LINKER_SCRIPT" -o "$KERNEL_ELF" "$KERNEL_ENTRY_OBJ" "$KERNEL_MAIN_OBJ"

# Create raw binary from ELF
echo "[6/7] Creating kernel binary..."
objcopy -O binary "$KERNEL_ELF" "$KERNEL_BIN"

# Create disk image
echo "[7/7] Creating disk image..."
# Create blank 1.44MB floppy image
dd if=/dev/zero of="$DISK_IMG" bs=512 count=2880 2>/dev/null
# Write bootloader to first sector
dd if="$BOOTLOADER_BIN" of="$DISK_IMG" conv=notrunc 2>/dev/null
# Write kernel after bootloader (starting at sector 2)
dd if="$KERNEL_BIN" of="$DISK_IMG" bs=512 seek=1 conv=notrunc 2>/dev/null

# Print build summary
echo -e "\nBuild complete!"
echo "Bootloader size: $(wc -c < "$BOOTLOADER_BIN") bytes"
echo "Kernel size: $(wc -c < "$KERNEL_BIN") bytes"
echo "Disk image: $DISK_IMG"
echo -e "\nTo test with QEMU:"
echo "qemu-system-i386 -fda $DISK_IMG -d int -no-reboot"
echo -e "\nFor debugging:"
echo "qemu-system-i386 -fda $DISK_IMG -s -S & gdb -ex 'target remote localhost:1234' -ex 'symbol-file $KERNEL_ELF'"
