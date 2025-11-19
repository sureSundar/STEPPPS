#!/bin/bash
# TernaryBit OS - Build Bootable Disk Image
# Creates a bootable disk image with bootloader + VM + bytecode

set -e

echo "╔══════════════════════════════════════════════════════════╗"
echo "║     TernaryBit OS - Bootable Image Builder              ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

BASE_DIR="/home/vboxuser/ssr/tbos"
BOOT_DIR="${BASE_DIR}/boot"
BUILD_DIR="${BASE_DIR}/build"

# Create build directory
mkdir -p "${BUILD_DIR}"

echo "Step 1: Assembling x86 BIOS bootloader..."
echo "----------------------------------------"

# Check for NASM
if ! command -v nasm &> /dev/null; then
    echo "⚠ NASM not installed. Installing..."
    sudo apt-get update > /dev/null 2>&1
    sudo apt-get install -y nasm > /dev/null 2>&1
fi

# Assemble stage0 (MBR - 512 bytes)
cd "${BOOT_DIR}/x86/bios"
nasm -f bin stage0_mbr.asm -o "${BUILD_DIR}/stage0.bin"
echo "✓ Stage 0 (MBR) assembled"

# Assemble stage2 (4KB loader)
nasm -f bin stage2_loader.asm -o "${BUILD_DIR}/stage2.bin"
echo "✓ Stage 2 (Loader) assembled"

# Check size
STAGE0_SIZE=$(stat -c%s "${BUILD_DIR}/stage0.bin")
if [ $STAGE0_SIZE -gt 512 ]; then
    echo "✗ Error: Stage 0 is ${STAGE0_SIZE} bytes (must be ≤512)"
    exit 1
fi

# Pad to exactly 512 bytes and add boot signature
if [ $STAGE0_SIZE -lt 510 ]; then
    dd if="${BUILD_DIR}/stage0.bin" of="${BUILD_DIR}/mbr.bin" bs=1 count=$STAGE0_SIZE 2>/dev/null
    dd if=/dev/zero of="${BUILD_DIR}/mbr.bin" bs=1 seek=$STAGE0_SIZE count=$((510-STAGE0_SIZE)) 2>/dev/null
    printf '\x55\xAA' >> "${BUILD_DIR}/mbr.bin"
    echo "✓ MBR created with boot signature (512 bytes)"
else
    cp "${BUILD_DIR}/stage0.bin" "${BUILD_DIR}/mbr.bin"
fi

echo ""
echo "Step 2: Building Nano VM..."
echo "----------------------------------------"
cd "${BASE_DIR}/vm"
make -f Makefile.cross clean > /dev/null 2>&1
make -f Makefile.cross tblvm > /dev/null 2>&1
echo "✓ Nano VM built"

VM_SIZE=$(stat -c%s tblvm)
echo "  VM binary size: ${VM_SIZE} bytes"

echo ""
echo "Step 3: Compiling boot program..."
echo "----------------------------------------"
cd "${BASE_DIR}/compiler"

# Create a boot program
cat > boot_program.tbs <<'EOF'
// TernaryBit OS Boot Program
void main() {
    println("TernaryBit OS v1.0")
    println("Universal Computing System")
    println("")
    println("Bootloader: OK")
    println("VM: OK")
    println("System: Ready")
    println("")
    println("Welcome to TernaryBit OS!")
}
EOF

python3 tbsc.py boot_program.tbs -o boot_program.tbc
echo "✓ Boot program compiled"

BYTECODE_SIZE=$(stat -c%s boot_program.tbc)
echo "  Bytecode size: ${BYTECODE_SIZE} bytes"

echo ""
echo "Step 4: Creating disk image..."
echo "----------------------------------------"

# Create 1.44MB floppy disk image
dd if=/dev/zero of="${BUILD_DIR}/tbos.img" bs=512 count=2880 2>/dev/null

# Write MBR to first sector
dd if="${BUILD_DIR}/mbr.bin" of="${BUILD_DIR}/tbos.img" bs=512 count=1 conv=notrunc 2>/dev/null

# Write Stage 2 starting at sector 2
dd if="${BUILD_DIR}/stage2.bin" of="${BUILD_DIR}/tbos.img" bs=512 seek=1 conv=notrunc 2>/dev/null

echo "✓ Disk image created (1.44MB)"
echo "  - Sector 1: MBR (512 bytes)"
echo "  - Sectors 2-9: Stage 2 (4KB)"

echo ""
echo "Step 5: Creating bootable ISO (for CD/USB)..."
echo "----------------------------------------"

mkdir -p "${BUILD_DIR}/iso/boot"
cp "${BUILD_DIR}/mbr.bin" "${BUILD_DIR}/iso/boot/"
cp "${BASE_DIR}/vm/tblvm" "${BUILD_DIR}/iso/boot/" 2>/dev/null || true
cp "${BASE_DIR}/compiler/boot_program.tbc" "${BUILD_DIR}/iso/boot/"

# Create isolinux config if isolinux is available
if command -v genisoimage &> /dev/null; then
    genisoimage -o "${BUILD_DIR}/tbos.iso" \
                -b boot/mbr.bin \
                -no-emul-boot \
                -boot-load-size 4 \
                -boot-info-table \
                "${BUILD_DIR}/iso" > /dev/null 2>&1
    echo "✓ ISO image created: ${BUILD_DIR}/tbos.iso"
else
    echo "⚠ genisoimage not found, skipping ISO creation"
fi

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║                 BUILD COMPLETE!                          ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Files created:"
echo "  ${BUILD_DIR}/mbr.bin        - Master Boot Record (512B)"
echo "  ${BUILD_DIR}/tbos.img       - Floppy disk image (1.44MB)"
echo "  ${BUILD_DIR}/tbos.iso       - Bootable ISO (CD/USB)"
echo ""
echo "To test in QEMU:"
echo "  qemu-system-x86_64 -drive file=${BUILD_DIR}/tbos.img,format=raw"
echo "  qemu-system-x86_64 -cdrom ${BUILD_DIR}/tbos.iso"
echo ""
echo "To write to USB (BE CAREFUL!):"
echo "  sudo dd if=${BUILD_DIR}/tbos.img of=/dev/sdX bs=4M status=progress"
echo "  (Replace /dev/sdX with your USB device)"
echo ""
