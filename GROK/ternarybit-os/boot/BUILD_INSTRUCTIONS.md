# TernaryBit OS Bootloader Build Instructions
🕉️ Swamiye Saranam Aiyappa 🕉️

## Overview

This document describes the bootloader build system for TernaryBit OS. The bootloader consists of two stages:

1. **Stage 1** (`boot.bin`): 512-byte MBR boot sector
2. **Stage 2** (`stage2.bin`): Protected mode transition loader

## Quick Start

### Build Bootloader Only
```bash
cd boot
./build_bootloader.sh
```

### Build Complete System
```bash
cd ..
./build.sh
```

## Bootloader Components

### Active Bootloader Files

| File | Purpose | Size | Status |
|------|---------|------|--------|
| `tbos_sacred.asm` | Stage 1 bootloader (MBR) | 512 bytes | ✅ PRIMARY |
| `stage2_linux.asm` | Stage 2 bootloader (Protected Mode) | 4 KB | ✅ PRIMARY |
| `stage2_protected.asm` | Alternative Stage 2 | 4 KB | ✅ BACKUP |
| `boot_hal.inc` | Hardware abstraction layer | Include | ✅ LIBRARY |
| `boot.asm` | Legacy bootloader | 512 bytes | 🟡 BACKUP |
| `debug.inc` | Debug utilities | Include | ✅ LIBRARY |

### Build Script

**`build_bootloader.sh`** - Automated bootloader build script

Features:
- Builds all bootloader components
- Validates boot sector size (must be 512 bytes)
- Reports file sizes and build status
- Selects active Stage 2 bootloader
- Error checking and reporting

## Build Process Details

### Stage 1: Boot Sector (tbos_sacred.asm)

```bash
nasm -f bin tbos_sacred.asm -o build/boot.bin
```

**Requirements:**
- Output size: EXACTLY 512 bytes
- Boot signature: 0xAA55 at bytes 510-511
- Loads from: MBR (Master Boot Record)
- Loads to: 0x7C00 in memory
- Responsibilities:
  - Initialize 16-bit real mode
  - Display boot message
  - Load Stage 2 from disk
  - Jump to Stage 2

**Key Features:**
- Uses `boot_hal.inc` for hardware abstraction
- Saves boot drive number at 0x0500
- Loads 8 sectors starting from LBA 1
- Sacred invocation: "Swamiye Saranam Aiyappa"

### Stage 2: Protected Mode Loader (stage2_linux.asm)

```bash
nasm -f bin stage2_linux.asm -o build/stage2_linux.bin
```

**Requirements:**
- Output size: ~4 KB (padded)
- Loads from: Disk sectors 1-8
- Loads to: 0x8000 in memory
- Responsibilities:
  - Initialize STEPPPS framework
  - Enable A20 line
  - Load kernel from disk
  - Build TBDS handoff structure
  - Setup GDT (Global Descriptor Table)
  - Switch to 32-bit protected mode
  - Jump to kernel at 0x10000

**Key Features:**
- Uses EDD 1.1 (INT 13h extensions) for disk I/O
- Loads kernel from LBA 10 (32 sectors)
- Sets up flat memory model
- Passes boot information via TBDS structure

## Memory Layout

### Real Mode (Stage 1)
```
0x0000:0x0000 - 0x0000:0x03FF  : Interrupt Vector Table
0x0000:0x0400 - 0x0000:0x04FF  : BIOS Data Area
0x0000:0x0500 - 0x0000:0x05FF  : Boot drive storage
0x0000:0x7C00 - 0x0000:0x7DFF  : Stage 1 bootloader (512 bytes)
0x0000:0x7E00 - 0x0000:0x7FFF  : Stack
0x0000:0x8000 - 0x0000:0x9FFF  : Stage 2 bootloader (8 KB max)
```

### Protected Mode (Stage 2)
```
0x00000000 - 0x000004FF : BIOS/IVT (legacy)
0x00000500 - 0x000005FF : Boot drive info
0x00007000 - 0x000070FF : TBDS handoff structure
0x00007C00 - 0x00007DFF : Stage 1 (resident)
0x00008000 - 0x00009FFF : Stage 2 (resident)
0x00010000 - 0x0001FFFF : Kernel (loaded here)
0x00090000 - 0x0009FFFF : Stack (32-bit)
```

## Disk Layout

| LBA | Sectors | Component | Size |
|-----|---------|-----------|------|
| 0 | 1 | Stage 1 (boot.bin) | 512 bytes |
| 1-8 | 8 | Stage 2 (stage2.bin) | 4 KB |
| 10-41 | 32 | Kernel (kernel.bin) | 16 KB |
| 500+ | Variable | Alpine initramfs | Variable |

## Build Validation

After building, verify the bootloader:

### Check Boot Signature
```bash
hexdump -C build/boot.bin | tail -2
```
Should show: `55 aa` at bytes 510-511

### Check File Sizes
```bash
ls -lh build/*.bin
```
Expected:
- `boot.bin`: 512 bytes (exact)
- `stage2_linux.bin`: 4096 bytes
- `stage2.bin`: 4096 bytes

### Test in QEMU
```bash
qemu-system-i386 -drive format=raw,file=build/tbos.img -serial stdio
```

## Troubleshooting

### Issue: Boot sector not 512 bytes
**Cause**: Assembly code too large or padding incorrect
**Fix**: Check `times 510-($-$$) db 0` and boot signature

### Issue: "Operating System Not Found"
**Cause**: Missing boot signature (0xAA55)
**Fix**: Verify boot signature at bytes 510-511

### Issue: Stage 2 not loading
**Cause**: Wrong sector count or LBA address
**Fix**: Verify `STAGE2_SECTOR_COUNT` in tbos_sacred.asm (should be 8)

### Issue: Kernel not found
**Cause**: Kernel not at expected LBA
**Fix**: Verify kernel written at LBA 10 in disk image (see build.sh line 72)

### Issue: Protected mode crash
**Cause**: GDT not properly configured or A20 not enabled
**Fix**: Check stage2_linux.asm:73-85 for GDT setup and A20 enabling

## STEPPPS Integration

The bootloader initializes the STEPPPS framework dimensions:

1. **SPACE** - Hardware detection and memory mapping
2. **TIME** - System timer initialization
3. **EVENT** - Interrupt handling preparation
4. **PSYCHOLOGY** - AI framework hooks
5. **PIXEL** - Display systems initialization
6. **PROMPT** - Command interface setup
7. **SCRIPT** - Automation preparation

These are initialized in stage2_linux.asm before kernel handoff.

## Integration with Main Build

The main build script (`../build.sh`) calls this bootloader build script:

```bash
# In build.sh line 14-15:
echo "[BUILD] Building bootloader components..."
cd boot && ./build_bootloader.sh && cd ..
```

This ensures bootloader binaries are always fresh before kernel compilation.

## Advanced: Custom Stage 2

To use a different Stage 2 bootloader:

1. Edit `build_bootloader.sh` line 51
2. Change: `cp "$BUILD_DIR/stage2_linux.bin" "$BUILD_DIR/stage2.bin"`
3. To: `cp "$BUILD_DIR/stage2_protected.bin" "$BUILD_DIR/stage2.bin"`

## Files Generated

After successful build:

```
build/
├── boot.bin              (Stage 1, 512 bytes)
├── stage2_linux.bin      (Stage 2 primary, 4 KB)
├── stage2_protected.bin  (Stage 2 alternative, 4 KB)
└── stage2.bin            (Active Stage 2, symlink/copy)
```

## References

- README_BOOTLOADER.md - Comprehensive bootloader documentation
- boot_hal.inc - HAL implementation details
- debug.inc - Debug utilities
- ../kernel/linker_sacred.ld - Kernel linker script

## Support

For issues or questions:
1. Check README_BOOTLOADER.md
2. Review boot_hal.inc for HAL details
3. Check GET_WELL_PLAN.md for roadmap

---

**Status**: ✅ ACTIVE - Bootloader build system operational
**Last Updated**: 2025-11-12
**Maintainer**: TernaryBit OS Team

🕉️ Swamiye Saranam Aiyappa 🕉️
