/**
 * @file tbos_ramdisk.h
 * @brief TernaryBit OS - In-Memory Ramdisk Filesystem
 *
 * Simple, fast in-memory filesystem for TBOS.
 * Conscious file operations with karma tracking.
 *
 * @version 1.0
 * @date 2025-11-03
 */

#ifndef TBOS_RAMDISK_H
#define TBOS_RAMDISK_H

#include "../tbos_base.h"
#include "../tbos_memory.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* CONSTANTS                                                                 */
/* ========================================================================= */

#define TBOS_MAX_FILENAME      256
#define TBOS_MAX_PATH          1024
#define TBOS_MAX_FILES         1024
#define TBOS_MAX_OPEN_FILES    64
#define TBOS_MAX_FILE_SIZE     (1024 * 1024)  /* 1MB per file */

/* File types */
#define TBOS_FT_REGULAR        1
#define TBOS_FT_DIRECTORY      2

/* File permissions */
#define TBOS_PERM_READ         0x01
#define TBOS_PERM_WRITE        0x02
#define TBOS_PERM_EXECUTE      0x04

/* Open flags */
#define TBOS_O_RDONLY          0x00
#define TBOS_O_WRONLY          0x01
#define TBOS_O_RDWR            0x02
#define TBOS_O_CREAT           0x04
#define TBOS_O_TRUNC           0x08
#define TBOS_O_APPEND          0x10

/* Seek modes */
#define TBOS_SEEK_SET          0
#define TBOS_SEEK_CUR          1
#define TBOS_SEEK_END          2

/* ========================================================================= */
/* FILE STRUCTURES                                                           */
/* ========================================================================= */

/**
 * @brief Inode - file metadata
 */
typedef struct tbos_inode {
    uint32_t inode_num;
    uint8_t file_type;
    uint32_t permissions;
    size_t file_size;
    void* data;
    uint64_t created_time;
    uint64_t modified_time;
    uint32_t ref_count;
    char path[TBOS_MAX_PATH];

    /* Consciousness */
    bool is_sacred;  /* Sacred files get extra protection */
    uint32_t access_count;
} tbos_inode_t;

/**
 * @brief Directory entry
 */
typedef struct tbos_dirent {
    char name[TBOS_MAX_FILENAME];
    uint32_t inode_num;
    struct tbos_dirent* next;
} tbos_dirent_t;

/**
 * @brief Directory structure
 */
typedef struct tbos_directory {
    uint32_t inode_num;
    char path[TBOS_MAX_PATH];
    tbos_dirent_t* entries;
    uint32_t entry_count;
    struct tbos_directory* parent;
    struct tbos_directory* next;
} tbos_directory_t;

/**
 * @brief Open file descriptor
 */
typedef struct {
    uint32_t inode_num;
    uint32_t flags;
    size_t position;
    bool in_use;
} tbos_fd_t;

/**
 * @brief Filesystem statistics
 */
typedef struct {
    uint32_t total_files;
    uint32_t total_directories;
    size_t total_bytes_used;
    size_t total_bytes_available;
    uint32_t open_files;
} tbos_fs_stats_t;

/* ========================================================================= */
/* FILESYSTEM API                                                            */
/* ========================================================================= */

/**
 * @brief Initialize ramdisk filesystem
 *
 * @param size_bytes Total ramdisk size in bytes
 * @return 0 on success, negative on error
 */
int tbos_ramdisk_init(size_t size_bytes);

/**
 * @brief Create root directory structure
 *
 * Creates /, /bin, /tmp, /home, /dev
 */
int tbos_ramdisk_create_root_structure(void);

/**
 * @brief Open a file
 *
 * @param path File path
 * @param flags Open flags (TBOS_O_*)
 * @return File descriptor >= 0, or negative on error
 */
int tbos_open(const char* path, uint32_t flags);

/**
 * @brief Close a file
 *
 * @param fd File descriptor
 * @return 0 on success, negative on error
 */
int tbos_close(int fd);

/**
 * @brief Read from file
 *
 * @param fd File descriptor
 * @param buffer Buffer to read into
 * @param count Number of bytes to read
 * @return Bytes read, or negative on error
 */
int tbos_read(int fd, void* buffer, size_t count);

/**
 * @brief Write to file
 *
 * @param fd File descriptor
 * @param buffer Data to write
 * @param count Number of bytes to write
 * @return Bytes written, or negative on error
 */
int tbos_write(int fd, const void* buffer, size_t count);

/**
 * @brief Seek to position in file
 *
 * @param fd File descriptor
 * @param offset Offset in bytes
 * @param whence TBOS_SEEK_SET/CUR/END
 * @return New position, or negative on error
 */
int tbos_seek(int fd, int offset, int whence);

/**
 * @brief Create directory
 *
 * @param path Directory path
 * @return 0 on success, negative on error
 */
int tbos_mkdir(const char* path);

/**
 * @brief Remove directory
 *
 * @param path Directory path
 * @return 0 on success, negative on error
 */
int tbos_rmdir(const char* path);

/**
 * @brief Delete file
 *
 * @param path File path
 * @return 0 on success, negative on error
 */
int tbos_unlink(const char* path);

/**
 * @brief List directory contents
 *
 * @param path Directory path
 * @param buffer Buffer to store entries
 * @param max_entries Maximum number of entries
 * @return Number of entries, or negative on error
 */
int tbos_readdir(const char* path, tbos_dirent_t* buffer, uint32_t max_entries);

/**
 * @brief Get filesystem statistics
 *
 * @param stats Pointer to stats structure
 * @return 0 on success, negative on error
 */
int tbos_fs_get_stats(tbos_fs_stats_t* stats);

/**
 * @brief Print filesystem status
 */
void tbos_fs_print_status(void);

/**
 * @brief Print directory tree
 *
 * @param path Root path to print from
 */
void tbos_fs_print_tree(const char* path);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_RAMDISK_H */
