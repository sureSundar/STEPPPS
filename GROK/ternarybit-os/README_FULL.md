# TBOS (TernaryBit OS) × STEPPPS: Universal Autonomous Framework

This document is the complete, canonical guide for the TBOS + STEPPPS repository. It blends vision, architecture, features, operations, reliability, and roadmap into one reference.

Contents
- Vision and Principles
- STEPPPS Overview (7 Dimensions)
- Architecture (Layers and Files)
- Key Features and Subsystems
- Build, Boot, and Demos
- Micro LLM (Constrained STEPPPS)
- Filesystems: PXFS, RF2S, PF2S
- Compute on PXFS (Caching + Offload)
- Sensors and Optics Pipeline
- Personas and Profiles
- Reliability and Health
- Developer Guide
- Roadmap

Vision and Principles
- Give “life” to devices using the STEPPPS seven-dimensional model: Space, Time, Event, Psychology, Pixel, Prompt, Script.
- Predictable behavior, graceful degradation, comprehensive monitoring, self‑healing (TBOS core ethos).
- Universal deployment: from calculators to servers. On-device when possible; offload when wise.
- Single‑file simplicity per component when helpful; small moving parts and composable tools.

STEPPPS Overview
- Space: where/what the system is interacting with (device, context, topology)
- Time: when/how things change; scheduling and temporal behavior
- Event: what happened; ingest, emit, correlate
- Psychology: motives/profiles/personas; runtime adaptation
- Pixel: visual, image, and raster representation; color science
- Prompt: LLM + constraints; translate intent to structured STEPPPS objects
- Script: orchestrations; repeatable, inspectable procedures

Architecture at a Glance
- Deploy layer: `deploy/alpine/`
  - Build scripts: `build-x86_64.sh`, QEMU QA, init, firstboot, shell
  - Sensors: `tbos-sensorctl.py` + sensord daemons
  - Filesystems (FUSE prototypes): PXFS, RF2S, PF2S + helpers
  - LLM: `tbos-llm-proxy.py`, `tbos-llmctl` + grammar/spec
  - Compute: `tbos-compute.py`, `tbos-jobd.py`, `tbos-offloadd.py`
- STEPPPS tools/runtime: `steppps/tools/steppps-tools.py` and schemas
- Docs: `docs/`, `deploy/alpine/qa/README_OPTICS.md`

Key Features
- Micro LLM with GBNF grammar to emit valid STEPPPS frames; JSON-schema validation and RF2S/PXFS I/O
- PXFS (Path eXtensible FS) with compute-on-demand and content-address caching
- RF2S (Radio Frequency File System) and PF2S (Photon File System) to treat signals as first‑class artifacts
- Job queue + offload to peers; results return to declared FS paths
- Optics pipeline: spectra generation, features, pixel↔spectrum transforms; cached via PXFS compute
- Persona-aware shell: `default`, `tiny`, `calc`

Build and Boot
- Build (x86_64): `deploy/alpine/build-x86_64.sh`
- Boot (QEMU): `deploy/alpine/qa/qemu-x86_64.sh --kernel /path/to/bzImage --initrd ../out/x86_64/initramfs.cpio.gz`
- On boot: `steppps-init` starts daemons per `TBOS_DAEMONS` in `etc/tbos/features.conf`

Micro LLM
- Proxy: `deploy/alpine/llm/tbos-llm-proxy.py` (127.0.0.1:8088)
  - `/generate`: pass-through to llama.cpp server; optional grammar
  - `/steppps`: constrained using `steppps_json.gbnf` → schema validation → optional write to RF2S/PXFS
- CLI: `tbos-llmctl`
- Grammar: `steppps_json.gbnf`
- Spec: `LLM_SPEC.md`

Filesystems
- PXFS: virtual paths encoding operations, inputs, and outputs; compute and cache
- RF2S: tree rooted by frequency; artifacts for radio/audio/SDR
- PF2S: tree rooted by wavelength/frequency; spectra and optic artifacts
- Mount helpers in `deploy/alpine/fs/helpers/`

Compute on PXFS
- Tool: `deploy/alpine/compute/tbos-compute.py`
- Ops (cached):
  - `steppps.transform` (pixel ops; spectrum.features)
  - `spectrum.from_pixel`, `pixel.from_spectrum`
  - `spectrum.features`
- Job daemon: `tbos-jobd.py` scans `/var/lib/tbos/jobs/` and `/mnt/px/jobs/`
- Offload daemon: `tbos-offloadd.py` listens on 0.0.0.0:9090

Sensors and Optics
- `tbos-sensorctl optic scan` → PF2S spectra
- `steppps-tools transform spectrum.features` → peaks JSON
- `spectrum.from_pixel` and `pixel.from_spectrum` (panel profile at `/etc/tbos/display-profile.json`)
- Demos: `deploy/alpine/qa/tbos-optic-demo.sh`, `tbos-optic-link-demo.sh`

Personas and Profiles
- `tbos-shell`: `mode default|tiny|calc`
- Profiles: `tbos-profile` (apply, list); hotkeys via `tbos-hotkeys`

Reliability and Health
- Init `steppps-init` launches daemons per `TBOS_DAEMONS`
- JSONL logs under `/var/log/tbos/`
- Design for fail-closed and fallbacks (e.g., LLM rules template if server missing)

Developer Guide
- Coding style: small modules, readable CLI, explicit logs
- Testing: run demos + scripts; unit tests may be added per tool
- Contributing: propose new FS ops, transforms, personas, and daemons as single files

Roadmap
- Health endpoints and supervision/backoff for daemons
- More transforms (merge/mask/threshold variants; wave transforms)
- Better display profiles and calibrated models
- Peer discovery and signed offload envelopes
- Multi-device STEPPPS orchestration
