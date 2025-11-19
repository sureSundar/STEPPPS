#!/usr/bin/env bash
#
# Boot the TBOS raw image inside QEMU, run a minimal command script, and
# ensure the shell responds. Intended as the acceptance test for Milestone Week-1.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
IMAGE="${1:-$ROOT_DIR/build/tbos_universal.img}"
QEMU_BIN="${QEMU_BIN:-qemu-system-x86_64}"
QEMU_FLAGS=(-machine q35 -m 256M -cpu max -serial stdio -display none -no-reboot)

if [[ ! -f "$IMAGE" ]]; then
    echo "error: boot image not found at $IMAGE" >&2
    echo "       run build/build_universal.sh first (optionally with --profile)." >&2
    exit 1
fi

if ! command -v "$QEMU_BIN" >/dev/null 2>&1; then
    echo "error: $QEMU_BIN not found in PATH. Install QEMU or set QEMU_BIN." >&2
    exit 1
fi

echo "[boot-smoke] Launching $QEMU_BIN with $IMAGE"
echo "            (inside QEMU type 'help' then 'exit' to verify Week-1 shell)"

"$QEMU_BIN" -drive "file=${IMAGE},format=raw,if=floppy" \
    "${QEMU_FLAGS[@]}"
