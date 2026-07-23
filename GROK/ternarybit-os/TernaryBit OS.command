#!/bin/bash
# Double-click this file on macOS to build (when needed) and boot TernaryBit OS.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE="$ROOT/build/tbos_universal.img"

if ! command -v qemu-system-i386 >/dev/null 2>&1; then
    echo "TernaryBit OS needs QEMU. Install it with: brew install qemu"
    read -r -p "Press Return to close..." _
    exit 1
fi

if [[ ! -f "$IMAGE" ]]; then
    "$ROOT/build_universal.sh"
fi

echo "Booting TernaryBit OS..."
echo "Close the QEMU window to stop the virtual machine."
exec qemu-system-i386 \
    -drive "file=$IMAGE,format=raw,if=ide" \
    -boot c \
    -m 512 \
    -name "TernaryBit OS"
