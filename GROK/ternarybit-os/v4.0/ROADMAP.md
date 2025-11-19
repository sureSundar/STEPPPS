# V4.0 Universal Boot Roadmap

| Item | Description | Owner | Status | Notes |
|------|-------------|-------|--------|-------|
| V4-001 | Define Boot Capability Block (BCB) header in `include/tbos/bcb.h` | Codex | 🔲 TODO | Draft structure in BOOTSTRAP_PLAN.md |
| V4-002 | Stage 2 writes BCB data (drive, memory, kernel addr) | Codex | 🔲 TODO | `boot/stage2_linux.asm` changes |
| V4-003 | Kernel consumes BCB (select HAL drivers) | Codex | 🔲 TODO | `kernel/kernel_main.c` |
| V4-004 | Hosted Linux stub populates BCB and jumps to kernel | Codex | 🔲 TODO | reuse `tbos_universal_bootstrap.c` |
| V4-005 | `build_v4.sh` emits `artifacts/baremetal/tbos-bios.img` | Codex | 🟡 IN PROGRESS | Currently repacks into `build/ternarybit.img` |
| V4-006 | `build_v4.sh` emits hosted Linux binary | TBD | 🔲 TODO | requires new host stub |
| V4-007 | Add smoke tests under `tests/v4/` for BIOS + hosted Linux | TBD | 🔲 TODO | integrate with CI later |
| V4-008 | Document VirtualBox + UEFI recipes | TBD | 🔲 TODO | link from `docs/` |
| V4-009 | Stretch: WASM boot prototype | TBD | 🔲 TODO | compile kernel to wasm32 |

Legend: 🔲 TODO | 🟡 In Progress | ✅ Done
