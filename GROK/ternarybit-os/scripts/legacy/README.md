# Legacy Build Scripts

These scripts have been archived as part of v4.0 production readiness cleanup.

## Why Archived?

These scripts were created during earlier development phases (v1-v3) and have been superseded by the canonical v4.0 build system.

## Canonical v4.0 Build Scripts

Use these instead:

| Script | Purpose |
|--------|---------|
| `build_v4.sh` | **PRIMARY** - Builds BIOS + Hosted Linux |
| `build_universal.sh` | Core bare-metal kernel build |
| `hosted/build_hosted_linux.sh` | Hosted Linux binary |
| `make test-bcb-host` | BCB unit tests |

## Archived Scripts

| Script | Original Purpose | Superseded By |
|--------|------------------|---------------|
| `build.sh` | Generic build | `build_v4.sh` |
| `build_bare_metal.sh` | Bare-metal variant | `build_universal.sh` |
| `build_boot.sh` | Boot-only build | `build_v4.sh` |
| `build_complete_git_version.sh` | Dev artifact | N/A |
| `build_day1.sh` | Day 1 demo | `build_v4.sh` |
| `build_fast_boot.sh` | Fast boot variant | `build_v4.sh` |
| `build_hardware_test.sh` | HW testing | `make test` |
| `build_interactive_shell.sh` | Shell-only | `build_v4.sh` |
| `build_minimal_kernel.sh` | Minimal kernel | Profile system |
| `build_simple.sh` | Simple build | `build_v4.sh` |
| `build_step3.sh` | Step 3 dev | `build_v4.sh` |
| `build_step3a.sh` | Step 3a dev | `build_v4.sh` |
| `build_tbos.sh` | Generic TBOS | `build_v4.sh` |
| `build_ternarybit_image.sh` | Image build | `build_v4.sh` |
| `build_timed.sh` | Timed build | `build_v4.sh` |

## Restoring a Script

If you need to restore a script, simply move it back to the project root:

```bash
mv scripts/legacy/build_foo.sh ./build_foo.sh
```

## Date Archived

2025-06 (v4.0 production readiness)
