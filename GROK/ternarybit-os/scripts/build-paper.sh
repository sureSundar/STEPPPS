#!/usr/bin/env bash
set -euo pipefail
ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
OUT_DIR="$ROOT_DIR/out/paper"
SRC_MD="$ROOT_DIR/RESEARCH_PAPER.md"
TITLE="TBOS × STEPPPS: A Universal, Autonomously Orchestrated Operating Substrate for Phygital Systems"

mkdir -p "$OUT_DIR"

have() { command -v "$1" >/dev/null 2>&1; }

if have pandoc; then
  echo "[build-paper] pandoc found: $(pandoc --version | head -n1)"
  # HTML
  pandoc "$SRC_MD" \
    -s -V title:"$TITLE" \
    -o "$OUT_DIR/RESEARCH_PAPER.html"
  # PDF: prefer wkhtmltopdf or weasyprint engine if available; else default
  PDF_ENGINE=""
  if have wkhtmltopdf; then PDF_ENGINE="--pdf-engine=wkhtmltopdf"; fi
  if have weasyprint; then PDF_ENGINE="--pdf-engine=weasyprint"; fi
  pandoc "$SRC_MD" -s -V title:"$TITLE" $PDF_ENGINE -o "$OUT_DIR/RESEARCH_PAPER.pdf" || {
    echo "[build-paper] PDF build failed; continuing with HTML only" >&2
  }
  echo "[build-paper] artifacts in $OUT_DIR"
else
  echo "[build-paper] pandoc not found. Install pandoc to render PDF/HTML."
  exit 2
fi
