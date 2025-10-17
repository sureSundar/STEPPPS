#!/usr/bin/env bash
set -euo pipefail

VC_FILE=docs/VC-23-09-2025.txt
REPO_ROOT=.

if ! command -v inotifywait >/dev/null 2>&1; then
  echo "[watcher] inotifywait not found. Install inotify-tools (sudo apt-get install -y inotify-tools)." | cat
  exit 1
fi

mkdir -p "$(dirname "$VC_FILE")"
{
  echo ""
  echo "@Watcher[start] $(date -u +"%Y-%m-%dT%H:%M:%SZ")"
} >> "$VC_FILE"

# Ignore common noise
IGNORE_REGEX='(^\./\.git/)|(^\./build/)|(^\./dist/)|(^\./node_modules/)|(^\./out/)|(^\./\.next/)|(~$)'

# Start watching
inotifywait -m -r -e modify,create,delete,move --format '%w %e %f' "$REPO_ROOT" | while IFS= read -r line; do
  dir="${line%% *}"
  rest="${line#* }"
  event="${rest%% *}"
  name="${rest#* }"
  file="${dir}${name}"
  rel="./${file#./}"
  if printf %s "$rel" | grep -Eq "$IGNORE_REGEX"; then
    continue
  fi
  ts=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
  printf "@Watcher[event] %s | %s | %s\n" "$ts" "$event" "$rel" >> "$VC_FILE"
done


