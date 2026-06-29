#!/bin/bash
#
# Build and run the network module test
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║     Building Network Module Test                             ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

mkdir -p "$REPO_ROOT/build/tests"

# Collect all source files needed
SOURCES=(
    "$SCRIPT_DIR/unit/test_network_module.c"
    "$REPO_ROOT/src/network/core/tbos_network.c"
    "$REPO_ROOT/src/network/sangha/tbos_sangha.c"
    "$REPO_ROOT/src/core/tbos_base.c"
    "$REPO_ROOT/src/core/tbos_modules.c"
)

# Include paths
INCLUDES=(
    "-I$REPO_ROOT/src/network/core"
    "-I$REPO_ROOT/src/network/sangha"
    "-I$REPO_ROOT/src/core"
    "-I$REPO_ROOT/src/rock/memory"
    "-I$REPO_ROOT/src/rock/error"
)

echo "Building with full dependencies..."

# Build the network test
if gcc -DHOST_BUILD -Wall -Wextra -g \
    "${INCLUDES[@]}" \
    "${SOURCES[@]}" \
    -o "$REPO_ROOT/build/tests/test_network_module" \
    -lpthread 2>&1; then
    echo "✅ Build successful!"
else
    echo ""
    echo "⚠️  Build failed, checking errors..."

    # Try compile-only to see errors
    gcc -DHOST_BUILD -Wall -c \
        "${INCLUDES[@]}" \
        "$REPO_ROOT/src/network/core/tbos_network.c" \
        -o /tmp/tbos_network.o 2>&1 || true

    echo "❌ Build failed - check error messages above"
    exit 1
fi

echo ""
echo "Running tests..."
echo ""

"$REPO_ROOT/build/tests/test_network_module"
