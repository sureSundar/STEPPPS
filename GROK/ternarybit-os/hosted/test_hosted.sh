#!/bin/bash
# Non-interactive smoke test for the hosted TBOS shell

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SHELL_BIN="$REPO_ROOT/src/shell/universal/tbos_shell"
LOG_DIR="$REPO_ROOT/build/hosted"
LOG_FILE="$LOG_DIR/test.log"

if [[ ! -x "$SHELL_BIN" ]]; then
    echo "Hosted shell binary not found. Building now..."
    "$SCRIPT_DIR/build_hosted.sh"
fi

mkdir -p "$LOG_DIR"

echo "==> Running hosted shell smoke test…"
{
    echo "help"
    echo "pwd"
    echo "exit"
} | "$SHELL_BIN" >"$LOG_FILE" 2>&1

echo "✅ Hosted shell test completed. Log saved to $LOG_FILE"
