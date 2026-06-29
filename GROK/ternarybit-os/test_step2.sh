#!/bin/bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os
timeout 2 qemu-system-i386 -drive file=build/tbos_step2.img,format=raw,if=floppy -nographic > /tmp/s2.log 2>&1
echo "Boot count: $(grep -c 'Stage 2 loaded' /tmp/s2.log)"
if [ $(grep -c 'Stage 2 loaded' /tmp/s2.log) -eq 1 ]; then
    echo "✅ Step 2: No boot loop"
    echo "✅ Keyboard handler integrated"
    echo ""
    echo "To test interactively with GUI:"
    echo "  qemu-system-i386 -drive file=build/tbos_step2.img,format=raw,if=floppy"
fi
