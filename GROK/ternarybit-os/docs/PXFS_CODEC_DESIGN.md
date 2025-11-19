# PXFS Codec & Integration Plan

**Objective:** Implement Phase 1 PXFS paths where each root/delimiter is a pixel value (RGB triple). Paths look like sequences of `{R,G,B}` tokens, and each component corresponds to a pixel in a compute grid. PXFS codecs translate these paths into canonical storage (4 KB blocks) backed by the existing VFS/FAT infrastructure.

---

## 1. Path Syntax

- A PXFS path starts with a pixel token defining the root: `{R,G,B}` where each component is 0‑255.
- The same pixel token acts as the delimiter for subsequent components; each component may contain arbitrary UTF‑8 text (file or directory name).

Example:
```
{255,0,0}system{255,0,0}kernel{255,0,0}boot.bin
```
Root/delimiter = pure red; components = `system`, `kernel`, `boot.bin`.

Brackets `{}` and commas are literal; whitespace is optional but discouraged.

### Token Grammar
```
pxfs_path := pixel_token component+
pixel_token := '{' decimal ',' decimal ',' decimal '}'
component   := [^'{'']* (UTF-8 literal until next pixel_token)
```

---

## 2. Canonical Representation

Similar to UCFS, PXFS canonicalizes to a POSIX-style path under `/pxfs/<pixel-id>/…`, where `<pixel-id>` is `RRGGBB` hex.

Example:
```
{255,0,0}system{255,0,0}kernel{255,0,0}boot.bin
→ canonical path: /pxfs/FF0000/system/kernel/boot.bin
```

This canonical path sits on top of an underlying store (initially FAT32 or RAMFS) so we can reuse current drivers.

---

## 3. Codec Module

`pxfs_codec.c/.h` responsibilities:

```c
typedef struct {
    uint8_t r, g, b;
    size_t component_count;
    char** components;
} pxfs_path_t;

int pxfs_parse(const char* path, pxfs_path_t* out);
void pxfs_free(pxfs_path_t* path);
int pxfs_to_canonical(const pxfs_path_t* path, char* buffer, size_t buflen);
```

Parsing steps:
1. Parse first `{R,G,B}` token → store as root pixel.
2. Split the rest of the string whenever the same `{R,G,B}` token reappears.
3. Reject empty components or invalid RGB values.

---

## 4. 4 KB Block Mapping

PXFS leans on pixel-based addressing for compute caches. For Phase 1 we map each canonical path to a 4 KB chunk in the backing filesystem:

- When writing data, split into 4 KB blocks; each block is stored in a FAT/VFS file named after the pixel path.
- Optionally store block metadata (e.g., block indices) under `/pxfs/.blocks/<pixel-id>/<hash>.meta`.

This keeps compatibility with existing FAT32 driver while we prototype pixel semantics.

---

## 5. VFS Overlay

Similar to UCFS:

- Add a PXFS overlay driver that detects `{R,G,B}` paths, converts them via the codec, ensures backing directories exist, and forwards read/write to the canonical path.
- Default backing root: `/pxfs_store`.
- Expose helper functions:
  ```c
  int pxfs_resolve_path(const char* pxfs_path, char* buffer, size_t buflen);
  int pxfs_write(const char* pxfs_path, const void* data, size_t size);
  int pxfs_read(const char* pxfs_path, void* buffer, size_t max_size, size_t* out_size);
  ```

---

## 6. CLI & Tooling

- `tbos-pxfs encode "{R,G,B}foo{R,G,B}bar.txt"` → show parsed components + canonical path.
- `tbos-pxfs ls {…}` → list pixel-root directories.
- `tbos-pxfs block-info {…}` → display 4 KB block metadata (later phases).

---

## 7. Testing Plan

1. **Codec unit tests**: host-level tests verifying parsing, canonicalization, and invalid cases (bad RGB, missing braces, empty components).
2. **Overlay tests**: mount PXFS overlay over RAMFS, write via PXFS paths, read directly from canonical path to confirm bridging.
3. **Integration**: extend shell resolver to detect `{R,G,B}` bracketed paths (similar to UCFS) and run scripted tests (`make test-pxfs-host`).

---

## 8. Incremental Steps

1. Implement `pxfs_codec` with unit tests.
2. Create overlay helpers and a default backing root.
3. Wire shell commands to resolve PXFS paths (alongside UCFS).
4. Add simple CLI utilities (`tbos-pxfs encode`).
5. Iterate on 4 KB block metadata (optional for later phases).

Once this is in place, we can layer in Pixel-specific behavior (color harmony ACLs, compute caching) while still leveraging the existing VFS/FAT infrastructure for persistence.
