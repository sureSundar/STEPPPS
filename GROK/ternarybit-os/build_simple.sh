#!/bin/bash
# TernaryBit OS - Simple Build Script (macOS/bash 3.x compatible)
# Avoids bash 4+ features like associative arrays

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

BUILD_DIR="$SCRIPT_DIR/build"
mkdir -p "$BUILD_DIR"
mkdir -p "$BUILD_DIR/generated/tbos"

echo "======================================="
echo " TernaryBit OS - Simple Build"
echo "======================================="

# Detect platform
if [ "$(uname)" = "Darwin" ]; then
    PLATFORM="macOS"
    # Try to find cross-compiler
    if command -v x86_64-elf-gcc >/dev/null 2>&1; then
        CC="x86_64-elf-gcc"
        LD="x86_64-elf-ld"
        OBJCOPY="x86_64-elf-objcopy"
    else
        echo "ERROR: x86_64-elf-gcc not found"
        echo "Install with: brew install x86_64-elf-gcc x86_64-elf-binutils"
        exit 1
    fi
else
    PLATFORM="Linux"
    CC="gcc"
    LD="ld"
    OBJCOPY="objcopy"
fi

echo "Platform: $PLATFORM"
echo "Compiler: $CC"

# Check for nasm
if ! command -v nasm >/dev/null 2>&1; then
    echo "ERROR: nasm not found"
    echo "Install with: brew install nasm (macOS) or apt install nasm (Linux)"
    exit 1
fi

# Generate config header
cat > "$BUILD_DIR/generated/tbos/config.h" << 'EOF'
#pragma once
#define CONFIG_PROFILE "universal"
#define CONFIG_SHELL 1
#define CONFIG_FS 1
#define CONFIG_VFS 1
#define CONFIG_KEYBOARD 1
#define CONFIG_INTERRUPTS 1
EOF

echo ""
echo "==> Compiling kernel..."

# Compiler flags
CFLAGS="-m64 -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -fno-pie -nostdlib -Wall -O2"
CFLAGS="$CFLAGS -I$BUILD_DIR/generated -I$SCRIPT_DIR/include -I$SCRIPT_DIR/src"

# Add optional flags if supported
if $CC -mcmodel=kernel -E -x c /dev/null >/dev/null 2>&1; then
    CFLAGS="$CFLAGS -mcmodel=kernel"
fi
if $CC -mno-red-zone -E -x c /dev/null >/dev/null 2>&1; then
    CFLAGS="$CFLAGS -mno-red-zone"
fi

# Helper function to get file size
get_size() {
    if [ "$(uname)" = "Darwin" ]; then
        stat -f%z "$1"
    else
        stat -c%s "$1"
    fi
}

# Compile assembly
echo "  [asm] kernel/kernel_sacred.asm"
nasm -f elf64 kernel/kernel_sacred.asm -o "$BUILD_DIR/kernel_entry.o"

echo "  [asm] kernel/isr.asm"
nasm -f elf64 kernel/isr.asm -o "$BUILD_DIR/isr.o"

# Compile C files
compile_c() {
    local src="$1"
    local name
    name=$(echo "$src" | tr '/' '_' | sed 's/\.c$/.o/')
    echo "  [cc] $src"
    $CC $CFLAGS -c "$src" -o "$BUILD_DIR/$name"
}

compile_c kernel/hal_baremetal.c
compile_c kernel/tbds.c
compile_c kernel/kernel_main.c
compile_c kernel/interrupt.c
compile_c kernel/libc.c
compile_c kernel/tbos_baremetal_stubs.c
compile_c kernel/fs/ramfs.c
compile_c kernel/fs/vfs.c
compile_c kernel/fs/ucfs_driver.c
compile_c src/core/filesystem/blockdev.c
compile_c src/core/filesystem/ucfs_codec.c
compile_c src/core/filesystem/ucfs_overlay.c
compile_c src/core/filesystem/ucfs_config.c

# Compile drivers
for f in drivers/vga/*.c; do
    [ -f "$f" ] && compile_c "$f"
done
for f in drivers/timer/*.c; do
    [ -f "$f" ] && compile_c "$f"
done
for f in drivers/keyboard/*.c; do
    [ -f "$f" ] && compile_c "$f"
done

# Compile shell
for f in shell/*.c; do
    [ -f "$f" ] && compile_c "$f"
done

echo ""
echo "==> Linking kernel..."
$LD -m elf_x86_64 -T kernel/linker_sacred.ld -o "$BUILD_DIR/kernel.elf" "$BUILD_DIR"/*.o

echo "==> Creating binary..."
$OBJCOPY -O binary "$BUILD_DIR/kernel.elf" "$BUILD_DIR/kernel.bin"
dd if="$BUILD_DIR/kernel.bin" of="$BUILD_DIR/kernel_padded.bin" bs=512 conv=sync 2>/dev/null

KERNEL_SIZE=$(get_size "$BUILD_DIR/kernel_padded.bin")
KERNEL_SECTORS=$((KERNEL_SIZE / 512))
echo "  Kernel: $KERNEL_SIZE bytes ($KERNEL_SECTORS sectors)"

echo ""
echo "==> Assembling bootloader..."
KERNEL_START=10

nasm -I boot/ -f bin \
    -D KERNEL_START_SECTOR=$KERNEL_START \
    -D KERNEL_SECTORS=$KERNEL_SECTORS \
    boot/stage2_long_mode.asm -o "$BUILD_DIR/stage2.bin"

STAGE2_SIZE=$(get_size "$BUILD_DIR/stage2.bin")
STAGE2_SECTORS=$(((STAGE2_SIZE + 511) / 512))

nasm -I boot/ -f bin \
    -D STAGE2_SECTOR_COUNT=$STAGE2_SECTORS \
    boot/tbos_sacred.asm -o "$BUILD_DIR/boot.bin"

BOOT_SIZE=$(get_size "$BUILD_DIR/boot.bin")
if [ "$BOOT_SIZE" -ne 512 ]; then
    echo "ERROR: boot.bin must be 512 bytes (got $BOOT_SIZE)"
    exit 1
fi

echo ""
echo "==> Creating disk image..."
dd if=/dev/zero of="$BUILD_DIR/tbos_universal.img" bs=512 count=2880 2>/dev/null
dd if="$BUILD_DIR/boot.bin" of="$BUILD_DIR/tbos_universal.img" conv=notrunc 2>/dev/null
dd if="$BUILD_DIR/stage2.bin" of="$BUILD_DIR/tbos_universal.img" bs=512 seek=1 conv=notrunc 2>/dev/null
dd if="$BUILD_DIR/kernel_padded.bin" of="$BUILD_DIR/tbos_universal.img" bs=512 seek=$KERNEL_START conv=notrunc 2>/dev/null

echo ""
echo "======================================="
echo " Build Complete!"
echo "======================================="
echo "  Image: $BUILD_DIR/tbos_universal.img"
echo ""
echo "Run with:"
echo "  qemu-system-x86_64 -drive file=$BUILD_DIR/tbos_universal.img,format=raw -m 512"
