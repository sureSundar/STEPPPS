# TernaryBit OS Architecture Guide

## Table of Contents
1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [Supported Architectures](#supported-architectures)
4. [Memory Management](#memory-management)
5. [Process Management](#process-management)
6. [Build System](#build-system)
7. [Porting Guide](#porting-guide)

## Overview

TernaryBit OS is a lightweight, modular OS supporting multiple architectures with a consistent API and user experience.

## System Architecture

```
+------------------+
|   Applications   |
+------------------+
|  System Libraries |
+------------------+
|      Kernel      |
+------------------+
|      HAL         |
+------------------+
|  Architecture    |
+------------------+
|    Hardware      |
+------------------+
```

## Supported Architectures

### x86_64
- **Boot Process**: BIOS/UEFI → Bootloader → Kernel
- **Memory Layout**:
  - 0x00100000-0x00EFFFFF: Kernel (14MB)
  - 0x01000000-0x0FFFFFFF: Kernel Heap
  - 0x10000000+: User Space

### ARM64
- **Boot Process**: Boot ROM → ATF → Kernel
- **Memory Layout**:
  - 0x00010000-0x01FFFFFF: Kernel
  - 0x02000000-0x0FFFFFFF: Heap
  - 0x10000000+: User Space

### RISC-V
- **Boot Process**: Boot ROM → OpenSBI → Kernel
- **Memory Layout**:
  - 0x80000000-0x801FFFFF: Kernel
  - 0x80200000-0x8FFFFFFF: Heap
  - 0x90000000+: User Space

## Memory Management

### Virtual Memory
- 4-level paging (x86_64, ARM64)
- 3-level paging (RISC-V)
- Demand paging
- Huge page support

### Physical Memory
- Buddy allocator
- Slab allocator
- Per-architecture zones

## Process Management

### Task States
- RUNNING, READY, BLOCKED, TERMINATED

### Scheduling
- Round-robin
- Priority-based
- Real-time support

## Build System

### Build Commands
```bash
# Default (x86_64)
make

# ARM64
make ARCH=arm64

# RISC-V
make ARCH=riscv64
```

## Porting Guide

### Adding New Architecture
1. Create `arch/<arch>/` directory
2. Implement boot code
3. Add memory management
4. Implement interrupts
5. Add drivers
6. Update build system

### Testing
- QEMU emulation
- Hardware testing
- Automated test suite

## Performance Tips
- x86_64: Optimize for OoO execution
- ARM64: Consider power efficiency
- RISC-V: Leverage custom extensions

## Security
- ASLR
- Stack protection
- KASLR
- Secure boot

## Debugging
- x86_64: GDB, perf
- ARM64: DS-5
- RISC-V: OpenOCD

## Future Work
- ARMv7 support
- Virtualization
- Real-time features
- Energy awareness
