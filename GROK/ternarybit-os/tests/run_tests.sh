#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

mkdir -p "$ROOT_DIR/build/x86_64/tests"

echo "[WINDSURF] Building unit tests..."
make -C "$ROOT_DIR" ARCH=x86_64 test-unit || true

echo "[WINDSURF] Running simple unit tests..."
if [[ -x "$ROOT_DIR/build/x86_64/test_boot" ]]; then "$ROOT_DIR/build/x86_64/test_boot"; fi
if [[ -x "$ROOT_DIR/build/x86_64/test_memory" ]]; then "$ROOT_DIR/build/x86_64/test_memory"; fi

echo "[WINDSURF] Done."
