#!/bin/bash
#
# TernaryBit OS - Interactive Feature Demo
# Demonstrates all features with visual output
#
# Usage: ./tests/demo_all_features.sh
#

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SHELL_BIN="$REPO_ROOT/src/shell/universal/tbos_shell"

# Check if shell exists
if [[ ! -x "$SHELL_BIN" ]]; then
    echo "Building shell first..."
    cd "$REPO_ROOT/src/shell/universal" && make clean && make
    cd "$REPO_ROOT"
fi

# Create demo commands file
DEMO_COMMANDS=$(mktemp)
trap 'rm -f "$DEMO_COMMANDS"' EXIT

cat > "$DEMO_COMMANDS" << 'DEMO'
help
pwd
ls -la
uname -a
whoami
id
groups
date
cal
uptime
hostname
df -h
du -h .
ps
env | head -10
echo "Hello from TernaryBit OS!"
echo "Testing text processing..."
echo -e "zebra\napple\nbanana" | sort
echo -e "hello world" | rev
echo -e "hello world" | tr a-z A-Z
echo "ABCD" | hexdump
echo "Testing complete!"
exit
DEMO

echo ""
echo "╔══════════════════════════════════════════════════════════════════╗"
echo "║     TernaryBit OS - Interactive Feature Demonstration            ║"
echo "║                                                                  ║"
echo "║     Watch as we demonstrate the Universal Shell capabilities    ║"
echo "╚══════════════════════════════════════════════════════════════════╝"
echo ""
echo "Starting demo in 3 seconds..."
sleep 3

# Run the demo
"$SHELL_BIN" < "$DEMO_COMMANDS"

echo ""
echo "╔══════════════════════════════════════════════════════════════════╗"
echo "║                    Demo Complete!                                ║"
echo "║                                                                  ║"
echo "║  To explore interactively, run:                                  ║"
echo "║    ./src/shell/universal/tbos_shell                              ║"
echo "║                                                                  ║"
echo "║  To run full test suite:                                         ║"
echo "║    ./tests/test_all_commands.sh                                  ║"
echo "╚══════════════════════════════════════════════════════════════════╝"
echo ""
