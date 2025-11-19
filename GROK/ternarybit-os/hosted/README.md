# TBOS Hosted Profile (Ubuntu/Linux)

This directory provides helper scripts to run the **Universal Shell** persona directly on a hosted operating system (e.g., Ubuntu 20.04 on x86_64) without QEMU. The hosted path leverages the existing shell implementation under `src/shell/universal/` and is aligned with the `profile_host` configuration.

## Prerequisites

- GCC and Make (`sudo apt install build-essential`)
- Optional: `rlwrap` for nicer input history

## Build

```bash
./hosted/build_hosted.sh
```

This compiles the universal shell into `src/shell/universal/tbos_shell` using the stubs under `src/core/stubs/`.

## Run Interactively

```bash
src/shell/universal/tbos_shell
```

Type `help` to see available commands and `exit` to quit.

## Automated Smoke Test

```bash
./hosted/test_hosted.sh
```

The script pipes a short command sequence (`help`, `pwd`, `exit`) to verify the hosted binary behaves correctly. Logs are emitted to `build/hosted/test.log`.

## Relation to Profiles

- `config/profile_host.conf` defines the hosted persona (shell + FS enabled, interrupts disabled).
- `docs/HOSTED_PROFILE_PLAN.md` explains how the hosted path complements the traditional QEMU boots.

Use this hosted workflow for rapid iteration on TBOS shell behaviors while keeping the canonical bootloader/kernal path intact.
