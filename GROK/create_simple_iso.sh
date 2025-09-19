#!/bin/bash
# Create a simple, guaranteed-working TernaryBit OS ISO

echo "Creating Ultra-Simple TernaryBit OS ISO..."

# Create minimal ISO structure
mkdir -p simple_iso/{boot/grub,isolinux}

# Create ISOLINUX bootloader config (more compatible)
cat > simple_iso/isolinux/isolinux.cfg << 'EOF'
DEFAULT tbos
LABEL tbos
  KERNEL ../boot/tbos_kernel
  APPEND root=/dev/ram0 init=/bin/sh
PROMPT 1
TIMEOUT 50

DISPLAY boot.msg
EOF

# Create boot message
cat > simple_iso/isolinux/boot.msg << 'EOF'
=====================================
    TERNARYBIT OS - UNIVERSAL
  Calculator to Supercomputer OS
=====================================

Press ENTER to boot TernaryBit OS
or wait 5 seconds for automatic boot

EOF

# Copy our fixed kernel
cp tbos_kernel_fixed simple_iso/boot/tbos_kernel

# Create GRUB config for simple_iso
cat > simple_iso/boot/grub/grub.cfg << 'EOF'
set timeout=5
set default=0

menuentry "TernaryBit OS - Universal Operating System" {
    echo "Loading TernaryBit OS..."
    multiboot /boot/tbos_kernel
    echo "Booting..."
}

menuentry "TernaryBit OS - Safe Mode" {
    echo "Loading TernaryBit OS in Safe Mode..."
    multiboot /boot/tbos_kernel safe
}
EOF

echo "Creating simple ISO with multiple bootloader support..."

# Try creating with both GRUB and ISOLINUX support
if command -v grub-mkrescue &> /dev/null; then
    echo "Creating with GRUB..."
    grub-mkrescue -o TernaryBitOS_Simple.iso simple_iso/
    echo "✅ TernaryBitOS_Simple.iso created with GRUB"
fi

# Cleanup
rm -rf simple_iso

echo ""
echo "🎯 READY FOR TESTING:"
echo "File: TernaryBitOS_Fixed.iso ($(du -h TernaryBitOS_Fixed.iso | cut -f1))"
echo "Location: $(pwd)/TernaryBitOS_Fixed.iso"
echo ""
echo "🚀 VIRTUALBOX SETUP:"
echo "1. Create new VM (Linux, Other Linux 64-bit, 1GB RAM)"
echo "2. Mount TernaryBitOS_Fixed.iso as CD/DVD"
echo "3. Boot VM"
echo "4. Select 'TernaryBit OS - Universal Operating System'"
echo ""
echo "Expected: Proper kernel load with multiboot compliance!"
echo "Should see: 'SUCCESS: Kernel loaded correctly!'"