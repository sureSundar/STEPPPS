# TernaryBit OS - UEFI Boot Guide

**Traceability:** v4.0/ROADMAP.md V4-008

This guide documents the UEFI boot architecture for TernaryBit OS V4.0.

## Current Status

| Component | Status | Notes |
|-----------|--------|-------|
| BIOS boot | ✅ Working | Primary boot path |
| UEFI stub | 🔲 Planned | PE/COFF loader for EFI |
| Secure Boot | 🔲 Future | Requires signing infrastructure |

## UEFI Boot Architecture

### Boot Flow Comparison

**BIOS Boot (Current):**
```
Stage 1 (MBR) → Stage 2 (Protected Mode) → Kernel
     ↓              ↓                        ↓
  0x7C00         0x8000                   0x10000
```

**UEFI Boot (Planned):**
```
UEFI Firmware → TBOS EFI App → Kernel
      ↓              ↓            ↓
   EFI Shell    PE/COFF stub   0x10000
```

### BCB Population in UEFI

The UEFI loader will populate the Boot Capability Block with:

```c
bcb->stage_flags = BCB_FLAG_UEFI | BCB_FLAG_64BIT;
bcb->boot_stage = 0;  // UEFI acts as Stage 0
bcb->mem_low_bytes = 0;  // UEFI memory map provides this
bcb->mem_high_bytes = uefi_get_memory_size();
bcb->kernel_entry = KERNEL_LOAD_ADDR;

// UEFI-specific: EFI runtime services pointer
bcb->stage_flags |= BCB_FLAG_EFI_RUNTIME;
```

## Building UEFI Target (Future)

### Prerequisites

```bash
# Install GNU-EFI development files
# Ubuntu/Debian:
sudo apt install gnu-efi

# macOS (via Homebrew):
brew install gnu-efi

# Fedora:
sudo dnf install gnu-efi-devel
```

### Build Commands (Planned)

```bash
# Build UEFI application
./build_v4.sh --target uefi

# Output: artifacts/baremetal/tbos-uefi.efi
```

### Directory Structure (Planned)

```
boot/
├── x86/
│   ├── bios/
│   │   └── stage1.asm      # Current MBR loader
│   └── uefi/
│       ├── tbos_efi.c      # EFI application entry
│       ├── efi_console.c   # EFI console driver
│       ├── efi_memory.c    # EFI memory map handling
│       └── Makefile
```

## Testing UEFI Boot with QEMU

### Create UEFI Test Environment

```bash
# Download OVMF (Open Virtual Machine Firmware)
# Ubuntu/Debian:
sudo apt install ovmf

# The firmware files are at:
# /usr/share/OVMF/OVMF_CODE.fd
# /usr/share/OVMF/OVMF_VARS.fd
```

### Create ESP (EFI System Partition)

```bash
# Create FAT32 image for ESP
dd if=/dev/zero of=esp.img bs=1M count=64
mkfs.fat -F 32 esp.img

# Mount and copy EFI application
mkdir -p mnt
sudo mount esp.img mnt
sudo mkdir -p mnt/EFI/BOOT
sudo cp artifacts/baremetal/tbos-uefi.efi mnt/EFI/BOOT/BOOTX64.EFI
sudo umount mnt
```

### Run with QEMU UEFI

```bash
qemu-system-x86_64 \
    -bios /usr/share/OVMF/OVMF_CODE.fd \
    -drive file=esp.img,format=raw \
    -m 256M \
    -serial stdio
```

## UEFI vs BIOS Feature Matrix

| Feature | BIOS | UEFI |
|---------|------|------|
| Boot mode | 16-bit Real → 32-bit Protected | 64-bit from start |
| Memory detection | INT 15h, E820 | GetMemoryMap() |
| Console output | INT 10h, VGA direct | EFI_SIMPLE_TEXT_OUTPUT |
| Disk access | INT 13h | EFI_BLOCK_IO_PROTOCOL |
| Graphics | VGA text mode | GOP (Graphics Output Protocol) |
| Secure Boot | Not supported | Supported |
| Boot time | Faster | Slightly slower |

## Implementation Roadmap

### Phase 1: Basic UEFI Stub
- [ ] Create minimal EFI application that prints to console
- [ ] Load kernel from ESP
- [ ] Populate BCB with UEFI flags

### Phase 2: Memory and Console
- [ ] Parse UEFI memory map
- [ ] Implement EFI console driver for kernel
- [ ] Exit boot services and jump to kernel

### Phase 3: Integration
- [ ] Add UEFI target to `build_v4.sh`
- [ ] Create ESP image builder
- [ ] Add QEMU UEFI test script

### Phase 4: Secure Boot (Stretch)
- [ ] Sign EFI application with test keys
- [ ] Document key enrollment process
- [ ] Test with Secure Boot enabled

## Code Template: UEFI Entry Point

```c
// boot/x86/uefi/tbos_efi.c (planned)
#include <efi.h>
#include <efilib.h>
#include "tbos/bcb.h"

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle,
                           EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    Print(L"TernaryBit OS UEFI Loader\r\n");

    // Allocate BCB
    tbos_bcb_v1_t bcb;
    bcb_init(&bcb);
    bcb.stage_flags = BCB_FLAG_UEFI | BCB_FLAG_64BIT;
    bcb.boot_stage = 0;

    // Get memory map
    UINTN MemMapSize = 0;
    UINTN MapKey, DescSize;
    UINT32 DescVersion;
    EFI_MEMORY_DESCRIPTOR *MemMap = NULL;

    SystemTable->BootServices->GetMemoryMap(
        &MemMapSize, MemMap, &MapKey, &DescSize, &DescVersion);

    // TODO: Parse memory map, load kernel, exit boot services

    // Jump to kernel with BCB pointer
    // kernel_entry(&bcb);

    return EFI_SUCCESS;
}
```

## See Also

- [VirtualBox Guide](VIRTUALBOX_GUIDE.md)
- [BCB Header](../include/tbos/bcb.h)
- [v4.0 Bootstrap Plan](../v4.0/BOOTSTRAP_PLAN.md)
- [GNU-EFI Documentation](https://sourceforge.net/projects/gnu-efi/)
