#!/bin/bash

# TernaryBit OS - Linux Integration Build Script
# Creates TBOS → Linux chain-loading bootable image

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
BUILDROOT_DIR="/tmp/buildroot-2024.02"

echo "🕉️ TernaryBit OS - Linux Integration Builder"
echo "Building consciousness-based computing → Linux chain-loading system"

# Create build directory
mkdir -p "$BUILD_DIR"

# Function to check if Buildroot kernel is ready
check_buildroot_kernel() {
    if [ -f "$BUILDROOT_DIR/output/images/bzImage" ]; then
        echo "✅ Buildroot Linux kernel found: $BUILDROOT_DIR/output/images/bzImage"
        return 0
    else
        echo "⏳ Buildroot kernel not ready yet. Current status:"
        if [ -d "$BUILDROOT_DIR" ]; then
            echo "   Buildroot directory exists, checking build progress..."
            tail -5 "$BUILDROOT_DIR/build_progress_base.log" 2>/dev/null || echo "   No build log found"
        else
            echo "   Buildroot not found"
        fi
        return 1
    fi
}

# Function to build TBOS Linux chain-loader
build_tbos_linux_chainloader() {
    echo "🔧 Building TBOS Linux chain-loading bootloader..."

    cd "$PROJECT_ROOT"

    # Assemble Linux chain-loading stage2
    echo "Assembling Linux chain-loading stage2..."
    nasm -f bin boot/stage2_linux_chain.asm -o "$BUILD_DIR/stage2_linux.bin"

    # Assemble boot sector (using existing)
    echo "Assembling boot sector..."
    nasm -f bin boot/boot_compact.asm -o "$BUILD_DIR/boot_sector.bin"

    # Create disk image
    echo "Creating TBOS-Linux disk image..."
    dd if=/dev/zero of="$BUILD_DIR/tbos_linux.img" bs=512 count=2880 2>/dev/null

    # Install boot sector
    dd if="$BUILD_DIR/boot_sector.bin" of="$BUILD_DIR/tbos_linux.img" bs=512 count=1 conv=notrunc 2>/dev/null

    # Install Linux chain-loading stage2 (sectors 2-9)
    dd if="$BUILD_DIR/stage2_linux.bin" of="$BUILD_DIR/tbos_linux.img" bs=512 seek=1 count=8 conv=notrunc 2>/dev/null

    echo "✅ TBOS Linux chain-loader built: $BUILD_DIR/tbos_linux.img"
}

# Function to integrate Buildroot kernel
integrate_buildroot_kernel() {
    echo "🔗 Integrating Buildroot Linux kernel..."

    local kernel_path="$BUILDROOT_DIR/output/images/bzImage"
    local initrd_path="$BUILDROOT_DIR/output/images/rootfs.cpio"

    # Calculate kernel size in sectors
    local kernel_size=$(stat -c%s "$kernel_path")
    local kernel_sectors=$(( (kernel_size + 511) / 512 ))

    echo "   Kernel size: $kernel_size bytes ($kernel_sectors sectors)"

    # Write kernel to disk image starting at sector 10
    dd if="$kernel_path" of="$BUILD_DIR/tbos_linux.img" bs=512 seek=9 conv=notrunc 2>/dev/null

    # If initrd exists, write it after kernel
    if [ -f "$initrd_path" ]; then
        local initrd_size=$(stat -c%s "$initrd_path")
        local initrd_sectors=$(( (initrd_size + 511) / 512 ))
        local initrd_start_sector=$(( 9 + kernel_sectors ))

        echo "   Initrd size: $initrd_size bytes ($initrd_sectors sectors)"
        echo "   Writing initrd at sector $initrd_start_sector"

        dd if="$initrd_path" of="$BUILD_DIR/tbos_linux.img" bs=512 seek=$initrd_start_sector conv=notrunc 2>/dev/null
    fi

    echo "✅ Buildroot kernel integrated successfully"
}

# Function to test the integration
test_tbos_linux() {
    echo "🧪 Testing TBOS → Linux integration..."

    if command -v qemu-system-i386 >/dev/null 2>&1; then
        echo "Starting QEMU test (use Ctrl+C to exit)..."
        echo "Expected sequence:"
        echo "1. TBOS consciousness awakening"
        echo "2. STEPPPS 7-dimensional activation"
        echo "3. Linux kernel chain-loading"
        echo "4. Buildroot Linux boot"
        echo ""
        echo "Press any key to start QEMU test..."
        read -n 1

        qemu-system-i386 -fda "$BUILD_DIR/tbos_linux.img" -boot a -m 64M
    else
        echo "⚠️ QEMU not available for testing"
        echo "Image ready: $BUILD_DIR/tbos_linux.img"
    fi
}

# Main execution flow
main() {
    echo "Step 1: Building TBOS Linux chain-loader..."
    build_tbos_linux_chainloader

    echo ""
    echo "Step 2: Checking for Buildroot kernel..."
    if check_buildroot_kernel; then
        echo ""
        echo "Step 3: Integrating Buildroot kernel..."
        integrate_buildroot_kernel

        echo ""
        echo "🎉 TBOS → Linux Integration Complete!"
        echo "   Boot image: $BUILD_DIR/tbos_linux.img"
        echo "   Features:"
        echo "   ✅ STEPPPS 7-dimensional consciousness computing"
        echo "   ✅ Dharmic computing philosophy"
        echo "   ✅ Linux kernel chain-loading"
        echo "   ✅ Buildroot userspace"
        echo ""

        read -p "Would you like to test in QEMU? (y/n): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            test_tbos_linux
        fi
    else
        echo ""
        echo "⏳ Buildroot kernel not ready yet."
        echo "   TBOS chain-loader prepared: $BUILD_DIR/tbos_linux.img"
        echo "   Re-run this script once Buildroot compilation completes."
        echo ""
        echo "   To monitor Buildroot progress:"
        echo "   tail -f $BUILDROOT_DIR/build_progress_base.log"
    fi
}

# Execute main function
main "$@"