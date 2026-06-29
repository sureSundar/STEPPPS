#!/bin/bash
# V4.0 Universal Boot Orchestrator
# Traceability: v4.0/ROADMAP.md V4-005
#
# Builds all V4.0 targets:
# 1. BIOS bare-metal image (artifacts/baremetal/tbos-bios.img)
# 2. Hosted Linux binary (artifacts/hosted/tbos-linux)
#
# BCB (Boot Capability Block) is now fully implemented:
# - Stage 2 writes BCB at 0x6000 (V4-002 COMPLETE)
# - Kernel reads BCB and selects HAL (V4-003 COMPLETE)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
ARTIFACT_DIR="$SCRIPT_DIR/artifacts"
BIOS_DIR="$ARTIFACT_DIR/baremetal"
HOSTED_DIR="$ARTIFACT_DIR/hosted"

# Cross-platform file size function
get_file_size() {
    if [[ "$(uname)" == "Darwin" ]]; then
        stat -f%z "$1"
    else
        stat -c%s "$1"
    fi
}

mkdir -p "$BUILD_DIR" "$BIOS_DIR" "$HOSTED_DIR"

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║   TernaryBit OS V4.0 - Universal Boot Builder                ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# ─────────────────────────────────────────────────────────────────────
# Build BIOS target
# ─────────────────────────────────────────────────────────────────────
echo "==> [V4] Building BIOS bare-metal target..."
"$SCRIPT_DIR/build_universal.sh" "$@"

BOOT_BIN="$BUILD_DIR/boot.bin"
STAGE2_BIN="$BUILD_DIR/stage2.bin"
KERNEL_BIN="$BUILD_DIR/kernel_padded.bin"

if [[ ! -f "$BOOT_BIN" || ! -f "$STAGE2_BIN" || ! -f "$KERNEL_BIN" ]]; then
    echo "error: expected artifacts missing under $BUILD_DIR" >&2
    exit 1
fi

stage2_bytes=$(get_file_size "$STAGE2_BIN")
stage2_sectors=$(((stage2_bytes + 511) / 512))

# Kernel start is calculated dynamically by build_universal.sh:
# kernel_start = 1 + stage2_sectors + 2 (padding)
# We must use the same formula here
kernel_start_sector=$((1 + stage2_sectors + 2))

kernel_bytes=$(get_file_size "$KERNEL_BIN")
kernel_sectors=$(((kernel_bytes + 511) / 512))
total_sectors=$((kernel_start_sector + kernel_sectors))

echo "==> [V4] Assembling BIOS floppy image..."
echo "    Stage 2: $stage2_sectors sectors (LBA 1-$((stage2_sectors)))"
echo "    Kernel: $kernel_sectors sectors ($kernel_bytes bytes) at LBA $kernel_start_sector"
echo "    Total: $total_sectors sectors required"

# Check if kernel fits on floppy (2880 sectors = 1.44MB)
if [[ $total_sectors -gt 2880 ]]; then
    echo "WARNING: Image exceeds 1.44MB floppy capacity ($total_sectors > 2880 sectors)"
    echo "         Creating larger disk image instead..."
    floppy_sectors=$((total_sectors + 100))  # Add some padding
else
    floppy_sectors=2880
fi

bios_img="$BUILD_DIR/ternarybit.img"

# Create disk image
dd if=/dev/zero of="$bios_img" bs=512 count=$floppy_sectors status=none 2>/dev/null || \
    truncate -s $((floppy_sectors * 512)) "$bios_img"

dd if="$BOOT_BIN" of="$bios_img" conv=notrunc status=none
dd if="$STAGE2_BIN" of="$bios_img" bs=512 seek=1 conv=notrunc status=none
dd if="$KERNEL_BIN" of="$bios_img" bs=512 seek="$kernel_start_sector" conv=notrunc status=none
cp "$bios_img" "$BIOS_DIR/tbos-bios.img"

echo "    [OK] BIOS image: $BIOS_DIR/tbos-bios.img"

# ─────────────────────────────────────────────────────────────────────
# Build Hosted Linux target
# ─────────────────────────────────────────────────────────────────────
echo ""
echo "==> [V4] Building Hosted Linux target..."
if [[ -f "$SCRIPT_DIR/hosted/build_hosted_linux.sh" ]]; then
    "$SCRIPT_DIR/hosted/build_hosted_linux.sh" || {
        echo "    [WARN] Hosted Linux build failed (may need gcc)"
    }
    if [[ -f "$HOSTED_DIR/tbos-linux" ]]; then
        echo "    [OK] Hosted Linux: $HOSTED_DIR/tbos-linux"
    fi
else
    echo "    [SKIP] Hosted Linux build script not found"
fi

# ─────────────────────────────────────────────────────────────────────
# Summary
# ─────────────────────────────────────────────────────────────────────
echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║   V4.0 Build Complete                                        ║"
echo "╠══════════════════════════════════════════════════════════════╣"
echo "║   Artifacts:                                                 ║"
echo "║     BIOS:   $BIOS_DIR/tbos-bios.img"
if [[ -f "$HOSTED_DIR/tbos-linux" ]]; then
echo "║     Linux:  $HOSTED_DIR/tbos-linux"
fi
echo "╠══════════════════════════════════════════════════════════════╣"
echo "║   Test Commands:                                             ║"
echo "║     qemu-system-i386 -drive file=$BIOS_DIR/tbos-bios.img,format=raw,if=floppy"
if [[ -f "$HOSTED_DIR/tbos-linux" ]]; then
echo "║     $HOSTED_DIR/tbos-linux"
fi
echo "╚══════════════════════════════════════════════════════════════╝"
