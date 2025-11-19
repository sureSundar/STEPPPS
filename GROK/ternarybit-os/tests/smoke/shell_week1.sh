#!/usr/bin/env bash
#
# Smoke-test the Week-1 shell commands against the hosted calc_4k persona.
# Builds the persona if needed, pipes the navigation commands in, and
# validates that no errors or ❌ markers appear in the output.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
PERSONA="calc_4k"
BUILD_SCRIPT="$REPO_ROOT/build_tbos_system.sh"
PERSONA_BIN="$REPO_ROOT/build_integrated/${PERSONA}/tbos"

if [[ ! -x "$BUILD_SCRIPT" ]]; then
    echo "error: cannot find build_tbos_system.sh at $BUILD_SCRIPT" >&2
    exit 1
fi

echo "[smoke] Building persona '${PERSONA}'..."
"$BUILD_SCRIPT" --persona "$PERSONA" >/dev/null

if [[ ! -x "$PERSONA_BIN" ]]; then
    echo "error: persona binary missing at $PERSONA_BIN" >&2
    exit 1
fi

TMP_LOG="$(mktemp "${TMPDIR:-/tmp}/tbos-smoke-week1.XXXXXX")"
trap 'rm -f "$TMP_LOG"' EXIT

cat <<'EOF' >"$TMP_LOG.commands"
q
help
ls
pwd
mkdir demo_dir
ls
cd demo_dir
pwd
cd /
rmdir demo_dir
touch demo_file
ls
exit
EOF

echo "[smoke] Running Week-1 command sequence..."
if ! "$PERSONA_BIN" <"$TMP_LOG.commands" >"$TMP_LOG" 2>&1; then
    echo "error: persona execution failed, log follows:" >&2
    cat "$TMP_LOG" >&2
    exit 1
fi

if grep -q "❌" "$TMP_LOG"; then
    echo "error: smoke test detected failures (see log)" >&2
    cat "$TMP_LOG" >&2
    exit 1
fi

echo "[smoke] Week-1 shell commands succeeded."
echo "Log: $TMP_LOG"
trap - EXIT
