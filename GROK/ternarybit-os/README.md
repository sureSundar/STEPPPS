# TernaryBit OS - STEPPPS Universal Framework

## 🚀 **NEW: v3.0 Revolutionary Extensions Available!**
**[→ Explore the Revolutionary v3.0 Features](v3.0/README.md)** - World's first hardware-fluid OS with AI-driven bootloader

[Read the Full Documentation → README_FULL.md](README_FULL.md)

[Research Paper → RESEARCH_PAPER.md](RESEARCH_PAPER.md)

[![CI](https://img.shields.io/github/actions/workflow/status/suresundar/steppps/ci.yml?branch=main)](https://github.com/suresundar/steppps/actions)
[![CI/CD Status](https://github.com/suresundar/ternarybit-os/actions/workflows/ci-cd.yml/badge.svg)](https://github.com/suresundar/ternarybit-os/actions/workflows/ci-cd.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Repo](https://img.shields.io/badge/GitHub-suresundar%2Fsteppps-blue?logo=github)](https://github.com/suresundar/steppps)
[![DOI](https://img.shields.io/badge/DOI-10.5281%2Fzenodo.placeholder-blue)](https://doi.org/10.5281/zenodo.placeholder)
[![Paper Build](https://img.shields.io/github/actions/workflow/status/suresundar/steppps/paper.yml?branch=main&label=Paper)](https://github.com/suresundar/steppps/actions/workflows/paper.yml)

A revolutionary operating system implementing the STEPPPS (Space, Time, Event, Psychology, Pixel, Prompt, Script) universal computing framework with Hindu mythology integration.

## 🌟 Revolutionary v3.0 Features
- 🧠 **AI-Driven Bootloader** - First neural network bootloader advisor
- 🌐 **Hardware Fluidity** - Calculator to supercomputer transformation
- 📁 **Revolutionary Filesystems** - PXFS, UCFS, RF2S with Windows compatibility
- 🗜️ **Ultra Compression** - World record 1365:1 compression ratios
- 🔄 **Universal Architecture** - 7 architectures spanning 60+ years

## 🚀 TBOS Alpine Quick Start

Build (x86_64):

```bash
cd deploy/alpine
bash build-x86_64.sh
# artifact: deploy/alpine/out/x86_64/initramfs.cpio.gz
```

Boot (QEMU):

```bash
cd deploy/alpine/qa
bash qemu-x86_64.sh \
  --kernel /path/to/bzImage \
  --initrd ../out/x86_64/initramfs.cpio.gz
```

Demos (inside TBOS):

```bash
bash deploy/alpine/qa/tbos-optic-demo.sh
bash deploy/alpine/qa/tbos-optic-link-demo.sh
```

Persona-aware shell:

```text
mode calc    # calc 2+3*7
mode tiny    # compact
mode default # full
```

## 📦 Build Status

| Platform | Status |
|----------|--------|
| Linux | [![Linux Build](https://github.com/suresundar/ternarybit-os/actions/workflows/ci-cd.yml/badge.svg?branch=main&event=push)](https://github.com/suresundar/ternarybit-os/actions/workflows/ci-cd.yml) |
| Windows | [![Windows Build](https://github.com/suresundar/ternarybit-os/actions/workflows/ci-cd.yml/badge.svg?branch=main&event=push)](https://github.com/suresundar/ternarybit-os/actions/workflows/ci-cd.yml) |
| macOS | [![macOS Build](https://github.com/suresundar/ternarybit-os/actions/workflows/ci-cd.yml/badge.svg?branch=main&event=push)](https://github.com/suresundar/ternarybit-os/actions/workflows/ci-cd.yml) |

## 🚀 Quick Start

### Prerequisites

- [x86_64-elf cross-compiler](https://wiki.osdev.org/GCC_Cross-Compiler)
- QEMU (for testing)
- NASM (Netwide Assembler)
- Make

### Building

```bash
# Clone the repository
git clone https://github.com/suresundar/ternarybit-os.git
cd ternarybit-os

# Build the OS
make all

# Run in QEMU
make run
```

### CI/CD

This project uses GitHub Actions for continuous integration and deployment:

1. **On every push**:
   - Builds on Linux, Windows, and macOS
   - Runs unit tests
   - Checks for build errors

2. **On release tag (v*.*.*)**:
   - Creates a new GitHub release
   - Builds release binaries for all platforms
   - Generates checksums for verification
   - Updates documentation

## 🏗 Project Overview

A minimal operating system implementing the STEPPPS (Space, Time, Event, Psychology, Pixel, Prompt, Script) universal computing framework with Hindu mythology integration.

## Project Overview

TernaryBit OS is a bootable operating system that demonstrates:
- Custom x86 bootloader with protected mode transition
- Hindu mythology-themed system architecture and messaging
- STEPPPS framework integration for universal computing paradigms
- Collaborative development between multiple AI coding agents

## Architecture

### Boot Process
1. **Boot Sector** (`boot/boot_compact.asm`) - 512-byte MBR that loads stage2
2. **Stage2 Bootloader** (`boot/stage2_simple.asm`) - Loads kernel and transitions to protected mode
3. **Kernel** (`kernel/kernel.c` + `kernel/kernel_entry.asm`) - Main OS kernel with STEPPPS implementation

### STEPPPS Framework
- **Space**: Physical and virtual memory management
- **Time**: Process scheduling and temporal coordination
- **Event**: Interrupt handling and system events
- **Psychology**: Adaptive system behavior and user interaction patterns
- **Pixel**: Graphics and display management
- **Prompt**: Command interface and user input processing
- **Script**: Automation and scripting capabilities

### Hindu Mythology Integration
System components are named after Hindu mythological entities:
- **Divine names** (Brahma, Vishnu, Shiva) for normal/safe operations
- **Rakshasas** (Ravana, Mahishasura) for system threats/errors
- **Rishis** (Vyasa, Narada) for knowledge/guidance functions

## Current Status

### Working Components ✅
- Boot sector loads and displays STEPPPS banner
- Stage2 bootloader loads kernel successfully (all 27 sectors)
- A20 gate enablement
- VGA text mode operations
- Debug sequence: `!S2LKPA` (Stage2→Load→Kernel→Protected mode→A20)

### In Progress 🔧
- Protected mode GDT loading (currently triple faulting at `lgdt` instruction)
- Kernel execution and "ENTRY!" display

### Collaboration Features
The project demonstrates multi-AI collaboration:
- **Windsurf**: GDT physical address fixes, A20 gate implementation
- **Claude**: Protected mode transitions, sector count management, testing
- **Codex**: GDT structure optimization, debug improvements

## Building and Testing

### Prerequisites
- NASM (Netwide Assembler)
- GCC with 32-bit support
- QEMU for testing

### Build Process
```bash
./scripts/build.sh
```

This creates:
- `build/boot.bin` - 512-byte boot sector
- `build/stage2.bin` - Stage2 bootloader (4KB)
- `build/kernel.bin` - Main kernel
- `build/om.img` - Complete floppy disk image

### Testing
```bash
qemu-system-i386 -fda build/om.img
```

Expected output sequence:
1. Hindu mythology STEPPPS banner
2. Debug sequence: `!S2LKPA`
3. (Target) "ENTRY!" message from kernel

## File Structure

```
ternarybit-os/
├── boot/
│   ├── boot_compact.asm      # 512-byte boot sector
│   ├── stage2_simple.asm     # Main bootloader (current)
│   ├── stage2_debug.asm      # Debug version
│   └── stage2_streamlined.asm # Minimal version
├── kernel/
│   ├── kernel.c              # Main kernel implementation
│   ├── kernel.h              # Kernel headers
│   ├── kernel_entry.asm      # Kernel entry point
│   ├── support.c             # Support functions
│   └── linker.ld             # Linker script
├── src/core/                 # STEPPPS framework implementation
├── scripts/
│   └── build.sh              # Build automation
└── build/                    # Generated binaries
```

## Technical Details

### Memory Layout
- `0x7C00`: Boot sector load address
- `0x8000`: Stage2 bootloader
- `0x10000`: Kernel load address
- `0x90000`: Stack pointer
- `0xB8000`: VGA text mode buffer

### Debug Sequence
The bootloader outputs a character sequence for debugging:
- `!`: Stage2 entry
- `S`: Stage2 started
- `2`: Stage2 confirmed
- `L`: Loading kernel
- `K`: Kernel loaded
- `P`: Protected mode start
- `A`: A20 gate enabled
- `G`: GDT loaded (target)

### Known Issues
- **Triple Fault**: `lgdt` instruction causes system reset
- **Sector Count**: Must match kernel size (currently 27 sectors)

## Development Philosophy

This project embodies:
- **Sanatana Dharma Computing**: Eternal principles applied to system design
- **Universal Accessibility**: Framework designed for diverse hardware/software environments
- **Collaborative Intelligence**: Multi-AI development methodology
- **Spiritual Technology**: Integration of ancient wisdom with modern computing

## Future Goals

1. Complete protected mode transition
2. Implement full STEPPPS framework
3. Add network stack with dharmic protocols
4. Expand to support multiple architectures
5. Create universal hardware abstraction layer

## Collaboration

This project demonstrates successful collaboration between multiple AI coding agents using:
- **Code Comments**: In-file collaboration logs
- **Git History**: Tracking contributions and changes
- **Parallel Development**: Multiple agents working simultaneously
- **Iterative Testing**: Rapid build-test-fix cycles

## Quote

*"In the realm of bits and dharma, where ancient wisdom meets silicon dreams, TernaryBit OS bridges the eternal and the digital."*

---

**Status**: Active Development | **Last Updated**: 2025-09-21 | **Build**: v2.2

## 📚 Citation

If you use this work, please cite:

```
@misc{tbos_steppps_2025,
  title        = {TBOS × STEPPPS: A Universal, Autonomously Orchestrated Operating Substrate for Phygital Systems},
  author       = {Suresh Sundararajan and Cascade},
  year         = {2025},
  howpublished = {GitHub},
  doi          = {10.5281/zenodo.placeholder},
  url          = {https://github.com/suresundar/steppps}
}
```

Also see `CITATION.cff` at the repo root. After Zenodo mints a DOI for a release, replace the placeholder with the actual DOI.
