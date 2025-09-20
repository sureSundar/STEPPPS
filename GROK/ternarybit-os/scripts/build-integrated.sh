#!/bin/bash
# TernaryBit OS - Integrated Build Script
# Builds complete TernaryBit OS with Buildroot Linux

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TBOS_DIR="$(dirname "$SCRIPT_DIR")"

echo "TernaryBit OS: Building integrated system..."

# Build TernaryBit bootloader with Linux support
echo "Building TernaryBit bootloader..."
cd "$TBOS_DIR"

# Create Linux-compatible bootloader
mkdir -p build
nasm -f bin boot/boot_compact.asm -o build/boot.bin
nasm -f bin boot/stage2_linux.asm -o build/stage2_linux.bin

echo "TernaryBit bootloader built successfully"
echo "Boot sector: build/boot.bin"
echo "Linux stage2: build/stage2_linux.bin"

# Check if Buildroot is built
BUILDROOT_DIR="../buildroot-2024.02"
if [ ! -f "$BUILDROOT_DIR/output/images/ternarybit-linux.img" ]; then
    echo "Building Buildroot Linux first..."
    ./scripts/build-buildroot.sh
fi

echo ""
echo "🎉 TernaryBit OS integration build completed!"
echo ""
echo "Available files:"
echo "  📀 $BUILDROOT_DIR/output/images/ternarybit-linux.img"
echo "  🚀 $BUILDROOT_DIR/output/images/run-ternarybit.sh"
echo "  🧪 $BUILDROOT_DIR/output/images/test-ternarybit.sh"
echo ""
echo "To run TernaryBit OS:"
echo "  $BUILDROOT_DIR/output/images/run-ternarybit.sh"