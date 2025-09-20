#!/bin/bash
# TernaryBit OS - Buildroot Linux Build Script
# Builds minimal Linux system for TernaryBit integration

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TBOS_DIR="$(dirname "$SCRIPT_DIR")"
BUILDROOT_DIR="../buildroot-2024.02"

echo "TernaryBit OS: Building Buildroot Linux..."

# Check if Buildroot is available
if [ ! -d "$BUILDROOT_DIR" ]; then
    echo "Error: Buildroot not found at $BUILDROOT_DIR"
    echo "Please extract Buildroot 2024.02 in the parent directory"
    exit 1
fi

cd "$BUILDROOT_DIR"

# Copy TernaryBit configuration selectively
echo "Setting up TernaryBit configuration..."
cp -r "$TBOS_DIR/buildroot/configs" ./
cp -r "$TBOS_DIR/buildroot/board" ./

# Configure Buildroot for TernaryBit
echo "Configuring Buildroot..."
make ternarybit_defconfig

# Build the system
echo "Building Linux system (this may take 30-60 minutes)..."
make -j$(nproc)

echo "Buildroot build completed successfully!"
echo "Linux kernel: output/images/bzImage"
echo "Root filesystem: output/images/rootfs.ext4"
echo "Integrated image: output/images/ternarybit-linux.img"