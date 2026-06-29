#!/bin/bash
#
# Build the TernaryBit OS Network Demo
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║     Building TernaryBit OS Network Demo                      ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# Create build directory
mkdir -p "$REPO_ROOT/build/tests"

# Compile network demo
echo "Compiling network demo..."

gcc -DHOST_BUILD -Wall -Wextra -g \
    -I"$REPO_ROOT/include" \
    -I"$REPO_ROOT/src" \
    -I"$REPO_ROOT/src/core" \
    -I"$REPO_ROOT/src/network/core" \
    -I"$REPO_ROOT/src/network/transport/tcp" \
    -I"$REPO_ROOT/src/network/protocols/http" \
    "$SCRIPT_DIR/demo_network.c" \
    "$REPO_ROOT/src/network/core/tbos_network.c" \
    "$REPO_ROOT/src/network/transport/tcp/tbos_tcp.c" \
    "$REPO_ROOT/src/network/protocols/http/tbos_http.c" \
    -o "$REPO_ROOT/build/tests/demo_network"

echo ""
echo "✅ Build successful!"
echo ""
echo "Run the demo:"
echo "  ./build/tests/demo_network"
echo ""
