# Filesystem Persistence & Retrieval Plan

**Objective:** Move from purely in-memory prototypes (RAMFS, PXFS stub) to durable storage that can be mounted, read, and restored across boots for both legacy and TBOS-native filesystems.

---

## 1. Persistence Targets

| Filesystem | Current State | Persistence Goal |
|------------|---------------|------------------|
| FAT12/16/32 | Host test image only | Attach real block devices (disk images, USB) via blockdev API; load at boot; optional write support |
| NTFS | Prototype only | Same flow as FAT: blockdev + read-only mount for shared partitions |
| UCFS | Documentation + codec plan | Overlay UCFS metadata atop POSIX/PXFS and serialize symbol maps to disk |
| PXFS | Stub | Serialize pixel trees and compute caches into blockdev-backed files (mkpxfs/mount) |
| RF2S/R2FS | Conceptual | Persist frequency buckets to SDR captures or local files for replay |
| ChemFS | Conceptual | Log chemical states to append-only journals per element/energy level |
| Ramdisk | In-memory only | Optional snapshot/restore using blockdev-backed files to keep compatibility |

---

## 2. Storage Backbone

```
┌──────────────────────────────┐
│ TBOS Block Device Registry   │
│  • blockdev_register/find    │
│  • RAM disks, image files,   │
│    BIOS/UEFI devices         │
└────────────┬─────────────────┘
             │
┌────────────▼────────────────┐
│ VFS Mounts (per filesystem) │
│  • vfs_mount_with_context    │
│  • driver ops dispatch       │
└────────────┬────────────────┘
             │
┌────────────▼────────────────┐
│ Persistence Hooks           │
│  • Boot: register blockdevs │
│  • Shutdown: flush FS state │
│  • Snapshot CLI: mkfs/mount │
└─────────────────────────────┘
```

Block devices can be:
- **Physical**: detected via Boot HAL (LBA, size) and exposed to the kernel.
- **Image-backed**: host-provided `.img` files accessed via a thin host shim (future).
- **RAM**: tests and temporary FS snapshots (`blockdev_create_ramdisk`).

---

## 3. Implementation Roadmap

### 3.1 Block Device Sources
1. **Boot HAL Integration**: extend TBDS descriptors with disk details (drive number, size) and register those blockdevs during kernel init.
2. **Host Shim**: for hosted builds, map files under `build/devices/*.img` to blockdevs so VFS drivers can load real images without QEMU.
3. **Snapshot CLI**: add `scripts/tbos-blockdev.sh` to create/list/dump device images for testing.

### 3.2 Legacy FS Persistence
1. **FAT32/NTFS Mount Command**: add shell command `mount fat32 <device> <mountpoint>` that looks up blockdevs and calls `fat32_mount_blockdev`.
2. **Write Support** (phase 2): implement FAT cluster allocation + journaling; ensure `vfs_write_file` persists back through blockdevs.
3. **Image Packaging**: extend build scripts to bundle FAT images (e.g., `assets/fat_demo.img`) for regression.

### 3.3 UCFS Overlay Persistence
1. **Metadata Store**: persist UCFS symbol tables to `/.tbos/ucfs_map.json` (or pixel path) so unicode roots survive reboots.
2. **Codec Hooks**: when UCFS path is mounted, map overlay entries to an underlying filesystem (PXFS or FAT) and store both the canonical path and the cultural metadata.
3. **Tests**: create UCFS overlay test harness that writes emoji paths to a RAM-backed store, unmounts, remounts, and validates retrieval.

### 3.4 PXFS Persistence
1. **mkpxfs tool**: convert a directory (or dataset) into pixel roots stored inside a blockdev file (store metadata + pixel blocks).
2. **pxfs_mount**: read the serialized structures into memory, keep dirty flags, and flush changes back when requested.
3. **Compute Cache**: persist compute results (spectrum transforms) into dedicated sections so repeated boots reuse cached RGB data.

### 3.5 RF2S/R2FS
1. **Frequency Buckets**: store metadata and payloads in a binary log (frequency, modulation, timestamp) that can be replayed.
2. **SDR Integration**: optionally tie to host-captured IQ files and treat them as blockdev streams.

### 3.6 ChemFS
1. **Schema Definition**: format for element/bond state logs (JSON or binary).
2. **Persistence**: append operations to `chemfs/<element>.log` inside PXFS/UCFS, ensuring transformation steps survive.

---

## 4. Boot & Shutdown Hooks

| Phase | Action |
|-------|--------|
| Early Boot | Parse TBDS for block devices (drive ID, size); call `blockdev_register` for each. |
| FS Init | Mount persistent FSes via config file (e.g., `/etc/tbos/fstab` specifying device + type). |
| Runtime | Expose CLI/API to mount/unmount, snapshot, and inspect devices (`tbos-fsctl`). |
| Shutdown | Traverse mounted FSes, flush dirty data (`blockdev_flush`), snapshot UCFS/PXFS metadata. |

---

## 5. Testing Strategy

1. **Host-level unit tests**: continue the pattern used in `make test-fat32-host` for FAT, UCFS overlays, PXFS caches, etc.
2. **Integration tests**: run QEMU or hosted harness that mounts an image, writes data, reboots (or reinitializes), and confirms persistence.
3. **CLI tests**: simple scripts under `tests/fs/` that exercise `tbos-fsctl` commands and verify logs.

---

## 6. Immediate Actions

1. **Mount CLI**: add a kernel/shell command to mount blockdev-backed filesystems (starting with FAT32) and list mounted devices.
2. **Device Discovery**: wire bootloader/Boot HAL to pass disk info so the kernel auto-registers physical devices.
3. **Snapshot tool**: create `tools/fs/mkfat32.sh` (or similar) to generate images for tests/demos.
4. **UCFS storage**: implement persistent mapping file and integrate with blockdev-backed FS so emoji paths are durable.

Once these are in place, we can iterate on PXFS and ChemFS persistence with confidence that the underlying storage mechanism is solid and testable.
