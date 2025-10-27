# Interactive Shell Input Fix

## THE PROBLEM

The shell displays but doesn't accept input because:

**Issue**: Running with `-nographic` mode sends output to serial console, but keyboard driver reads from PS/2 hardware port (0x60).

**Result**: You see the output, but keypresses go to serial, not to the PS/2 port the kernel reads from.

## THE SOLUTION

### Option 1: Use Graphical Mode (Recommended for Local)
```bash
# This opens a QEMU window with real VGA + PS/2 keyboard
qemu-system-i386 -drive format=raw,file=build/tbos.img -m 128M

# Keyboard input will work!
# Click in window and type commands
```

### Option 2: Use Serial Console (Better for SSH/Remote)
The kernel needs to support serial input. We need to modify the shell to read from serial port instead of PS/2 keyboard.

## QUICK FIX - Try Graphical Mode Now

```bash
# Build if needed
./build.sh

# Run with graphical output (not -nographic)
qemu-system-i386 -drive format=raw,file=build/tbos.img -m 128M
```

**Then**:
1. QEMU window will open
2. Wait for `TBOS>` prompt
3. Click in the QEMU window
4. Type commands - they will work!

## Why This Happens

```
YOUR KEYBOARD
     ↓
  Terminal/SSH
     ↓
  QEMU (nographic mode)
     ↓
  Serial Port → /dev/null (kernel doesn't read serial yet!)

Meanwhile:
  Kernel Shell → Reads PS/2 Port (0x60) → Nothing there!
```

**In Graphical Mode:**
```
YOUR KEYBOARD
     ↓
  QEMU Window
     ↓
  Emulated PS/2 Keyboard (port 0x60)
     ↓
  Kernel reads port 0x60 → Gets your input!
```

## Long-term Fix (For SSH/Serial Support)

Modify `kernel/shell.c` to support serial input:

```c
// Add serial port reading
#define SERIAL_PORT 0x3F8

static inline uint8_t serial_read_char(void) {
    // Check if data available
    if (!(inb(SERIAL_PORT + 5) & 0x01)) {
        return 0;
    }
    return inb(SERIAL_PORT);
}

// In shell_loop(), change:
// OLD:
uint8_t ch = keyboard_read_char();

// NEW:
uint8_t ch = keyboard_read_char();
if (ch == 0) ch = serial_read_char();  // Also check serial
```

