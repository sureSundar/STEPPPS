# TernaryBit OS - Unified Bootloader Documentation

**Date**: 2025-11-05
**Status**: ✅ UNIFIED & WORKING

---

## 🕉️ ONE BOOTLOADER NAME: `tbos_boot`

All bootloader code is now consolidated under a single, unified name: **`tbos_boot`**

---

## 📁 Unified Bootloader Files

### Assembly Files (boot/)
```
boot/tbos_boot.asm         - Stage 1: MBR bootloader (512 bytes)
boot/tbos_boot_stage2.asm  - Stage 2: Protected mode setup (4KB)
```

### C Files (src/boot/)
```
src/boot/tbos_boot.h       - Boot API header
src/boot/tbos_boot.c       - Boot utilities & functions
```

### Build Output
```
build/boot.bin             - Stage 1 binary (512 bytes)
build/stage2.bin           - Stage 2 binary (~4KB)
build/tbos_bare_metal.img  - Complete bootable disk image
```

---

## 🔧 Build Process

**Single Command:**
```bash
./build_universal.sh
```

**Build Steps:**
1. Assemble `tbos_boot.asm` → `boot.bin` (MBR)
2. Assemble `tbos_boot_stage2.asm` → `stage2.bin`
3. Compile kernel
4. Link everything
5. Create disk image

**Result:** Complete bootable system in `build/tbos_bare_metal.img`

---

## 📊 Boot Flow: `tbos_boot` → HAL → Kernel → Shell

```
┌────────────────────────────────────────────────┐
│  LAYER 1: tbos_boot (Stage 1 MBR)            │
│  File: boot/tbos_boot.asm                     │
│  Size: 512 bytes                              │
│  Load Address: 0x7C00                         │
│  Action: Load Stage 2                         │
└──────────────┬─────────────────────────────────┘
               ↓
┌────────────────────────────────────────────────┐
│  LAYER 2: tbos_boot_stage2 (Protected Mode)  │
│  File: boot/tbos_boot_stage2.asm             │
│  Size: ~4KB                                   │
│  Load Address: 0x8000                         │
│  Actions:                                     │
│  • Enable A20 line                            │
│  • Setup GDT                                  │
│  • Enter protected mode                       │
│  • Load kernel to 0x10000                     │
│  • Jump to kernel                             │
└──────────────┬─────────────────────────────────┘
               ↓
┌────────────────────────────────────────────────┐
│  LAYER 3: HAL (Hardware Abstraction)         │
│  File: src/hal/tbos_hal.{h,c}                │
│  Features:                                     │
│  • 16 capability flags                        │
│  • 5 hardware profiles                        │
│  • Universal API                              │
│  • Graceful degradation                       │
└──────────────┬─────────────────────────────────┘
               ↓
┌────────────────────────────────────────────────┐
│  LAYER 4: Kernel (OS Core)                   │
│  Entry: kernel/kernel_sacred.asm              │
│  Main: kernel/kernel_main.c                   │
│  Features:                                     │
│  • IDT setup                                  │
│  • Interrupt handling                         │
│  • VGA text mode                              │
└──────────────┬─────────────────────────────────┘
               ↓
┌────────────────────────────────────────────────┐
│  LAYER 5: Shell (User Interface)             │
│  File: shell/shell.c                          │
│  Commands: 11 bare-metal, expanding to 400    │
└────────────────────────────────────────────────┘
```

---

## 🎯 Integration Status

| Transition | Status | Notes |
|------------|--------|-------|
| tbos_boot → tbos_boot_stage2 | ✅ WORKING | Disk loading functional |
| tbos_boot_stage2 → Kernel | ✅ WORKING | Protected mode transition works |
| Kernel → Shell | ✅ WORKING | Shell starts successfully |
| Boot → HAL | ⏳ PENDING | HAL exists but not yet integrated |
| HAL → Kernel | ⏳ PENDING | Kernel uses direct hardware access |

**Next Integration Work:** Wire HAL into boot flow (Epic 5: Bootstrap Chain)

---

## 🚀 Testing

### QEMU (Fastest)
```bash
qemu-system-i386 -fda build/tbos_bare_metal.img
```

### VirtualBox
```bash
VBoxManage convertfromraw build/tbos_bare_metal.img build/tbos.vdi --format VDI
# Then attach to VM
```

### Real Hardware (USB Boot)
```bash
sudo dd if=build/tbos_bare_metal.img of=/dev/sdX bs=4M status=progress
```

---

## 📝 Memory Map

```
0x00007C00  Stage 1 (tbos_boot) loaded by BIOS
0x00008000  Stage 2 (tbos_boot_stage2) loaded by Stage 1
0x00010000  Kernel loaded by Stage 2
0x00090000  Stack
0x000B8000  VGA text buffer
```

---

## 🔄 Migrating Old Code

**Old References → New References:**
```c
// OLD (multiple names):
#include "tbos_minimal_boot.h"
#include "universal_boot.h"
#include "tbos_bootstrap.h"

// NEW (unified):
#include "tbos_boot.h"
```

**All code should use:** `tbos_boot` only!

---

## 📋 Archive

Old bootloader files have been moved to `boot/archive/` for reference but are **NOT USED**.

**Active files only:**
- `boot/tbos_boot.asm` (Stage 1)
- `boot/tbos_boot_stage2.asm` (Stage 2)
- `src/boot/tbos_boot.{h,c}` (C API)

---

## ✅ Build Verification

**Build Status:** ✅ SUCCESS
**Test Status:** ✅ BOOTS TO SHELL
**Disk Image:** 10MB bootable image created
**Boot Time:** ~1 second in QEMU

**Last Build:** 2025-11-05
**Build Output:**
```
=== Build Complete! ===
Disk layout:
  Sector 0:      Stage 1 bootloader (512 bytes)
  Sectors 1-8:   Stage 2 bootloader (4KB)
  Sectors 10-59: Kernel (up to 25KB)
```

---

## 🎓 Philosophy

**From WISDOM.md:**
> "Simplicity is the ultimate sophistication"

**ONE NAME. ONE BOOTLOADER. ONE PATH.**

`tbos_boot` → HAL → Kernel → Shell

**Clear. Simple. Working.**

---

**Status**: ✅ UNIFIED BOOTLOADER COMPLETE
**Ready for**: 8 commands/week acceleration plan!

🕉️ **Om Shanti** 🕉️
