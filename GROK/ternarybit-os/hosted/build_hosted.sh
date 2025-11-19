#!/bin/bash
# Build the TBOS hosted (universal shell) binary on Linux/macOS

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SHELL_DIR="$REPO_ROOT/src/shell/universal"

echo "==> Building TBOS hosted shell (profile_host)…"
echo "    Source directory: $SHELL_DIR"

make -C "$SHELL_DIR"

echo ""
echo "✅ Hosted shell ready:"
echo "   $SHELL_DIR/tbos_shell"
echo "Run './hosted/test_hosted.sh' or invoke the binary directly."
