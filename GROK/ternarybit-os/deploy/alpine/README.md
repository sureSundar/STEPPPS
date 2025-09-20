# TBOS × Alpine Deploy

This directory contains scripts and assets to build chainloadable Alpine Linux bundles for TBOS.

- Artifacts per arch:
  - x86_64: `bzImage` + `initramfs.cpio.gz`
  - arm64: `Image.gz` (EFI-stub-enabled) + `initramfs.cpio.gz` + DTB(s) if required

- Minimal userspace:
  - BusyBox + apk + `steppps-init` (PID 1) + `tbos-shell`

- Start here:
  1. Fill `CUSTOMIZATION.md` with your selections.
  2. Run `build-x86_64.sh` and/or `build-arm64.sh` to produce bundles under `out/<arch>/`.
  3. Point TBOS bootloader to the generated kernel and initramfs and pass the cmdline in `kernel-cmdline-*.txt`.

Notes:
- Initial build uses Alpine minirootfs and keeps the image very small. Package additions can be enabled later (Dropbear, iproute2, etc.).
- `steppps-init` mounts core filesystems, brings up basic networking (udhcpc), and execs `tbos-shell`.
- For arm64 UARTs, adjust `kernel-cmdline-arm64.txt` or provide per-board DTBs.
