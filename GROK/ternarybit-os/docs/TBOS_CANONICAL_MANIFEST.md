# TBOS canonical responsibility manifest

Status: **CX proposal, amended by CC after independent verification. Every row
has now been checked by CC against the actual tree — build scripts, line counts,
live references. Awaiting Guru sign-off.**

Branch policy: `canonical` is a short-lived integration branch. After this
manifest and its conformance checks are accepted, the branch must merge into
`main` and be deleted.

## Wholesale duplicate trees (new — not in the original proposal)

`backup_32bit/` is a complete parallel copy of `kernel/` and `boot/` (~65 files:
`kernel_main.c`, `kernel_old.c`, `kernel_simple.c`, `hal_baremetal.c`,
`hal_hosted.c`, `shell.c`, `gui.c`, `steppps.c`, `fs/vfs.c`, `fs/ramfs.c`, and
more). Diffing confirms it is an older revision of the same files, not a
distinct design. Zero build scripts or Makefiles reference anything under
`backup_32bit/` anywhere in the repo. This does not need row-by-row treatment
in the table below — every file in it is already represented by its live
counterpart's row — but the migration needs one explicit action: move the
whole directory to `archive/` as a single unit rather than leaving it to be
rediscovered file-by-file during consolidation.

## Rules

1. TBOS has one authoritative implementation per responsibility.
2. Platform-specific behavior is reachable only through an explicit adapter.
3. A candidate is not deleted during consolidation. Superseded experiments move
   to `archive/` with their history and a note identifying their replacement.
4. Generated objects, binaries, screenshots and debug bundles do not belong in
   source directories or the canonical source manifest.
5. A move is accepted only when hosted and bare-metal conformance checks remain
   at least as green as they were before the move.
6. New features target the selected canonical implementation, not a competing
   prototype.

## Proposed ownership

“Canonical” means the implementation new work targets. “Adapter” means retained
behind the canonical contract. “Archive candidate” means preserved but frozen
after its useful behavior is covered by a test or migrated.

| Responsibility | Proposed canonical implementation | Other current implementations | Disposition before moving |
|---|---|---|---|
| Universal build entry | `build_universal.sh` | `build.sh`, `build_simple.sh`, `build_day1.sh`, `build_tbos_system.sh`, other milestone builders | Keep as legacy build recipes until equivalent profiles exist; then archive. **CC critical finding: `build_universal.sh` is not internally consistent about what "canonical" means. For a bare-metal build it produces `shell/shell.c` (matches this manifest). But `build_universal.sh --profile host` (its `handle_host_profile()`, line 246) skips the bare-metal build entirely and delegates to `hosted/build_hosted.sh`, which builds `src/shell/universal/tbos_shell` — reporting that as `HOSTED_BIN`. So "the canonical build script," when asked for a hosted build, canonizes a completely different shell than the one this manifest names as canonical. This directly contradicts the shell row below and needs a decision, not just a note** |
| Kernel entry and initialization | `kernel/kernel_entry.asm`, `kernel/kernel_main.c` | `kernel/kernel.c`, `kernel/kernel_simple.c`, `kernel/simple_kernel.c`, `kernel/kernel_old.c`, `kernel/kernel_main_simple.c`, `kernel/kernel_minimal_working.c`, `kernel/kernel_entry.c`, minimal/step/test kernels; **also `boot/tbos_bootstrap.c`+`tbos_bootstrap_fast.c` (JVM-based bootstrap, live via `build_timed.sh`/`build_fast_boot.sh`) and `build_integrated/tbos_main.c` (POSIX boot→HAL→shell orchestrator, live via `build_tbos_system.sh`) — CC addition: these aren't under `kernel/` but compete for the same "bring the system up" responsibility under different names** | Keep named reference targets; archive milestone-only entries after boot tests. The two CC-added entries need their own decision, not just archival — they're currently active, not milestone-only |
| Platform/HAL contract | `include/tbos/hal.h` | `src/hal/tbos_hal.h`, `src/core/tbos_interfaces.h`, `src/core/tbos_universal_hal.c`; **also `src/core/tbos_hal.c` (CC addition: a third "TRUE HAL" with its own `tbos_arch_t` enum including `ARCH_QUANTUM`/`ARCH_NEURAL`/`ARCH_TERNARY` and an embedded allocator `tbos_hal_alloc()` — no build-script reference found, orphaned)** | Migrate required capabilities into the include-level contract, then archive duplicates. Note `src/hal/tbos_hal.c` (the second HAL) is itself actively built via `build_tbos_system.sh` — archiving it is a real migration, not just a doc label |
| Bare-metal HAL adapter | `kernel/hal_baremetal.c` | hardware logic embedded in historical kernels | Retain as adapter; extract embedded behavior only when covered by a test |
| Hosted HAL adapter | `kernel/hal_hosted.c` | `src/hal/tbos_hal.c`, `src/core/stubs/tbos_hal.c`, `hosted/tbos_hosted_linux.c` | Retain required host services behind the canonical HAL; archive duplicate stubs |
| Command runtime and native shell | `shell/shell.c`, `shell/shell.h`, `shell/command_provider.*` | `src/core/shell/*`, `src/shell/tbos_complete_shell.c`, `src/shell/universal/*`, `kernel/shell.c`; **CC correction — this list is missing what the repo's own build actually produces today: `src/shell/command_shell.c` + `universal_shell_compat.c` + `shell_main.c` is what the root `Makefile`'s `make shell` target compiles (`Makefile:386`), not `shell/shell.c`. Also missing `src/shell/tbos_tier_shell.c`, live via the Makefile's `tier-shell-*` targets. And `kernel/shell.c` isn't just "an other implementation" — it defines the identical `shell_init()`/`shell_loop()` symbols as `shell/shell.c`, so the two are mutually exclusive at link time, and different build scripts (`build.sh` vs `build_universal.sh`) currently pick different winners** | Canonical shell owns semantics; migrate completion/line-editing/host-provider features, then archive or reduce others to frontends. **CC correction after actually running it (not just reading the Makefile): `make shell` does not silently build `command_shell.c` as this document previously claimed — it's currently non-functional. `SHELL_MAIN` is referenced in the shell rule but never assigned anywhere in the Makefile or `config/`, so `$(BUILD_DIR)/$(SHELL_MAIN)` collapses to a bare directory path (`build/x86_64/`), and `make shell` fails trying to execute a directory (confirmed via `make -n shell`). Supplying `SHELL_MAIN=tbos_shell` by hand gets further but then fails to compile — `steppps_framework.h` has duplicate typedefs that `-Werror=pedantic` treats as fatal. So the correct framing isn't "make shell builds a different shell than expected," it's "make shell is currently broken, independent of which shell ends up canonical."** |
| Shell language adapters | `shell/shell_morph.c`, `shell/sh_interpreter.c`, `shell/tbos_interpreter.c` | compatibility parsing embedded in universal shells | Retain as adapters invoking the canonical command runtime |
| VFS contract and namespace | `include/tbos/vfs.h`, `kernel/fs/vfs.c` | `src/core/tbos_filesystem.h`, filesystem-specific namespace logic | Canonical VFS owns names and mounts; filesystem modules must not create parallel namespaces |
| Bootstrap filesystem | `kernel/fs/ramfs.c` | `src/core/filesystem/tbos_ramdisk.*`, `src/core/stubs/tbos_ramdisk.c` | Retain RAMFS; migrate unique tests/features before archiving duplicates. **CC note: possible misclassification — `tbos_ramdisk.c` is a block-level ramdisk (backing storage), not a `vfs_driver_t` namespace driver like `ramfs.c`. It may be a different layer (closer to `blockdev.c`) rather than a genuine competitor. Confirm intent before treating it as an archive candidate** |
| Persistent filesystem codecs | `src/core/filesystem/` | `fs/pxfs*.c`, compression copies under `src/core/compression/`; **CC additions below** | Select codec by on-disk compatibility tests before moving; no winner assumed where formats differ. **CC warning: `src/core/filesystem/` is not internally clean — `src/core/filesystem/pxfs_codec.c` (path parsing) and `src/core/compression/pxfs_codec.c` (pixel encode/decode) are two different files with the identical name and different jobs, and a third independent copy of the same raw/dense encode/decode logic is inlined in `src/shell/universal/commands_pxfs.c`. Separately, `v3.0/bootloader/pxfs_bootloader_driver.c` uses "PXFS" to mean something unrelated ("Persona eXchange," not pixel-based) — grepping for "PXFS" during migration risks archiving the wrong subsystem. Also unaccounted for anywhere in this manifest or in `docs/TBOS_FIRST_ARCHITECTURE.md`: `deploy/alpine/fs/pf2s_fuse.py` implements a fourth filesystem type, PF2S (photon/wavelength-keyed paths), with no C counterpart** |
| Hosted filesystem adapters | `src/core/filesystem/*_fuse.c` | deployment helper scripts | Retain as platform adapters, never as the VFS authority |
| STEPPPS normative schemas | `include/steppps/schema/` | `steppps/schema/`, `steppps/schemas/` | Include-level schemas are canonical; preserve older schemas as versioned historical inputs. **CC note: `steppps/schemas/steppps_event.json` and `steppps_frame.json` (used by the Python event/frame-store track below) are a third pair not covered by the two listed alternates — different concept (telemetry frames, not STEPPPS entities) but still a schema fork worth tracking explicitly** |
| STEPPPS C runtime | `steppps/v2/steppps_runtime.c`, `steppps/v2/steppps.h` | `steppps/steppps.c`, `src/core/steppps/*`, `kernel/steppps.c`, `src/core/tbos_steppps_full.c`; **CC additions: `src/core/steppps_framework.c` (674 lines, directly under `src/core/`, not `src/core/steppps/` — NOT orphaned, actively compiled and run as part of `make test` via the root Makefile's `test_steppps_framework` target) and `v3.0/bootloader/steppps_bootloader_integration.c` (live via the v3.0 Makefile). Also worth splitting out of "`src/core/steppps/*`": `steppps_entity.c` implements a completely different object model (128-bit URI/identity/messaging) from `tbos_steppps.h` (shell-command-facing) — they share a directory, not a design** | Extend v2 to pass schema/examples; migrate unique behavior and archive competing runtimes. **CC note: a separate track this row doesn't cover at all — `steppps/runtime/stepppsd.py` + `steppps/adapters/tbos-steppps-ingest.py` implement an event/telemetry frame store, a different concept from the entity/runtime C implementations, against the third schema pair noted above. And `deploy/alpine/llm/tbos-llm-proxy.py`'s prompt extraction must already cope with both v1 and v2 JSON dialects (confirmed by `tests/unit/test_steppps_prompt_interaction.py`), so consolidating the C runtime doesn't remove the need to handle both dialects at that layer** |
| STEPPPS governance tooling | `tools/steppps_validate.py` | `steppps/tools/` utilities | Retain validated registry/chain commands; assess utilities individually |
| GUI model and renderer | `src/gui/tbos_pixel.*`, `src/gui/tbos_gui.*`, `src/gui/tbos_desktop.*` | `kernel/gui.c` (CC-verified: 486 lines, freestanding VGA-level primitives, live via `build.sh` — different tier, not a real competitor), demonstrations embedded elsewhere | Retain portable C GUI; SDL is an adapter and demos are test/example frontends. **CC-verified this row directly: `tbos_desktop.c`/`tbos_desktop_demo.c` were never wired into `src/gui/Makefile` before this pass — fixed (`make desktop`, `make desktop-sdl` now build both), along with a font-rendering bug (all text was rendering as filled blocks, not glyphs), a window-shadow corner-artifact bug, and a desktop-icon layout bug. Row confirmed accurate; renderer is now actually buildable and legible, not just present in source** |
| Hosted display/input adapter | `src/gui/tbos_platform_sdl.c` | host-specific rendering in prototypes | Retain behind `src/gui/tbos_platform.h` |
| Network contract and stack | `src/network/core/tbos_network.*`, then `src/network/{transport,protocols,dns}` | `kernel/network.c`, `src/core/tbos_network.c`, `src/network/tbos_net.c`; **CC verified and adds a fourth: `src/network/core/tbos_network.c` itself (568 lines) is a metrics/policy layer that opens no sockets — the actual HAL-delegating socket wrapper is `src/network/tbos_net.c` (253 lines), already listed as "other" but arguably load-bearing enough to be co-canonical, since it's what's actually linked into `src/shell/universal`'s `netstat`/`nc`/`http`/`serve` commands. `src/core/tbos_core_impl.c` also embeds a fifth in-memory network abstraction** | Prove hosted and native boundaries before selecting lower layers; freeze duplicate feature work now. **CC finding, substantiates the "Early" rating in CX's own WISDOM.md Reality Check: `kernel/hal_baremetal.c`'s dispatch table has no `.network` entry at all (`caps.has_network = 0`) — there is no native/freestanding network implementation behind the canonical HAL contract, only the hosted adapter (`kernel/hal_hosted.c`, real POSIX sockets). `kernel/network.c` (589 lines, "Hour 10 Sprints") looks like a native stack but is confirmed fully simulated — `ethernet_send()` just increments a counter and logs "would send to hardware," no actual I/O anywhere in it. Of the `src/network/{transport,protocols,dns}` layering this row proposes as the target shape, only `tbos_udp.c` and `tbos_dns.c`/`tbos_wow.c` are actually wired into a build target; `tbos_tcp.c`, `tbos_http.c`, `tbos_ftp.c` were not found referenced by any Makefile — that target layering is aspirational, not yet real** |
| Process and scheduler contract | **Decision required** | `kernel/process.c`, `kernel/scheduler.c`, `src/core/process/tbos_process.c`, `src/core/tbos_scheduler.c`; **CC addition: `src/core/tbos_process.h` (top-level, distinct from `src/core/process/tbos_process.h`) — a "SOLID v2" header with no `.c` implementation anywhere in the active tree; it's a near-duplicate of `archive/deprecated_v1/tbos_process_v2.h`, already superseded once. `kernel/process.c` and `kernel/scheduler.c` also each declare their own copy of `process_state_t` rather than sharing one** | Build a behavior matrix and tests before choosing; do not move yet. **CC confirms this is the right call — found more competing designs than listed, not fewer. `src/core/process/tbos_process.c` (karma-based scheduling) is the most actively used today (referenced by shell and metrics code), worth weighting that in the eventual decision** |
| Memory contract | **Decision required** | `kernel/memory.c`, `src/core/memory_manager.*`, `src/core/tbos_memory.*`, tier memory; **CC addition: `hosted/tbos_hosted_linux.c` has its own mmap-backed heap, and `src/core/tbos_hal.c` embeds yet another allocator (`tbos_hal_alloc`) — at least 6 independent designs total, not 4** | Define allocator tiers and freestanding ABI before choosing; do not move yet. **CC note: `kernel/kernel_main.c` (the canonical kernel entry) actually uses neither `kernel/memory.c` nor any of the `src/core/` managers for its own allocation — it uses a bump allocator embedded in `kernel/libc.c`'s `malloc()`. Worth adding as a fifth/sixth candidate, or at minimum flagging that the canonical kernel entry isn't wired to any of the "memory contract" candidates yet** |
| Lifecycle/supervision | `src/core/lifecycle/tbos_lifecycle.*` as starting candidate | shutdown/init logic across `src/core` and kernels | Accept only after it can supervise the first vertical slice. **Not yet independently checked by CC (out of scope this pass) — CC can confirm `tbos_lifecycle.*` is at least a real, referenced consumer of `src/core/steppps/tbos_steppps.h`, but has not audited this row's completeness** |
| Hosted deployment | `deploy/hosted/` with profiles under `deploy/<host>/` | `hosted/`, distribution experiments; **CC verified and expands this list considerably** | Deployment consumes canonical core; it never owns TBOS semantics. **CC finding: "distribution experiments" undersells how much is actually live here. `deploy/hosted/` (new, untracked) builds a real headless GUI renderer (`make check`, three PPM screenshots) and a real SDL2-windowed desktop (`tbos-desktop`) — this is the deployment path for the GUI work verified elsewhere in this manifest, and it's already installed/built on this machine (`.dSYM` present under `deploy/hosted/build/`). `deploy/ubuntu/install-user.sh` (new) is purely a packaging wrapper around that binary — not a competing entry point. But `deploy/linux/tbos-ubuntu-installer.sh` (pre-existing, older) *is* a genuine duplicate of `deploy/ubuntu/`'s responsibility (both are "install TBOS for a user on Ubuntu"), targeting a different install layout and config format, and critically has no working binary to install — it assumes a `tbos` command already exists on `PATH`. Separately, `hosted/tbos_hosted_linux.c` — the file `docs/TBOS_FIRST_ARCHITECTURE.md` names as "the Hosted runtime prototype" — is NOT what `build_universal.sh --profile host` actually builds (see the Universal build entry row above); that delegates to `src/shell/universal/`'s shell instead, a third, much larger, actively-developed hosted shell (~15,000 lines, 100+ commands) that this row doesn't mention at all. And `deploy/alpine/` is a fourth, architecturally distinct hosted path — it boots a real minimal Linux kernel under QEMU with TBOS's own PID-1 init (`steppps-init`) rather than running as a host userspace process; not a competitor to the others so much as a distinct low-memory distribution tier** |
| Tests | `tests/unit`, `tests/integration`, `tests/smoke` | root-level milestone scripts and demo tests | Keep useful legacy tests until mapped into the conformance suite |

## Decisions requiring sign-off

Guru, CC and CX should explicitly accept or amend these points before file moves:

- `shell/shell.c` owns command semantics; other shells become frontends or
  archive candidates. **CC amendment required before accepting as-is**: the
  root Makefile's `make shell` target does not build `shell/shell.c` today —
  it builds `src/shell/command_shell.c`. Accepting this bullet unamended means
  deciding, explicitly, to repoint `make shell` at a different binary than it
  produces now. That's a fine decision to make, but it should be made on
  purpose, not fall out silently from accepting this bullet.
- `include/tbos/hal.h` is the only platform contract. CC: accept, with the
  note that `src/hal/tbos_hal.c` is itself actively built via
  `build_tbos_system.sh`, so this is a real migration of a live path, not
  archiving a stub.
- `include/tbos/vfs.h` plus `kernel/fs/vfs.c` owns the filesystem namespace.
  CC: accept.
- `steppps/v2` is the C runtime paired with the normative schemas under
  `include/steppps/schema`. CC: accept, with `src/core/steppps_framework.c`
  added to the migration list since it's actively tested (`make test`), not
  orphaned, and the Python event/frame-store track (`stepppsd.py` +
  `steppps/adapters/`) flagged as a separate, uncovered concern.
- `src/gui` is the portable GUI; SDL2 is only one hosted adapter. **CC signs
  off on this one directly** — verified by building it, not just reading it.
- Process/scheduler and memory remain deliberately undecided until behavior and
  ABI tests distinguish their competing implementations. CC: accept, and
  raises the count of competing implementations found in both categories
  above what this document originally listed (see table).
- **New, not in CX's original list**: `backup_32bit/` (complete duplicate of
  `kernel/` and `boot/`) needs an explicit disposition — proposed: archive as
  one unit, not file-by-file.
- **New, not in CX's original list**: the "PXFS" name is used for three
  unrelated things in this repo (pixel-based compression, a path codec, and
  `v3.0/bootloader`'s persona-exchange scheme) — migration tooling that
  matches by name alone will make mistakes here.
- **New, not in CX's original list, and blocking**: `build_universal.sh` — the
  script `IMPLEMENTATION_COMPLETE.md` calls the canonical build script — does
  not agree with this manifest about what "canonical" means. Its bare-metal
  path builds `shell/shell.c` (matches). Its `--profile host` path builds
  `src/shell/universal/tbos_shell` instead (~15,000 lines, a completely
  separate hosted shell this manifest never names). Meanwhile
  `docs/TBOS_FIRST_ARCHITECTURE.md` names a *third* file,
  `hosted/tbos_hosted_linux.c`, as "the Hosted runtime prototype" — which the
  build script never touches at all. Three documents/scripts, three different
  answers for "what runs when TBOS boots hosted." This needs a decision before
  the shell row's disposition can actually be executed, not just documented.
- **New, not in CX's original list**: `deploy/linux/tbos-ubuntu-installer.sh`
  (pre-existing) and `deploy/ubuntu/install-user.sh` (new) both claim the same
  responsibility — installing TBOS for a user on Ubuntu — with different
  install layouts and config formats. Only the new one installs a binary that
  actually exists (`deploy/hosted/`'s SDL2 desktop); the older one assumes a
  `tbos` command is already on `PATH`. Propose: `deploy/ubuntu/` canonical,
  `deploy/linux/` archived, since it has no working artifact behind it today.
- **New, not in CX's original list**: there is no native/freestanding network
  implementation anywhere in the tree. `kernel/hal_baremetal.c`'s dispatch
  table has no `.network` entry (`caps.has_network = 0`); `kernel/network.c`
  looks like a bare-metal stack but every send/receive path is simulated
  (increments a counter, logs "would send to hardware," touches no device).
  The only real networking anywhere is hosted POSIX sockets, three different
  times over (`src/network/tbos_net.c` via HAL, `src/core/tbos_network.c`
  raw, `src/core/tbos_core_impl.c` in-memory). This is worth stating plainly
  rather than leaving implicit in "prove hosted and native boundaries" —
  there is currently no native boundary to prove; it doesn't exist yet.

## Required conformance gate

No physical consolidation PR may merge until one command verifies at least:

```sh
tests/canonical_conformance.sh
```

1. hosted command-provider execution;
2. freestanding compilation of public core headers;
3. STEPPPS v1, v2 and Earth entity parsing;
4. VFS/RAMFS `pwd`, `ls`, create, read and write behavior;
5. PXFS STEPPPS metadata round-trip;
6. headless GUI rendering;
7. an inventory check showing every moved path as canonical, adapter, test,
   generated artifact or archived implementation.

## Migration sequence after approval

1. Add the conformance command and inventory checker.
2. Remove generated artifacts from source control and ignore their build paths.
3. Consolidate shell frontends around `shell_execute_command()`.
4. Consolidate HAL declarations and adapters.
5. Consolidate VFS and filesystem adapter boundaries.
6. Consolidate STEPPPS runtime implementations.
7. Connect the GUI Terminal and Files surfaces to the canonical runtime.
8. Merge the short-lived branch into `main`, delete it, and require subsequent
   implementation through focused PRs.
