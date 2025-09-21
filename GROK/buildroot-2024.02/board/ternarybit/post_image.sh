#!/bin/bash

# TernaryBit OS Post-Image Script
# Integrates Buildroot Linux with TernaryBit bootloader

set -e

BINARIES_DIR="$1"
BOARD_DIR="$(dirname "$0")"
TERNARYBIT_DIR="$(realpath "${BOARD_DIR}/../../..")"

echo "TernaryBit OS: Creating integrated Linux image..."
echo "Binaries dir: $BINARIES_DIR"
echo "TernaryBit dir: $TERNARYBIT_DIR"

# Ensure we have the required files
if [ ! -f "${BINARIES_DIR}/bzImage" ]; then
    echo "Error: Linux kernel (bzImage) not found!"
    exit 1
fi

if [ ! -f "${BINARIES_DIR}/rootfs.ext4" ]; then
    echo "Error: Root filesystem not found!"
    exit 1
fi

# Build TernaryBit bootloader if needed
echo "Building TernaryBit bootloader..."
cd "${TERNARYBIT_DIR}"
if [ ! -f "build/boot.bin" ] || [ ! -f "build/stage2.bin" ]; then
    ./scripts/build.sh
fi

# Create integrated image directory
mkdir -p "${BINARIES_DIR}/ternarybit"
cd "${BINARIES_DIR}/ternarybit"

# Copy bootloader components
cp "${TERNARYBIT_DIR}/build/boot.bin" .
cp "${TERNARYBIT_DIR}/build/stage2.bin" .

# Copy Linux kernel and rename for bootloader
cp "${BINARIES_DIR}/bzImage" kernel.bin

# Copy root filesystem
cp "${BINARIES_DIR}/rootfs.ext4" rootfs.img

# Calculate sizes and sectors
BOOT_SIZE=$(stat -c%s boot.bin)
STAGE2_SIZE=$(stat -c%s stage2.bin)
KERNEL_SIZE=$(stat -c%s kernel.bin)
ROOTFS_SIZE=$(stat -c%s rootfs.img)

# Pad stage2 to sector boundary
STAGE2_SECTORS=$(((STAGE2_SIZE + 511) / 512))
STAGE2_PADDED_SIZE=$((STAGE2_SECTORS * 512))

# Pad kernel to sector boundary
KERNEL_SECTORS=$(((KERNEL_SIZE + 511) / 512))
KERNEL_PADDED_SIZE=$((KERNEL_SECTORS * 512))

# Calculate layout
STAGE2_START_SECTOR=1
KERNEL_START_SECTOR=$((STAGE2_START_SECTOR + STAGE2_SECTORS))
ROOTFS_START_SECTOR=$((KERNEL_START_SECTOR + KERNEL_SECTORS))

echo "Disk layout:"
echo "  Boot sector:    sector 0          (${BOOT_SIZE} bytes)"
echo "  Stage2:         sectors ${STAGE2_START_SECTOR}-$((KERNEL_START_SECTOR-1))      (${STAGE2_SIZE} bytes, ${STAGE2_SECTORS} sectors)"
echo "  Linux kernel:   sectors ${KERNEL_START_SECTOR}-$((ROOTFS_START_SECTOR-1))    (${KERNEL_SIZE} bytes, ${KERNEL_SECTORS} sectors)"
echo "  Root filesystem: sector ${ROOTFS_START_SECTOR}+        (${ROOTFS_SIZE} bytes)"

# Create padded files
dd if=stage2.bin of=stage2_padded.bin bs=512 conv=sync 2>/dev/null
dd if=kernel.bin of=kernel_padded.bin bs=512 conv=sync 2>/dev/null

# Calculate total image size (minimum 32MB for reasonable root filesystem space)
MIN_SIZE_MB=64
MIN_SIZE_SECTORS=$((MIN_SIZE_MB * 1024 * 1024 / 512))
REQUIRED_SECTORS=$((ROOTFS_START_SECTOR + (ROOTFS_SIZE + 511) / 512))
TOTAL_SECTORS=$((REQUIRED_SECTORS > MIN_SIZE_SECTORS ? REQUIRED_SECTORS : MIN_SIZE_SECTORS))

echo "Creating ${TOTAL_SECTORS} sector image ($(($TOTAL_SECTORS * 512 / 1024 / 1024))MB)..."

# Create the integrated image
IMAGE_NAME="ternarybit-linux.img"
dd if=/dev/zero of="${IMAGE_NAME}" bs=512 count=${TOTAL_SECTORS} 2>/dev/null

# Write components to image
echo "Writing bootloader components..."
dd if=boot.bin of="${IMAGE_NAME}" bs=512 count=1 conv=notrunc 2>/dev/null
dd if=stage2_padded.bin of="${IMAGE_NAME}" bs=512 seek=${STAGE2_START_SECTOR} conv=notrunc 2>/dev/null
dd if=kernel_padded.bin of="${IMAGE_NAME}" bs=512 seek=${KERNEL_START_SECTOR} conv=notrunc 2>/dev/null
dd if=rootfs.img of="${IMAGE_NAME}" bs=512 seek=${ROOTFS_START_SECTOR} conv=notrunc 2>/dev/null

# Copy final image to binaries directory
cp "${IMAGE_NAME}" "${BINARIES_DIR}/"

# Create QEMU launch script
cat > "${BINARIES_DIR}/run-ternarybit.sh" << EOF
#!/bin/bash
# TernaryBit OS QEMU Launch Script

echo "Starting TernaryBit OS with Buildroot Linux..."
echo "Kernel: Linux $(file ${BINARIES_DIR}/bzImage | cut -d',' -f2)"
echo "Root FS: $(du -h ${BINARIES_DIR}/rootfs.ext4 | cut -f1) ext4"
echo ""

qemu-system-i386 \\
    -drive format=raw,file=${BINARIES_DIR}/${IMAGE_NAME} \\
    -m 512M \\
    -netdev user,id=net0 \\
    -device rtl8139,netdev=net0 \\
    -serial mon:stdio \\
    -display gtk
EOF

chmod +x "${BINARIES_DIR}/run-ternarybit.sh"

# Create test script without GUI
cat > "${BINARIES_DIR}/test-ternarybit.sh" << EOF
#!/bin/bash
# TernaryBit OS Test Script (headless)

echo "Testing TernaryBit OS (headless mode)..."
timeout 30 qemu-system-i386 \\
    -drive format=raw,file=${BINARIES_DIR}/${IMAGE_NAME} \\
    -m 512M \\
    -nographic \\
    -serial stdio
EOF

chmod +x "${BINARIES_DIR}/test-ternarybit.sh"

echo ""
echo "🎉 TernaryBit OS integration completed successfully!"
echo ""
echo "Generated files:"
echo "  📀 ${BINARIES_DIR}/${IMAGE_NAME}"
echo "  🚀 ${BINARIES_DIR}/run-ternarybit.sh"
echo "  🧪 ${BINARIES_DIR}/test-ternarybit.sh"
echo ""
echo "To run TernaryBit OS:"
echo "  ${BINARIES_DIR}/run-ternarybit.sh"
echo ""
echo "To test (headless):"
echo "  ${BINARIES_DIR}/test-ternarybit.sh"