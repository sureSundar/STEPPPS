# Universal Bootloader Architecture
## TernaryBit OS - From Calculator to Supercomputer

**Version**: 1.0
**Date**: 2025-10-23
**Goal**: Single bootloader framework that runs on ANY processor architecture from 4-bit to 128-bit+

---

## Executive Summary

The Universal Bootloader is a modular, adaptive bootloader system that can initialize and boot TernaryBit OS on any computing device, from classic 4-bit calculators to quantum supercomputers. It achieves universality through:

1. **Hardware Detection Layer** - Identifies processor, memory, and boot method
2. **Adaptive Boot Stages** - Selects appropriate boot path based on capabilities
3. **Minimal Footprint** - Core bootloader fits in 512 bytes (MBR compatible)
4. **Extensible Design** - Stage2+ loaders provide architecture-specific initialization

---

## Design Principles

### 1. **Minimalism First**
- Stage1 must fit in 512 bytes (MBR requirement)
- Each stage only loads what's necessary
- No bloat, every byte counts

### 2. **Universal Detection**
- Auto-detect CPU architecture (4/8/16/32/64/128-bit)
- Auto-detect memory size (1KB to exabytes)
- Auto-detect boot method (ROM, BIOS, UEFI, Device Tree, SBI)

### 3. **Graceful Degradation**
- Full features on capable hardware
- Reduced features on constrained devices
- Always functional, never fails to adapt

### 4. **Forward Compatible**
- Architecture designed for unknown future processors
- Extensible detection mechanisms
- Plugin-based architecture support

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│                    HARDWARE LAYER                       │
│  Calculator │ Embedded │ Retro │ Modern │ Super │ Future│
│   4-bit     │  8/16-bit│ 8-bit │ 32/64  │ 64+   │ ?-bit │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│              UNIVERSAL BOOT STAGE 0 (UBS0)              │
│                  Hardware Detection                     │
│  • CPU Type Detection (4/8/16/32/64/128-bit)           │
│  • Memory Size Detection                                │
│  • Boot Method Detection (BIOS/UEFI/DT/SBI/ROM)        │
│  • Platform Identification                              │
│                   [512 bytes max]                       │
└──────────────────────┬──────────────────────────────────┘
                       │
         ┌─────────────┼─────────────┐
         │             │             │
┌────────▼───┐  ┌──────▼──────┐  ┌──▼────────────┐
│  Variant A │  │  Variant B  │  │  Variant C-E  │
│ Calculator │  │  x86 BIOS   │  │  Modern Arch  │
│  Embedded  │  │  x86 UEFI   │  │ ARM64, RISC-V │
└────────┬───┘  └──────┬──────┘  └──┬────────────┘
         │             │             │
         └─────────────┼─────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│         UNIVERSAL BOOT STAGE 2 (UBS2)                   │
│              Kernel Loader & HAL                        │
│  • Load Alpine Linux / TBOS Kernel                      │
│  • Initialize STEPPPS Framework                         │
│  • Setup Memory Management                              │
│  • Transfer Control to Kernel                           │
└─────────────────────────────────────────────────────────┘
```

---

## Stage 0: Universal Boot Sector (UBS0)

**Size**: 512 bytes (MBR compatible)
**Location**: First sector of boot media
**Languages**: x86 Assembly (primary), with fallback detection

### Responsibilities

1. **Minimal Environment Setup**
   - Initialize stack
   - Setup basic segments (x86) or registers (ARM/RISC-V)
   - Clear screen if display available

2. **Hardware Detection**
   ```
   Detection Priority:
   1. CPU Architecture (4/8/16/32/64/128-bit)
   2. Available Memory (1KB to exabytes)
   3. Boot Method (BIOS/UEFI/DT/SBI/ROM)
   4. Display Capability (text/graphics/none)
   5. Storage Access Method (BIOS INT 13h/UEFI/direct)
   ```

3. **Variant Selection**
   - Based on detection results, select appropriate Stage 1.5/2
   - Load variant-specific bootloader
   - Transfer control

### Detection Algorithm

```assembly
; Pseudo-code for UBS0 detection logic

detect_cpu:
    ; Test for x86 (CPUID instruction)
    if cpuid_available:
        variant = "x86"
        check_if_64bit()
        check_if_uefi()

    ; Test for ARM (by checking known ARM registers/opcodes)
    elif arm_signature_detected:
        variant = "ARM"
        check_arm_version()

    ; Test for RISC-V (SBI detection)
    elif riscv_sbi_available:
        variant = "RISC-V"

    ; Minimal/Calculator mode (default fallback)
    else:
        variant = "Minimal"
        bitwidth = detect_register_size()

detect_memory:
    ; Try BIOS INT 12h (x86)
    if bios_available:
        memory = bios_get_memory()

    ; Try UEFI memory map
    elif uefi_available:
        memory = uefi_get_memory_map()

    ; Try direct detection (embedded)
    else:
        memory = probe_memory_direct()

select_stage2:
    if variant == "Minimal" and memory < 64KB:
        load_stage2_variant_A()  ; Calculator/Embedded
    elif variant == "x86" and bios_mode:
        load_stage2_variant_B()  ; x86 BIOS
    elif variant == "x86" and uefi_mode:
        load_stage2_variant_C()  ; x86 UEFI
    elif variant == "ARM":
        load_stage2_variant_D()  ; ARM64
    elif variant == "RISC-V":
        load_stage2_variant_E()  ; RISC-V
```

### Stage 0 Memory Layout

```
0x0000-0x03FF: Interrupt Vector Table (IVT) - x86 BIOS
0x0400-0x04FF: BIOS Data Area
0x0500-0x05FF: Boot drive info & detection results
0x0600-0x07BF: Available for Stage 0 stack
0x07C0-0x07FF: Stage 0 code (loaded at 0x7C00)
0x0800-0x7FFF: Available for Stage 2 loading
0x8000+:       Stage 2 load location
```

---

## Boot Variants

### Variant A: Calculator & Embedded Systems

**Target Hardware**:
- 4-bit to 16-bit processors
- 1KB to 64KB RAM
- No MMU, no virtual memory
- ROM or minimal flash storage

**Boot Method**:
- ROM-based (burned into firmware)
- Or hosted by calculator OS

**Features**:
- Minimal STEPPPS initialization
- Direct hardware access
- No file system (code in ROM)
- Text-only or segment display output

**Memory Footprint**:
- Stage 0: N/A (may be ROM-based)
- Stage 1: 256 bytes
- Kernel: 2-8KB total

**Implementation File**: `boot/minimal/calculator_boot.asm`

---

### Variant B: x86 BIOS (Legacy)

**Target Hardware**:
- x86 (8086 to modern x86_64 in BIOS mode)
- 640KB+ RAM (real mode) or GB+ (protected mode)
- BIOS firmware
- MBR boot

**Boot Method**:
- BIOS loads MBR (Stage 0) at 0x7C00
- Stage 0 loads Stage 2 from disk

**Features**:
- Full STEPPPS initialization
- Real mode → Protected mode → Long mode (64-bit)
- Linux kernel loading support
- VGA text and graphics

**Memory Footprint**:
- Stage 0: 512 bytes (MBR)
- Stage 2: 4KB-8KB
- Kernel loader: varies

**Implementation Files**:
- `boot/x86/bios/stage0_mbr.asm`
- `boot/x86/bios/stage2_bios.asm`

---

### Variant C: x86 UEFI (Modern)

**Target Hardware**:
- x86_64 with UEFI firmware
- GB+ RAM
- GPT partition table
- UEFI boot services

**Boot Method**:
- UEFI loads bootloader as EFI application
- Already in 64-bit mode
- UEFI provides boot services

**Features**:
- Full STEPPPS initialization
- Native 64-bit execution
- UEFI boot services access
- Graphics output protocol (GOP)

**Memory Footprint**:
- Bootloader: 16KB-32KB (EFI PE binary)

**Implementation File**: `boot/x86/uefi/tbos_uefi_loader.c`

---

### Variant D: ARM64

**Target Hardware**:
- ARMv8-A 64-bit processors
- Raspberry Pi 3/4, smartphones, servers
- Device Tree based
- MB to GB+ RAM

**Boot Method**:
- Firmware loads bootloader to fixed address
- Device Tree provides hardware info
- May use U-Boot or direct boot

**Features**:
- Full STEPPPS initialization
- Device Tree parsing
- ARM exception levels (EL0-EL3)
- Linux kernel loading (ARM boot protocol)

**Memory Footprint**:
- Bootloader: 8KB-16KB

**Implementation Files**:
- `boot/arm64/start.S`
- `boot/arm64/boot_arm64.c`

---

### Variant E: RISC-V

**Target Hardware**:
- RISC-V RV32/RV64 processors
- SiFive boards, embedded to server-class
- SBI (Supervisor Binary Interface)
- MB to GB+ RAM

**Boot Method**:
- Firmware (OpenSBI/U-Boot) loads bootloader
- SBI provides runtime services
- Device Tree for hardware info

**Features**:
- Full STEPPPS initialization
- SBI calls for platform services
- Device Tree parsing
- Linux kernel loading (RISC-V boot protocol)

**Memory Footprint**:
- Bootloader: 8KB-16KB

**Implementation Files**:
- `boot/riscv64/start.S`
- `boot/riscv64/boot_riscv.c`

---

## Stage 2: Universal Kernel Loader (UBS2)

**Purpose**: Load and initialize the operating system kernel

### Common Stage 2 Responsibilities

1. **Extended Hardware Initialization**
   - Initialize all STEPPPS dimensions
   - Setup memory management
   - Initialize display subsystem
   - Setup storage drivers

2. **Kernel Loading**
   - Detect kernel format (bzImage, ELF, raw binary)
   - Load kernel from storage
   - Verify kernel integrity (optional checksum)
   - Decompress if necessary

3. **Boot Parameter Setup**
   - Prepare kernel command line
   - Setup boot info structure
   - Provide memory map
   - Device Tree (ARM/RISC-V) or ACPI (x86)

4. **Transfer Control**
   - Setup final CPU state
   - Jump to kernel entry point

### Stage 2 Variants

Each architecture has its own Stage 2 implementation:

| Variant | File | Format |
|---------|------|--------|
| Calculator | `boot/minimal/stage2_calc.asm` | Assembly |
| x86 BIOS | `boot/x86/bios/stage2_bios.asm` | Assembly |
| x86 UEFI | `boot/x86/uefi/stage2_uefi.c` | C (EFI) |
| ARM64 | `boot/arm64/stage2_arm64.c` | C + Assembly |
| RISC-V | `boot/riscv64/stage2_riscv.c` | C + Assembly |

---

## STEPPPS Framework Integration

Every bootloader variant initializes the 7 STEPPPS dimensions:

### Boot-Time STEPPPS Initialization

```c
// Pseudo-code for STEPPPS boot initialization

void init_steppps_bootloader(void) {
    // SPACE: Hardware detection and resource allocation
    detect_cpu_architecture();
    detect_memory_size();
    detect_storage_devices();

    // TIME: Initialize timing systems
    init_system_timer();
    calibrate_delays();

    // EVENT: Setup interrupt handling (if applicable)
    init_interrupt_controller();
    setup_basic_handlers();

    // PSYCHOLOGY: User interface initialization
    init_display();
    show_boot_banner();

    // PIXEL: Graphics/display setup
    init_framebuffer();  // or text mode

    // PROMPT: Command line preparation
    setup_kernel_cmdline();
    parse_boot_options();

    // SCRIPT: Boot automation
    execute_boot_script();  // if present
}
```

---

## Hardware Detection Database

The bootloader maintains a detection database for known platforms:

```c
// Platform detection signatures
typedef struct {
    char name[32];
    uint32_t cpu_signature;
    uint32_t chipset_id;
    uint16_t boot_method;
    void (*init_func)(void);
} platform_info_t;

// Example entries
platform_info_t platforms[] = {
    {"Generic x86 BIOS", 0x0000, 0x0000, BOOT_BIOS, init_x86_bios},
    {"Raspberry Pi 4", 0xARM8, 0xBCM2711, BOOT_DEVICETREE, init_rpi4},
    {"SiFive U740", 0xRISCV, 0x0740, BOOT_SBI, init_sifive_u740},
    // ... more platforms
};
```

---

## Boot Configuration

### Boot Configuration File Format

```ini
# tbos_boot.cfg - Universal Bootloader Configuration

[global]
timeout=5
default_variant=auto
verbose=true

[variant_a_calculator]
enabled=true
max_memory=64KB
display=segment

[variant_b_x86_bios]
enabled=true
kernel=/boot/vmlinuz
initrd=/boot/initrd.img
cmdline=root=/dev/sda1 console=ttyS0,115200

[variant_c_x86_uefi]
enabled=true
kernel=\EFI\TBOS\vmlinuz.efi
secure_boot=false

[variant_d_arm64]
enabled=true
kernel=/boot/Image
dtb=/boot/bcm2711-rpi-4-b.dtb

[variant_e_riscv]
enabled=true
kernel=/boot/vmlinux
opensbi=/boot/fw_jump.elf
```

---

## File Structure

```
boot/
├── universal/
│   ├── stage0_universal.asm      # Universal MBR (512 bytes)
│   ├── detect.asm                # Hardware detection routines
│   └── common.inc                # Shared constants/macros
│
├── minimal/                      # Variant A: Calculator/Embedded
│   ├── calculator_boot.asm       # 4-bit to 16-bit boot
│   ├── embedded_boot.asm         # Minimal embedded loader
│   └── README.md
│
├── x86/
│   ├── bios/                     # Variant B: x86 BIOS
│   │   ├── stage0_mbr.asm        # MBR bootloader
│   │   ├── stage2_bios.asm       # BIOS Stage 2
│   │   └── protected_mode.asm    # Mode switching code
│   │
│   └── uefi/                     # Variant C: x86 UEFI
│       ├── tbos_uefi_loader.c    # UEFI bootloader
│       ├── uefi_stage2.c         # UEFI Stage 2
│       └── Makefile
│
├── arm64/                        # Variant D: ARM64
│   ├── start.S                   # ARM64 entry point
│   ├── boot_arm64.c              # ARM64 bootloader
│   ├── devicetree.c              # Device Tree parser
│   └── Makefile
│
├── riscv64/                      # Variant E: RISC-V
│   ├── start.S                   # RISC-V entry point
│   ├── boot_riscv.c              # RISC-V bootloader
│   ├── sbi.c                     # SBI interface
│   └── Makefile
│
├── common/                       # Shared code
│   ├── steppps_init.c            # STEPPPS initialization
│   ├── kernel_loader.c           # Generic kernel loader
│   ├── memory_detect.c           # Memory detection
│   └── display.c                 # Display abstraction
│
└── archive/                      # Historical bootloaders
    └── (40+ old variants)
```

---

## Build System

### Multi-Architecture Build

```makefile
# Makefile for Universal Bootloader

ARCHITECTURES = minimal x86_bios x86_uefi arm64 riscv64

.PHONY: all clean $(ARCHITECTURES)

all: $(ARCHITECTURES)

minimal:
	$(MAKE) -C boot/minimal

x86_bios:
	nasm -f bin boot/x86/bios/stage0_mbr.asm -o build/mbr.bin
	nasm -f bin boot/x86/bios/stage2_bios.asm -o build/stage2.bin

x86_uefi:
	$(MAKE) -C boot/x86/uefi

arm64:
	aarch64-linux-gnu-gcc -c boot/arm64/start.S -o build/arm64_start.o
	aarch64-linux-gnu-gcc -c boot/arm64/boot_arm64.c -o build/arm64_boot.o
	aarch64-linux-gnu-ld -T boot/arm64/linker.ld -o build/bootloader_arm64.elf

riscv64:
	riscv64-unknown-elf-gcc -c boot/riscv64/start.S -o build/riscv_start.o
	riscv64-unknown-elf-gcc -c boot/riscv64/boot_riscv.c -o build/riscv_boot.o
	riscv64-unknown-elf-ld -T boot/riscv64/linker.ld -o build/bootloader_riscv.elf

clean:
	rm -rf build/*
```

---

## Testing Strategy

### Emulation-Based Testing

| Architecture | Emulator | Test Command |
|--------------|----------|--------------|
| x86 BIOS | QEMU | `qemu-system-i386 -drive format=raw,file=tbos.img` |
| x86 UEFI | QEMU + OVMF | `qemu-system-x86_64 -bios OVMF.fd -drive file=tbos.img` |
| ARM64 | QEMU | `qemu-system-aarch64 -M virt -cpu cortex-a72 -kernel bootloader.elf` |
| RISC-V | QEMU | `qemu-system-riscv64 -M virt -kernel bootloader.elf` |
| Calculator | Custom Sim | TBD |

### Test Matrix

```
✓ Boot on 4-bit calculator emulator
✓ Boot on 8-bit retro computer (C64 emulator)
✓ Boot on 16-bit PC (8086 emulator)
✓ Boot on 32-bit x86 (QEMU i386)
✓ Boot on 64-bit x86 (QEMU x86_64, BIOS mode)
✓ Boot on 64-bit x86 (QEMU x86_64, UEFI mode)
✓ Boot on ARM64 (QEMU aarch64, Raspberry Pi 4)
✓ Boot on RISC-V (QEMU riscv64, SiFive U740)
✓ Detect and adapt to memory: 1KB, 4KB, 64KB, 1MB, 1GB, 1TB
✓ Load Linux kernel (Alpine bzImage)
✓ Load TBOS custom kernel
✓ Transfer control successfully on all platforms
```

---

## Performance Targets

| Metric | Calculator | Embedded | x86 BIOS | x86 UEFI | ARM64 | RISC-V |
|--------|-----------|----------|----------|----------|-------|--------|
| Boot Time | <100ms | <500ms | <2s | <3s | <2s | <2s |
| Memory Usage | <1KB | <8KB | <32KB | <64KB | <64KB | <64KB |
| Code Size | <2KB | <4KB | <8KB | <32KB | <16KB | <16KB |

---

## Security Considerations

1. **Secure Boot Support** (UEFI/ARM)
   - Verify kernel signatures
   - Use platform secure boot mechanisms

2. **Memory Protection**
   - Validate loaded code boundaries
   - Prevent buffer overflows in bootloader

3. **Storage Verification**
   - Optional checksums for kernel/initrd
   - Detect boot media tampering

---

## Future Extensions

### Planned Architecture Support

- **MIPS**: Add MIPS32/MIPS64 bootloader
- **PowerPC**: Add PPC/PPC64 support
- **SPARC**: Add SPARC64 support
- **Quantum**: Research quantum computer boot mechanisms
- **Neural**: Research neuromorphic computing boot

### Planned Features

- **Network Boot**: PXE/TFTP support for all architectures
- **Encrypted Boot**: Full-disk encryption support
- **Multi-Boot**: Boot multiple OSes from single bootloader
- **Recovery Mode**: Built-in recovery environment
- **Remote Management**: Serial/network console for remote boot

---

## Conclusion

The Universal Bootloader represents a fundamental shift in boot architecture - one bootloader framework that truly works on ANY computing device. Through careful hardware detection, modular variants, and adaptive initialization, we achieve the goal of "write once, boot anywhere."

**Next Steps**:
1. Implement Stage 0 universal detection
2. Implement each variant (A through E)
3. Test on all target platforms
4. Integrate with STEPPPS framework
5. Deploy to production

---

**Document Status**: ✅ Architecture Complete
**Next Document**: Implementation Specifications for Each Variant
**Author**: TernaryBit OS Team + Claude Code
**Last Updated**: 2025-10-23
