# TBOS Sensor Control Plane (Spec)

Goals
- Provide a unified CLI (`tbos-sensorctl`) to calibrate and control sensors: WiFi, Microphone, Bluetooth, and SDR devices.
- Expose measurements and streams via `RF2S` filesystem and JSON logs under `/var/log/tbos/`.
- Enforce safety/regulatory constraints: scanning is generally OK; transmission constrained by hardware and regional laws.

Scope (Linux tier)
- WiFi: passive scan (channel sweeps), RSSI, noise floor estimates, optional spectral scan if hardware/driver supports.
- Microphone: capture + frequency analysis; publish dominant frequencies or spectra; bridge to `RF2S` at e.g. `/mnt/rf/<freq>/audio.raw`.
- Bluetooth: discovery and advertisement control. Transmission of AM/FM over Bluetooth is NOT RF broadcast; we can stream audio over BT profiles (A2DP) but cannot radiate AM/FM bands. For true RF TX you need SDR (e.g., HackRF) and legal authorization.
- SDR (optional): rtl-sdr (RX), HackRF/USRP (TX/RX). Provide hooks only; default disabled.

Interfaces
- CLI: `tbos-sensorctl <domain> <command> [opts]`
  - wifi: `scan`, `sweep`, `setchan`, `info`
  - mic: `record`, `analyze`, `monitor`, `bridge-rf2s`
  - bt: `scan`, `advertise`, `pair`, `info`
  - sdr: `rx`, `tx` (gated; require explicit `--i-accept-regulatory-risk`)
- RF2S Bridge: helper scripts place files under a mounted RF2S backing path (default `/var/lib/rf2s`).

Dependencies (installed via first boot when enabled)
- WiFi: `iw`, `wireless-tools` (optional)
- BT: `bluez`, `bluez-deprecated` (if needed)
- Audio: `alsa-utils`, `sox`
- SDR: `rtl-sdr`, `hackrf` (optional)

Safety & Regulatory
- Scanning (passive) is generally permitted but follow local rules.
- Transmitting (e.g., arbitrary RF) requires appropriate hardware and licenses. `tbos-sensorctl sdr tx` is blocked by default and requires `--i-accept-regulatory-risk` plus whitelist of frequencies and power levels in `/etc/tbos/radio-policy.json`.

Micro-tier Strategy
- Provide stubs that report "unsupported" or route via a TBOS orchestrator.
- For audio, basic frequency estimation can be implemented using integer FFT or Goertzel; RF2S bridge remains a simple file API.

Acceptance Criteria
- `tbos-sensorctl wifi scan` lists networks on supported adapters.
- `tbos-sensorctl mic analyze` prints dominant bins and writes JSON to `/var/log/tbos/mic.jsonl`.
- `tbos-sensorctl bt scan` lists nearby devices (if BT present).
- RF2S bridge writes under backing path and visible via `rf2s_fuse.py` mount.
