# TernaryBit OS - VirtualBox Setup Guide

**Traceability:** v4.0/ROADMAP.md V4-008

This guide explains how to run TernaryBit OS in VirtualBox for development and testing.

## Prerequisites

- VirtualBox 6.1+ (or 7.x)
- TernaryBit OS BIOS image: `artifacts/baremetal/tbos-bios.img`

## Quick Start

### Option 1: QEMU (Fastest for Development)

```bash
# Build the V4.0 image
./build_v4.sh

# Run with QEMU
qemu-system-i386 -fda artifacts/baremetal/tbos-bios.img
```

### Option 2: VirtualBox

#### Step 1: Convert Floppy Image to VDI (Optional)

VirtualBox can boot directly from floppy images, but for disk-based testing:

```bash
# Convert raw image to VDI format
VBoxManage convertfromraw artifacts/baremetal/tbos-bios.img tbos.vdi --format VDI
```

#### Step 2: Create Virtual Machine

**Via GUI:**
1. Open VirtualBox Manager
2. Click "New"
3. Name: `TernaryBit-OS`
4. Type: `Other`
5. Version: `Other/Unknown`
6. Memory: `64 MB` (minimum) or `128 MB` (recommended)
7. Hard disk: Skip (we'll use floppy boot)

**Via Command Line:**
```bash
# Create VM
VBoxManage createvm --name "TernaryBit-OS" --ostype Other --register

# Configure memory and boot order
VBoxManage modifyvm "TernaryBit-OS" \
    --memory 128 \
    --vram 16 \
    --boot1 floppy \
    --boot2 none \
    --boot3 none \
    --boot4 none

# Attach floppy image
VBoxManage storagectl "TernaryBit-OS" --name "Floppy" --add floppy
VBoxManage storageattach "TernaryBit-OS" \
    --storagectl "Floppy" \
    --port 0 \
    --device 0 \
    --type fdd \
    --medium "$(pwd)/artifacts/baremetal/tbos-bios.img"
```

#### Step 3: Configure Serial Console (Optional)

For debugging output via serial port:

```bash
VBoxManage modifyvm "TernaryBit-OS" \
    --uart1 0x3F8 4 \
    --uartmode1 file /tmp/tbos-serial.log
```

#### Step 4: Start the VM

```bash
VBoxManage startvm "TernaryBit-OS"
```

Or use the GUI to start.

## Creating an OVA Template

To distribute a pre-configured VM:

```bash
# Export to OVA
VBoxManage export "TernaryBit-OS" \
    --output tbos-v4.ova \
    --ovf20 \
    --manifest \
    --vsys 0 \
    --product "TernaryBit OS" \
    --producturl "https://github.com/ternarybit/ternarybit-os" \
    --version "4.0"
```

To import:
```bash
VBoxManage import tbos-v4.ova
```

## Troubleshooting

### "No bootable medium found"

Ensure the floppy image is attached:
```bash
VBoxManage showvminfo "TernaryBit-OS" | grep -i floppy
```

### Black screen / No output

1. Check VGA mode is enabled in VM settings
2. Verify the image was built correctly:
   ```bash
   hexdump -C artifacts/baremetal/tbos-bios.img | head -5
   # Should show boot signature 0x55 0xAA at offset 510
   ```

### Kernel panic or hang

Enable serial logging and check `/tmp/tbos-serial.log` for debug output.

## Boot Capability Block (BCB)

When booting in VirtualBox, the BCB will report:
- `stage_flags`: `BCB_FLAG_BIOS | BCB_FLAG_VGA` (0x0201)
- Boot environment: Bare-Metal (BIOS)
- Memory: Detected via INT 12h / INT 15h

The kernel automatically detects this and uses the BIOS HAL drivers.

## Comparison: QEMU vs VirtualBox

| Feature | QEMU | VirtualBox |
|---------|------|------------|
| Boot speed | Fast | Moderate |
| Serial debugging | `-serial stdio` | File-based |
| GDB debugging | `-s -S` flags | Limited |
| Snapshots | Supported | Supported |
| Nested virt | Supported | Supported |
| Best for | Dev/debug | User testing |

## Recommended Development Workflow

1. **Build:** `./build_v4.sh`
2. **Quick test:** `qemu-system-i386 -fda artifacts/baremetal/tbos-bios.img`
3. **Debug:** Add `-serial stdio -d int` for interrupt debugging
4. **Full test:** Run in VirtualBox for realistic hardware emulation

## See Also

- [UEFI Boot Guide](UEFI_BOOT_GUIDE.md)
- [v4.0 Bootstrap Plan](../v4.0/BOOTSTRAP_PLAN.md)
- [BCB Specification](../include/tbos/bcb.h)
