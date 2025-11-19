#!/bin/bash
# TernaryBit OS - Alpine Linux Integration Builder
# Creates a bootable Alpine Linux system with TernaryBit OS integrated

set -e

echo "╔══════════════════════════════════════════════════════════╗"
echo "║   TernaryBit OS - Alpine Linux Integration Builder      ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

BASE_DIR="/home/vboxuser/ssr/tbos"
BUILD_DIR="${BASE_DIR}/build/alpine"
ALPINE_VERSION="3.19"
ALPINE_ARCH="x86_64"

# Create build directories
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo "Step 1: Downloading Alpine Linux..."
echo "────────────────────────────────────────────────────────────"

ALPINE_ISO="alpine-standard-${ALPINE_VERSION}.0-${ALPINE_ARCH}.iso"
ALPINE_URL="https://dl-cdn.alpinelinux.org/alpine/v${ALPINE_VERSION}/releases/${ALPINE_ARCH}/${ALPINE_ISO}"

if [ ! -f "${ALPINE_ISO}" ]; then
    echo "Downloading ${ALPINE_ISO}..."
    wget -q --show-progress "${ALPINE_URL}" || {
        echo "✗ Download failed. Trying mirror..."
        wget -q --show-progress "https://mirrors.edge.kernel.org/alpine/v${ALPINE_VERSION}/releases/${ALPINE_ARCH}/${ALPINE_ISO}"
    }
    echo "✓ Alpine ISO downloaded"
else
    echo "✓ Alpine ISO already exists"
fi

echo ""
echo "Step 2: Extracting Alpine Linux..."
echo "────────────────────────────────────────────────────────────"

# Mount ISO (read-only)
MOUNT_DIR="${BUILD_DIR}/alpine_mount"
mkdir -p "${MOUNT_DIR}"

if mountpoint -q "${MOUNT_DIR}"; then
    sudo umount "${MOUNT_DIR}" 2>/dev/null || true
fi

sudo mount -o loop,ro "${ALPINE_ISO}" "${MOUNT_DIR}"
echo "✓ Alpine ISO mounted"

# Extract files
EXTRACT_DIR="${BUILD_DIR}/alpine_root"
rm -rf "${EXTRACT_DIR}"
mkdir -p "${EXTRACT_DIR}"

echo "Extracting kernel and initramfs..."
cp "${MOUNT_DIR}/boot/vmlinuz-lts" "${EXTRACT_DIR}/" 2>/dev/null || \
   cp "${MOUNT_DIR}/boot/vmlinuz-virt" "${EXTRACT_DIR}/" 2>/dev/null || \
   cp "${MOUNT_DIR}/boot/vmlinuz"* "${EXTRACT_DIR}/" 2>/dev/null

cp "${MOUNT_DIR}/boot/initramfs-lts" "${EXTRACT_DIR}/" 2>/dev/null || \
   cp "${MOUNT_DIR}/boot/initramfs-virt" "${EXTRACT_DIR}/" 2>/dev/null || \
   cp "${MOUNT_DIR}/boot/initramfs"* "${EXTRACT_DIR}/" 2>/dev/null

echo "✓ Kernel and initramfs extracted"

# Copy APK packages for offline install
mkdir -p "${EXTRACT_DIR}/apks"
if [ -d "${MOUNT_DIR}/apks" ]; then
    echo "Copying APK packages..."
    cp -r "${MOUNT_DIR}/apks/"* "${EXTRACT_DIR}/apks/" 2>/dev/null || true
fi

sudo umount "${MOUNT_DIR}"
echo "✓ ISO unmounted"

echo ""
echo "Step 3: Integrating TernaryBit OS..."
echo "────────────────────────────────────────────────────────────"

# Create custom overlay for TernaryBit
OVERLAY_DIR="${BUILD_DIR}/tbos_overlay"
mkdir -p "${OVERLAY_DIR}/usr/local/bin"
mkdir -p "${OVERLAY_DIR}/etc/init.d"
mkdir -p "${OVERLAY_DIR}/home/tbos"

# Copy TernaryBit VM
echo "Copying Nano VM..."
cp "${BASE_DIR}/vm/tblvm" "${OVERLAY_DIR}/usr/local/bin/" 2>/dev/null || {
    echo "Building VM first..."
    cd "${BASE_DIR}/vm"
    make -f Makefile.cross clean > /dev/null 2>&1
    make -f Makefile.cross tblvm > /dev/null 2>&1
    cd "${BUILD_DIR}"
    cp "${BASE_DIR}/vm/tblvm" "${OVERLAY_DIR}/usr/local/bin/"
}
chmod +x "${OVERLAY_DIR}/usr/local/bin/tblvm"
echo "✓ Nano VM copied"

# Copy TBScript compiler
echo "Copying compiler..."
mkdir -p "${OVERLAY_DIR}/usr/local/lib/tbos"
cp "${BASE_DIR}/compiler/tbsc.py" "${OVERLAY_DIR}/usr/local/lib/tbos/"
cp -r "${BASE_DIR}/compiler/examples" "${OVERLAY_DIR}/home/tbos/" 2>/dev/null || true
echo "✓ Compiler copied"

# Copy example programs
echo "Copying example programs..."
mkdir -p "${OVERLAY_DIR}/home/tbos/examples"
cp "${BASE_DIR}/compiler/examples/"*.tbs "${OVERLAY_DIR}/home/tbos/examples/" 2>/dev/null || true
echo "✓ Examples copied"

# Create TernaryBit startup script
cat > "${OVERLAY_DIR}/usr/local/bin/tbos" <<'EOF'
#!/bin/sh
# TernaryBit OS - Main wrapper script

echo "╔══════════════════════════════════════════════════════════╗"
echo "║              TernaryBit OS on Alpine Linux              ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "TernaryBit OS is ready!"
echo ""
echo "Commands:"
echo "  tbrun <file.tbs>    - Run a TBScript program"
echo "  tbcompile <file>    - Compile TBScript to bytecode"
echo "  tbvm <file.tbc>     - Run bytecode directly"
echo ""
echo "Examples in: /home/tbos/examples/"
echo ""
EOF
chmod +x "${OVERLAY_DIR}/usr/local/bin/tbos"

# Create convenient wrapper scripts
cat > "${OVERLAY_DIR}/usr/local/bin/tbrun" <<'EOF'
#!/bin/sh
# Compile and run TBScript program
if [ $# -eq 0 ]; then
    echo "Usage: tbrun <program.tbs>"
    exit 1
fi
BASENAME=$(basename "$1" .tbs)
python3 /usr/local/lib/tbos/tbsc.py "$1" -o "/tmp/${BASENAME}.tbc" && \
tblvm "/tmp/${BASENAME}.tbc"
EOF
chmod +x "${OVERLAY_DIR}/usr/local/bin/tbrun"

cat > "${OVERLAY_DIR}/usr/local/bin/tbcompile" <<'EOF'
#!/bin/sh
# Compile TBScript program
if [ $# -eq 0 ]; then
    echo "Usage: tbcompile <program.tbs>"
    exit 1
fi
python3 /usr/local/lib/tbos/tbsc.py "$@"
EOF
chmod +x "${OVERLAY_DIR}/usr/local/bin/tbcompile"

cat > "${OVERLAY_DIR}/usr/local/bin/tbvm" <<'EOF'
#!/bin/sh
# Run TernaryBit bytecode
if [ $# -eq 0 ]; then
    echo "Usage: tbvm <program.tbc>"
    exit 1
fi
tblvm "$@"
EOF
chmod +x "${OVERLAY_DIR}/usr/local/bin/tbvm"

# Create /etc/motd (message of the day)
cat > "${OVERLAY_DIR}/etc/motd" <<'EOF'
╔══════════════════════════════════════════════════════════╗
║              TernaryBit OS on Alpine Linux              ║
║                                                          ║
║  Universal Computing - Calculator to Supercomputer      ║
╚══════════════════════════════════════════════════════════╝

Welcome to TernaryBit OS!

Quick Start:
  tbrun /home/tbos/examples/hello.tbs   - Run Hello World
  tbrun /home/tbos/examples/calculator.tbs
  tbrun /home/tbos/examples/loop.tbs

Commands:
  tbos       - Show help
  tbrun      - Compile and run TBScript
  tbcompile  - Compile TBScript to bytecode
  tbvm       - Run bytecode directly

Documentation: /home/tbos/
EOF

echo "✓ TernaryBit integration complete"

echo ""
echo "Step 4: Creating custom initramfs with TernaryBit..."
echo "────────────────────────────────────────────────────────────"

# Create compressed overlay
cd "${OVERLAY_DIR}"
find . | cpio -o -H newc 2>/dev/null | gzip > "${BUILD_DIR}/tbos_overlay.gz"
cd "${BUILD_DIR}"
echo "✓ Overlay created"

echo ""
echo "Step 5: Building bootable image..."
echo "────────────────────────────────────────────────────────────"

# Create disk structure
DISK_DIR="${BUILD_DIR}/disk"
rm -rf "${DISK_DIR}"
mkdir -p "${DISK_DIR}/boot"

# Copy kernel
cp "${EXTRACT_DIR}/vmlinuz"* "${DISK_DIR}/boot/vmlinuz"
echo "✓ Kernel copied"

# Combine initramfs with overlay
cat "${EXTRACT_DIR}/initramfs"* "${BUILD_DIR}/tbos_overlay.gz" > "${DISK_DIR}/boot/initramfs"
echo "✓ Initramfs with TernaryBit overlay created"

# Create GRUB config
mkdir -p "${DISK_DIR}/boot/grub"
cat > "${DISK_DIR}/boot/grub/grub.cfg" <<EOF
set timeout=3
set default=0

menuentry "TernaryBit OS (Alpine Linux)" {
    linux /boot/vmlinuz modules=loop,squashfs,sd-mod,usb-storage quiet
    initrd /boot/initramfs
}

menuentry "TernaryBit OS (Safe Mode)" {
    linux /boot/vmlinuz modules=loop,squashfs,sd-mod,usb-storage
    initrd /boot/initramfs
}
EOF
echo "✓ GRUB config created"

# Create ISO
echo "Creating bootable ISO..."
if command -v grub-mkrescue &> /dev/null; then
    grub-mkrescue -o "${BUILD_DIR}/tbos-alpine.iso" "${DISK_DIR}" 2>/dev/null
    echo "✓ Bootable ISO created: ${BUILD_DIR}/tbos-alpine.iso"
elif command -v xorriso &> /dev/null; then
    echo "Using xorriso..."
    xorriso -as mkisofs -o "${BUILD_DIR}/tbos-alpine.iso" \
            -b boot/grub/grub.cfg -no-emul-boot \
            -boot-load-size 4 -boot-info-table \
            "${DISK_DIR}" 2>/dev/null
    echo "✓ Bootable ISO created: ${BUILD_DIR}/tbos-alpine.iso"
else
    echo "⚠ No ISO creation tool found. Install grub-mkrescue or xorriso"
fi

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║                   BUILD COMPLETE!                        ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Files created:"
echo "  ${BUILD_DIR}/tbos-alpine.iso    - Bootable ISO"
echo "  ${BUILD_DIR}/alpine_root/       - Extracted Alpine"
echo "  ${BUILD_DIR}/tbos_overlay/      - TernaryBit files"
echo ""
echo "To test:"
echo "  qemu-system-x86_64 -m 512 -cdrom ${BUILD_DIR}/tbos-alpine.iso"
echo ""
echo "TernaryBit OS is now integrated with Alpine Linux!"
echo "Boot Alpine, login (root/no password), then run: tbos"
echo ""
