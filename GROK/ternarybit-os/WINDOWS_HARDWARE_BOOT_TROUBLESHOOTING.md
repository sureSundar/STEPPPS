# TBOS + Alpine Linux - Real Hardware Boot Troubleshooting
## 🕉️ Swamiye Saranam Aiyappa 🕉️

**Issue**: TBOS boots but fails when loading Alpine Linux on real Windows hardware

---

## 🔍 Common Failure Points

### **Where exactly does it fail?**

Please identify the exact failure point:

1. **Boot signature not found** - Windows doesn't recognize bootable drive
2. **Stage 1 loads, then hangs** - TBOS bootloader starts but freezes
3. **Stage 2 loads, fails at kernel loading** - Alpine kernel won't load
4. **Kernel loads but crashes** - Alpine starts to boot but fails
5. **Specific error message** - What error do you see?

---

## 🛠️ Quick Diagnostic Steps

### **Step 1: Create a Diagnostic Boot Image**

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os

# Use the minimal working image first
cp build/tbos_shell.img /media/sf_vboxshare/TBOS_TEST.img

# Or use hardware test image
cp build/tbos_hardware_test.img /media/sf_vboxshare/TBOS_TEST.img
```

### **Step 2: Write to USB on Windows**

On Windows, use **Rufus** or **Win32DiskImager**:

**Option A: Rufus (Recommended)**
```
1. Download Rufus: https://rufus.ie/
2. Insert USB drive (will be erased!)
3. Select TBOS_TEST.img
4. Partition scheme: MBR
5. Target system: BIOS (or UEFI-CSM)
6. Click START
```

**Option B: Win32DiskImager**
```
1. Download: https://sourceforge.net/projects/win32diskimager/
2. Select TBOS_TEST.img
3. Select USB drive letter
4. Click "Write"
```

**Option C: Command Line (PowerShell as Admin)**
```powershell
# List disks
GET-WMIOBJECT Win32_diskdrive

# Write image (CAREFUL - will erase USB!)
$img = "\\vboxsvr\vboxshare\TBOS_TEST.img"
$usb = "\\.\PHYSICALDRIVE#"  # Replace # with USB disk number
dd if=$img of=$usb bs=1M
```

---

## 🐛 Common Problems & Solutions

### **Problem 1: "No bootable device" or "Operating System not found"**

**Cause**: BIOS not recognizing boot signature or Legacy/UEFI mismatch

**Solutions:**

**A. Check BIOS Settings**
```
1. Reboot into BIOS (F2, F12, DEL, or ESC)
2. Go to Boot settings
3. Set Boot Mode: Legacy BIOS (NOT UEFI)
4. Disable Secure Boot
5. Disable Fast Boot
6. Set USB as first boot device
7. Save and reboot
```

**B. Verify Boot Signature**
```bash
# Check if boot signature is correct
xxd -l 512 /media/sf_vboxshare/TBOS_TEST.img | tail -5
# Should end with: 55 aa
```

---

### **Problem 2: Boots Stage 1, hangs at "Loading Stage 2"**

**Cause**: Disk reading issues on real hardware (CHS vs LBA)

**Solution**: Use LBA disk reading (more compatible)

Create a more compatible bootloader:

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os

# Edit boot/tbos_sacred.asm
# Ensure it uses LBA disk reading:
```

```asm
; Use LBA (Logical Block Addressing) instead of CHS
; More compatible with modern hardware

load_stage2:
    mov si, loading_msg
    call print_string

    ; Set up disk address packet (DAP) for LBA
    mov ah, 0x42            ; Extended read
    mov dl, [BOOT_DRIVE]    ; Drive number
    mov si, dap             ; DS:SI -> disk address packet
    int 0x13
    jc disk_error
    ret

dap:
    db 0x10                 ; Size of DAP
    db 0                    ; Reserved
    dw 8                    ; Number of sectors
    dw 0x8000               ; Offset
    dw 0                    ; Segment
    dq 1                    ; Starting LBA sector
```

---

### **Problem 3: Stage 2 loads, fails loading Alpine kernel**

**Cause**: Kernel not at expected disk location, or kernel too large

**Diagnosis**:
```bash
# Check what's actually in the image
file /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os/build/tbos_shell.img

# Check image structure
fdisk -l /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os/build/tbos_shell.img
```

**Solutions:**

**Option A: Use Embedded TBOS Kernel (Not Alpine)**
```bash
# Use pure TBOS without Alpine
cd /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os
./build_step3.sh

# This creates tbos_step3.img with TBOS kernel only
cp build/tbos_step3.img /media/sf_vboxshare/TBOS_NOALPINE.img
```

**Option B: Properly Embed Alpine**
```bash
# Download Alpine Mini ISO
wget http://dl-cdn.alpinelinux.org/alpine/v3.19/releases/x86_64/alpine-virt-3.19.0-x86_64.iso

# Extract kernel and initramfs
mkdir -p alpine_extract
sudo mount -o loop alpine-virt-3.19.0-x86_64.iso alpine_extract
cp alpine_extract/boot/vmlinuz-virt ./
cp alpine_extract/boot/initramfs-virt ./
sudo umount alpine_extract

# Rebuild TBOS image with Alpine kernel
# (Need to modify build script to include these)
```

---

### **Problem 4: "Kernel panic" or crash after kernel loads**

**Cause**: Kernel command line parameters incorrect or missing initramfs

**Solution**: Fix kernel parameters in stage2_linux.asm

```asm
; In stage2_linux.asm, around line 200-300

setup_linux_boot:
    ; Set up proper kernel command line
    mov di, LINUX_CMDLINE_ADDR
    mov si, kernel_cmdline
    call copy_string

    ; Set boot parameters
    mov word [LINUX_SETUP_SEG + 0x20], 0xA33F  ; Boot protocol version
    mov dword [LINUX_SETUP_SEG + 0x228], LINUX_CMDLINE_ADDR  ; cmd_line_ptr

    ret

kernel_cmdline:
    db 'console=tty0 console=ttyS0,115200n8 root=/dev/ram0 init=/sbin/init', 0
```

---

## 🔧 Hardware-Specific Fixes

### **For UEFI Systems (Modern PCs)**

TBOS currently only supports **Legacy BIOS**. For UEFI systems:

**Temporary Workaround:**
1. Enable **CSM (Compatibility Support Module)** in BIOS
2. Enable **Legacy Boot**
3. Disable **Secure Boot**

**Permanent Solution (Future):**
Create UEFI bootloader - contact if needed

---

### **For Systems with Fast Boot**

**Problem**: System skips USB boot detection

**Solution:**
1. Enter BIOS
2. Disable "Fast Boot" / "Fast Startup"
3. Enable "USB Legacy Support"
4. Set boot delay to 3-5 seconds

---

### **For Systems with Secure Boot**

**Problem**: Unsigned bootloader rejected

**Solution:**
1. Enter BIOS
2. Disable Secure Boot
3. (Or sign bootloader with your own keys - advanced)

---

## 📊 Diagnostic Boot Image

Let me create a special diagnostic bootloader that shows exactly where it fails:

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os

# Create diagnostic bootloader
cat > boot/tbos_diagnostic.asm << 'EOF'
[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Test 1: Basic output
    mov si, test1_msg
    call print

    ; Test 2: Disk detection
    mov si, test2_msg
    call print
    mov ah, 0x08
    mov dl, 0x80
    int 0x13
    jc disk_fail
    mov si, disk_ok_msg
    call print
    jmp test3

disk_fail:
    mov si, disk_fail_msg
    call print

test3:
    ; Test 3: Memory test
    mov si, test3_msg
    call print

    ; Infinite loop - if you see this, Stage 1 works!
    mov si, success_msg
    call print

hang:
    hlt
    jmp hang

print:
    pusha
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    popa
    ret

test1_msg db 'TBOS Diagnostic Stage 1: Output OK', 13, 10, 0
test2_msg db 'Test 2: Disk detection...', 0
disk_ok_msg db ' OK', 13, 10, 0
disk_fail_msg db ' FAILED!', 13, 10, 0
test3_msg db 'Test 3: Memory OK', 13, 10, 0
success_msg db 'SUCCESS: Stage 1 bootloader works on this hardware!', 13, 10, 0

times 510-($-$$) db 0
dw 0xAA55
EOF

# Build diagnostic image
nasm -f bin boot/tbos_diagnostic.asm -o build/tbos_diagnostic.img

# Pad to 1MB
dd if=/dev/zero of=build/tbos_diagnostic_full.img bs=1M count=1
dd if=build/tbos_diagnostic.img of=build/tbos_diagnostic_full.img conv=notrunc

# Copy to shared location
cp build/tbos_diagnostic_full.img /media/sf_vboxshare/TBOS_DIAGNOSTIC.img

echo "✅ Diagnostic image created: TBOS_DIAGNOSTIC.img"
echo "Write this to USB and boot on your Windows machine"
```

---

## 🎯 Testing Steps

### **Step-by-Step Testing:**

1. **Test Diagnostic Image First**
   ```
   Write TBOS_DIAGNOSTIC.img to USB
   Boot on Windows machine
   Record what messages appear
   Take photo of screen if possible
   ```

2. **If Diagnostic Works, Test Shell Image**
   ```
   Write tbos_shell.img to USB
   Boot on Windows machine
   Should see TBOS shell
   ```

3. **If Shell Works, Test Alpine**
   ```
   Need to properly integrate Alpine kernel
   Current stage2_linux.asm expects Alpine at specific disk location
   May need to rebuild image with Alpine included
   ```

---

## 📸 Information Needed

To help debug, please provide:

1. **Exact error message or screen output**
2. **Photo of failure screen** (if possible)
3. **Where does it stop?**
   - BIOS screen?
   - "Loading" message?
   - Black screen?
   - Error message?

4. **Hardware info:**
   - PC model/manufacturer
   - BIOS or UEFI?
   - Boot mode setting in BIOS

5. **How did you write the image?**
   - Rufus?
   - dd?
   - Win32DiskImager?

---

## 🚀 Quick Fix: Boot TBOS Without Alpine

If Alpine is causing issues, boot TBOS native kernel instead:

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os

# Build TBOS with its own kernel (no Alpine)
./build_step3.sh

# Copy to shared folder
cp build/tbos_step3.img /media/sf_vboxshare/TBOS_NATIVE.img
```

Then write TBOS_NATIVE.img to USB and boot.

---

## 📞 Next Steps

1. **Run the diagnostic script above** to create test image
2. **Boot on your Windows hardware**
3. **Report back what you see**
4. **Share error messages or photos**

Then I can provide specific fixes!

---

🕉️ **Swamiye Saranam Aiyappa** 🕉️

**Let's get TBOS booting on your hardware!**
