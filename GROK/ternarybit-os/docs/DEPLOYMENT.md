# TernaryBit OS Deployment Guide

## Table of Contents
1. [System Requirements](#system-requirements)
2. [Installation](#installation)
3. [Configuration](#configuration)
4. [Building from Source](#building-from-source)
5. [Running in QEMU](#running-in-qemu)
6. [Deployment to Physical Hardware](#deployment-to-physical-hardware)
7. [TBDS (TernaryBoot Data Structure)](#tbds-ternaryboot-data-structure)
8. [Troubleshooting](#troubleshooting)
9. [Uninstallation](#uninstallation)

## System Requirements

### Minimum Requirements
- x86_64, ARM, or RISC-V processor
- 64MB RAM (128MB recommended)
- 1GB storage (for development)
- USB boot support (for physical hardware)

### Development Environment
- Linux/macOS/Windows with WSL2
- GCC cross-compiler toolchain
- QEMU for emulation
- NASM (Netwide Assembler)
- Git

## Installation

### Pre-built Binaries
1. Download the latest release from [GitHub Releases](https://github.com/yourusername/ternarybit-os/releases)
2. Choose the appropriate architecture:
   - `ternarybit-x86_64.iso` for x86_64 systems
   - `ternarybit-arm64.img` for ARM64 (Raspberry Pi, etc.)
   - `ternarybit-riscv64.img` for RISC-V systems

### Building from Source

#### Prerequisites
```bash
# On Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential nasm qemu-system-x86 qemu-utils

# For cross-compilation
sudo apt-get install -y gcc-aarch64-linux-gnu gcc-riscv64-unknown-elf
```

#### Build Process
```bash
# Clone the repository
git clone https://github.com/yourusername/ternarybit-os.git
cd ternarybit-os

# Build for x86_64 (default)
make

# Or specify architecture
make ARCH=arm64    # For ARM64
make ARCH=riscv64  # For RISC-V
```

## Configuration

### Build Configuration
Edit `config/flags.mk` to customize build options:
```makefile
# Enable debug symbols
DEBUG = 1

# Enable specific features
FEATURE_SERIAL = 1
FEATURE_GRAPHICS = 1

# Kernel parameters
KERNEL_STACK_SIZE = 0x4000
```

### Runtime Configuration
Create a `config.h` file in the root directory to override default settings:
```c
// Example config.h
#define DEFAULT_CONSOLE_WIDTH 80
#define DEFAULT_CONSOLE_HEIGHT 25
#define ENABLE_EXTRA_LOGGING 1
```

## Running in QEMU

### x86_64
```bash
make run  # Uses QEMU with default settings

# Advanced QEMU options
make run QEMU_OPTS="-m 2G -smp 4 -enable-kvm"
```

### ARM64
```bash
make ARCH=arm64 run
```

### RISC-V
```bash
make ARCH=riscv64 run
```

## Deployment to Physical Hardware

### Creating Bootable Media

#### USB Drive (x86_64)
```bash
# WARNING: This will erase the target device
sudo dd if=build/x86_64/ternarybit.iso of=/dev/sdX bs=4M status=progress
```

#### Raspberry Pi (ARM64)
1. Flash the image to an SD card:
   ```bash
   sudo dd if=build/arm64/om.img of=/dev/sdX bs=4M status=progress
   ```
2. Insert into Raspberry Pi and power on

### Network Boot (PXE)
1. Configure your DHCP server to point to the TFTP server
2. Place the kernel and initramfs in the TFTP root
3. Configure your bootloader (e.g., GRUB) to load the kernel

## TBDS (TernaryBoot Data Structure)

The TernaryBoot Data Structure (TBDS) is passed from the bootloader to the kernel and contains essential system information.

### Structure
```c
struct tbds_header {
    uint32_t signature;      // 'TBDS' (0x53425444)
    uint16_t version;        // Structure version
    uint16_t header_size;    // Size of this header
    uint32_t total_size;     // Total size of TBDS including this header
    uint32_t crc32;          // CRC32 of the entire structure
} __attribute__((packed));

// Example TBDS entry
struct tbds_entry {
    uint8_t type;           // Entry type
    uint8_t flags;          // Entry flags
    uint16_t size;          // Size of this entry
    uint8_t data[];         // Entry data (variable length)
} __attribute__((packed));
```

### Entry Types
- `0x01`: Memory map
- `0x02`: ACPI tables
- `0x03`: SMBIOS information
- `0x04`: Boot parameters
- `0x05`: Device tree (ARM/RISC-V)

### Debugging TBDS
To verify TBDS is being passed correctly:
1. Check serial output for "TBDS stream consecrated for kernel darshan"
2. Use QEMU monitor to inspect memory:
   ```
   (qemu) xp/4x 0x1000  # Replace with actual TBDS address
   ```

## Troubleshooting

### Common Issues

#### Boot Failure
- **Symptom**: System hangs or reboots after bootloader
- **Solution**: 
  - Verify memory map in TBDS
  - Check for memory conflicts
  - Enable early debug output

#### No Video Output
- **Symptom**: Screen remains blank
- **Solution**:
  - Check video mode settings
  - Verify framebuffer initialization
  - Test with serial console

#### Kernel Panic
- **Symptom**: "Kernel panic" message
- **Solution**:
  - Check stack trace
  - Verify memory allocations
  - Enable debug symbols in build

### Debugging

#### Serial Console
```bash
# Start QEMU with serial output to file
make run QEMU_OPTS="-serial file:serial.log"

# Or connect to serial console
screen /dev/ttyS0 115200
```

#### GDB Debugging
```bash
# Start QEMU in debug mode
make debug

# In another terminal
gdb -x .gdbinit
```

## Uninstallation

### Removing from Development System
```bash
make clean          # Remove build artifacts
make distclean      # Remove all generated files
```

### Removing from Physical Hardware
1. Boot from a live USB
2. Reformat the drive:
   ```bash
   sudo mkfs.ext4 /dev/sdX
   ```
3. Reinstall your preferred operating system

## Support
For additional help, please open an issue on [GitHub](https://github.com/yourusername/ternarybit-os/issues).

## License
TernaryBit OS is licensed under the [MIT License](LICENSE).
