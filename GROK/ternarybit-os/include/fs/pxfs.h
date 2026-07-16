/**
 * @file pxfs.h
 * @brief PXFS - Pixel eXchange FileSystem
 *
 * Revolutionary filesystem that stores data as RGB pixel patterns.
 * Achieves compression ratios up to 1365:1 for structured data.
 *
 * On-Disk Format:
 * ┌─────────────────────────────────────────────────────────────┐
 * │ Superblock (512 bytes)                                     │
 * ├─────────────────────────────────────────────────────────────┤
 * │ Inode Table (configurable size)                            │
 * ├─────────────────────────────────────────────────────────────┤
 * │ Pixel Block Bitmap                                         │
 * ├─────────────────────────────────────────────────────────────┤
 * │ Pixel Blocks (data storage)                                │
 * └─────────────────────────────────────────────────────────────┘
 *
 * @version 2.0
 * @date 2025
 */

#ifndef PXFS_H
#define PXFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>  /* ssize_t, off_t */

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSTANTS
 * ═══════════════════════════════════════════════════════════════════════════ */

#define PXFS_MAGIC              0x50584653  /* "PXFS" */
#define PXFS_VERSION            0x0200      /* v2.0 */
#define PXFS_BLOCK_SIZE         4096        /* 4KB blocks */
#define PXFS_SUPERBLOCK_SIZE    512
#define PXFS_NAME_MAX           255
#define PXFS_PATH_MAX           4096

/* Compression modes */
#define PXFS_MODE_RAW           0x00    /* 3 bytes per pixel (1:1) */
#define PXFS_MODE_DENSE         0x01    /* Bit-packed (1:1) */
#define PXFS_MODE_FRACTAL       0x02    /* Pattern-based (~2:1) */
#define PXFS_MODE_QUANTUM       0x03    /* Extreme compression (1365:1) */
#define PXFS_MODE_AUTO          0xFF    /* Auto-detect best mode */

/* Quantum patterns for extreme compression */
#define PXFS_PATTERN_NONE       0x00
#define PXFS_PATTERN_ZERO       0x01    /* All zeros */
#define PXFS_PATTERN_ONES       0x02    /* All 0xFF */
#define PXFS_PATTERN_REPEAT     0x03    /* Single byte repeated */
#define PXFS_PATTERN_SEQUENCE   0x04    /* Sequential values */
#define PXFS_PATTERN_BYTECODE   0x05    /* VM bytecode pattern */
#define PXFS_PATTERN_TEXT       0x06    /* ASCII text pattern */
#define PXFS_PATTERN_LZ         0x07    /* LZ-compressed data */
#define PXFS_PATTERN_CUSTOM     0xFF    /* Custom dictionary */

/* Inode types */
#define PXFS_INODE_FILE         0x01
#define PXFS_INODE_DIR          0x02
#define PXFS_INODE_SYMLINK      0x03
#define PXFS_INODE_SPECIAL      0x04

/* ═══════════════════════════════════════════════════════════════════════════
 * ON-DISK STRUCTURES
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief RGB Pixel - fundamental storage unit
 */
typedef struct __attribute__((packed)) {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} pxfs_pixel_t;

/**
 * @brief Compression header (3 bytes)
 * Stored at the beginning of each compressed block
 */
typedef struct __attribute__((packed)) {
    uint8_t magic;      /* 0x50 'P' */
    uint8_t mode;       /* Compression mode */
    uint8_t pattern;    /* Pattern type (for QUANTUM mode) */
} pxfs_comp_header_t;

/**
 * @brief Superblock - filesystem metadata
 * Located at byte 0, size 512 bytes
 */
typedef struct __attribute__((packed)) {
    uint32_t magic;             /* PXFS_MAGIC */
    uint16_t version;           /* PXFS_VERSION */
    uint16_t flags;             /* Mount flags */

    uint64_t total_blocks;      /* Total blocks in filesystem */
    uint64_t free_blocks;       /* Free blocks available */
    uint64_t total_inodes;      /* Total inodes */
    uint64_t free_inodes;       /* Free inodes */

    uint32_t block_size;        /* Block size (default 4096) */
    uint32_t inode_size;        /* Inode size (default 256) */

    uint64_t inode_table_start; /* Block number of inode table */
    uint64_t bitmap_start;      /* Block number of bitmap */
    uint64_t data_start;        /* Block number of first data block */

    uint8_t  default_mode;      /* Default compression mode */
    uint8_t  reserved1[3];

    uint64_t create_time;       /* Filesystem creation time */
    uint64_t mount_time;        /* Last mount time */
    uint64_t write_time;        /* Last write time */
    uint32_t mount_count;       /* Number of mounts */
    uint32_t max_mount_count;   /* Max mounts before fsck */

    uint8_t  uuid[16];          /* Filesystem UUID */
    char     label[32];         /* Volume label */

    /* Compression statistics */
    uint64_t bytes_stored;      /* Total bytes stored (compressed) */
    uint64_t bytes_original;    /* Total bytes original size */

    /* Pixel-specific metadata */
    uint8_t  separator_rgb[3];  /* Default path separator color */
    uint8_t  reserved2;

    uint8_t  padding[512 - 176]; /* Pad to 512 bytes */
} pxfs_superblock_t;

/**
 * @brief Inode - file/directory metadata
 * Size: 256 bytes
 */
typedef struct __attribute__((packed)) {
    uint32_t mode;              /* File type and permissions */
    uint16_t uid;               /* Owner user ID */
    uint16_t gid;               /* Owner group ID */

    uint64_t size;              /* File size in bytes */
    uint64_t size_compressed;   /* Compressed size in bytes */

    uint64_t atime;             /* Access time */
    uint64_t mtime;             /* Modification time */
    uint64_t ctime;             /* Change time */
    uint64_t crtime;            /* Creation time */

    uint32_t links;             /* Number of hard links */
    uint32_t flags;             /* Inode flags */

    uint8_t  comp_mode;         /* Compression mode used */
    uint8_t  comp_pattern;      /* Pattern (for QUANTUM mode) */
    uint16_t reserved1;

    /* Block pointers */
    uint64_t direct[12];        /* Direct block pointers */
    uint64_t indirect;          /* Single indirect block */
    uint64_t double_indirect;   /* Double indirect block */
    uint64_t triple_indirect;   /* Triple indirect block */

    /* For QUANTUM mode: 3-byte reconstruction data */
    uint8_t  quantum_data[16];  /* Quantum reconstruction info */

    uint8_t  padding[256 - 204]; /* Pad to 256 bytes */
} pxfs_inode_t;

/**
 * @brief Directory entry
 */
typedef struct __attribute__((packed)) {
    uint64_t inode;             /* Inode number */
    uint16_t rec_len;           /* Record length */
    uint8_t  name_len;          /* Name length */
    uint8_t  file_type;         /* File type */
    char     name[PXFS_NAME_MAX + 1];  /* Filename (null-terminated) */
} pxfs_dirent_t;

/**
 * @brief Pixel block header
 * Each data block starts with this header
 */
typedef struct __attribute__((packed)) {
    pxfs_comp_header_t comp;    /* Compression header */
    uint8_t  checksum;          /* Simple checksum */
    uint32_t original_size;     /* Original uncompressed size */
    uint32_t pixel_count;       /* Number of pixels in block */
    /* Followed by pixel data */
} pxfs_block_header_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * COMPRESSION API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Compress data using specified mode
 * @param data Input data
 * @param data_len Input data length
 * @param out Output buffer (must be at least data_len + 12 bytes)
 * @param out_len Output: compressed size
 * @param mode Compression mode
 * @return 0 on success, negative on error
 */
int pxfs_compress(const uint8_t* data, size_t data_len,
                  uint8_t* out, size_t* out_len,
                  uint8_t mode);

/**
 * @brief Decompress data
 * @param data Compressed data
 * @param data_len Compressed data length
 * @param out Output buffer
 * @param out_len Output: decompressed size
 * @return 0 on success, negative on error
 */
int pxfs_decompress(const uint8_t* data, size_t data_len,
                    uint8_t* out, size_t* out_len);

/**
 * @brief Detect best compression mode for data
 * @param data Input data
 * @param data_len Input data length
 * @return Best compression mode
 */
uint8_t pxfs_detect_mode(const uint8_t* data, size_t data_len);

/**
 * @brief Calculate compression ratio
 * @param original Original size
 * @param compressed Compressed size
 * @return Ratio (e.g., 1365.0 for 1365:1)
 */
float pxfs_compression_ratio(size_t original, size_t compressed);

/* ═══════════════════════════════════════════════════════════════════════════
 * FILESYSTEM API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief PXFS filesystem context
 */
typedef struct pxfs_fs pxfs_fs_t;

/**
 * @brief Mount options
 */
typedef struct {
    const char* device;         /* Device path or backing directory */
    const char* mountpoint;     /* Mount point */
    uint8_t     default_mode;   /* Default compression mode */
    bool        read_only;      /* Read-only mount */
    bool        no_compression; /* Disable compression */
} pxfs_mount_opts_t;

/**
 * @brief Create/format a PXFS filesystem
 * @param device Device path
 * @param opts Format options (NULL for defaults)
 * @return 0 on success, negative on error
 */
int pxfs_mkfs(const char* device, const pxfs_mount_opts_t* opts);

/**
 * @brief Mount a PXFS filesystem
 * @param opts Mount options
 * @param fs Output: filesystem context
 * @return 0 on success, negative on error
 */
int pxfs_mount(const pxfs_mount_opts_t* opts, pxfs_fs_t** fs);

/**
 * @brief Unmount a PXFS filesystem
 * @param fs Filesystem context
 * @return 0 on success, negative on error
 */
int pxfs_umount(pxfs_fs_t* fs);

/**
 * @brief File handle
 */
typedef struct pxfs_file pxfs_file_t;

/**
 * @brief Open a file
 */
int pxfs_open(pxfs_fs_t* fs, const char* path, int flags, pxfs_file_t** file);

/**
 * @brief Read from a file
 */
ssize_t pxfs_read(pxfs_file_t* file, void* buf, size_t count, off_t offset);

/**
 * @brief Write to a file
 */
ssize_t pxfs_write(pxfs_file_t* file, const void* buf, size_t count, off_t offset);

/**
 * @brief Close a file
 */
int pxfs_close(pxfs_file_t* file);

/**
 * @brief Get file statistics
 */
int pxfs_stat(pxfs_fs_t* fs, const char* path, pxfs_inode_t* inode);

/**
 * @brief Create a directory
 */
int pxfs_mkdir(pxfs_fs_t* fs, const char* path, uint32_t mode);

/**
 * @brief Remove a file
 */
int pxfs_unlink(pxfs_fs_t* fs, const char* path);

/**
 * @brief Remove a directory
 */
int pxfs_rmdir(pxfs_fs_t* fs, const char* path);

/* ═══════════════════════════════════════════════════════════════════════════
 * UTILITY FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Encode data to RGB pixel array
 */
int pxfs_encode_pixels(const uint8_t* data, size_t len,
                       pxfs_pixel_t* pixels, size_t* pixel_count,
                       uint8_t mode);

/**
 * @brief Decode RGB pixel array to data
 */
int pxfs_decode_pixels(const pxfs_pixel_t* pixels, size_t pixel_count,
                       uint8_t* data, size_t* data_len,
                       uint8_t mode);

/**
 * @brief Convert PXFS path to canonical path
 * Example: {255,0,0}docs{255,0,0}file.txt -> /pxfs/FF0000/docs/file.txt
 */
int pxfs_path_to_canonical(const char* pxfs_path, char* out, size_t out_len);

/**
 * @brief Convert canonical path to PXFS path
 */
int pxfs_path_from_canonical(const char* canonical, uint8_t r, uint8_t g, uint8_t b,
                              char* out, size_t out_len);

/**
 * @brief Print superblock info
 */
void pxfs_print_superblock(const pxfs_superblock_t* sb);

/**
 * @brief Print inode info
 */
void pxfs_print_inode(const pxfs_inode_t* inode);

#ifdef __cplusplus
}
#endif

#endif /* PXFS_H */
