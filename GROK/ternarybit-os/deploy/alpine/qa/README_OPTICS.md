# TBOS Optics Quickstart (PF2S + STEPPPS + PXFS Compute)

This guide shows how to run the optics pipeline end-to-end on TBOS.

## Prereqs
- Built initramfs (x86_64 or arm64) from `deploy/alpine/build-*.sh`
- Boot with kernel + initramfs (see `deploy/alpine/qa/qemu-*.sh`)

## One-Command Demos

- Optics pipeline demo:
  ```sh
  bash deploy/alpine/qa/tbos-optic-demo.sh
  ```
  What it does:
  - Mounts PF2S/PXFS if available
  - `tbos-sensorctl optic scan` → PF2S spectrum
  - `steppps-tools transform spectrum.features` (peaks)
  - `tbos-compute` cached ops: `spectrum.from_pixel`, `pixel.from_spectrum`

- Optic link demo (stubs for modulate/receive + compute):
  ```sh
  bash deploy/alpine/qa/tbos-optic-link-demo.sh
  ```
  What it does:
  - `tbos-sensorctl optic modulate` → PF2S modulate.json
  - `tbos-sensorctl optic receive` → PF2S capture.json
  - Scan → Features, Spectrum ↔ Pixel transforms via `tbos-compute`

## Manual Steps

1) Mount PF2S/PXFS (optional but recommended)
```sh
mount-pf2s /mnt/pf --backing /var/lib/pf2s &
mount-pxfs /mnt/px --sep 255,255,255 &
```

2) Generate a spectrum to PF2S
```sh
tbos-sensorctl optic scan \
  --range 520-580 --points 128 --center 550.0 \
  --out pf2s://550.0/spectrum.json
```
Inspect: `/mnt/pf/550.000000/spectrum.json` (or `/var/lib/pf2s/550.000000/spectrum.json`)

3) Extract spectral features
```sh
steppps-tools transform spectrum.features pf2s://550.0/spectrum.json \
  --max-peaks 5 --window 3 --mode minima > /tmp/features.json
cat /tmp/features.json
```

4) Pixel → Spectrum (cached via PXFS compute)
```sh
echo '{"pixel":{"rgb":[200,120,40]}}' > /tmp/pixel.json

tbos-compute "pxfs://compute/spectrum.from_pixel/profile=/etc/tbos/display-profile.json/input=/tmp/pixel.json/out=spectrum.json" \
  --link pf2s://550.0/from_pixel.json
```
Inspect: `/mnt/pf/550.000000/from_pixel.json`

5) Spectrum → Pixel (cached via PXFS compute)
```sh
tbos-compute "pxfs://compute/pixel.from_spectrum/profile=/etc/tbos/display-profile.json/clip=warn/input=pf2s://550.0/spectrum.json/out=pixel.json" \
  --link pxfs://optic_demo/pixel_from_spec.json
```
Inspect: `/mnt/px/optic_demo/pixel_from_spec.json`

## Useful Paths & Logs
- PF2S spectra & artifacts: `/mnt/pf/<lambda_nm>/...` (or `/var/lib/pf2s/<lambda>/...`)
- PXFS artifacts: `/mnt/px/...` (or `/var/lib/pxfs/...` encoded)
- Optics logs: `/var/log/tbos/optic.jsonl`
- Display profile (override as needed): `/etc/tbos/display-profile.json`

## Notes
- `spectrum.from_pixel` and `pixel.from_spectrum` use a simplified panel model (Gaussian SPDs per primary, gamma). For accuracy, provide a real profile at `/etc/tbos/display-profile.json`.
- All compute paths are cached under `/var/lib/pxfs/compute/<sha256>/...` and reused across runs.
- If PF2S/PXFS aren’t mounted, scripts fall back to backing stores.
