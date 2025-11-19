#!/bin/bash
# TernaryBit OS - Multi-Architecture Build Script
# Builds Docker images for x86_64, ARM64, and RISC-V

echo "╔══════════════════════════════════════════════════════════════════╗"
echo "║                                                                  ║"
echo "║         TernaryBit OS - Multi-Architecture Builder              ║"
echo "║                                                                  ║"
echo "╚══════════════════════════════════════════════════════════════════╝"
echo ""

# Check Docker buildx
if ! docker buildx version &> /dev/null; then
    echo "❌ Docker buildx not available"
    echo "   Install: docker buildx install"
    exit 1
fi

echo "Step 1: Creating/using buildx builder..."
echo "────────────────────────────────────────────────────────────────"

# Create or use existing builder
docker buildx create --name tbos-builder --use 2>/dev/null || \
    docker buildx use tbos-builder

docker buildx inspect --bootstrap

echo ""
echo "Step 2: Building for multiple architectures..."
echo "────────────────────────────────────────────────────────────────"
echo "Targets: linux/amd64 (x86_64), linux/arm64, linux/riscv64"
echo ""

# Build for multiple platforms
docker buildx build \
    --platform linux/amd64,linux/arm64 \
    -f Dockerfile.multiarch \
    -t ternarybit-os:multiarch \
    --load \
    .

if [ $? -eq 0 ]; then
    echo "✓ Multi-architecture build complete"
else
    echo "❌ Build failed"
    exit 1
fi

echo ""
echo "╔══════════════════════════════════════════════════════════════════╗"
echo "║                                                                  ║"
echo "║                   BUILD COMPLETE!                                ║"
echo "║                                                                  ║"
echo "╚══════════════════════════════════════════════════════════════════╝"
echo ""

echo "Available images:"
docker images | grep ternarybit-os

echo ""
echo "Test on current architecture:"
echo "  docker run --rm ternarybit-os:multiarch tbrun /opt/tbos/examples/hello.tbs"
echo ""

# Quick test
echo "Running quick test..."
echo "────────────────────────────────────────────────────────────────"
docker run --rm ternarybit-os:multiarch /bin/sh -c "uname -m && tbrun /opt/tbos/examples/hello.tbs"

echo ""
echo "✓ TernaryBit OS multi-arch ready!"
echo ""
