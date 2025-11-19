# Hosted Profile Plan (Option 3)

**Objective:** Keep the existing QEMU/bare-metal loop as the reliable baseline while prototyping a hosted Linux profile in parallel. The hosted track reuses the HAL dispatch abstraction so TBOS components can run as a regular user-space process on Ubuntu 20.04 x86_64 without abandoning the current BIOS/QEMU story.

---

## 1. Continue Shipping via QEMU (Baseline)
- Build the current image: `./build_tbos.sh` or `make ARCH=x86_64`.
- Test on Ubuntu using QEMU (already installed on your workstation):
  ```bash
  qemu-system-i386 -drive format=raw,file=build/tbos.img -serial stdio
  ```
- Use this path for all regression demos, automation, and calc/tiny persona validation until the hosted profile is production ready.

---

## 2. Prototype Hosted HAL Backend
- New file `kernel/hal_hosted.c` (guarded by `TBOS_HOSTED`) maps HAL console/timer/input calls to ANSI-compatible stdout/stderr plus POSIX timers.
- Bare-metal HAL (`kernel/hal_baremetal.c`) now compiles only when `TBOS_HOSTED` is *not* defined, so hosted builds can link a different dispatch table without code duplication.
- `config/flags.mk` honours `EXTRA_CFLAGS` / `EXTRA_LDFLAGS`, enabling ad-hoc builds:
  ```bash
  make EXTRA_CFLAGS=-DTBOS_HOSTED
  ```
  (This still produces the traditional kernel binary, but the HAL references are now host-aware—perfect for incremental tests linked into future host harnesses.)

---

## 3. Near-Term Hosted Tasks
1. **Host Harness:** add a thin executable (e.g., `hosted/main_hosted.c`) that initializes the kernel core/shell in process space without relying on BIOS entry code. Use the hosted HAL so stdout behaves like the VGA buffer and stdin drives the persona shell.
2. **Profile Metadata:** introduce `config/profile_host.conf` and `build_tbos.sh --profile host` to stamp artifacts with STEPPPS persona defaults (`default` shell, PXFS on host FS paths).
3. **CI Matrix:** activate the pending `hosted-linux` slot from `docs/CI_STRATEGY.md` using GitHub Actions or local scripts so every push runs both QEMU smoke tests and the hosted harness.
4. **Telemetry Bridges:** write adapters that export host process metrics (uptime, memory, CPU) into STEPPPS telemetry frames, matching the POST ➜ TBOS expectations outlined in `docs/POST_TBOS_LOADING_GUIDE.md`.

---

## 4. How to Use Today
1. Keep running QEMU builds for demos and validation. This remains the authoritative execution environment.
2. Use the new top-level commands:
   - `make hosted` → compiles the universal shell via `hosted/build_hosted.sh`
   - `make hosted-test` → runs the scripted smoke test piping `help`, `pwd`, `exit`
   - `./build_universal.sh --profile host` → delegates to the hosted workflow automatically, then runs the same smoke test and prints the binary path.
3. For experimentation, compile components/tests with `EXTRA_CFLAGS=-DTBOS_HOSTED` and link them into small host-side utilities (e.g., the existing `drivers-host` target or new unit tests) to exercise STEPPPS logic without a VM.
4. Track progress in the backlog (US1 plus follow-on hosted stories) so each milestone—host harness, profile selection, CI coverage—links back to PRD requirements.

This blended approach lets us ship confidently on real hardware/emulators while steadily building the hosted experience you need on Ubuntu 20.04.
