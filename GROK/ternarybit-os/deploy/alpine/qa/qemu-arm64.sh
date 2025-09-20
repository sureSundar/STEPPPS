#!/usr/bin/env bash
set -euo pipefail

KERNEL=""
INITRD=""
EXTRA=""

usage() {
  echo "Usage: $0 --kernel /path/to/Image.gz --initrd ../out/aarch64/initramfs.cpio.gz [--extra 'console=...']" >&2
  exit 1
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --kernel) KERNEL="$2"; shift 2 ;;
    --initrd) INITRD="$2"; shift 2 ;;
    --extra) EXTRA="$2"; shift 2 ;;
    *) echo "Unknown arg: $1" >&2; usage ;;
  esac
done

[[ -n "$KERNEL" && -n "$INITRD" ]] || usage

CMDLINE_FILE="$(dirname "$0")/../kernel-cmdline-arm64.txt"
if [[ -f "$CMDLINE_FILE" ]]; then
  KCMDLINE="$(cat "$CMDLINE_FILE")"
else
  KCMDLINE="console=ttyAMA0,115200 console=tty0 root=/dev/ram0 rdinit=/sbin/steppps-init"
fi

if [[ -n "$EXTRA" ]]; then
  KCMDLINE="$KCMDLINE $EXTRA"
fi

exec qemu-system-aarch64 \
  -M virt \
  -cpu cortex-a57 \
  -m 1024 \
  -smp 2 \
  -kernel "$KERNEL" \
  -initrd "$INITRD" \
  -append "$KCMDLINE" \
  -nographic -serial mon:stdio \
  -net nic -net user
