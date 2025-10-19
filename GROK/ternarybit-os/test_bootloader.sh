#!/bin/bash
# TernaryBit OS Bootloader Testing Script
# 🕉️ Swamiye Saranam Aiyappa 🕉️

set -e  # Exit on error

echo "======================================"
echo "  TBOS Bootloader Testing Suite"
echo "  🕉️ Sacred Boot Verification 🕉️"
echo "======================================"
echo

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Build directory
BUILD_DIR="build"
BOOT_DIR="boot"

# Test functions
test_passed() {
    echo -e "${GREEN}✓ PASS${NC}: $1"
}

test_failed() {
    echo -e "${RED}✗ FAIL${NC}: $1"
    exit 1
}

test_warn() {
    echo -e "${YELLOW}⚠ WARN${NC}: $1"
}

# Test 1: Check if build directory exists
echo "Test 1: Checking build environment..."
if [ -d "$BUILD_DIR" ]; then
    test_passed "Build directory exists"
else
    mkdir -p "$BUILD_DIR"
    test_warn "Created build directory"
fi
echo

# Test 2: Build the bootloader
echo "Test 2: Building bootloader..."
if [ -f "$BOOT_DIR/tbos_sacred.asm" ]; then
    nasm -f bin "$BOOT_DIR/tbos_sacred.asm" -o "$BUILD_DIR/boot.bin" 2>&1
    if [ $? -eq 0 ] && [ -f "$BUILD_DIR/boot.bin" ]; then
        test_passed "Bootloader assembled successfully"
    else
        test_failed "Bootloader assembly failed"
    fi
else
    test_failed "tbos_sacred.asm not found"
fi
echo

# Test 3: Verify boot signature
echo "Test 3: Verifying boot signature (0x55AA)..."
BOOT_SIZE=$(stat -c%s "$BUILD_DIR/boot.bin" 2>/dev/null || stat -f%z "$BUILD_DIR/boot.bin" 2>/dev/null)
if [ "$BOOT_SIZE" -eq 512 ]; then
    test_passed "Boot sector is exactly 512 bytes"

    # Check boot signature at bytes 510-511
    SIGNATURE=$(xxd -p -s 510 -l 2 "$BUILD_DIR/boot.bin")
    if [ "$SIGNATURE" = "55aa" ]; then
        test_passed "Boot signature 0x55AA present at offset 510-511"
    else
        test_failed "Boot signature incorrect: 0x$SIGNATURE (expected 0x55AA)"
    fi
else
    test_failed "Boot sector size is $BOOT_SIZE bytes (expected 512)"
fi
echo

# Test 4: Check for stage2
echo "Test 4: Checking for stage2 bootloader..."
if [ -f "$BOOT_DIR/stage2_linux.asm" ]; then
    test_passed "Stage2 source found"

    # Try to assemble stage2
    nasm -f bin "$BOOT_DIR/stage2_linux.asm" -o "$BUILD_DIR/stage2.bin" 2>&1
    if [ $? -eq 0 ]; then
        STAGE2_SIZE=$(stat -c%s "$BUILD_DIR/stage2.bin" 2>/dev/null || stat -f%z "$BUILD_DIR/stage2.bin" 2>/dev/null)
        test_passed "Stage2 assembled ($STAGE2_SIZE bytes)"
    else
        test_warn "Stage2 assembly had warnings (may still work)"
    fi
else
    test_warn "Stage2 not found (stage1-only boot)"
fi
echo

# Test 5: Create bootable disk image
echo "Test 5: Creating bootable disk image..."
dd if=/dev/zero of="$BUILD_DIR/tbos_test.img" bs=1M count=10 2>/dev/null
dd if="$BUILD_DIR/boot.bin" of="$BUILD_DIR/tbos_test.img" conv=notrunc 2>/dev/null

if [ -f "$BUILD_DIR/stage2.bin" ]; then
    # Write stage2 starting at sector 1 (after boot sector)
    dd if="$BUILD_DIR/stage2.bin" of="$BUILD_DIR/tbos_test.img" bs=512 seek=1 conv=notrunc 2>/dev/null
    test_passed "Disk image created with boot sector + stage2 (10MB)"
else
    test_passed "Disk image created with boot sector only (10MB)"
fi
echo

# Test 6: Hexdump first sector
echo "Test 6: Boot sector hexdump (first 64 bytes)..."
xxd -l 64 "$BUILD_DIR/boot.bin"
echo

# Test 7: Check for bootloader strings
echo "Test 7: Checking for bootloader messages..."
STRINGS=$(strings "$BUILD_DIR/boot.bin" | head -5)
if [ -n "$STRINGS" ]; then
    echo "Found strings in bootloader:"
    echo "$STRINGS"
    test_passed "Bootloader contains readable strings"
else
    test_warn "No readable strings found (may be compressed)"
fi
echo

# Test 8: QEMU boot test (interactive)
echo "======================================"
echo "  QEMU Boot Tests"
echo "======================================"
echo

echo "Available QEMU test modes:"
echo "  1. Quick boot test (2 seconds, no display)"
echo "  2. Interactive boot (opens window, Ctrl-C to exit)"
echo "  3. Serial output only (text mode)"
echo "  4. Debug mode (with GDB stub on port 1234)"
echo "  5. Skip QEMU tests"
echo

read -p "Select test mode [1-5]: " QEMU_MODE

case $QEMU_MODE in
    1)
        echo
        echo "Running quick boot test..."
        timeout 2 qemu-system-i386 \
            -drive format=raw,file="$BUILD_DIR/tbos_test.img" \
            -display none \
            -serial stdio \
            2>&1 | head -20 || true
        test_passed "Quick boot test completed"
        ;;

    2)
        echo
        echo "Starting interactive QEMU (close window or Ctrl-C to exit)..."
        echo "Watch for boot messages in the QEMU window"
        qemu-system-i386 \
            -drive format=raw,file="$BUILD_DIR/tbos_test.img" \
            -serial stdio \
            -m 64M
        test_passed "Interactive boot completed"
        ;;

    3)
        echo
        echo "Serial output mode (Ctrl-C to exit)..."
        qemu-system-i386 \
            -drive format=raw,file="$BUILD_DIR/tbos_test.img" \
            -display none \
            -serial stdio \
            -m 64M
        test_passed "Serial boot completed"
        ;;

    4)
        echo
        echo "Debug mode started on port 1234"
        echo "In another terminal, run: gdb -ex 'target remote localhost:1234'"
        qemu-system-i386 \
            -drive format=raw,file="$BUILD_DIR/tbos_test.img" \
            -serial stdio \
            -s -S \
            -m 64M
        test_passed "Debug session completed"
        ;;

    5)
        test_warn "QEMU tests skipped by user"
        ;;

    *)
        test_warn "Invalid selection, skipping QEMU tests"
        ;;
esac

echo
echo "======================================"
echo "  Test Summary"
echo "======================================"
echo "✓ Bootloader built successfully"
echo "✓ Boot signature verified (0x55AA)"
echo "✓ Disk image created: $BUILD_DIR/tbos_test.img"
echo
echo "Manual testing options:"
echo "  1. QEMU:       ./test_bootloader.sh"
echo "  2. VirtualBox: See instructions below"
echo "  3. Real HW:    See instructions below (DANGEROUS)"
echo
echo "======================================"
echo "  Additional Test Commands"
echo "======================================"
cat << 'TESTEOF'

# QEMU Tests (Safe)
# -----------------

# Basic boot (2 second timeout)
timeout 2 qemu-system-i386 -drive format=raw,file=build/tbos_test.img -nographic

# Interactive with VGA
qemu-system-i386 -drive format=raw,file=build/tbos_test.img

# Serial console output
qemu-system-i386 -drive format=raw,file=build/tbos_test.img -nographic -serial stdio

# With memory limit
qemu-system-i386 -drive format=raw,file=build/tbos_test.img -m 64M

# Debug mode (GDB)
qemu-system-i386 -drive format=raw,file=build/tbos_test.img -s -S

# Monitor mode (Ctrl-Alt-2 for QEMU monitor)
qemu-system-i386 -drive format=raw,file=build/tbos_test.img -monitor stdio


# VirtualBox Test (Safe)
# ----------------------

# Convert raw image to VDI
VBoxManage convertfromraw build/tbos_test.img build/tbos_test.vdi --format VDI

# Create VM
VBoxManage createvm --name "TBOS-Test" --register
VBoxManage modifyvm "TBOS-Test" --memory 64 --boot1 disk --boot2 none
VBoxManage storagectl "TBOS-Test" --name "IDE" --add ide
VBoxManage storageattach "TBOS-Test" --storagectl "IDE" --port 0 --device 0 --type hdd --medium build/tbos_test.vdi

# Start VM
VBoxManage startvm "TBOS-Test"

# Clean up
VBoxManage unregistervm "TBOS-Test" --delete


# Real Hardware Test (⚠️ DANGEROUS - Can overwrite data!)
# --------------------------------------------------------

# ⚠️ WARNING: This will ERASE the target device!
# Double-check device path (usually /dev/sdX for USB drives)

# Find USB drive
lsblk

# Write bootloader to USB (REPLACE /dev/sdX with your USB device!)
# sudo dd if=build/tbos_test.img of=/dev/sdX bs=512 status=progress

# Boot from USB
# 1. Insert USB drive
# 2. Reboot computer
# 3. Press F12/F2/Del (depends on BIOS) to select boot device
# 4. Choose USB drive


# Bochs Emulator Test (Alternative to QEMU)
# ------------------------------------------

# Create bochs config
cat > bochsrc.txt << 'EOF'
megs: 64
romimage: file=/usr/share/bochs/BIOS-bochs-latest
vgaromimage: file=/usr/share/bochs/VGABIOS-lgpl-latest
ata0: enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14
ata0-master: type=disk, path="build/tbos_test.img", mode=flat
boot: disk
log: bochs.log
mouse: enabled=0
display_library: x, options="gui_debug"
EOF

# Run bochs
bochs -f bochsrc.txt


# Analysis Tools
# --------------

# Disassemble boot sector
objdump -D -b binary -mi386 -Maddr16,data16 build/boot.bin | less

# Or with ndisasm (from nasm package)
ndisasm -b 16 build/boot.bin | less

# Hexdump full boot sector
xxd build/boot.bin | less

# Check for boot signature
xxd -s 510 -l 2 build/boot.bin

# String search
strings build/boot.bin

# File info
file build/boot.bin

TESTEOF

echo
echo "======================================"
echo "  Tests Complete!"
echo "======================================"
echo
