# TernaryBit OS - Interactive Shell Demo Guide
## 🕉️ Complete Shell Testing & Demonstration 🕉️

**Created**: 2025-10-19
**Status**: Ready for Interactive Testing

---

## 🚀 QUICK START

### Start TBOS Shell (Recommended Method)
```bash
# Text console mode (works via SSH)
qemu-system-i386 -drive format=raw,file=build/tbos.img -m 128M -nographic

# Exit: Press Ctrl-A then X
```

### Alternative Methods
```bash
# Graphical window (requires X11)
qemu-system-i386 -drive format=raw,file=build/tbos.img -m 128M

# Using the run script
./run_tbos.sh

# With more memory
qemu-system-i386 -drive format=raw,file=build/tbos.img -m 256M -nographic
```

---

## 📋 WHAT YOU'LL SEE

### 1. Boot Sequence (First ~3 seconds)
```
SeaBIOS (version 1.13.0-1ubuntu1.1)
Booting from Hard Disk...

=====================================
     TBOS x STEPPPS Framework
   Swamiye Saranam Aiyappa
=====================================
[BOOT] Initializing Sacred System...
[BOOT] Stage 2 loaded successfully
[BOOT] Transferring control...

TernaryBit OS - Linux Integration Active
Initializing STEPPPS Framework for Linux...
  [SPACE] Hardware detection...
  [TIME] Timing systems...
  [EVENT] Event handling...
  [PSYCHOLOGY] User interface...
  [PIXEL] Display systems...
  [PROMPT] Command line...
  [SCRIPT] Automation systems...

[SHELL] Initializing interactive shell...
[SHELL] Shell ready
```

### 2. Shell Prompt
```
=== TernaryBit OS Interactive Shell ===
Type 'help' for available commands

TBOS> _
```

---

## 🎯 AVAILABLE SHELL COMMANDS

### Basic Commands

#### 1. **help** - Show all commands
```
TBOS> help

=== TernaryBit OS Shell Commands ===
help       - Show this help message
clear/cls  - Clear the screen
ps         - List running processes
mem        - Display memory information
steppps    - Show STEPPPS framework status
calc <exp> - Simple calculator (e.g., calc 5+3)
echo <txt> - Print text to screen
time       - Display system uptime
test       - Run system tests
about      - About TernaryBit OS
reboot     - Restart the system
```

#### 2. **about** - System information
```
TBOS> about

=== TernaryBit OS ===
Version: 1.0 (Development)
Architecture: x86_64
Framework: STEPPPS (7 Dimensions)

An educational operating system
demonstrating OS concepts.

For details: README_HONEST.md
```

#### 3. **steppps** - Framework status
```
TBOS> steppps

=== STEPPPS Framework Status ===
[SPACE]      Memory Management    - PARTIAL
[TIME]       Process Scheduling   - IN PROGRESS
[EVENT]      Interrupt System     - FUNCTIONAL
[PSYCHOLOGY] AI/ML Framework      - PLANNED
[PIXEL]      Graphics System      - PLANNED
[PROMPT]     Natural Language     - PLANNED
[SCRIPT]     Scripting Engine     - PLANNED

See GET_WELL_PLAN.md for roadmap
```

---

### System Information Commands

#### 4. **ps** - Process list
```
TBOS> ps

=== Process List ===
PID  STATE    NAME
---  -------  ------------
0    RUNNING  kernel
1    RUNNING  shell
(Process manager implementation in progress)
```

#### 5. **mem** - Memory status
```
TBOS> mem

=== Memory Status ===
Total Memory: 64 MB
Used Memory:  12 MB
Free Memory:  52 MB
(Virtual memory manager in development)
```

#### 6. **time** - System uptime
```
TBOS> time

=== System Time ===
Uptime: 0:00:00
(Timer driver implementation in progress)
```

---

### Utility Commands

#### 7. **calc** - Calculator
```
TBOS> calc 5+3
Result: 8

TBOS> calc 42-8
Result: 34

TBOS> calc 6*7
Result: 42

TBOS> calc 100/4
Result: 25

TBOS> calc 17%5
Result: 2
```

**Supported Operators**: `+`, `-`, `*`, `/`, `%`

#### 8. **echo** - Print text
```
TBOS> echo Hello TBOS!
Hello TBOS!

TBOS> echo Swamiye Saranam Aiyappa
Swamiye Saranam Aiyappa
```

#### 9. **test** - Run system tests
```
TBOS> test

=== Running System Tests ===
[PASS] Shell: Command parsing
[PASS] Memory: Basic allocation
[PASS] Interrupt: IDT initialized
[SKIP] Network: Not implemented
[SKIP] Graphics: Not implemented

Some tests skipped - see GET_WELL_PLAN.md
```

---

### Screen Control

#### 10. **clear** or **cls** - Clear screen
```
TBOS> clear

TernaryBit OS v1.0 - Interactive Shell
Type 'help' for available commands
```

---

### Sacred Command

#### 11. **om** - Sacred mantra
```
TBOS> om

Om Swamiye Saranam Aiyappa
```

---

### System Control

#### 12. **reboot** - Restart system
```
TBOS> reboot

Rebooting system...
[System restarts]
```

---

## 🎓 DEMO SCRIPT - COMPLETE WALKTHROUGH

Here's a suggested demo sequence to showcase all features:

```bash
# Step 1: Start TBOS
qemu-system-i386 -drive format=raw,file=build/tbos.img -m 128M -nographic

# Wait for boot to complete and shell prompt appears...

# Step 2: Get help
TBOS> help

# Step 3: Show system info
TBOS> about

# Step 4: Check STEPPPS status
TBOS> steppps

# Step 5: System information
TBOS> ps
TBOS> mem
TBOS> time

# Step 6: Calculator demo
TBOS> calc 42+8
TBOS> calc 100/4
TBOS> calc 7*6

# Step 7: Echo demo
TBOS> echo TernaryBit OS is working!

# Step 8: Run tests
TBOS> test

# Step 9: Sacred mantra
TBOS> om

# Step 10: Clear screen
TBOS> clear

# Step 11: Exit QEMU
Press: Ctrl-A then X
```

---

## 🔧 KEYBOARD SHORTCUTS

### In QEMU (nographic mode)
- **Ctrl-A then X**: Exit QEMU
- **Ctrl-A then C**: Switch to QEMU monitor
- **Ctrl-A then H**: Show help
- **Ctrl-A then S**: Send break signal

### In Shell
- **Enter**: Execute command
- **Backspace**: Delete character
- **Regular typing**: Enter commands

### Currently NOT Supported
- ❌ Arrow keys (up/down for history)
- ❌ Tab completion
- ❌ Ctrl-C (interrupt)
- ❌ Ctrl-D (EOF)

---

## 🐛 TROUBLESHOOTING

### Shell Not Appearing
**Symptoms**: Boot completes but no prompt
**Solution**:
1. Wait 5-10 seconds (initialization may be slow)
2. Try pressing Enter
3. Check if QEMU has -nographic flag
4. Rebuild: `./build.sh`

### Commands Not Working
**Symptoms**: Typing has no effect
**Solution**:
1. Check keyboard input is going to QEMU window
2. Try clicking in QEMU window (if graphical)
3. Verify you're not in QEMU monitor (Ctrl-A then C to toggle)

### Screen Garbled
**Symptoms**: Strange characters
**Solution**:
```
TBOS> clear
```

### Cursor Not Visible
**Symptoms**: Can't see where you're typing
**Solution**: Normal - shell doesn't show cursor (feature not implemented)

### Can't Exit QEMU
**Symptoms**: Ctrl-C doesn't work
**Solution**:
- **nographic mode**: Press `Ctrl-A` then `X`
- **Graphical mode**: Close the window
- **Emergency**: `killall qemu-system-i386` from another terminal

---

## 🎬 RECORDING A DEMO

### Using Script Command
```bash
# Start recording
script tbos_demo.txt

# Run TBOS
qemu-system-i386 -drive format=raw,file=build/tbos.img -m 128M -nographic

# Do your demo commands...

# Exit QEMU (Ctrl-A then X)

# Stop recording
exit

# View recording
cat tbos_demo.txt
```

### Using asciinema
```bash
# Install asciinema
sudo apt install asciinema

# Record session
asciinema rec tbos_demo.cast

# Run TBOS and demo...

# Exit recording (Ctrl-D)

# Play back
asciinema play tbos_demo.cast
```

---

## 📸 TAKING SCREENSHOTS

### From QEMU Monitor
```bash
# Start QEMU
qemu-system-i386 -drive format=raw,file=build/tbos.img -m 128M

# Switch to QEMU monitor: Ctrl-Alt-2

# Take screenshot
(qemu) screendump tbos_screenshot.ppm

# Convert to PNG
convert tbos_screenshot.ppm tbos_screenshot.png

# Switch back to guest: Ctrl-Alt-1
```

### Using External Tool
```bash
# With scrot (if in graphical mode)
scrot tbos_screenshot.png

# With import (ImageMagick)
import tbos_screenshot.png
# Then click on QEMU window
```

---

## 🎯 TESTING SCENARIOS

### Scenario 1: Basic Functionality Test
```
1. Boot system
2. Run: help
3. Run: about
4. Run: calc 5+5
5. Verify: Results appear correctly
```

### Scenario 2: Calculator Stress Test
```
calc 1+1
calc 10+20
calc 100+200
calc 50-25
calc 10*10
calc 100/4
calc 17%5
```

### Scenario 3: All Commands Test
```
help
about
steppps
ps
mem
time
calc 42+8
echo Hello World
test
om
clear
```

### Scenario 4: Error Handling Test
```
invalid_command
calc
calc abc
echo
(Press Enter on empty line)
```

---

## 📊 EXPECTED BEHAVIOR

### ✅ WORKING Features
- Command parsing
- Built-in commands (12 total)
- Calculator (5 operators)
- Text output
- Command not found errors
- Screen clearing
- System information display

### 🔄 PARTIAL Features
- Memory management (placeholder values)
- Process list (placeholder data)
- Timer (placeholder output)
- Tests (some skipped)

### ❌ NOT YET Implemented
- Command history (up/down arrows)
- Tab completion
- Multi-line commands
- Piping (|)
- Redirection (>, <)
- Background jobs (&)
- Environment variables
- Aliases

---

## 🚀 ADVANCED USAGE

### Run with Serial Output to File
```bash
qemu-system-i386 \
    -drive format=raw,file=build/tbos.img \
    -m 128M \
    -nographic \
    -serial file:tbos_output.log
```

### Run with Multiple Serial Ports
```bash
qemu-system-i386 \
    -drive format=raw,file=build/tbos.img \
    -m 128M \
    -serial stdio \
    -serial file:debug.log
```

### Run with Debugging
```bash
# Terminal 1: Start QEMU with GDB server
qemu-system-i386 \
    -drive format=raw,file=build/tbos.img \
    -m 128M \
    -s -S \
    -nographic

# Terminal 2: Connect GDB
gdb build/kernel.bin
(gdb) target remote localhost:1234
(gdb) break shell_process_command
(gdb) continue
```

---

## 📝 NOTES FOR PRESENTERS

### Key Talking Points
1. **Bootloader**: "Notice the sacred boot message - TBOS uses a custom bootloader"
2. **STEPPPS**: "The framework initializes all 7 dimensions - Space, Time, Event, Psychology, Pixel, Prompt, Script"
3. **Shell**: "This is a custom shell written from scratch in C, not bash/sh"
4. **Calculator**: "Even has a built-in calculator - try 'calc 42+8'"
5. **Status**: "Check 'steppps' command to see current implementation status"

### Demo Tips
- Start with `help` to show available commands
- Use `about` to explain the project
- Show `calc` for something interactive
- Run `test` to show system components
- Use `steppps` to show honest implementation status
- End with `om` for the sacred touch

### Common Questions

**Q: "Is this a real OS?"**
A: "Yes, it boots from a custom bootloader, runs its own kernel, and has a custom shell. However, it currently has Linux integration for some features."

**Q: "What can it do?"**
A: "It demonstrates core OS concepts: bootloader, kernel, shell, memory management, process management, interrupts. Check the STEPPPS status for current implementation."

**Q: "Can I use it as my daily driver?"**
A: "No, it's an educational OS demonstrating OS development concepts. See README_HONEST.md for current limitations."

---

## 🔗 RELATED DOCUMENTATION

- **GET_WELL_PLAN.md** - Development roadmap
- **README_HONEST.md** - Honest project status
- **boot/README_BOOTLOADER.md** - Bootloader details
- **PXFS_INTEGRATION_PLAN.md** - Future file system
- **test_bootloader.sh** - Automated testing

---

## ✨ CONCLUSION

The TernaryBit OS shell is fully functional and ready for demonstration!

**To start your demo:**
```bash
qemu-system-i386 -drive format=raw,file=build/tbos.img -m 128M -nographic
```

**Remember**: Press `Ctrl-A` then `X` to exit QEMU

**Enjoy exploring TernaryBit OS!** 🕉️

---

*Last Updated: 2025-10-19*
*Document Version: 1.0*
