# TBOS Manual Testing Guide
## 🕉️ Swamiye Saranam Aiyappa 🕉️

**How to manually test and "feel" TBOS yourself!**

---

## 🎮 INTERACTIVE TESTING

### Method 1: Easy Interactive Script (RECOMMENDED)

Just run this:
```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK
./RUN_BOOTLOADER.sh
```

It will give you 3 options:
1. **Graphical Window** - See the boot process visually
2. **Text Mode** - Like a server console
3. **With More RAM** - Test different classification

**Pick option 1 for the best experience!**

---

### Method 2: Manual Commands

#### Test Application Mode:
```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/app
./tbos_detector --detect
```
**You'll see**: Instant hardware detection of YOUR system

#### Test Bootloader Mode:
```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os
qemu-system-i386 -fda build/tbos_hardware_test.img -m 128M
```
**You'll see**: A virtual machine boot up and detect hardware!

---

## 👁️ WHAT YOU'LL SEE

### Application Mode (Instant):
```
╔══════════════════════════════════════════════════════════╗
║     TBOS Universal Hardware Detection Report            ║
╚══════════════════════════════════════════════════════════╝

=== SYSTEM INFORMATION ===
  Operating System: Linux 5.15.0-46-generic
  Architecture: x86_64

=== CPU INFORMATION ===
  Vendor: AuthenticAMD
  Cores: 4
  Speed: 2096 MHz (2.10 GHz)

=== MEMORY INFORMATION ===
  Total RAM: 9.72 GB

=== DEVICE CLASSIFICATION ===
  Device Class: DESKTOP (4MB-16MB)
  Class ID: 3

=== RECOMMENDED OS ===
  Recommended: Ubuntu Desktop / Alpine Linux Desktop
```

**This is YOUR actual system!** ✅

---

### Bootloader Mode (QEMU Window):

When you run the bootloader in QEMU, you'll see:

**1. BIOS Boot (2-3 seconds):**
```
SeaBIOS (version 1.13.0)
Booting from Floppy...
```

**2. TBOS Banner:**
```
=====================================
     TBOS x STEPPPS Framework
   Swamiye Saranam Aiyappa
=====================================
[BOOT] Initializing Sacred System...
[BOOT] Stage 2 loaded successfully
[BOOT] Transferring control...
```

**3. Hardware Detection:**
```
╔══════════════════════════════════════════════╗
║  TBOS Universal Hardware Detection Test     ║
║     Swamiye Saranam Aiyappa                 ║
╚══════════════════════════════════════════════╝

=== Hardware Detection ===
[CPU] Detecting processor...
  Vendor: GenuineIntel
  Signature: 0x00000800

[MEMORY] Detecting RAM...
  Total: 0x00020000 KB
  Total: 0x00000080 MB

[CLASS] Classifying device...
  Class: CLUSTER (64MB-128MB)

=== Hardware Summary ===
  Device Class: 0x06 (CLUSTER)
  Memory (MB): 128 MB
  CPUID Support: Yes

System halted.
```

**This is the QEMU virtual machine detected by the bootloader!** ✅

---

## 🎯 EXPERIMENT: Change RAM Size

Try different RAM amounts to see different classifications:

```bash
# 32 MB - Should classify as MOBILE
qemu-system-i386 -fda build/tbos_hardware_test.img -m 32M

# 128 MB - Should classify as CLUSTER
qemu-system-i386 -fda build/tbos_hardware_test.img -m 128M

# 256 MB - Should classify as SERVER/SUPERCOMPUTER
qemu-system-i386 -fda build/tbos_hardware_test.img -m 256M

# 512 MB - Should classify as SUPERCOMPUTER
qemu-system-i386 -fda build/tbos_hardware_test.img -m 512M
```

**Watch the classification change!** This proves the universal algorithm works!

---

## 🎮 INTERACTIVE CONTROLS

### In QEMU Graphical Window:

| Key Combination | Action |
|----------------|--------|
| `Ctrl+Alt+1` | Switch to guest display |
| `Ctrl+Alt+2` | Switch to QEMU monitor |
| `Ctrl+Alt+3` | Switch to serial console |
| `Ctrl+Alt+F` | Toggle fullscreen |
| `Ctrl+Alt+G` | Release mouse |

### In QEMU Monitor:
```
info registers    - Show CPU state
info mem          - Show memory
quit              - Exit QEMU
```

### In Text Mode (nographic):
```
Ctrl+A then X     - Exit QEMU
Ctrl+A then H     - Help
```

---

## 📸 TAKING SCREENSHOTS

### From QEMU:
```bash
# In QEMU monitor (Ctrl+Alt+2):
screendump /tmp/tbos_boot.ppm

# Convert to PNG:
convert /tmp/tbos_boot.ppm /tmp/tbos_boot.png
```

---

## 🔄 COMPARISON TEST

Run both modes side-by-side:

**Terminal 1** (Application):
```bash
cd app
./tbos_detector --detect
```

**Terminal 2** (Bootloader):
```bash
cd ternarybit-os
qemu-system-i386 -fda build/tbos_hardware_test.img -m 128M
```

**Compare the results!**
- Application detects YOUR hardware
- Bootloader detects QEMU virtual hardware
- SAME classification algorithm
- SAME display format

---

## 🎬 STEP-BY-STEP DEMO

### Full Experience (5 minutes):

1. **Open Terminal**
```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK
```

2. **Test Application Mode**
```bash
cd app
./tbos_detector --detect
# Read the results - this is YOUR system
```

3. **Test Bootloader Mode**
```bash
cd ../ternarybit-os
qemu-system-i386 -fda build/tbos_hardware_test.img -m 128M
# Watch it boot!
# See hardware detection!
# Note the classification!
```

4. **Try Different RAM**
```bash
qemu-system-i386 -fda build/tbos_hardware_test.img -m 32M
# See MOBILE classification

qemu-system-i386 -fda build/tbos_hardware_test.img -m 512M
# See SUPERCOMPUTER classification
```

5. **Compare Results**
- Your system: ~10 GB RAM → DESKTOP
- QEMU with 32M RAM → MOBILE
- QEMU with 128M RAM → CLUSTER
- QEMU with 512M RAM → SUPERCOMPUTER

**All using the SAME universal algorithm!** ✅

---

## 🎓 UNDERSTANDING WHAT YOU'RE SEEING

### Application Mode:
- Reads `/proc/cpuinfo` for CPU
- Uses `sysinfo()` for RAM
- Runs in **user space**
- Takes ~1 millisecond

### Bootloader Mode:
- Uses **CPUID instruction** for CPU
- Uses **BIOS INT 15h** for RAM
- Runs in **real mode** (16-bit)
- Takes ~3 seconds to boot

**Both classify the SAME way!**

---

## 🏆 CHALLENGES TO TRY

### Challenge 1: Find the Boundary
Run QEMU with different RAM amounts to find the exact boundary between CLUSTER and SUPERCOMPUTER:
```bash
qemu-system-i386 -fda build/tbos_hardware_test.img -m 127M  # CLUSTER?
qemu-system-i386 -fda build/tbos_hardware_test.img -m 128M  # SUPERCOMPUTER?
```

### Challenge 2: Compare Vendors
The bootloader detects "GenuineIntel" (QEMU)
Your application detects "AuthenticAMD" (your CPU)

Can you run QEMU with AMD emulation?

### Challenge 3: Real Hardware
**Advanced**: Write the image to a USB and boot a real computer!
```bash
sudo dd if=build/tbos_hardware_test.img of=/dev/sdX bs=4M
# Boot computer from USB
```

---

## 📊 RECORDING YOUR TESTS

Create a log:
```bash
# Application test
./tbos_detector --json > test_app.json

# Bootloader test (text mode)
qemu-system-i386 -fda build/tbos_hardware_test.img -nographic -m 128M > test_boot.log 2>&1

# Compare
cat test_app.json
cat test_boot.log
```

---

## 🎉 SHARING YOUR RESULTS

After testing, you can share:
1. Screenshots of QEMU window
2. JSON output from application
3. Text output from bootloader
4. Comparison of different RAM sizes
5. Comparison of application vs bootloader

---

🕉️ **Swamiye Saranam Aiyappa** 🕉️

**Experience TBOS yourself - Run it, see it, feel it!**

**Quick start**: `./RUN_BOOTLOADER.sh` and choose option 1!
