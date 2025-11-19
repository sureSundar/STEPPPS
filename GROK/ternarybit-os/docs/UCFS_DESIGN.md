# UCFS (Unicode Character Filesystem) – Phase 1 Design

**Goal:** Support paths where a single Unicode character acts as both the *root identifier* and the *path delimiter*. **Phase 1 syntax wraps the delimiter in brackets** so we can distinguish it from literal characters inside component names. Examples:

- `[a]Home[a]Docs[a]resume.txt` (root/delimiter = `a`)
- `[🕉️]मंत्र[🕉️]गायत्री[🕉️]mantra.mp3` (root/delimiter = `🕉️`)
- `[🌍]🏠[🌍]👤[🌍]documents[🌍]resume.pdf` (root/delimiter = `🌍`)

The POSIX `/` path continues to exist, but UCFS lets users address files via culturally meaningful delimiters. Phase 1 focuses on **single-delimiter paths**; mixed delimiters will come later.

---

## 1. Core Concepts

1. **Delimiter as Root:** The first bracketed Unicode symbol (`[UC]`) defines the UCFS namespace (root). Everything between subsequent `[UC]` tokens are path components.  
   Example: `[🕉️]music[🕉️]chants[🕉️]108.mp3` → root `🕉️`, components `music`, `chants`, `108.mp3`.

2. **Overlay Model:** UCFS acts as a translation layer on top of an underlying filesystem (PXFS, FAT, etc.). Each UCFS component maps to a canonical path segment (e.g., `/ucfs/🕉️/…`). We store metadata (culture, chakra, semantics) alongside the mapping.

3. **Normalization:** Input paths are normalized (NFC) before parsing to avoid duplicate representations of the same symbol (e.g., composed vs. decomposed characters).

---

## 2. Path Parsing Algorithm

```
function parse_ucfs_path(s):
    if s empty or s[0] != '[': error
    delimiter = unicode scalar between first '[' and matching ']'
    components = []
    buffer = ""
    cursor = position after closing ']'
    while cursor < len(s):
        if substring starting at cursor == "[" + delimiter + "]":
            if buffer empty: error (no empty components)
            append buffer to components
            buffer = ""
            cursor += len("[delimiter]")
        else:
            buffer += next UTF-8 scalar
            cursor += scalar length
    if buffer != "":
        components.append(buffer)
    return delimiter, components
```

Edge cases:
- Double delimiter (`aa`): represents an empty component; we treat it as invalid for now.
- Trailing delimiter (`aFooa`): last component empty → invalid.

---

## 3. Internal Representation

After parsing, we canonicalize the path into:
```
/ucfs/<delimiter_hex>/<component0>/<component1>/... 
```

Example:
```
[🕉️]music[🕉️]chants[🕉️]108.mp3
→ delimiter = 🕉️ (U+1F549)
→ components = ["music", "chants", "108.mp3"]
→ canonical path = /ucfs/U+1F549/music/chants/108.mp3
```

The canonical path lives in the existing VFS namespace so downstream drivers operate on UTF‑8 subpaths. Metadata (culture ID, chakra mapping) may be stored in sidecar files under `/ucfs/.meta/U+1F549/...`.

---

## 4. UCFS Codec Module

New module `src/core/filesystem/ucfs_codec.c` will provide:

```c
typedef struct {
    uint32_t delimiter;        // Unicode code point
    char delimiter_utf8[8];    // UTF-8 bytes (plus variation selectors)
    size_t delimiter_len;
    size_t component_count;
    char** components;
} ucfs_path_t;

int ucfs_parse(const char* utf8_path, ucfs_path_t* out);
void ucfs_free(ucfs_path_t* path);
int ucfs_to_canonical(const ucfs_path_t* path, char* buffer, size_t buflen);
int ucfs_to_canonical_with_base(const ucfs_path_t* path, const char* base, char* buffer, size_t buflen);
```

Normalization will rely on a simple UTF-8 helper initially; full NFC may leverage a lightweight normalization table later.

---

## 5. UCFS VFS Driver (Phase 1)

- Acts as an overlay driver that captures UCFS paths, converts them to canonical paths, and forwards to a backing driver (default: RAMFS or PXFS).
- Configuration file `/etc/tbos/ucfs.conf` indicates which backing mount to use for each delimiter (initially default to `/ucfs_store`).
- For now, reads/writes simply delegate to the underlying store; future phases will attach metadata.

Mount example:
```
vfs_mount("/ucfs", &ucfs_driver);
```
User command:
```
cat [🕉️]music[🕉️]chants[🕉️]108.mp3
```
Driver flow:
1. Parse path via codec.
2. Construct canonical path `/ucfs_store/U+1F549/music/chants/108.mp3`.
3. Call backing driver’s read/write operations.

---

## 6. CLI & Tooling

- `tbos-ucfs encode <path>` → shows delimiter, components, canonical path.
- `tbos-ucfs map <delimiter> <target>` → map delimiter to custom backing root.
- `tbos-ucfs list` → list registered delimiters and metadata.

These tools will write to `/etc/tbos/ucfs.conf` and optionally to metadata files inside the backing store.

---

## 7. Testing Strategy

1. **Unit tests** for the codec (parse/normalize, canonical path generation) using host builds.
2. **Integration tests** mounting UCFS over RAMFS:
   - `make test-ucfs-host`: create files using UCFS paths, read them back, restart test harness to simulate persistence.
3. Future: QEMU test that uses UCFS paths in the shell to ensure kernel VFS handles them end-to-end.

---

## 8. Phase 1 Deliverables

1. `ucfs_codec.c/.h` with parsing + canonicalization.
2. UCFS overlay driver hooking into VFS and a configurable backing path.
3. Host test exercising read/write via UCFS paths.
4. CLI helper `tbos-ucfs` for inspection (basic encode/list).

After this, we can expand to:
- Mixed delimiters per path.
- Metadata persistence (culture, chakra).
- Two-way mapping (canonical path → UCFS path for listings).
- Integration with PXFS and RF2S.

Let me know if you want any tweaks before coding starts.
