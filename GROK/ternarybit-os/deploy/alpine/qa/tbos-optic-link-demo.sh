#!/usr/bin/env bash
set -euo pipefail

log() { echo "[optic-link-demo] $*"; }

# 0) Defaults
CENTER_NM=${CENTER_NM:-550.0}
RATE=${RATE:-120.0}
DURATION=${DURATION:-2.0}
AREA=${AREA:-0,0,100,100}
COLOR=${COLOR:-255,255,255}
PROFILE=${PROFILE:-/etc/tbos/display-profile.json}

# 1) Mount PF2S/PXFS if available
if command -v mount-pf2s >/dev/null 2>&1; then
  mkdir -p /mnt/pf /var/lib/pf2s
  if ! mount | grep -q "/mnt/pf"; then
    log "mounting PF2S at /mnt/pf"
    mount-pf2s /mnt/pf --backing /var/lib/pf2s >/dev/null 2>&1 &
    sleep 1
  fi
else
  log "PF2S helper not found; will use backing /var/lib/pf2s"
  mkdir -p /var/lib/pf2s
fi
if command -v mount-pxfs >/dev/null 2>&1; then
  mkdir -p /mnt/px /var/lib/pxfs
  if ! mount | grep -q "/mnt/px"; then
    log "mounting PXFS at /mnt/px"
    mount-pxfs /mnt/px --sep 255,255,255 >/dev/null 2>&1 &
    sleep 1
  fi
else
  log "PXFS helper not found; will use backing /var/lib/pxfs"
  mkdir -p /var/lib/pxfs
fi

# 2) Modulate and Receive (stubs write PF2S artifacts)
log "modulating display area ${AREA} at ${RATE}Hz for ${DURATION}s"
tbos-sensorctl optic modulate --pattern pwm --rate ${RATE} --duration ${DURATION} \
  --area ${AREA} --color ${COLOR} --center ${CENTER_NM} --out pf2s://${CENTER_NM}/modulate.json || true

log "receiving optical signal for ${DURATION}s"
tbos-sensorctl optic receive --duration ${DURATION} --center ${CENTER_NM} --out pf2s://${CENTER_NM}/capture.json || true

# 3) Scan spectrum and extract features
log "scanning spectrum and extracting features"
tbos-sensorctl optic scan --range $(echo "${CENTER_NM}-$(echo ${CENTER_NM} | awk '{print $1+30}')") --points 128 \
  --center ${CENTER_NM} --out pf2s://${CENTER_NM}/spectrum.json || true
steppps-tools transform spectrum.features pf2s://${CENTER_NM}/spectrum.json --max-peaks 5 --window 3 --mode minima > /tmp/spectrum_features.json || true
head -n 20 /tmp/spectrum_features.json 2>/dev/null || true

# 4) Cached PXFS compute: spectrum -> pixel best-fit, and pixel -> spectrum
log "computing pixel.from_spectrum (cached)"
RES_PIX=$(tbos-compute "pxfs://compute/pixel.from_spectrum/profile=${PROFILE}/clip=warn/input=pf2s://${CENTER_NM}/spectrum.json/out=pixel.json" \
  --link pxfs://optic_demo/pixel_from_spec.json || true)
log "pixel_from_spec -> ${RES_PIX:-"(see PXFS/backing)"}"

log "computing spectrum.from_pixel (cached)"
cat > /tmp/seed_pixel.json <<EOF
{ "pixel": { "rgb": [200, 120, 40] } }
EOF
RES_SPEC=$(tbos-compute "pxfs://compute/spectrum.from_pixel/profile=${PROFILE}/input=/tmp/seed_pixel.json/out=spectrum.json" \
  --link pf2s://${CENTER_NM}/spectrum_from_pixel.json || true)
log "spectrum_from_pixel -> ${RES_SPEC:-"(see PF2S/backing)"}"

log "Done. Inspect:"
log "  PF2S modulate:        /mnt/pf/${CENTER_NM}/modulate.json (or /var/lib/pf2s/${CENTER_NM}/modulate.json)"
log "  PF2S capture:         /mnt/pf/${CENTER_NM}/capture.json (or /var/lib/pf2s/${CENTER_NM}/capture.json)"
log "  PF2S spectrum:        /mnt/pf/${CENTER_NM}/spectrum.json"
log "  PF2S spectrum_from_pixel: /mnt/pf/${CENTER_NM}/spectrum_from_pixel.json"
log "  PXFS pixel_from_spec: /mnt/px/optic_demo/pixel_from_spec.json"
log "  Logs:                 /var/log/tbos/optic.jsonl"
