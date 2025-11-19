#!/usr/bin/env bash
# Build a bare-metal bootable image: Stage1 (MBR) -> Stage2 (protected mode) -> shell

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD="$ROOT/build/universal"

STAGE2_SECTORS=8          # must match boot/universal_stage1.asm
STAGE2_START_SECTOR=2     # CHS sector number (1-based)

mkdir -p "$BUILD"

banner() {
    echo "==> $*"
}

banner "Compiling universal kernel"
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib \
    -c "$ROOT/kernel/universal.c" -o "$BUILD/kernel.o"
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib \
    -c "$ROOT/kernel/universal_isr.S" -o "$BUILD/universal_isr.o"

ld -m elf_i386 -T "$ROOT/kernel/universal_linker.ld" \
    -o "$BUILD/kernel.elf" "$BUILD/kernel.o" "$BUILD/universal_isr.o"

objcopy -O binary "$BUILD/kernel.elf" "$BUILD/kernel.bin"
KERNEL_SIZE=$(stat -c%s "$BUILD/kernel.bin")
KERNEL_SECTORS=$(( (KERNEL_SIZE + 511) / 512 ))
KERNEL_LBA=$((1 + STAGE2_SECTORS)) # boot sector + stage2

echo "    Kernel size: $KERNEL_SIZE bytes ($KERNEL_SECTORS sectors)"

banner "Assembling boot stages"
nasm -f bin "$ROOT/boot/universal_stage1.asm" \
    -DSTAGE2_SECTORS=$STAGE2_SECTORS \
    -DSTAGE2_START_SECTOR=$STAGE2_START_SECTOR \
    -o "$BUILD/stage1.bin"

nasm -f bin "$ROOT/boot/universal_stage2.asm" \
    -DSTAGE2_SECTORS=$STAGE2_SECTORS \
    -DKERNEL_START_LBA=$KERNEL_LBA \
    -DKERNEL_SECTORS=$KERNEL_SECTORS \
    -o "$BUILD/stage2.bin"

banner "Creating disk image"
IMG="$BUILD/tbos_universal.img"
TOTAL_SECTORS=$(( KERNEL_LBA + KERNEL_SECTORS + 32 ))
dd if=/dev/zero of="$IMG" bs=512 count=$TOTAL_SECTORS status=none

dd if="$BUILD/stage1.bin" of="$IMG" conv=notrunc status=none
dd if="$BUILD/stage2.bin" of="$IMG" conv=notrunc seek=1 status=none
dd if="$BUILD/kernel.bin" of="$IMG" conv=notrunc seek=$KERNEL_LBA status=none

echo ""
echo "Image ready: $IMG"
echo "Boot with: qemu-system-i386 -drive format=raw,file=$IMG -serial stdio"
echo "On real hardware: write the image to a USB drive (e.g., with 'dd') and boot in BIOS mode."
