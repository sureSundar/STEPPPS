# TernaryBit OS – Version 4.0 Kickoff

## Vision
Deliver a **Universal Boot** experience that works on:
1. **Bare-metal hardware** (BIOS/UEFI, legacy + modern x86, future ARM/RISC-V targets)
2. **Hosted environments** (Linux/Windows/macOS processes, containers, WASM, hypervisors)
3. **Application sandboxes** (VirtualBox, QEMU, emulators, browser-based VMs)

Regardless of where TBOS starts, it must:
- Initialize its own HAL stack
- Bring up console + shell + filesystem
- Negotiate network/IPC channels when the platform allows
- Expose the same “TBOS interface contract” to higher layers

## Baseline Reality Check (Nov 2025)
- v1/v2 achieved “boot-to-shell” on BIOS/QEMU but only with a fixed kernel.
- v3 introduced hardware-fluid concepts but the shipped floppy (`ternarybit.img`) still contains a static consciousness banner—no shell.
- `build_universal.sh` already builds a richer kernel (shell/fs/net). We will treat that as the canonical payload for v4.

## v4.0 Guiding Principles
1. **Single Source of Truth** – One kernel + driver tree, parameterized by profiles.
2. **Pluggable Stage 0/1/2** – Stage chain composes per target (BIOS, UEFI PEI, hosted stub, WASM loader, etc.) but shares TBOS Boot HAL definitions.
3. **Environment Detection Contracts** – Each stage writes a _Boot Capability Block_ that the kernel can query to discover CPU mode, virtual vs physical, available devices, timing sources and host IPC hooks.
4. **Reproducible Builds** – Every target (`baremetal`, `virtualbox`, `hosted-linux`, `hosted-win`, `wasm`) is produced via a single orchestrator script with explicit artifacts.
5. **Test Before Trust** – For each target we keep a tiny health script that boots the artifact and drops into the shell, reporting success via serial/log.

## Immediate Objectives
1. **Document boot flows** for the three deployment classes.
2. **Create orchestration scaffolding** (`build_v4.sh`) that drives `build_universal.sh`, then repackages artifacts for each target.
3. **Define capability descriptors** (drafted as `BCB v1`) that Stage 2 and hosted loaders must populate.
4. **Track bring-up tasks** in a living checklist (`v4.0/ROADMAP.md`).

## Repository Additions (this commit)
- `v4.0/README.md` (this file) – captures goals/constraints.
- `v4.0/BOOTSTRAP_PLAN.md` – phased plan for boot stages & targets.
- `v4.0/ROADMAP.md` – actionable checklist with status columns.
- `build_v4.sh` – top-level scaffold for producing multi-target artifacts (currently wraps `build_universal.sh`, emits TODOs for per-target packaging).

## Next Steps
1. Flesh out the BCB structure + serialization (shared header in `include/tbos/bcb.h`).
2. Teach `build_v4.sh` to emit:
   - `artifacts/baremetal/tbos-bios.img`
   - `artifacts/hosted/tbos-linux.bin`
   - `artifacts/hosted/tbos-win.exe` (using the hosted stub already in `tbos_universal_bootstrap.c`)
   - `artifacts/wasm/tbos.wasm`
3. Stand up smoke-test scripts under `tests/v4/`.
