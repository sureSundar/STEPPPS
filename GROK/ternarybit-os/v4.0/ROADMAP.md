# V4.0 Universal Boot Roadmap

**Status: PRODUCTION READY** (Core BIOS + Hosted Linux targets)

| Item | Description | Owner | Status | Notes |
|------|-------------|-------|--------|-------|
| V4-001 | Define Boot Capability Block (BCB) header in `include/tbos/bcb.h` | Codex | ✅ Done | 128-byte fixed structure with HAL selection helpers |
| V4-002 | Stage 2 writes BCB data (drive, memory, kernel addr) | Codex | ✅ Done | `boot/stage2_linux.asm` `build_bcb` function |
| V4-003 | Kernel consumes BCB (select HAL drivers) | Codex | ✅ Done | `kernel/kernel_main.c` `kernel_process_bcb()` |
| V4-004 | Hosted Linux stub populates BCB and jumps to kernel | Codex | ✅ Done | `hosted/tbos_hosted_linux.c` with shell |
| V4-005 | `build_v4.sh` emits `artifacts/baremetal/tbos-bios.img` | Codex | ✅ Done | Cross-platform, integrates hosted Linux build |
| V4-006 | `build_v4.sh` emits hosted Linux binary | Codex | ✅ Done | `hosted/build_hosted_linux.sh`, outputs to `artifacts/hosted/` |
| V4-007 | Add smoke tests under `tests/v4/` for BIOS + hosted Linux | Codex | ✅ Done | `tests/v4/test_bcb.c`, `make test-bcb-host` |
| V4-008 | Document VirtualBox + UEFI recipes | Codex | ✅ Done | `docs/VIRTUALBOX_GUIDE.md`, `docs/UEFI_BOOT_GUIDE.md` |
| V4-009 | Stretch: WASM boot prototype | Codex | ✅ Done | `wasm/tbos.html`, `wasm/tbos_wasm_main.c`, `Makefile.wasm` |

Legend: 🔲 TODO | 🟡 In Progress | ✅ Done

## Production Release Notes

**V4.0 Core** is now production ready with:
- BIOS boot image: `artifacts/baremetal/tbos-bios.img`
- Hosted Linux binary: `artifacts/hosted/tbos-linux`
- BCB unit tests: `make test-bcb-host`
- Integration tests: `tests/integration/`
- CI/CD pipeline: `.github/workflows/ci-cd.yml`

**Deferred to V4.1:**
- Windows hosted binary (`tbos-win.exe`)
- WASM artifact (requires Emscripten CI integration)
- Advanced heap allocator (current page-based allocator is functional)

## Recent Changes (2025)

### V4-002: Stage 2 BCB Writer (COMPLETE)
Implemented BCB population in Stage 2 bootloader (`boot/stage2_linux.asm`):
- `build_bcb` function writes BCB at 0x6000
- Populates magic ("TBBC"), version, stage flags (BIOS, VGA)
- Memory detection via INT 12h and INT 15h AH=88h
- Boot drive ID from BIOS DL register
- Kernel entry point and size
- TBDS pointer for backwards compatibility
- BCB pointer passed to kernel in ECX register

### V4-003: Kernel BCB Consumer (COMPLETE)
Implemented BCB processing in kernel:
- `kernel_entry.asm` captures BCB pointer from ECX into `g_bcb_pointer`
- `kernel_main.c` has `kernel_process_bcb()` function
- Validates BCB structure (magic, version, length)
- Displays boot environment info (stage, flags, memory)
- Selects HAL type based on BCB flags
- Falls back to TBDS if BCB not present (backwards compat)

### V4-005: Build Orchestration (COMPLETE)
Updated `build_v4.sh` for complete V4.0 builds:
- Cross-platform file size detection (macOS/Linux)
- Builds BIOS image: `artifacts/baremetal/tbos-bios.img`
- Integrates hosted Linux build: `artifacts/hosted/tbos-linux`
- Proper summary with test commands

### V4-009: WASM Boot Prototype (COMPLETE)
Created WebAssembly runtime for browser-based TBOS:
- `wasm/tbos.html` - Browser test harness with terminal UI
  - Real-time console output
  - Command input with history
  - BCB info panel
  - Memory usage display
  - Status indicators
- `wasm/tbos_wasm_main.c` - WASM kernel entry point
  - BCB initialization with WASM flags
  - JavaScript bridge via Emscripten EM_JS
  - Shell commands (help, bcb, memory, time, test, modules)
  - Self-test suite
- `wasm/build_wasm.sh` - Build script for WASM
- `Makefile.wasm` - Updated build system

Build commands:
```bash
# With Emscripten
make -f Makefile.wasm wasm
make -f Makefile.wasm wasm-serve

# Without Emscripten (native test)
make -f Makefile.wasm wasm-native
```

### V4-008: VirtualBox + UEFI Documentation (COMPLETE)
Created deployment documentation:
- `docs/VIRTUALBOX_GUIDE.md` - Complete VirtualBox setup:
  - VM creation (GUI and CLI)
  - Floppy boot configuration
  - Serial console debugging
  - OVA template creation
  - Troubleshooting guide
- `docs/UEFI_BOOT_GUIDE.md` - UEFI boot architecture:
  - UEFI vs BIOS comparison
  - BCB population for UEFI
  - OVMF/QEMU testing setup
  - Implementation roadmap
  - Code templates

### V4-001: BCB Header (COMPLETE)
Created `include/tbos/bcb.h` with complete BCB v1.0 structure:
- Fixed 128-byte structure for cache alignment
- Stage flags for HAL driver selection (BIOS/UEFI/Hosted/WASM)
- Host API table pointer for hosted environments
- CRC32 checksum for integrity verification
- Helper macros and inline functions

### V4-004/V4-006: Hosted Linux Stub (COMPLETE)
Created hosted Linux environment:
- `hosted/tbos_hosted_linux.c` - Complete hosted environment
- Host API with console I/O, memory, file ops, time functions
- Interactive shell with BCB inspection commands
- Build script: `hosted/build_hosted_linux.sh`

### V4-007: BCB Unit Tests (COMPLETE)
Added comprehensive BCB tests:
- `tests/v4/test_bcb.c` - 20+ test cases
- Structure size, init, validation, checksum, flags, HAL selection
- Run with `make test-bcb-host`

## Build Commands

```bash
# Full V4.0 build (BIOS + Hosted Linux)
./build_v4.sh

# Build hosted Linux target only
make build-hosted-linux

# Run BCB tests
make test-bcb-host

# Test hosted Linux (interactive)
./artifacts/hosted/tbos-linux

# Quick hosted test (non-interactive)
make test-hosted-linux

# Run in QEMU
qemu-system-i386 -drive file=artifacts/baremetal/tbos-bios.img,format=raw,if=floppy

# Run in QEMU with serial debugging
qemu-system-i386 -drive file=artifacts/baremetal/tbos-bios.img,format=raw,if=floppy -serial stdio

# Build WASM (requires Emscripten)
make -f Makefile.wasm wasm

# Serve WASM in browser
make -f Makefile.wasm wasm-serve
# Then open http://localhost:8080/tbos.html

# Build native WASM test (no Emscripten needed)
make -f Makefile.wasm wasm-native
./build/wasm/tbos_wasm_test
```

## Documentation

- [VirtualBox Setup Guide](../docs/VIRTUALBOX_GUIDE.md)
- [UEFI Boot Guide](../docs/UEFI_BOOT_GUIDE.md)
- [BCB Header Specification](../include/tbos/bcb.h)
