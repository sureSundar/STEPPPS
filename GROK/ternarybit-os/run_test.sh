#!/bin/bash

# Build the OS
./scripts/build.sh

# Run QEMU with debug output
qemu-system-i386 \
    -fda build/om.img \
    -serial stdio \
    -d int,cpu_reset \
    -no-reboot \
    -D qemu_debug.log \
    -nographic \
    -monitor none

echo "QEMU log saved to qemu_debug.log"
