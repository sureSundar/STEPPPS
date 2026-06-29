#!/bin/bash
#
# Build and run the memory reclamation test
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║     Building Memory Reclamation Test                         ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

mkdir -p "$REPO_ROOT/build/tests"

gcc -DHOST_BUILD -Wall -Wextra -g \
    -I"$REPO_ROOT/src/rock/memory" \
    -I"$REPO_ROOT/src/rock/error" \
    "$SCRIPT_DIR/unit/test_memory_reclaim.c" \
    "$REPO_ROOT/src/rock/memory/memory_reclaim.c" \
    "$REPO_ROOT/src/rock/error/error.c" \
    -o "$REPO_ROOT/build/tests/test_memory_reclaim" \
    -lpthread

echo "✅ Build successful!"
echo ""
echo "Running tests..."
echo ""

"$REPO_ROOT/build/tests/test_memory_reclaim"
