# Universal POST ➜ TBOS Loading Guide

**Purpose:** Document a repeatable boot philosophy that starts at power-on self-test (POST) and ends with TBOS personas ready to run on any device class (calculator, SBC, desktop, server, hosted). This guide binds firmware realities, HAL introspection, TBDS descriptors, and profile-aware builds into one narrative.

---

## 1. Stage Map

```
┌────────────┬──────────────────────┬─────────────────────────────────────────────┐
│ Stage      │ Owner                │ Responsibilities                           │
├────────────┼──────────────────────┼─────────────────────────────────────────────┤
│ S0: POST   │ Device firmware      │ Power rails, CPU init, minimal diagnostics  │
│ S1: FW↔HAL │ Firmware + Boot HAL  │ Detect capabilities, expose services        │
│ S2: Stage1 │ TBOS MBR loader      │ Load Stage 2, keep footprint ≤512B          │
│ S3: Stage2 │ TBOS protected mode  │ GDT, A20, paging prep, TBDS emission        │
│ S4: TBDS   │ Boot HAL + kernel    │ Pass descriptor stream + STEPPPS telemetry  │
│ S5: Profile│ Host tooling         │ `tbos-profile-suggest` build orchestration  │
│ S6: Kernel │ TBOS kernel          │ HAL dispatch, scheduler, filesystems        │
│ S7: Persona│ STEPPPS shell        │ Select calc/tiny/default personas           │
└────────────┴──────────────────────┴─────────────────────────────────────────────┘
```

Each stage must leave explicit breadcrumbs (logs, descriptors, checksums) so downstream components inherit truthful state without hidden assumptions.

---

## 2. Stage Details

### S0 – Device POST
1. **Power-On Self-Test** verifies CPU, RAM, timer, and peripheral presence.  
2. **Firmware inventory** (BIOS, UEFI, ROM, Coreboot, SoC boot ROM) picks a boot medium.  
3. **Requirement:** expose POST status + error codes to later stages (serial print, debug LED, or TBDS `FIRMWARE_INFO`).  

> Reference: `boot/README_BOOTLOADER.md`, `docs/arch/x86_64.md`.

### S1 – Firmware ↔ HAL Handshake
- Firmware hands execution to TBOS Stage 1 while TBOS Boot HAL (`boot/boot_hal.inc`) records available consoles, disks, timers, RAM windows, and optional NIC/UART details.  
- HAL populates capability bits that later inform profile suggestion.  
- For non-BIOS targets (UEFI, ARM, RISC-V), shim modules convert native hand-off structures (EFI tables, device trees, SBI data) into the same HAL format.

### S2 – Stage 1 Loader (MBR, ≤512 B)
- Lives at 0x7C00 (BIOS) or EFI partition entry (UEFI).  
- Responsibilities: print sacred boot string, load Stage 2 sectors, transfer control with a minimal register contract.  
- Must not attempt hardware detection; rely on Boot HAL.

### S3 – Stage 2 Loader (Protected Mode)
- Configures GDT/IDT, enables A20, and switches to 32-bit protected mode (or long mode when available).  
- Loads the TBOS kernel at 1 MB, builds the TBOS Boot Descriptor Stream (TBDS), and attaches STEPPPS telemetry snapshot from the HAL.  
- Emits descriptor IDs defined in `docs/BOOT_DESCRIPTOR_SPEC.md` (ARCH_INFO, MEMORY_MAP, BOOT_DEVICE, VIDEO_INFO, CONSOLE_INFO, STEPPPS_TELEMETRY, SECURITY_ATTESTATION).

### S4 – TBDS Validation
- Kernel entry point consumes TBDS, validates signature + length, and hydrates the runtime HAL dispatch table (`include/tbos/hal.h`).  
- Fail fast if critical descriptors are unrecognized, surfacing the error via serial/LED codes.  
- Persist TBDS copy in PXFS under `/pxfs/roots/system/boot/<timestamp>/tbds.bin` for auditability.

### S5 – Profile Suggestion & Auto-Build
- A host-side utility (`scripts/tbos-profile-suggest.sh`, see backlog US1) parses TBDS or runtime `hal_capabilities_t`, classifies the device (calculator, tiny SBC, desktop, server, hosted), and recommends a profile along with the exact build invocation.  
- Optional `--auto-build` flag triggers `build_tbos.sh --profile <name> --manifest <cap.json>` and tags outputs with STEPPPS metadata.  
- Intent: anywhere TBOS boots, we can immediately regenerate the matching artifact without guesswork.

### S6 – Kernel Bring-Up
- Kernel mounts the HAL dispatch, initializes memory (buddy + slab), scheduler, IPC, PXFS/RF2S/PF2S stubs, and logging.  
- Health endpoints queue initial Sangha heartbeat packets (even if networking absent, the intent is logged).  
- Failure during subsystem init should downgrade gracefully based on capability bits (e.g., skip PXFS compute on calculator profile).

### S7 – Persona + STEPPPS Activation
- `steppps-init` launches daemons per `TBOS_DAEMONS` and prepares personas (`tbos-shell mode <persona>`).  
- Default persona determined by profile suggestion: `calc` for low-RAM, `tiny` for SBC, `default` for desktops, `universal` for full features.  
- STEPPPS telemetry records persona, boot duration, and success KPIs.

---

## 3. Device-Class Guidelines

| Device Class | POST Nuances | Required Capabilities | Recommended Profile | Notes |
|--------------|--------------|-----------------------|---------------------|-------|
| Calculator / Microcontroller | Minimal or absent POST; may need external guardian MCU | Console (serial/LCD), timer, ≥64 KB RAM, optional SPI storage | `calc` | Provide guardian firmware that emits TBDS via UART for host capture. |
| Radio-bridged SBC | Firmware POST + SDR handshake | UART + RF transceiver, ≥128 MB RAM, removable storage | `tiny` | Boot HAL should annotate RF2S readiness; Stage 2 loads SDR firmware blobs if present. |
| Desktop / Laptop | Full BIOS/UEFI POST | VGA/serial, ≥2 GB RAM, disk/NVMe, optional NIC | `default` | Use PXFS compute cache + persona selection at shell start. |
| Server / Cloud | Firmware POST + BMC telemetry | Multi-core, ≥8 GB RAM, NICs | `universal` | Trigger Sangha formation automatically post-boot. |
| Hosted / Web | No real POST; rely on runtime harness | Virtual console/timers, host FS access | `hosted` (future) | Use HAL TTY/SDL backends; POST stage simulated by harness logs. |

---

## 4. Telemetry & Validation

1. **POST Logs:** Capture firmware codes to `/var/log/tbos/post.log` (or host console).  
2. **TBDS Snapshot:** Store every descriptor stream for later diffing; include hash in boot banner.  
3. **Profile Decision:** Log recommendation, rule hits, and final build command to PXFS and console.  
4. **Boot KPIs:** Measure time spent in POST (firmware reported), Stage 1/2, kernel init, persona ready.  
5. **Regression Harness:** Script QEMU and hardware tests that ingest mocked TBDS files to ensure profile logic is deterministic.

---

## 5. Implementation Checklist

- [ ] Boot HAL emits capability flags for RAM tier, storage, console, network, optional sensors.  
- [ ] Stage 2 builder writes TBDS + STEPPPS telemetry before jumping to kernel.  
- [ ] Kernel validates TBDS, persists copy, and exposes capability JSON via `/proc/tbos_capabilities`.  
- [ ] `tbos-profile-suggest` consumes JSON/TBDS and optionally calls `build_tbos.sh`.  
- [ ] Docs (`docs/DEPLOYMENT.md`, `README_FULL.md`) reference this guide and link to scripts/demos.  
- [ ] Test coverage: BIOS/QEMU, UEFI (mock), guardian-based calculator demo, hosted harness.

---

## 6. Related Documents
- `docs/BOOT_DESCRIPTOR_SPEC.md` – Descriptor definitions.  
- `docs/HAL_PORTABILITY_PLAN.md` – HAL dispatch abstraction.  
- `docs/DEPLOYMENT.md` – Build + run instructions, TBDS entry types.  
- `boot/README_BOOTLOADER.md` – Stage-by-stage boot details.  
- `docs/PRD_TBOS_VISION.md` – Vision + KPIs tied to universal boot promises.  
- `docs/PRODUCT_BACKLOG.md` – US1 defining the auto-profiling bootloader enhancement.

Use this guide as the canonical reference when planning firmware integrations, new hardware bring-up, or Sangha-wide auto-provisioning flows.
