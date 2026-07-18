# Archived: backup_32bit/

Moved here 2026-07-18 per Guru sign-off on
`docs/TBOS_CANONICAL_MANIFEST.md` ("Wholesale duplicate trees").

This was a complete parallel copy of `kernel/` and `boot/` (kernel_main.c,
kernel_old.c, hal_baremetal.c, hal_hosted.c, shell.c, gui.c, steppps.c,
fs/vfs.c, fs/ramfs.c, and more) — an older revision of the same files, not a
distinct design. No build script or Makefile referenced anything under this
directory before the move.

Replacement: the live `kernel/` and `boot/` trees at the repository root.
