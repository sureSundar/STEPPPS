#!/bin/bash
#
# Build and run the error handling test
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║     Building Error Handling Test                             ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

mkdir -p "$REPO_ROOT/build/tests"

gcc -DHOST_BUILD -Wall -Wextra -g \
    -I"$REPO_ROOT/src/rock/error" \
    "$SCRIPT_DIR/unit/test_error_handling.c" \
    "$REPO_ROOT/src/rock/error/error.c" \
    -o "$REPO_ROOT/build/tests/test_error_handling"

echo "✅ Build successful!"
echo ""
echo "Running tests..."
echo ""

"$REPO_ROOT/build/tests/test_error_handling"
