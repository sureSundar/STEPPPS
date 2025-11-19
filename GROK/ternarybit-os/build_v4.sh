#!/bin/bash
# V4.0 Universal Boot orchestrator (bootstrap scaffolding)
# 1. Runs build_universal.sh (full kernel with shell/fs/net)
# 2. Repackages the BIOS floppy image as build/ternarybit.img
# 3. Lays out artifact directories for future targets

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
ARTIFACT_DIR="$SCRIPT_DIR/artifacts"
BIOS_DIR="$ARTIFACT_DIR/baremetal"
HOSTED_DIR="$ARTIFACT_DIR/hosted"
BINARY_NAME="tbos_universal.img"

mkdir -p "$BUILD_DIR" "$BIOS_DIR" "$HOSTED_DIR"

echo "==> [V4] Building universal payload"
"$SCRIPT_DIR/build_universal.sh" "$@"

BOOT_BIN="$BUILD_DIR/boot.bin"
STAGE2_BIN="$BUILD_DIR/stage2.bin"
KERNEL_BIN="$BUILD_DIR/kernel_padded.bin"

if [[ ! -f "$BOOT_BIN" || ! -f "$STAGE2_BIN" || ! -f "$KERNEL_BIN" ]]; then
    echo "error: expected artifacts missing under $BUILD_DIR" >&2
    exit 1
fi

stage2_bytes=$(stat -c%s "$STAGE2_BIN")
stage2_sectors=$(((stage2_bytes + 511) / 512))
kernel_start_sector=$((1 + stage2_sectors))

echo "==> [V4] Repacking BIOS floppy (stage2 sectors: $stage2_sectors, kernel LBA: $kernel_start_sector)"
bios_img="$BUILD_DIR/ternarybit.img"
truncate -s $((2880 * 512)) "$bios_img"
dd if="$BOOT_BIN" of="$bios_img" conv=notrunc status=none
dd if="$STAGE2_BIN" of="$bios_img" bs=512 seek=1 conv=notrunc status=none
dd if="$KERNEL_BIN" of="$bios_img" bs=512 seek="$kernel_start_sector" conv=notrunc status=none
cp "$bios_img" "$BIOS_DIR/tbos-bios.img"

echo "==> [V4] Placeholder hosted artifact"
cp "$BUILD_DIR/$BINARY_NAME" "$HOSTED_DIR/tbos-hosted.bin"
cat > "$HOSTED_DIR/README.txt" <<EOF
TBOS v4 hosted placeholder
- TODO: replace with ELF/PE hosted stubs that populate the Boot Capability Block.
- Current content is a raw floppy image; not usable as a hosted binary yet.
EOF

echo ""
echo "Artifacts:"
echo "  BIOS image   : $BIOS_DIR/tbos-bios.img"
echo "  Hosted proto : $HOSTED_DIR/tbos-hosted.bin (placeholder)"
echo ""
echo "Next actions:"
echo "  - implement Boot Capability Block (BCB) writer/reader"
echo "  - replace hosted placeholder with real stub executables"
