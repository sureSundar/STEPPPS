#!/bin/bash
(sleep 3; echo "xp /160bx 0xb8000"; sleep 1; echo "quit") | qemu-system-i386 -drive format=raw,file=build/ternarybit.img -nographic -monitor stdio 2>/dev/null | grep "0x000b8" | head -10
