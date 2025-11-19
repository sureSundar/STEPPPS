# Continuous Integration Strategy

To certify TernaryBit OS across calculators, bare-metal targets, hosted apps, and web builds we
standardise on a matrix-based CI pipeline.

## Build Matrix

| Job                | Profile         | Artifact                          |
|--------------------|-----------------|-----------------------------------|
| `calc`             | `--profile calc`| Minimal floppy image              |
| `universal`        | default         | `tbos_universal.img`              |
| `hosted-linux`     | `--profile host` (future) | ELF binary + SDL assets |
| `hosted-web`       | `--profile web` (future)  | WASM/JS bundle          |

Each job invokes `./build_universal.sh --profile <name>` and archives the produced image/binary.

## Smoke Tests

- **QEMU boot test**: run `qemu-system-i386 -nographic` against the universal and calc images,
  capturing serial output and asserting the presence of the `TBOS>` prompt (when shell enabled).
- **Hosted harness** (future): execute the host binary with scripted input to verify CLI commands.
- **Unit tests**: once HAL mocks exist, run shell/VFS unit tests under the native toolchain.

## Quality Gates

- Enable `-Wall -Wextra` in the build (already active) and add `-Werror` for CI builds.
- Track artifact sizes; fail if the calculator profile exceeds its target footprint.
- Upload QEMU serial logs and screenshots (where available) as build artifacts.

## Tooling

- GitHub Actions (or equivalent) workflow invoking the matrix above.
- Cache NASM/GCC toolchains to keep runtimes short.
- Optional nightly job runs extended tests (e.g., long-running shell scripts, RAMFS stress).

## Release Pipeline

- Tag builds trigger the CI matrix plus packaging.
- Publish universal image, calc image, hosted binaries, and documentation bundle to the release page.
- Attach generated `config/profile_<name>.conf` snapshots to the release for reproducibility.
