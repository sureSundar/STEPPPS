#!/bin/bash
# Rebuilds build/ternarybit.img with the full universal kernel (shell included)
# by reusing the artifacts produced by build_universal.sh.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
UNIVERSAL_SCRIPT="$SCRIPT_DIR/build_universal.sh"
BUILD_DIR="$SCRIPT_DIR/build"
OUTPUT_IMG="$BUILD_DIR/ternarybit.img"
FLOPPY_SECTORS=2880        # 1.44MB
SECTOR_BYTES=512

echo "[1/4] Building universal image (kernel + drivers + shell)"
"$UNIVERSAL_SCRIPT" "$@"

boot_bin="$BUILD_DIR/boot.bin"
stage2_bin="$BUILD_DIR/stage2.bin"
kernel_bin="$BUILD_DIR/kernel_padded.bin"

[[ -f "$boot_bin" && -f "$stage2_bin" && -f "$kernel_bin" ]] || {
    echo "error: missing build artifacts in $BUILD_DIR" >&2
    exit 1
}

stage2_sectors=$(( ( $(stat -c%s "$stage2_bin") + SECTOR_BYTES - 1 ) / SECTOR_BYTES ))
kernel_start_sector=$((1 + stage2_sectors))

if (( kernel_start_sector * SECTOR_BYTES >= FLOPPY_SECTORS * SECTOR_BYTES )); then
    echo "error: kernel does not fit inside 1.44MB floppy image" >&2
    exit 1
fi

echo "[2/4] Creating 1.44MB floppy image at $OUTPUT_IMG"
truncate -s $((FLOPPY_SECTORS * SECTOR_BYTES)) "$OUTPUT_IMG"

echo "[3/4] Writing bootloader and stage2"
dd if="$boot_bin" of="$OUTPUT_IMG" conv=notrunc status=none
dd if="$stage2_bin" of="$OUTPUT_IMG" bs=$SECTOR_BYTES seek=1 conv=notrunc status=none

echo "[4/4] Writing kernel (start sector: $kernel_start_sector)"
dd if="$kernel_bin" of="$OUTPUT_IMG" bs=$SECTOR_BYTES seek="$kernel_start_sector" conv=notrunc status=none

echo ""
echo "✅ build/ternarybit.img refreshed with the universal (shell-enabled) kernel"
echo "   - Total size : $((FLOPPY_SECTORS * SECTOR_BYTES)) bytes"
echo "   - Stage2 LBA : 1"
echo "   - Kernel LBA : $kernel_start_sector"
echo ""
echo "Boot with: qemu-system-i386 -fda build/ternarybit.img -m 512 -boot order=a -net none"
