# TBOS canonical responsibility manifest

Status: **proposed for Guru, CC and CX sign-off**

Branch policy: `canonical` is a short-lived integration branch. After this
manifest and its conformance checks are accepted, the branch must merge into
`main` and be deleted.

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
| Universal build entry | `build_universal.sh` | `build.sh`, `build_simple.sh`, `build_day1.sh`, `build_tbos_system.sh`, other milestone builders | Keep as legacy build recipes until equivalent profiles exist; then archive |
| Kernel entry and initialization | `kernel/kernel_entry.asm`, `kernel/kernel_main.c` | `kernel/kernel.c`, `kernel/kernel_simple.c`, `kernel/simple_kernel.c`, minimal/step/test kernels | Keep named reference targets; archive milestone-only entries after boot tests |
| Platform/HAL contract | `include/tbos/hal.h` | `src/hal/tbos_hal.h`, `src/core/tbos_interfaces.h`, `src/core/tbos_universal_hal.c` | Migrate required capabilities into the include-level contract, then archive duplicates |
| Bare-metal HAL adapter | `kernel/hal_baremetal.c` | hardware logic embedded in historical kernels | Retain as adapter; extract embedded behavior only when covered by a test |
| Hosted HAL adapter | `kernel/hal_hosted.c` | `src/hal/tbos_hal.c`, `src/core/stubs/tbos_hal.c`, `hosted/tbos_hosted_linux.c` | Retain required host services behind the canonical HAL; archive duplicate stubs |
| Command runtime and native shell | `shell/shell.c`, `shell/shell.h`, `shell/command_provider.*` | `src/core/shell/*`, `src/shell/tbos_complete_shell.c`, `src/shell/universal/*`, `kernel/shell.c` | Canonical shell owns semantics; migrate completion/line-editing/host-provider features, then archive or reduce others to frontends |
| Shell language adapters | `shell/shell_morph.c`, `shell/sh_interpreter.c`, `shell/tbos_interpreter.c` | compatibility parsing embedded in universal shells | Retain as adapters invoking the canonical command runtime |
| VFS contract and namespace | `include/tbos/vfs.h`, `kernel/fs/vfs.c` | `src/core/tbos_filesystem.h`, filesystem-specific namespace logic | Canonical VFS owns names and mounts; filesystem modules must not create parallel namespaces |
| Bootstrap filesystem | `kernel/fs/ramfs.c` | `src/core/filesystem/tbos_ramdisk.*`, `src/core/stubs/tbos_ramdisk.c` | Retain RAMFS; migrate unique tests/features before archiving duplicates |
| Persistent filesystem codecs | `src/core/filesystem/` | `fs/pxfs*.c`, compression copies under `src/core/compression/` | Select codec by on-disk compatibility tests before moving; no winner assumed where formats differ |
| Hosted filesystem adapters | `src/core/filesystem/*_fuse.c` | deployment helper scripts | Retain as platform adapters, never as the VFS authority |
| STEPPPS normative schemas | `include/steppps/schema/` | `steppps/schema/`, `steppps/schemas/` | Include-level schemas are canonical; preserve older schemas as versioned historical inputs |
| STEPPPS C runtime | `steppps/v2/steppps_runtime.c`, `steppps/v2/steppps.h` | `steppps/steppps.c`, `src/core/steppps/*`, `kernel/steppps.c`, `src/core/tbos_steppps_full.c` | Extend v2 to pass schema/examples; migrate unique behavior and archive competing runtimes |
| STEPPPS governance tooling | `tools/steppps_validate.py` | `steppps/tools/` utilities | Retain validated registry/chain commands; assess utilities individually |
| GUI model and renderer | `src/gui/tbos_pixel.*`, `src/gui/tbos_gui.*`, `src/gui/tbos_desktop.*` | `kernel/gui.c`, demonstrations embedded elsewhere | Retain portable C GUI; SDL is an adapter and demos are test/example frontends |
| Hosted display/input adapter | `src/gui/tbos_platform_sdl.c` | host-specific rendering in prototypes | Retain behind `src/gui/tbos_platform.h` |
| Network contract and stack | `src/network/core/tbos_network.*`, then `src/network/{transport,protocols,dns}` | `kernel/network.c`, `src/core/tbos_network.c`, `src/network/tbos_net.c` | Prove hosted and native boundaries before selecting lower layers; freeze duplicate feature work now |
| Process and scheduler contract | **Decision required** | `kernel/process.c`, `kernel/scheduler.c`, `src/core/process/tbos_process.c`, `src/core/tbos_scheduler.c` | Build a behavior matrix and tests before choosing; do not move yet |
| Memory contract | **Decision required** | `kernel/memory.c`, `src/core/memory_manager.*`, `src/core/tbos_memory.*`, tier memory | Define allocator tiers and freestanding ABI before choosing; do not move yet |
| Lifecycle/supervision | `src/core/lifecycle/tbos_lifecycle.*` as starting candidate | shutdown/init logic across `src/core` and kernels | Accept only after it can supervise the first vertical slice |
| Hosted deployment | `deploy/hosted/` with profiles under `deploy/<host>/` | `hosted/`, distribution experiments | Deployment consumes canonical core; it never owns TBOS semantics |
| Tests | `tests/unit`, `tests/integration`, `tests/smoke` | root-level milestone scripts and demo tests | Keep useful legacy tests until mapped into the conformance suite |

## Decisions requiring sign-off

Guru, CC and CX should explicitly accept or amend these points before file moves:

- `shell/shell.c` owns command semantics; other shells become frontends or
  archive candidates.
- `include/tbos/hal.h` is the only platform contract.
- `include/tbos/vfs.h` plus `kernel/fs/vfs.c` owns the filesystem namespace.
- `steppps/v2` is the C runtime paired with the normative schemas under
  `include/steppps/schema`.
- `src/gui` is the portable GUI; SDL2 is only one hosted adapter.
- Process/scheduler and memory remain deliberately undecided until behavior and
  ABI tests distinguish their competing implementations.

## Required conformance gate

No physical consolidation PR may merge until one command verifies at least:

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
