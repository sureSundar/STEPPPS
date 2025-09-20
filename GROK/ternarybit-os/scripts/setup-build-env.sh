#!/bin/bash
# Setup build environment for TernaryBit OS
# Traceability: PRD-048 (Development Environment Setup)

echo "🚀 Setting up TernaryBit OS build environment..."

# Check for required tools
check_tool() {
    if ! command -v $1 &> /dev/null; then
        echo "❌ Error: $1 is required but not installed"
        exit 1
    fi
    echo "✅ Found $1: $(which $1)"
}

# Check build tools
check_tool gcc
check_tool make
check_tool qemu-system-i386
check_tool gdb

# Create build directory
echo "📂 Setting up build directories..."
mkdir -p build tests/unit tests/integration

# Make setup script executable
chmod +x scripts/*.sh

echo "✨ Build environment setup complete!"
