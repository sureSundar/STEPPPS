#!/usr/bin/env bash
set -euo pipefail
ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
SRC_MD="$ROOT_DIR/RESEARCH_PAPER.md"
OUT_TEX="$ROOT_DIR/arxiv/RESEARCH_PAPER.tex"
TITLE="TBOS × STEPPPS: A Universal, Autonomously Orchestrated Operating Substrate for Phygital Systems"

if ! command -v pandoc >/dev/null 2>&1; then
  echo "pandoc not found; please install pandoc" >&2
  exit 2
fi

pandoc "$SRC_MD" -s -V title:"$TITLE" -o "$OUT_TEX"
echo "[arxiv] generated $OUT_TEX"
