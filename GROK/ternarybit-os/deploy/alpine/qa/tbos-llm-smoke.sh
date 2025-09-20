#!/usr/bin/env bash
set -euo pipefail

# TBOS LLM smoke test: mounts RF2S/PXFS (if possible), calls tbos-llmctl to generate
# STEPPPS frames into rf2s:// and pxfs://, then validates outputs exist.

log() { echo "[llm-smoke] $*"; }
fail() { echo "[llm-smoke][FAIL] $*" >&2; exit 1; }

# Try to mount RF2S and PXFS (no-op if not available)
if command -v rf2s_fuse.py >/dev/null 2>&1; then
  mkdir -p /mnt/rf /var/lib/rf2s
  if ! mount | grep -q "/mnt/rf"; then
    log "mounting RF2S at /mnt/rf"
    rf2s_fuse.py --backing /var/lib/rf2s --unit MHz /mnt/rf >/dev/null 2>&1 &
    sleep 1
  fi
else
  log "rf2s_fuse.py not present; will write to backing"
fi

if command -v mount-pxfs >/dev/null 2>&1; then
  mkdir -p /mnt/px /var/lib/pxfs
  if ! mount | grep -q "/mnt/px"; then
    log "mounting PXFS at /mnt/px"
    mount-pxfs /mnt/px --sep 255,255,255 >/dev/null 2>&1 &
    sleep 1
  fi
else
  log "mount-pxfs not present; will write to backing"
fi

# Ensure proxy is up (if not, try to start it)
PROXY_URL="${TBOS_LLM_PROXY:-http://127.0.0.1:8088}"
if ! curl -fsS "$PROXY_URL/does-not-exist" >/dev/null 2>&1; then
  if command -v tbos-llm-proxy >/dev/null 2>&1; then
    log "starting tbos-llm-proxy"
    tbos-llm-proxy >/dev/null 2>&1 &
    sleep 2
  fi
fi

command -v tbos-llmctl >/dev/null 2>&1 || fail "tbos-llmctl missing"

# RF2S target
FREQ="99.500000"
log "generating STEPPPS frame to rf2s://${FREQ}/frame.json"
if ! tbos-llmctl steppps --intent "Create a minimal frame with wifi event" --out "rf2s://${FREQ}/frame.json"; then
  log "warning: /steppps returned non-ok (proxy fallback?)"
fi
# Check result exists in mount or backing
if [ -f "/mnt/rf/${FREQ}/frame.json" ] || [ -f "/var/lib/rf2s/${FREQ}/frame.json" ]; then
  log "RF2S frame OK"
else
  fail "RF2S frame not found"
fi

# PXFS target
log "generating STEPPPS frame to pxfs://A/B/C/frame.json"
if ! tbos-llmctl steppps --intent "Frame with environment summary and links" --out "pxfs://A/B/C/frame.json"; then
  log "warning: /steppps returned non-ok (proxy fallback?)"
fi
# Check result exists
if [ -f "/mnt/px/A/B/C/frame.json" ]; then
  log "PXFS frame OK (mounted)"
else
  # Check backing with encoded name
  if ls /var/lib/pxfs/*frame.json >/dev/null 2>&1; then
    log "PXFS frame OK (backing encoded)"
  else
    fail "PXFS frame not found"
  fi
fi

log "LLM smoke test completed"
