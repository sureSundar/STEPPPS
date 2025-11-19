# TernaryBit OS HAL Portability Plan

The Hardware Abstraction Layer (HAL) decouples kernel services from the underlying
platform so we can target calculators, supercomputers, and hosted environments
with the same core code.

## Boot HAL (Stage 0/1/2)

- **Implementation**: `boot/boot_hal.inc`
- **Purpose**: shared BIOS-backed primitives for both boot stages (console init,
  string output, LBA disk reads, fatal halt).
- **Impact**: stage 1 and stage 2 now route their INT 10h/13h usage through a single
  dispatcher, making it trivial to swap in UEFI/Coreboot shims later and ensuring
  boot diagnostics are uniform.
- **TBDS tie-in**: Boot HAL gathers console/disk capability flags for inclusion in
  the TBDS stream so the protected-mode kernel HAL knows exactly which services
  the firmware exposed.

## Dispatch Table

The canonical interface lives in `include/tbos/hal.h`. It exposes:

- `hal_capabilities_t` bitset to advertise console, storage, timer, input, and networking support.
- `hal_console_ops_t`, `hal_input_ops_t`, `hal_timer_ops_t`, `hal_storage_ops_t` capability-specific
  operation tables. The console backend currently offers `put_char_xy`, `clear`, and `write_serial`.
- `hal_dispatch_table_t` aggregating the capability flags and operation tables.
- `hal_get_dispatch()` entry point that returns the active backend.

## Backend Responsibilities

Each platform backend implements `hal_get_dispatch()` and registers platform behaviour:

| Backend      | Location             | Notes                                               |
|--------------|----------------------|-----------------------------------------------------|
| Bare metal   | `kernel/hal_baremetal.c` | Direct VGA/PS2/PIC implementation (default)        |
| Hosted (SDL) | `hosted/hal_sdl.c`       | SDL window for console, host keyboard events       |
| Hosted (TTY) | `hosted/hal_tty.c`       | Uses POSIX termios/ANSI escape sequences           |
| Web/Emscripten | `hosted/hal_web.c`    | JavaScript shim for DOM rendering and events       |

Backends populate only the operations they can support. For example, the calculator profile
would expose console + timer but leave storage/network pointers `NULL`.

## Kernel Integration

1. During early boot, call `hal_get_dispatch()` and cache the result.
2. Replace direct VGA/PS2/polling usage with calls into the dispatch table.
3. Gate optional subsystems on `hal_capabilities_t`.
4. Host builds link a different backend implementation while sharing the same kernel core.

## Next Steps

- Implement `kernel/hal_baremetal.c` with real VGA/PS2 bindings.
- Provide SDL/TTY shims under `hosted/` for desktop builds.
- Add unit tests for mock HAL implementations to validate shell/VFS behaviour without hardware.
- Extend the build profiles (`config/profile_*.conf`) with `CONFIG_HAL_BACKEND=<name>` and
  wire the selection into `build_universal.sh`.
