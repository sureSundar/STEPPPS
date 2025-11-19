# TBOS Filesystem Focus Plan

**Scope:** Align engineering work across legacy filesystems (FAT12/16/32, NTFS), TBOS-native stacks (UCFS, PXFS, RF2S/R2FS), and ChemOS storage semantics so every persona can mount, read, and evolve data coherently.

---

## 1. Current State Snapshot

| Filesystem | Intent | Code Status | References |
|------------|--------|-------------|------------|
| FAT12/16/32 | Legacy boot + removable media | Prototype glue in `v3.0/prototypes/windows_fs_driver_prototype.c`; no kernel driver | v3.0 prototype lines 6-602 |
| NTFS | Desktop interoperability | Same Windows prototype stubs; no core integration | Same as above |
| UCFS | Unicode/emoji semantic paths | Codec + overlay implemented; needs canonical mount + metadata persistence | `include/fs/ucfs_*`, `src/core/filesystem/ucfs_*` |
| PXFS | Pixel-based consciousness FS | Codec + overlay implemented; needs canonical mount + block metadata | `include/fs/pxfs_*`, `src/core/filesystem/pxfs_*` |
| RF2S/R2FS | Radio-frequency storage | Documented but no driver | docs/TBOS_UNIVERSAL_FILESYSTEM.md §Order 4 |
| ChemOS (ChemFS) | Chemical/quantum storage | Persona + docs only; needs schema + driver | README.md, v3.0 docs |

**Gap:** Kernel/VFS has no modular driver registry; shell references PXFS/UCFS/RF2S conceptually but can't read/write them yet (`src/shell/universal/tbos_universal_shell.c:526`).

---

## 2. Unified Filesystem Architecture

```
┌───────────────────────────────┐
│ TBOS VFS Core                 │
│  • tbos_vfs_mount(struct fs)  │
│  • tbos_vfs_lookup(path)      │
│  • tbos_vfs_read/write        │
└──────────────┬────────────────┘
               │ driver ops table
┌──────────────┼────────────────┬──────────────────────┐
│ Legacy Layer │ TBOS Native    │ Experimental (ChemOS)│
│  • FAT12     │  • UCFS        │  • ChemFS (molecular)│
│  • FAT16     │  • PXFS        │                      │
│  • FAT32     │  • RF2S/R2FS   │                      │
│  • NTFS      │                │                      │
└──────────────┴────────────────┴──────────────────────┘
```

Each driver implements:

```c
typedef struct {
    int  (*probe)(tbos_blockdev_t*);
    int  (*mount)(tbos_blockdev_t*, tbos_fs_context_t*);
    int  (*unmount)(tbos_fs_context_t*);
    int  (*lookup)(tbos_fs_context_t*, const tbos_vfs_path_t*, tbos_vnode_t*);
    int  (*read)(tbos_vnode_t*, void*, size_t, off_t);
    int  (*write)(tbos_vnode_t*, const void*, size_t, off_t);
} tbos_fs_driver_t;
```

---

## 3. Filesystem Track Breakdown

### 3.1 Legacy FAT/NTFS
- **Goal:** Read-only mount for boot media, USB drives, Windows partitions.
- **Steps:**
  1. Extract reusable FAT structures from `v3.0/prototypes/windows_fs_driver_prototype.c` into `fs/legacy/fat_driver.c`.
  2. Implement block-device shim backed by BIOS/Boot HAL (existing `boot/boot_hal.inc`) so Stage 2/Kernel share sector I/O.
  3. Start with FAT32 read-only (directory traversal + file read). Add FAT12/16 once driver stabilizes.
  4. NTFS: leverage simplified parser (MFT walker) focusing on read-only metadata; reuse Windows prototype algorithms.
  5. Provide CLI demo: `tbos-fs mount --type fat32 /dev/sdX1 /mnt/fat32`.

### 3.2 UCFS
- **Goal:** Unicode-aware path translator layered atop POSIX/PXFS.
- **Steps:**
  1. Promote `v3.0/bootloader/ucfs_unicode_support.c` into `fs/ucfs/ucfs.c` with normalization, emoji hashing, culture metadata.
  2. Implement path codec: UCFS path ↔ canonical POSIX path (emoji-to-ASCII, etc.).
  3. Expose `ucfs_mount(root_path, mapping_policy)` that overlays UCFS on top of existing storage.
  4. Extend shell commands to accept UCFS tokens; update `tbos_universal_shell` to call the codec.

### 3.3 PXFS
- **Goal:** Replace stub with actual pixel tree + compute cache.
- **Steps:**
  1. Define on-disk/in-memory structures (PINode, pixel blocks) per `docs/PXFS_ARCHITECTURE.md`.
  2. Implement in-RAM prototype first (pixel root registry, compute callback for transforms).
  3. Hook into `fs/tools/mkpxfs` to serialize/dump pixel paths (tools currently placeholder).
  4. Provide `pxfsctl` CLI for color navigation and bridging to UCFS (color-bookmarks).

### 3.4 RF2S / R2FS
- **Goal:** Map RF spectrum to hierarchical storage that can stream over SDR or simulated bus.
- **Steps:**
  1. Define `tbos_rf_handle` representing frequency, modulation, bandwidth.
  2. Create simulation backend (no SDR hardware needed initially) to store blobs keyed by frequency buckets.
  3. Integrate with PXFS compute: convert pixel spectra to RF payloads (per `docs/CALCULATOR_RADIO_QUANTUM_LEAP.md`).
  4. Extend persona shell with `rf2s ls 432Hz` etc.

### 3.5 ChemOS / ChemFS
- **Goal:** Represent chemical states (elements, bonds) as addressable storage for the ChemOS persona.
- **Steps:**
  1. Define schema: `chemfs_node { element_id, oxidation_state, energy_level, data_hash }`.
  2. Provide translator between ChemFS addresses and PXFS/UCFS paths (since ChemOS uses both).
  3. Simulate reaction logs as append-only files (tie into `persona=ChemOS` telemetry).
  4. Align with safety metadata described in `v3.0/bootloader/TBOS_V3_BOOTLOADER_DOCUMENTATION.md`.

---

## 4. Execution Roadmap

| Phase | Duration | Deliverables | Notes |
|-------|----------|--------------|-------|
| Phase 0 | 1 week | TBOS VFS driver table + blockdev shim | ✅ Completed: blockdev API + FAT32 driver scaffold + tests |
| Phase 1 | 2 weeks | FAT32 read-only mount + CLI demo | use QEMU disk image |
| Phase 2 | 2 weeks | UCFS codec + overlay mount | integrate with shell |
| Phase 3 | 3 weeks | PXFS in-memory prototype + mkpxfs tool | instrumentation + tests |
| Phase 4 | 2 weeks | RF2S simulation + CLI | depends on PXFS spectral bridge |
| Phase 5 | 3 weeks | ChemFS schema + telemetry storage | ties into persona/energy systems |
| Phase 6 | ongoing | FAT16/12, NTFS read-only, write support, hardware backends | follow-up |

Testing per phase: QEMU disk images, hosted shell regression (use `make hosted-test`), RF2S simulation harness, ChemOS persona tests.

---

## 5. Immediate Next Actions

1. **Create VFS scaffold** under `src/fs/vfs.c` with driver registry + blockdev interface to unblock all drivers.
2. **Lift FAT32 parser** from prototype into `fs/legacy/` with a unit test reading a known FAT image (store under `tests/fs/fixtures/fat32.img`).
3. **Port UCFS unicode helpers** into shared lib and expose CLI (`tbos-ucfs encode "🕉️/📁"`).
4. **Expand PXFS module** beyond stub using docs as schema and wire into `tbos_universal_shell`.
5. **Document ChemFS schema** (extend this plan with detailed spec once VFS + PXFS progress).

This plan keeps FAT/NTFS compatibility, TBOS-native innovation, and ChemOS ambitions aligned so “focus on filesystems” turns into a staging backlog with clear ownership and test criteria.
