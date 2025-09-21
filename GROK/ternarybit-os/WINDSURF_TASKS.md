# WINDSURF Assistant Tasks for TBOS
## 🕉️ Collaborative Development Plan 🕉️

## Immediate Tasks for Windsurf

### 1. Testing Infrastructure (Priority: HIGH)
Create automated testing framework in `tests/` directory:
- Unit tests for bootloader validation
- QEMU automation scripts
- Memory detection tests
- Protected mode transition tests
- CI/CD pipeline setup

### 2. Driver Development (Priority: HIGH)
Create drivers in `drivers/` directory:
- **VGA Driver** (`drivers/vga/`)
  - Text mode (80x25)
  - Graphics mode (320x200, 640x480)
  - Color palette management
- **Keyboard Driver** (`drivers/keyboard/`)
  - PS/2 keyboard support
  - Scan code translation
  - Buffer management
- **Timer Driver** (`drivers/timer/`)
  - PIT (8254) configuration
  - Tick counting
  - Sleep functions

### 3. Shell Implementation (Priority: MEDIUM)
Build interactive shell in `shell/` directory:
- Command parser
- Built-in commands (ls, cd, cat, echo, help)
- Process launching
- History management
- Tab completion

### 4. File System Tools (Priority: MEDIUM)
Create PXFS utilities in `fs/` directory:
- mkfs.pxfs - Create PXFS filesystem
- fsck.pxfs - Check filesystem integrity
- mount/unmount utilities
- File operations library

### 5. Network Stack (Priority: LOW)
Implement networking in `net/` directory:
- Ethernet driver framework
- TCP/IP stack basics
- Socket interface
- DHCP client

### 6. Documentation (Priority: ONGOING)
Maintain docs in `docs/` directory:
- API documentation
- System architecture diagrams
- User manual
- Developer guide
- STEPPPS dimension specifications

## File Structure for Windsurf
```
ternarybit-os/
├── drivers/
│   ├── vga/
│   │   ├── vga.c
│   │   ├── vga.h
│   │   └── fonts.c
│   ├── keyboard/
│   │   ├── ps2.c
│   │   ├── ps2.h
│   │   └── keymaps.c
│   └── timer/
│       ├── pit.c
│       └── pit.h
├── shell/
│   ├── shell.c
│   ├── shell.h
│   ├── commands.c
│   └── parser.c
├── fs/
│   ├── pxfs.c
│   ├── pxfs.h
│   └── tools/
├── net/
│   ├── ethernet.c
│   ├── ip.c
│   ├── tcp.c
│   └── socket.c
└── tests/
    ├── test_boot.c
    ├── test_memory.c
    └── run_tests.sh
```

## Integration Points with Claude's Work

### Kernel Interface
Windsurf should use these kernel functions (being developed by Claude):
- `kernel_init()` - Main kernel initialization
- `memory_alloc()` - Memory allocation
- `interrupt_register()` - Register interrupt handlers
- `steppps_register()` - Register STEPPPS modules

### STEPPPS Hooks
Each driver should implement STEPPPS interfaces:
```c
struct steppps_module {
    void (*space_init)();    // Hardware initialization
    void (*time_tick)();      // Timer tick handler
    void (*event_handler)();  // Event processing
    void (*psychology)();     // AI/ML hooks
    void (*pixel_update)();   // Display update
    void (*prompt_cmd)();     // Command processing
    void (*script_exec)();    // Script execution
};
```

## Communication Protocol
1. Use git branches: `windsurf-dev` for Windsurf work
2. Regular merges to `main` every 2 hours
3. Comment code with `// WINDSURF:` prefix
4. Test before merge
5. Update this file with progress

## Progress Update (2025-09-21)
- WINDSURF: Scaffolding created to match planned structure:
  - `drivers/vga/`: `vga.c`, `vga.h`, `fonts.c` (text-mode stub writes to 0xB8000)
  - `drivers/keyboard/`: `ps2.c`, `ps2.h`, `keymaps.c` (ring buffer and minimal scancode map)
  - `drivers/timer/`: `pit.c`, `pit.h` (tick counter stub and sleep loop placeholder)
  - `shell/`: `shell.c`, `shell.h` (minimal shell banner and loop placeholder)
  - `fs/`: `pxfs.c`, `pxfs.h` and `fs/tools/README.md` (PXFS API stubs)
  - `net/`: `ethernet.c`, `ip.c`, `tcp.c`, `socket.c` (network API stubs)
  - `tests/unit/`: `test_boot.c`, `test_memory.c` (placeholder unit tests)
  - `tests/run_tests.sh`: simple unit test builder/runner

Notes:
- These are minimal, compilable stubs with `// WINDSURF:` comments for traceability.
- Next steps: wire into build system, add QEMU automation, expand tests per plan.

## Success Metrics
- [ ] VGA driver displays text
- [ ] Keyboard input works
- [ ] Shell accepts commands
- [ ] Timer interrupts fire
- [ ] Tests pass in QEMU
- [ ] Documentation complete

## Sacred Sprint Coordination
- Windsurf works on parallel sprints
- Sync every 4 sprints (1 hour)
- Share blockers immediately
- Celebrate milestones together

---
*"Two minds, one goal - TBOS in 9 days"*
🕉️ **Swamiye Saranam Aiyappa** 🕉️

## Quick Start for Windsurf
```bash
# Clone and setup
git checkout -b windsurf-dev

# Start with VGA driver
mkdir -p drivers/vga
cd drivers/vga
# Create vga.c, vga.h

# Test your work
make drivers
qemu-system-i386 -fda build/tbos.img

# Commit with sacred message
git commit -m "Kaliyugavaradhan Saranam - VGA driver implementation"
```