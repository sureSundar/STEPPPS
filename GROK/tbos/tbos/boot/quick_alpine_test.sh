#!/bin/bash
# Quick Alpine + TernaryBit Test
# Creates a simple bootable system

echo "╔══════════════════════════════════════════════════════════╗"
echo "║     TernaryBit OS - Quick Alpine Integration Test       ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

BASE_DIR="/home/vboxuser/ssr/tbos"
BUILD_DIR="${BASE_DIR}/build/alpine-quick"

mkdir -p "$BUILD_DIR"

echo "Step 1: Using existing Alpine ISO..."
ALPINE_ISO="${BASE_DIR}/build/alpine/build/alpine/alpine-virt-3.19.1-x86_64.iso"

if [ ! -f "$ALPINE_ISO" ]; then
    echo "❌ Alpine ISO not found"
    exit 1
fi

echo "✓ Alpine ISO found: $(ls -lh "$ALPINE_ISO" | awk '{print $5}')"

echo ""
echo "Step 2: Creating TernaryBit overlay disk..."

# Create a small disk with TernaryBit tools
dd if=/dev/zero of="$BUILD_DIR/tbos-data.img" bs=1M count=10 2>/dev/null
mkfs.ext2 -F "$BUILD_DIR/tbos-data.img" > /dev/null 2>&1

# Mount and add TernaryBit files
MOUNT_DIR="$BUILD_DIR/mount"
mkdir -p "$MOUNT_DIR"
sudo mount -o loop "$BUILD_DIR/tbos-data.img" "$MOUNT_DIR"

# Copy TernaryBit files
sudo mkdir -p "$MOUNT_DIR/tbos"
sudo cp "$BASE_DIR/vm/tblvm" "$MOUNT_DIR/tbos/" 2>/dev/null || {
    cd "$BASE_DIR/vm"
    make -f Makefile.cross tblvm > /dev/null 2>&1
    sudo cp "$BASE_DIR/vm/tblvm" "$MOUNT_DIR/tbos/"
}
sudo cp "$BASE_DIR/compiler/tbsc.py" "$MOUNT_DIR/tbos/"
sudo cp -r "$BASE_DIR/compiler/examples" "$MOUNT_DIR/tbos/" 2>/dev/null

# Create readme
sudo tee "$MOUNT_DIR/tbos/README.txt" > /dev/null <<EOF
TernaryBit OS on Alpine Linux

To use:
1. Mount this disk in Alpine
2. Run: /mnt/tbos/tblvm
3. Compile programs with: python3 /mnt/tbos/tbsc.py

Examples in: /mnt/tbos/examples/
EOF

sudo umount "$MOUNT_DIR"

echo "✓ TernaryBit data disk created"

echo ""
echo "Step 3: Creating boot script..."

cat > "$BUILD_DIR/boot_alpine_tbos.sh" <<'BOOTSCRIPT'
#!/bin/bash
# Boot Alpine with TernaryBit data disk

echo "╔══════════════════════════════════════════════════════════╗"
echo "║         Alpine Linux + TernaryBit OS                     ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Booting Alpine Linux..."
echo "Login as: root (no password)"
echo ""
echo "Then mount TernaryBit tools:"
echo "  mkdir /mnt/tbos"
echo "  mount /dev/sdb /mnt/tbos"
echo "  /mnt/tbos/tblvm --help"
echo ""
echo "Press Ctrl+A then X to exit QEMU"
echo ""

qemu-system-x86_64 \
    -m 512 \
    -cdrom "/home/vboxuser/ssr/tbos/build/alpine/build/alpine/alpine-virt-3.19.1-x86_64.iso" \
    -hda "/home/vboxuser/ssr/tbos/build/alpine-quick/tbos-data.img" \
    -nographic
BOOTSCRIPT

chmod +x "$BUILD_DIR/boot_alpine_tbos.sh"

echo "✓ Boot script created"

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║                   SETUP COMPLETE!                        ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "To test Alpine + TernaryBit:"
echo "  $BUILD_DIR/boot_alpine_tbos.sh"
echo ""
echo "Inside Alpine, run:"
echo "  mkdir /mnt/tbos"
echo "  mount /dev/sda /mnt/tbos"
echo "  ls /mnt/tbos"
echo ""
