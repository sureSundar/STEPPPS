# TernaryBit OS - Bootable System Guide

## ✅ Current Status

We have successfully built **bootable disk images** for x86 systems!

### What's Been Created:

```
/home/vboxuser/ssr/tbos/build/
├── mbr.bin          - Master Boot Record (512 bytes) ✅
├── stage0.bin       - Stage 0 bootloader (512 bytes) ✅
├── stage2.bin       - Stage 2 loader (4KB) ✅
├── tbos.img         - Bootable floppy image (1.44MB) ✅
└── tbos.iso         - Bootable ISO (CD/USB) ✅
```

## 🚀 How to Build the Bootable System

### Build Everything:

```bash
cd /home/vboxuser/ssr/tbos
./boot/build_bootable.sh
```

This creates:
1. **MBR (512 bytes)** - First stage bootloader
2. **Stage 2 (4KB)** - Protected mode transition
3. **Disk Image (1.44MB)** - Ready to write to floppy/USB
4. **ISO Image** - Ready to burn to CD or write to USB

## 🧪 Testing the Boot

### Option 1: QEMU (Recommended for Testing)

**Interactive Test:**
```bash
cd /home/vboxuser/ssr/tbos
./boot/test_boot.sh
```

Then select:
- `1` for text mode (see serial output)
- `2` for GUI mode (graphical window)
- `3` for screenshot capture

**Manual QEMU Commands:**

Text mode:
```bash
qemu-system-x86_64 \
    -drive file=/home/vboxuser/ssr/tbos/build/tbos.img,format=raw \
    -serial stdio \
    -nographic
```

GUI mode:
```bash
qemu-system-x86_64 \
    -drive file=/home/vboxuser/ssr/tbos/build/tbos.img,format=raw
```

ISO boot:
```bash
qemu-system-x86_64 \
    -cdrom /home/vboxuser/ssr/tbos/build/tbos.iso
```

### Option 2: VirtualBox

1. Create new VM:
   - Type: Linux
   - Version: Other Linux (32-bit)
   - Memory: 64MB (minimum)

2. Use disk image:
   - Storage → Add Floppy Controller
   - Add `/home/vboxuser/ssr/tbos/build/tbos.img`

3. Or use ISO:
   - Storage → IDE Controller
   - Add `/home/vboxuser/ssr/tbos/build/tbos.iso`

### Option 3: Real Hardware (USB Boot)

**⚠️ WARNING: Be very careful with device names!**

1. Insert USB drive
2. Find device name:
```bash
lsblk
```

3. Write image (REPLACE sdX with your USB device):
```bash
sudo dd if=/home/vboxuser/ssr/tbos/build/tbos.img \
        of=/dev/sdX \
        bs=4M \
        status=progress
sync
```

4. Boot from USB

## 📋 Boot Sequence

### Current Implementation:

```
Power On
    ↓
BIOS/UEFI loads MBR (Sector 0, 512 bytes)
    ↓
MBR prints "TernaryBit OS Bootloader"
    ↓
MBR loads Stage 2 (Sectors 2-9, 4KB) to 0x8000
    ↓
Stage 2 prints "Stage 2 Loaded"
    ↓
Stage 2 enables A20 line
    ↓
Stage 2 loads GDT
    ↓
Stage 2 enters Protected Mode
    ↓
Protected Mode prints "TernaryBit OS - Protected Mode Active"
    ↓
[VM integration - IN PROGRESS]
```

### Expected Output:

```
TernaryBit OS Bootloader
Loading Stage 2...
Stage 2 Loaded
Entering Protected Mode...
TernaryBit OS - Protected Mode Active
```

## 🔧 Boot Components

### 1. Stage 0 (MBR) - `stage0_mbr.asm`
- **Size**: 512 bytes (fits in MBR)
- **Location**: Sector 0
- **Functions**:
  - Initialize segments
  - Print boot message
  - Load Stage 2 from disk
  - Jump to Stage 2

### 2. Stage 2 - `stage2_loader.asm`
- **Size**: 4KB (8 sectors)
- **Location**: Loaded at 0x8000
- **Functions**:
  - Enable A20 line (access >1MB memory)
  - Setup GDT (Global Descriptor Table)
  - Transition to 32-bit Protected Mode
  - Initialize VGA text mode
  - [TODO] Load and execute VM

## 🎯 Next Steps (Integration)

### Phase 1: VM Integration (IN PROGRESS)
- [ ] Compile VM as flat binary
- [ ] Load VM into memory from bootloader
- [ ] Jump to VM entry point
- [ ] Pass bytecode location to VM

### Phase 2: Bytecode Loading
- [ ] Store bytecode on disk image
- [ ] Load bytecode into memory
- [ ] Execute first TBScript program at boot

### Phase 3: Full System
- [ ] Boot menu
- [ ] Program selection
- [ ] File system support
- [ ] Multiple programs

## 🛠️ Build Files

### Bootloader Source:
```
/home/vboxuser/ssr/tbos/boot/x86/bios/
├── stage0_mbr.asm        - MBR bootloader (512B)
└── stage2_loader.asm     - Protected mode loader (4KB)
```

### Build Scripts:
```
/home/vboxuser/ssr/tbos/boot/
├── build_bootable.sh     - Complete build process
└── test_boot.sh          - Test in QEMU
```

## 📊 Image Layout

### Disk Image (tbos.img - 1.44MB):

```
┌─────────────────────────────────────────┐
│ Sector 0 (512B)                         │
│ MBR + Boot Signature (0x55AA)           │
├─────────────────────────────────────────┤
│ Sectors 1-8 (4KB)                       │
│ Stage 2 Loader                          │
├─────────────────────────────────────────┤
│ Sectors 9+ (Available)                  │
│ [Reserved for VM + Bytecode]            │
│                                         │
│                                         │
└─────────────────────────────────────────┘
```

## ✅ Verification

### Check Boot Signature:
```bash
xxd -l 512 /home/vboxuser/ssr/tbos/build/mbr.bin | tail -1
```
Should show: `55aa` at end

### Check File Sizes:
```bash
ls -lh /home/vboxuser/ssr/tbos/build/
```

Expected:
- mbr.bin: 512 bytes
- stage0.bin: 512 bytes
- stage2.bin: 4096 bytes
- tbos.img: 1.5M
- tbos.iso: ~376K

## 🎉 Current Achievement

✅ **Bootable system created!**
- MBR bootloader assembled and working
- Stage 2 protected mode loader complete
- Disk images ready for testing
- ISO image for CD/USB boot ready

### What Works:
- Boot from floppy image
- Boot from ISO
- MBR loading
- Stage 2 loading
- Protected mode transition
- VGA text output

### In Progress:
- VM integration with bootloader
- Bytecode execution at boot
- Full system startup

## 📖 Documentation

Related documents:
- `UNIVERSAL_BOOTLOADER_ARCHITECTURE.md` - Full bootloader design
- `PRODUCTION_READY.md` - Complete system overview
- `RUNNING_PROGRAMS.md` - How to run TBScript programs

---

**Status**: Bootable system created ✅
**Next**: Integrate VM for program execution
**Test**: Use `./boot/test_boot.sh` or QEMU
