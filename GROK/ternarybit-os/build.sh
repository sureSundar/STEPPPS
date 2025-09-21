#!/bin/bash
# TBOS Build Script - Sprint 1
# 🕉️ Swamiye Saranam Aiyappa 🕉️

echo "======================================"
echo "    TBOS Sacred Build System"
echo "    Swamiye Saranam Aiyappa"
echo "======================================"

# Create build directory
mkdir -p build

# Assemble boot sector
echo "[BUILD] Assembling boot sector..."
nasm -f bin boot/tbos_sacred.asm -o build/boot.bin

# Assemble stage 2
echo "[BUILD] Assembling stage 2..."
nasm -f bin boot/stage2_sacred.asm -o build/stage2.bin

# Create disk image
echo "[BUILD] Creating disk image..."
dd if=/dev/zero of=build/tbos.img bs=1M count=10 2>/dev/null
dd if=build/boot.bin of=build/tbos.img conv=notrunc 2>/dev/null
dd if=build/stage2.bin of=build/tbos.img seek=1 conv=notrunc 2>/dev/null

echo "[BUILD] Build complete!"
echo ""
echo "To test: qemu-system-i386 -fda build/tbos.img"
echo ""
echo "🕉️ Sprint 1 Complete - Hariharaputram Saranam 🕉️"