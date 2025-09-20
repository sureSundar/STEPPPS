# TBOS Custom Filesystems: Spec and Linux-tier Strategy

This document defines semantics and Linux-tier implementation approach for the requested filesystems. On Linux we prototype with FUSE.

- POSIX FS: native Linux filesystems (ext4/xfs/btrfs) — no custom spec here.
- FAT family (FAT16–64): use `vfat` and `exfat` (exFAT) with tooling.
- NTFS: use `ntfs-3g` (FUSE).
- PXFS: Pixel Filesystem — path separator is an RGB pixel value.
- UCFS: Unicode-delimited path filesystem — path delimiter is any single Unicode char (configurable at mount time).
- RF2S: Radio Frequency Filesystem — directory hierarchy keyed by frequency; files represent channels/streams.

General approach on Linux tier
- Implement PXFS/UCFS/RF2S as userland filesystems using FUSE (Python `fusepy` or C/Go variants). Mount points expose a virtual tree over a backing store (e.g., a database, local directory, network source, or SDR driver).
- Provide mount helpers and configs under `deploy/alpine/fs/`.
- Keep semantics deterministic and well-documented to make it portable to micro-tier later.

---

## 1) PXFS (Pixel Filesystem)

Concept
- Path separator is an RGB triple, e.g., `(R,G,B)`. A path is tokenized by occurrences of a specific pixel value. The same concept can be represented in ASCII using a canonical token like `__PX_255_0_0__` for red.
- Use cases: STEPPPS Pixel dimension navigation, transforming image-derived hierarchies.

Linux-tier representation
- Mount option `sep=R,G,B` defines the separator pixel (default `255,255,255`).
- Paths presented by the FUSE FS will use normal `/` as OS delimiter, but internally we translate between `/` and the pixel token to map to backing keys.
- Backing store: default local directory with encoded names; optionally a SQLite DB keyed by pixel-aware tokens.

Examples
- Mount: `pxfs_fuse.py --backing /var/lib/pxfs --sep 0,0,255 /mnt/px`
- File named `A__PX_0_0_255__B__PX_0_0_255__C` maps to logical segments `A`, `B`, `C` separated by (0,0,255).

Portability note
- On micro-tier, represent the pixel token in the TBOS shell and retain the same encoding.

---

## 2) UCFS (Unicode-delimited FS)

Concept
- The path delimiter is any single Unicode character chosen at mount time, e.g., `·` (middle dot) or `∕`.
- Paths are split on that character to form hierarchy segments.

Linux-tier representation
- Mount option `delim=<UTF8-char>` (default `·`).
- The underlying OS still uses `/`; we encode each logical path using a reversible mapping (e.g., replacing delimiter with `__UC_<hex>__`).

Examples
- Mount: `ucfs_fuse.py --backing /var/lib/ucfs --delim '·' /mnt/uc`
- Logical `foo·bar·baz` appears as `/mnt/uc/foo/bar/baz`, backed by `/var/lib/ucfs/foo__UC_e2_97_87__/bar__UC_e2_97_87__/baz`.

Portability note
- Micro-tier exposes the same delimiter semantics in the shell and APIs. Encoding remains the same.

---

## 3) RF2S (Radio Frequency FS)

Concept
- Directory names are frequencies (Hz/kHz/MHz/GHz). Files represent channels, waveforms, or metadata at that frequency. Navigating the FS selects a frequency context.

Linux-tier representation
- Mount options: `unit=[Hz|kHz|MHz|GHz]` (default MHz), `step=<float>` (optional), `backing=<path>` or `device=<sdr>`.
- Backing store abstraction:
  - `backing` path mode maps each frequency directory to a local directory and files to ordinary files.
  - `device` mode integrates with SDR/driver (future), exposing live streams as read-only files.
- Normalization: frequency directory names canonicalized to a normalized float with 3–6 decimals based on unit.

Examples
- Mount: `rf2s_fuse.py --backing /var/lib/rf2s --unit MHz /mnt/rf`
- Path `/mnt/rf/99.5/metadata.json` → backing `/var/lib/rf2s/99.500000/metadata.json`.

Portability note
- Micro-tier uses the same normalized naming; device integration surface defined by simple read/write APIs.

---

## 4) Package & Mount Plan (Alpine)

Alpine packages to include when FS features are enabled:
- Core: `fuse`, `fuse3` (kernel should have FUSE), `python3`, `py3-pip`.
- NTFS: `ntfs-3g`.
- exFAT: `exfatprogs`.
- FAT: `dosfstools` for mkfs; kernel has `vfat` driver.
- Optional: `sqlite` (for PXFS backing DB).
- Python deps: `fusepy` via `pip install fusepy` into rootfs at build time.

Mount helpers
- Provide simple scripts `mount-pxfs`, `mount-ucfs`, `mount-rf2s` that wrap the Python FUSE implementations.

Security note
- FUSE mounts typically require `user_allow_other` config in `/etc/fuse.conf` if `-o allow_other` is used.

---

## 5) Acceptance Criteria
- PXFS, UCFS, RF2S mount, list, read/write basic files against a backing directory without kernel panics.
- Encodings are reversible and collision-free.
- Performance: adequate for small control-plane data; heavy data deferred to native FS.
- Clear error handling and logs to `/var/log/tbos/`.
