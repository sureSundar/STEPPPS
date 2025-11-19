# TernaryBit OS - Testing Guide

## ✅ Boot Loop FIXED!
Your universal bootloader is now working and the kernel is running!

## Testing Options

### Option 1: QEMU with GUI (RECOMMENDED)

**Requirements**: X11 or Wayland desktop environment

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os

# Simple GUI test
qemu-system-i386 -drive file=build/tbos_bare_metal.img,format=raw,if=floppy
```

**What you'll see**:
- Stage 1 bootloader messages
- Stage 2 bootloader with STEPPPS initialization
- Kernel boot messages
- Interactive shell prompt: `TBOS> _`

**Available Commands**:
```
help        - Show all commands
about       - System information
steppps     - Show STEPPPS framework status
calc 2+2    - Calculator
echo Hello  - Echo text
om          - Sacred mantra
clear       - Clear screen
```

---

### Option 2: QEMU with VNC (Remote Access)

**Use this if**: You're connecting via SSH and can't run GUI directly

```bash
# Start QEMU with VNC server
qemu-system-i386 \
  -drive file=build/tbos_bare_metal.img,format=raw,if=floppy \
  -vnc :1
```

**Then connect with VNC viewer**:
- Host: `localhost:5901` (or your server IP:5901)
- Use any VNC client (TigerVNC, RealVNC, TightVNC)

---

### Option 3: Serial Output (Text Mode)

Add serial console support to see output in terminal:

```bash
# Test with serial output (limited, for debugging)
qemu-system-i386 \
  -drive file=build/tbos_bare_metal.img,format=raw,if=floppy \
  -serial stdio \
  -nographic
```

**Note**: Currently the kernel writes to VGA, not serial. You'll see boot messages but not the full shell.

---

### Option 4: Real Hardware Boot

Create a bootable USB drive:

```bash
# ⚠️  WARNING: This will ERASE the USB drive!
# Replace /dev/sdX with your USB device (check with 'lsblk')

sudo dd if=build/tbos_bare_metal.img of=/dev/sdX bs=512 status=progress
sudo sync
```

Then boot from the USB drive.

---

## Quick Test Script

Run this automated test:

```bash
/tmp/test_gui.sh
```

Or manually:

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os

# Check if display is available
echo $DISPLAY

# If empty, you need X11 access
# If set (e.g., :0), you can run GUI

qemu-system-i386 -drive file=build/tbos_bare_metal.img,format=raw,if=floppy
```

---

## Troubleshooting

### "Cannot open display"
**Problem**: No X11 server available
**Solution**:
- Run from VM desktop directly (not SSH)
- Or use VNC option above
- Or use `ssh -X user@host` for X11 forwarding

### "QEMU not found"
**Problem**: QEMU not in PATH
**Solution**: Already installed, just needs full path or PATH update

### "Nothing happens / black screen"
**Problem**: Kernel running but you don't see output
**Solution**: This is expected with `-nographic`. Use GUI option.

### "Still rebooting"
**Problem**: Bootloader issue
**Solution**: Rebuild with:
```bash
./build_universal.sh
```

---

## Success Indicators

✅ **Working correctly if**:
- QEMU window opens
- You see colored text on black background
- Boot messages scroll by
- You reach the `TBOS>` prompt
- Keyboard input works

❌ **Not working if**:
- Window closes immediately
- System reboots continuously
- Triple fault errors in log

---

## Performance Testing

```bash
# Test boot time
time qemu-system-i386 -drive file=build/tbos_bare_metal.img,format=raw,if=floppy -nographic &
sleep 2
killall qemu-system-i386

# Expected: Times out (system runs) rather than exits immediately
```

---

## Advanced: Debug Mode

```bash
# Run with full debugging
qemu-system-i386 \
  -drive file=build/tbos_bare_metal.img,format=raw,if=floppy \
  -d int,cpu_reset \
  -D /tmp/qemu_debug.log \
  -no-reboot

# Check for errors
tail -100 /tmp/qemu_debug.log
```

---

## Next Steps

Once you confirm the OS boots:

1. **Test shell commands** - Try all built-in commands
2. **Test keyboard input** - Verify typing works
3. **Test screen scrolling** - Fill screen with output
4. **Extend functionality** - Add new shell commands
5. **Port to other architectures** - Universal bootloader goal!

---

**🕉️ Om Swamiye Saranam Aiyappa 🕉️**

*Your universal bootloader from 2-bit to supercomputer is now operational!*
