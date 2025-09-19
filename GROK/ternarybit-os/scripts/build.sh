#!/bin/bash
# TernaryBit OS Build Script

set -euo pipefail

BUILD_DIR="build"
BOOT_DIR="boot"
KERNEL_DIR="kernel"
TBOS_ROOT=$(pwd)

echo "======================================="
echo "Building TernaryBit OS (boot + kernel)"
echo "======================================="

display_step() {
    echo ""
    echo "==> $1"
}

mkdir -p "$BUILD_DIR"

# ---------------------------------------------------------------------------
# Step 1: Build 32-bit protected-mode kernel
# ---------------------------------------------------------------------------
display_step "Compiling protected-mode kernel"

nasm -f elf32 "$KERNEL_DIR/kernel_entry.asm" -o "$BUILD_DIR/kernel_entry.o"

gcc -m32 -ffreestanding -fno-stack-protector -nostdlib \
    -I"$TBOS_ROOT/include" \
    -c "$KERNEL_DIR/kernel.c" -o "$BUILD_DIR/kernel.o"

gcc -m32 -ffreestanding -fno-stack-protector -nostdlib \
    -I"$TBOS_ROOT/include" \
    -c "$KERNEL_DIR/support.c" -o "$BUILD_DIR/support.o"

gcc -m32 -ffreestanding -nostdlib -T "$KERNEL_DIR/linker.ld" \
    -o "$BUILD_DIR/kernel.elf" \
    "$BUILD_DIR/kernel_entry.o" \
    "$BUILD_DIR/kernel.o" \
    "$BUILD_DIR/support.o"

objcopy -O binary "$BUILD_DIR/kernel.elf" "$BUILD_DIR/kernel.bin"

dd if="$BUILD_DIR/kernel.bin" of="$BUILD_DIR/kernel_padded.bin" \
    bs=512 conv=sync status=none

KERNEL_SIZE=$(stat -c%s "$BUILD_DIR/kernel_padded.bin")
KERNEL_SECTORS=$((KERNEL_SIZE / 512))
PLACEHOLDER_STAGE2_SECTORS=8
PLACEHOLDER_KERNEL_LBA_START=$((1 + PLACEHOLDER_STAGE2_SECTORS))

echo "   Kernel size   : $KERNEL_SIZE bytes"
echo "   Kernel sectors: $KERNEL_SECTORS"

# ---------------------------------------------------------------------------
# Step 2: Build bootloader stages
# ---------------------------------------------------------------------------
display_step "Assembling bootloader stages"

nasm -f bin \
    -D BOOT_KERNEL_LBA_START=$PLACEHOLDER_KERNEL_LBA_START \
    -D BOOT_KERNEL_SECTOR_COUNT=$KERNEL_SECTORS \
    -D STAGE2_SECTOR_COUNT=$PLACEHOLDER_STAGE2_SECTORS \
    "$BOOT_DIR/stage2_simple.asm" -o "$BUILD_DIR/stage2.bin"

STAGE2_SIZE=$(stat -c%s "$BUILD_DIR/stage2.bin")
STAGE2_SECTORS=$(((STAGE2_SIZE + 511) / 512))
KERNEL_START_SECTOR=$((1 + STAGE2_SECTORS))

if [[ $STAGE2_SECTORS -ne $PLACEHOLDER_STAGE2_SECTORS || $KERNEL_START_SECTOR -ne $PLACEHOLDER_KERNEL_LBA_START ]]; then
    nasm -f bin \
        -D BOOT_KERNEL_LBA_START=$KERNEL_START_SECTOR \
        -D BOOT_KERNEL_SECTOR_COUNT=$KERNEL_SECTORS \
        -D STAGE2_SECTOR_COUNT=$STAGE2_SECTORS \
        "$BOOT_DIR/stage2_simple.asm" -o "$BUILD_DIR/stage2.bin"
    STAGE2_SIZE=$(stat -c%s "$BUILD_DIR/stage2.bin")
    STAGE2_SECTORS=$(((STAGE2_SIZE + 511) / 512))
fi

nasm -f bin \
    -D STAGE2_SECTOR_COUNT=$STAGE2_SECTORS \
    "$BOOT_DIR/boot_compact.asm" -o "$BUILD_DIR/boot.bin"

BOOT_SIZE=$(stat -c%s "$BUILD_DIR/boot.bin")
if [[ $BOOT_SIZE -ne 512 ]]; then
    echo "Error: boot sector must be exactly 512 bytes (got $BOOT_SIZE)" >&2
    exit 1
fi

echo "   Stage2 size   : $STAGE2_SIZE bytes"
echo "   Stage2 sectors: $STAGE2_SECTORS"
echo "   Kernel start  : sector $KERNEL_START_SECTOR"

# ---------------------------------------------------------------------------
# Step 3: Create floppy disk image
# ---------------------------------------------------------------------------
display_step "Creating bootable disk image"

IMG="$BUILD_DIR/ternarybit.img"

dd if=/dev/zero of="$IMG" bs=512 count=2880 status=none

dd if="$BUILD_DIR/boot.bin" of="$IMG" bs=512 count=1 conv=notrunc status=none

dd if="$BUILD_DIR/stage2.bin" of="$IMG" bs=512 seek=1 conv=notrunc status=none

dd if="$BUILD_DIR/kernel_padded.bin" of="$IMG" bs=512 seek=$((KERNEL_START_SECTOR - 1)) \
   conv=notrunc status=none

echo ""
echo "Build complete!"
echo "  Boot sector : $BOOT_SIZE bytes"
echo "  Stage 2     : $STAGE2_SIZE bytes"
echo "  Kernel      : $KERNEL_SIZE bytes (${KERNEL_SECTORS} sectors)"
echo "  Disk image  : $(stat -c%s "$IMG") bytes"

echo ""
echo "Run with:"
echo "  qemu-system-i386 -fda $IMG"
