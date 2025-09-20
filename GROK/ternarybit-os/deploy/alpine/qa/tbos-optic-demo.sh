#!/usr/bin/env bash
set -euo pipefail

log() { echo "[optic-demo] $*"; }

# 1) Mount PF2S and PXFS if possible
if command -v mount-pf2s >/dev/null 2>&1; then
  mkdir -p /mnt/pf /var/lib/pf2s
  if ! mount | grep -q "/mnt/pf"; then
    log "mounting PF2S at /mnt/pf"
    mount-pf2s /mnt/pf --backing /var/lib/pf2s >/dev/null 2>&1 &
    sleep 1
  fi
else
  log "PF2S helper not found; will use backing at /var/lib/pf2s"
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
  log "PXFS helper not found; will use backing at /var/lib/pxfs"
  mkdir -p /var/lib/pxfs
fi

# 2) Generate a synthetic spectrum into PF2S
FREQNM=550.0
log "scanning spectrum around ${FREQNM}nm into PF2S"
if ! tbos-sensorctl optic scan --range 520-580 --points 128 --center ${FREQNM} --out pf2s://${FREQNM}/spectrum.json; then
  log "warning: optic scan failed; continuing"
fi

# 3) Extract features (peaks) from the spectrum
log "extracting peaks"
steppps-tools transform spectrum.features pf2s://${FREQNM}/spectrum.json --max-peaks 5 --window 3 --mode minima > /tmp/spectrum_features.json || true
head -n 20 /tmp/spectrum_features.json 2>/dev/null || true

# 4) Pixel -> Spectrum via PXFS compute, then link to PF2S
log "computing spectrum.from_pixel via PXFS compute and linking to PF2S"
cat > /tmp/pixel.json <<'JSON'
{ "pixel": { "rgb": [200, 120, 40] } }
JSON
RES1=$(tbos-compute "pxfs://compute/spectrum.from_pixel/profile=/etc/tbos/display-profile.json/input=/tmp/pixel.json/out=spectrum.json" \
  --link pf2s://${FREQNM}/from_pixel.json || true)
log "from_pixel result -> ${RES1:-"(see PF2S)"}"

# 5) Spectrum -> Pixel via PXFS compute, link to PXFS
log "computing pixel.from_spectrum via PXFS compute and linking to PXFS"
RES2=$(tbos-compute "pxfs://compute/pixel.from_spectrum/profile=/etc/tbos/display-profile.json/clip=warn/input=pf2s://${FREQNM}/spectrum.json/out=pixel.json" \
  --link pxfs://demo/pixel_from_spec.json || true)
log "pixel_from_spectrum result -> ${RES2:-"(see PXFS backing)"}"

log "done. Inspect:"
log "  PF2S spectrum:      /mnt/pf/${FREQNM}/spectrum.json (or /var/lib/pf2s/${FREQNM}/spectrum.json)"
log "  PF2S from_pixel:     /mnt/pf/${FREQNM}/from_pixel.json (or /var/lib/pf2s/${FREQNM}/from_pixel.json)"
log "  PXFS pixel_from_spec: /mnt/px/demo/pixel_from_spec.json (or /var/lib/pxfs/<encoded>/pixel_from_spec.json)"
