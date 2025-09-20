#!/usr/bin/env bash
set -euo pipefail

KERNEL=""
INITRD=""
EXTRA=""

usage() {
  echo "Usage: $0 --kernel /path/to/bzImage --initrd ../out/x86_64/initramfs.cpio.gz [--extra 'console=...']" >&2
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

CMDLINE_FILE="$(dirname "$0")/../kernel-cmdline-x86_64.txt"
if [[ -f "$CMDLINE_FILE" ]]; then
  KCMDLINE="$(cat "$CMDLINE_FILE")"
else
  KCMDLINE="console=ttyS0,115200n8 console=tty0 root=/dev/ram0 rdinit=/sbin/steppps-init"
fi

if [[ -n "$EXTRA" ]]; then
  KCMDLINE="$KCMDLINE $EXTRA"
fi

exec qemu-system-x86_64 \
  -m 1024 \
  -cpu max \
  -smp 2 \
  -kernel "$KERNEL" \
  -initrd "$INITRD" \
  -append "$KCMDLINE" \
  -nographic -serial mon:stdio \
  -net nic -net user
