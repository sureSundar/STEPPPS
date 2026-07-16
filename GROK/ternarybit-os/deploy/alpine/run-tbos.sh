#!/bin/bash
DIR="$(cd "$(dirname "$0")" && pwd)"
qemu-system-x86_64 \
  -kernel "$DIR/out/x86_64/vmlinuz-virt" \
  -initrd "$DIR/out/x86_64/initramfs-quick.cpio.gz" \
  -append "console=ttyS0,115200 root=/dev/ram0 rdinit=/sbin/steppps-init quiet" \
  -m 256M \
  -nographic
