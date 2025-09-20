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

gcc -m32 -ffreestanding -fno-stack-protector -fno-pie -fno-pic -nostdlib \
    -I"$TBOS_ROOT/include" \
    -c "$KERNEL_DIR/kernel.c" -o "$BUILD_DIR/kernel.o"

gcc -m32 -ffreestanding -fno-stack-protector -fno-pie -fno-pic -nostdlib \
    -I"$TBOS_ROOT/include" \
    -c "$KERNEL_DIR/support.c" -o "$BUILD_DIR/support.o"

gcc -m32 -ffreestanding -fno-pie -nostdlib -no-pie -Wl,--build-id=none -T "$KERNEL_DIR/linker.ld" \
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

    # Pair-Programming Note (Cascade):
    # - Fixed kernel placement off-by-one.
    # - Rationale: Stage2 computes BOOT_KERNEL_LBA_START=1+STAGE2_SECTORS (0-based LBA).
    #   Boot sector is at LBA0, stage2 occupies LBA1..LBA(1+S-1), so kernel must start at LBA(1+S).
    #   Using seek=KERNEL_START_SECTOR aligns the disk image with what stage2 reads via int 13h.
    # - Expected effect: prevent kernel overlap/corruption and avoid PM jump faults when jmp 0x10000.
dd if="$BUILD_DIR/kernel_padded.bin" of="$IMG" bs=512 seek=$((KERNEL_START_SECTOR)) \
   conv=notrunc status=none

# ---------------------------------------------------------------------------
# Step 4: Pixelize kernel using TBPX (Mode L) and verify roundtrip
# ---------------------------------------------------------------------------
display_step "Pixelizing kernel via TBPX (lossless image)"

TBPX_TOOL="tools/tbpx_codec.py"
if [[ -f "$TBPX_TOOL" ]]; then
    # Prefer PNG if Pillow is available, else use PPM (no deps)
    if python3 -c "import PIL" >/dev/null 2>&1; then
        TBPX_FORMAT="png"
    else
        TBPX_FORMAT="ppm"
    fi
    TBPX_OUT="$BUILD_DIR/kernel_tbpx.$TBPX_FORMAT"
    TBPX_ENC_OPTS=( --width 256 --repeat-header --format "$TBPX_FORMAT" )
    # Optional integrity/signing
    if [[ "${TBPX_SHA256:-}" == "1" ]]; then
        TBPX_ENC_OPTS+=( --sha256 )
    fi
    if [[ -n "${TBPX_SIGN_PRIV:-}" ]]; then
        TBPX_ENC_OPTS+=( --sign "$TBPX_SIGN_PRIV" )
    fi
    python3 "$TBPX_TOOL" encode "$BUILD_DIR/kernel.bin" "$TBPX_OUT" "${TBPX_ENC_OPTS[@]}" >/dev/null

    TBPX_DEC_OPTS=()
    if [[ -n "${TBPX_VERIFY_PUB:-}" ]]; then
        TBPX_DEC_OPTS+=( --verify "$TBPX_VERIFY_PUB" )
    fi
    python3 "$TBPX_TOOL" decode "$TBPX_OUT" "$BUILD_DIR/kernel_from_tbpx.bin" "${TBPX_DEC_OPTS[@]}" >/dev/null
    if cmp -s "$BUILD_DIR/kernel.bin" "$BUILD_DIR/kernel_from_tbpx.bin"; then
        echo "   TBPX roundtrip: OK (kernel.bin -> $TBPX_FORMAT -> bin)"
    else
        echo "Error: TBPX roundtrip failed (kernel mismatch after decode)" >&2
        exit 1
    fi
else
    echo "   Skipping TBPX step (tools/tbpx_codec.py not found)"
fi

echo ""
echo "Build complete!"
echo "  Boot sector : $BOOT_SIZE bytes"
echo "  Stage 2     : $STAGE2_SIZE bytes"
echo "  Kernel      : $KERNEL_SIZE bytes (${KERNEL_SECTORS} sectors)"
echo "  Disk image  : $(stat -c%s "$IMG") bytes"

echo ""
echo "Run with:"
echo "  qemu-system-i386 -fda $IMG"
