# V4.0 Universal Boot Bootstrap Plan

## Stage Architecture

| Stage | Purpose | Runs On | Outputs |
|-------|---------|---------|---------|
| Stage 0 | Transport bootstrap (host stub, ROM init, hypervisor shim) | Hosted apps, cloud sandboxes, firmware pre-init | Copies Stage 1/2 payload to executable region, fills Boot Capability Block (BCB) with host hints |
| Stage 1 | 512-byte “sacred” loader (`tbos_sacred.asm` profile) | BIOS/legacy boot sectors, VHD MBRs | Loads Stage 2 using geometry-aware TB Boot HAL, seeds BCB with disk info |
| Stage 2 | Protected-mode transition + kernel loader (`stage2_universal.asm` profile) | BIOS, UEFI CSM, Stage 0 trampolines | Enables A20, sets up GDT, copies kernel, finalizes BCB |
| Stage 3 | Kernel image (`kernel/kernel_main.c`) | All targets | Reads BCB, selects HAL drivers (real HW vs hosted shims), launches shell/network stack |

## Boot Capability Block (BCB) Draft
Structure (little endian):
```
struct tbos_bcb_v1 {
    char     magic[4];      // "TBBC"
    uint16_t version;       // 0x0001
    uint16_t length;        // bytes following header
    uint32_t stage_flags;   // bitmask (bios, uefi, hosted, virt, secure, etc.)
    uint64_t boot_drive_id; // BIOS DL value or host descriptor
    uint64_t mem_low_bytes; // below 1 MB
    uint64_t mem_high_bytes;// above 1 MB
    uint64_t kernel_entry;  // physical addr chosen by loader
    uint64_t host_api_ptr;  // optional pointer to hosted ABI table
    uint8_t  reserved[64];  // future use (network cfg, PV channel, etc.)
};
```

Actions:
1. Stage 1 writes magic/version, boot drive.
2. Stage 2 fills in segment info, memory map snapshot, kernel entry.
3. Hosted Stage 0 stubs populate `host_api_ptr` with call table for syscalls/IPC.
4. Kernel reads BCB, configures HAL dispatch accordingly.

## Target Matrix

| Target | Artifact | Loader Chain | Required Work |
|--------|----------|--------------|---------------|
| BIOS Bare-metal | `artifacts/baremetal/tbos-bios.img` | Stage 1 (MBR) → Stage 2 → Kernel | Already working; integrate BCB + repackage |
| UEFI x86_64 | `artifacts/baremetal/tbos-uefi.img` | PEI stub → Stage 2 (loaded as PE payload) → Kernel | Port Stage 2 to work as flat PE loader |
| Hosted Linux | `artifacts/hosted/tbos-linux.bin` | ELF loader stub → Kernel (no stage1/2) | Use `tbos_universal_bootstrap.c` as host shim; supply fake BCB |
| Hosted Windows | `artifacts/hosted/tbos-win.exe` | Win32 console stub → Kernel | Reuse existing bootstrap, link with MinGW |
| VirtualBox/QEMU | `artifacts/virtual/tbos-hyper.img` | BIOS Stage 1/2 | Provide helper scripts + `.vbox` template |
| WASM / Browser | `artifacts/wasm/tbos.wasm` | WASM runtime shim → Kernel (compiled for wasm32) | Requires alternate kernel build (future phase) |

## Milestones
1. **Week 1** – BCB v1 defined, Stage 2 writes data, kernel reads it.
2. **Week 2** – `build_v4.sh` emits BIOS + hosted Linux artifacts, adds smoke tests.
3. **Week 3** – Hosted Windows + VirtualBox packaging (OVF template, docs).
4. **Week 4** – UEFI prototype loader + HAL unification.
5. **Stretch** – WASM build proof-of-concept, network stack validation on hosted targets.

## Testing Strategy
- Each artifact gains a shell script under `tests/v4/`:
  - `tests/v4/test_bios.sh` → boots via QEMU, expects prompt `tbos>`
  - `tests/v4/test_hosted_linux.sh` → runs hosted stub, pipes a command, verifies output
  - Additional scripts for VirtualBox/UEFI use existing automation hooks
- Serial logs saved in `logs/v4/<target>.log` for regression diffing.

## Tooling TODO
- [ ] Define `include/tbos/bcb.h`
- [ ] Update `boot/stage2_linux.asm` to write BCB data
- [ ] Teach kernel to parse BCB (fallback to defaults if absent)
- [ ] Extend `build_v4.sh` to create per-target directories with metadata (manifest + run instructions)
- [ ] Add GitHub-style status table to `v4.0/ROADMAP.md`
