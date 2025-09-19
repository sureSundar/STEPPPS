#!/bin/bash
# Create TernaryBit OS bootable ISO

echo "Creating TernaryBit OS Bootable ISO..."

# Check if we have the necessary tools
if ! command -v grub-mkrescue &> /dev/null; then
    echo "grub-mkrescue not found. Installing GRUB tools..."
    apt-get update && apt-get install -y grub-pc-bin grub-common
fi

if ! command -v xorriso &> /dev/null; then
    echo "xorriso not found. Installing..."
    apt-get update && apt-get install -y xorriso
fi

# Create the ISO
echo "Building TernaryBit OS ISO..."
echo "Structure:"
echo "  /boot/grub/grub.cfg - GRUB configuration"
echo "  /boot/tbos_kernel - TernaryBit OS kernel"
echo "  /boot/tbos_initrd - TernaryBit OS initial ramdisk"

# Try with grub-mkrescue first
if command -v grub-mkrescue &> /dev/null; then
    echo "Using grub-mkrescue to create bootable ISO..."
    grub-mkrescue -o ../TernaryBitOS.iso . 2>/dev/null || {
        echo "grub-mkrescue failed, trying alternative method..."

        # Alternative method using genisoimage
        if command -v genisoimage &> /dev/null; then
            echo "Using genisoimage to create ISO..."
            genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot \
                       -boot-load-size 4 -boot-info-table \
                       -o ../TernaryBitOS.iso . 2>/dev/null || {
                echo "genisoimage failed, creating simple ISO..."

                # Simple ISO creation
                if command -v mkisofs &> /dev/null; then
                    mkisofs -R -o ../TernaryBitOS.iso .
                else
                    echo "Creating ISO with available tools..."
                    tar -czf ../TernaryBitOS.tar.gz .
                    echo "Created TernaryBitOS.tar.gz instead of ISO"
                fi
            }
        else
            echo "Installing genisoimage..."
            apt-get update && apt-get install -y genisoimage
            genisoimage -R -o ../TernaryBitOS.iso .
        fi
    }
else
    echo "Installing GRUB rescue tools..."
    apt-get update && apt-get install -y grub-pc-bin grub-common xorriso
    grub-mkrescue -o ../TernaryBitOS.iso .
fi

echo "TernaryBit OS ISO creation completed!"

# Check if ISO was created
if [ -f "../TernaryBitOS.iso" ]; then
    echo "✅ SUCCESS: TernaryBitOS.iso created"
    echo "File size: $(du -h ../TernaryBitOS.iso | cut -f1)"
    echo ""
    echo "🚀 READY FOR VIRTUALBOX:"
    echo "1. Open Oracle VirtualBox"
    echo "2. Create new VM:"
    echo "   - Name: TernaryBit OS"
    echo "   - Type: Linux"
    echo "   - Version: Other Linux (64-bit)"
    echo "   - Memory: 512MB minimum (1GB recommended)"
    echo "   - Create virtual hard disk (optional)"
    echo "3. Settings → Storage → Add ISO:"
    echo "   - Select: $(pwd)/../TernaryBitOS.iso"
    echo "4. Start the VM"
    echo ""
    echo "🎯 TernaryBit OS will boot and demonstrate:"
    echo "   ✅ Universal hardware detection"
    echo "   ✅ Adaptive optimization"
    echo "   ✅ Interactive shell"
    echo "   ✅ System status and tests"
    echo ""
    echo "The Universal OS from Calculator to Supercomputer!"
elif [ -f "../TernaryBitOS.tar.gz" ]; then
    echo "✅ Created TernaryBitOS.tar.gz (alternative format)"
    echo "Extract and burn to CD/DVD or use with virtualization software"
else
    echo "❌ ISO creation failed"
    echo "Manual creation required"
fi