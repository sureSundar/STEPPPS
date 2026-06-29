#!/bin/bash
# Build TernaryBit OS Hosted Linux Target
# Traceability: v4.0/ROADMAP.md V4-006

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_ROOT}/build/hosted"
ARTIFACTS_DIR="${PROJECT_ROOT}/artifacts/hosted"

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║   Building TernaryBit OS - Hosted Linux Target               ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# Create directories
mkdir -p "$BUILD_DIR"
mkdir -p "$ARTIFACTS_DIR"

# Compiler flags
CC="${CC:-gcc}"
CFLAGS="-Wall -Wextra -O2 -g"
CFLAGS="$CFLAGS -I${PROJECT_ROOT}/include"
CFLAGS="$CFLAGS -DHOSTED_BUILD"
CFLAGS="$CFLAGS -D_GNU_SOURCE"

# Build hosted Linux stub
echo "[BUILD] Compiling hosted Linux stub..."
$CC $CFLAGS \
    "${SCRIPT_DIR}/tbos_hosted_linux.c" \
    -o "${BUILD_DIR}/tbos-linux"

echo "[BUILD] Build successful!"

# Copy to artifacts
cp "${BUILD_DIR}/tbos-linux" "${ARTIFACTS_DIR}/tbos-linux"
chmod +x "${ARTIFACTS_DIR}/tbos-linux"

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║   Build Complete                                             ║"
echo "╠══════════════════════════════════════════════════════════════╣"
echo "║   Binary:  ${ARTIFACTS_DIR}/tbos-linux"
echo "║   Size:    $(du -h "${ARTIFACTS_DIR}/tbos-linux" | cut -f1)"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""
echo "Run with: ${ARTIFACTS_DIR}/tbos-linux"
echo ""
