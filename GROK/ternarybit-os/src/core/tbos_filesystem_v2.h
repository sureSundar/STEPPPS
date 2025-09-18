/**
 * @file tbos_filesystem_v2.h
 * @brief TernaryBit OS File System - SOLID Principles Implementation
 *
 * Refactored file system following SOLID principles:
 * - Single Responsibility: Each class has one reason to change
 * - Open/Closed: Open for extension, closed for modification
 * - Liskov Substitution: Implementations are substitutable
 * - Interface Segregation: Small, focused interfaces
 * - Dependency Inversion: Depend on abstractions, not concretions
 *
 * @version 2.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#ifndef TBOS_FILESYSTEM_V2_H
#define TBOS_FILESYSTEM_V2_H

#include "tbos_interfaces.h"
#include "tbos_base.h"
#include <sys/stat.h>

/**
 * @brief Block Device Interface (ISP)
 * Single responsibility: Raw block I/O operations
 */
typedef struct {
    tbos_result_t (*read)(void* self, uint32_t block, void* buffer, size_t count);
    tbos_result_t (*write)(void* self, uint32_t block, const void* buffer, size_t count);
    tbos_result_t (*flush)(void* self);
    tbos_result_t (*get_size)(void* self);
    tbos_result_t (*format)(void* self, const char* fs_type);
} tbos_block_device_interface_t;

/**
 * @brief Inode Manager Interface (SRP)
 * Single responsibility: Inode allocation and management
 */
typedef struct {
    tbos_result_t (*allocate)(void* self);
    tbos_result_t (*deallocate)(void* self, uint32_t inode_num);
    tbos_result_t (*read_inode)(void* self, uint32_t inode_num);
    tbos_result_t (*write_inode)(void* self, uint32_t inode_num, const void* inode);
    tbos_result_t (*get_stats)(void* self);
} tbos_inode_manager_interface_t;

/**
 * @brief Block Allocator Interface (SRP)
 * Single responsibility: Block allocation and deallocation
 */
typedef struct {
    tbos_result_t (*allocate_block)(void* self);
    tbos_result_t (*allocate_blocks)(void* self, size_t count);
    tbos_result_t (*deallocate_block)(void* self, uint32_t block_num);
    tbos_result_t (*is_allocated)(void* self, uint32_t block_num);
    tbos_result_t (*get_free_count)(void* self);
} tbos_block_allocator_interface_t;

/**
 * @brief Directory Manager Interface (SRP)
 * Single responsibility: Directory operations
 */
typedef struct {
    tbos_result_t (*create_entry)(void* self, uint32_t parent_inode, const char* name, uint32_t inode);
    tbos_result_t (*remove_entry)(void* self, uint32_t parent_inode, const char* name);
    tbos_result_t (*lookup)(void* self, uint32_t parent_inode, const char* name);
    tbos_result_t (*list_entries)(void* self, uint32_t dir_inode);
    tbos_result_t (*is_empty)(void* self, uint32_t dir_inode);
} tbos_directory_manager_interface_t;

/**
 * @brief Path Resolver Interface (SRP)
 * Single responsibility: Path resolution and validation
 */
typedef struct {
    tbos_result_t (*resolve)(void* self, const char* path);
    tbos_result_t (*resolve_parent)(void* self, const char* path);
    tbos_result_t (*validate_path)(void* self, const char* path);
    tbos_result_t (*normalize_path)(void* self, const char* path);
} tbos_path_resolver_interface_t;

/**
 * @brief File Cache Interface (SRP)
 * Single responsibility: Caching file system data
 */
typedef struct {
    tbos_result_t (*cache_inode)(void* self, uint32_t inode_num, const void* inode);
    tbos_result_t (*get_cached_inode)(void* self, uint32_t inode_num);
    tbos_result_t (*cache_block)(void* self, uint32_t block_num, const void* block);
    tbos_result_t (*get_cached_block)(void* self, uint32_t block_num);
    tbos_result_t (*invalidate)(void* self, uint32_t inode_num);
    tbos_result_t (*flush_cache)(void* self);
} tbos_file_cache_interface_t;

/**
 * @brief Inode Implementation (following SRP)
 */
typedef struct tbos_inode_v2 {
    tbos_base_t base;

    uint32_t inode_number;
    uint16_t mode;              // File type and permissions
    uint16_t links_count;       // Hard links count
    uint32_t uid;               // User ID
    uint32_t gid;               // Group ID
    uint64_t size;              // File size in bytes
    uint64_t blocks;            // Number of 512-byte blocks allocated
    uint64_t atime;             // Access time
    uint64_t ctime;             // Change time
    uint64_t mtime;             // Modify time

    // Block pointers (ext2-style)
    uint32_t direct[12];        // Direct block pointers
    uint32_t indirect;          // Single indirect
    uint32_t double_indirect;   // Double indirect
    uint32_t triple_indirect;   // Triple indirect

    // Extended attributes (flexible)
    void* extended_attrs;
    size_t extended_attrs_size;
} tbos_inode_v2_t;

/**
 * @brief Directory Entry (compact design)
 */
typedef struct {
    uint32_t inode;             // Inode number
    uint16_t rec_len;           // Record length
    uint8_t name_len;           // Name length
    uint8_t file_type;          // File type
    char name[];                // Variable-length name
} tbos_dirent_v2_t;

/**
 * @brief File Descriptor (lightweight)
 */
typedef struct {
    uint32_t inode_num;
    uint64_t position;
    uint32_t flags;
    void* private_data;         // For caching
} tbos_file_handle_t;

/**
 * @brief Superblock (metadata container)
 */
typedef struct {
    uint32_t magic;             // File system magic number
    uint32_t version;           // File system version
    uint32_t block_size;        // Block size in bytes
    uint32_t blocks_count;      // Total blocks
    uint32_t free_blocks_count; // Free blocks
    uint32_t inodes_count;      // Total inodes
    uint32_t free_inodes_count; // Free inodes
    uint32_t first_data_block;  // First data block
    uint32_t blocks_per_group;  // Blocks per block group
    uint32_t inodes_per_group;  // Inodes per block group
    uint64_t mount_time;        // Last mount time
    uint64_t write_time;        // Last write time
    uint16_t mount_count;       // Mount count
    uint16_t max_mount_count;   // Maximum mount count
    uint16_t state;             // File system state
    uint16_t errors;            // Error behavior
    char uuid[16];              // File system UUID
    char volume_name[16];       // Volume name
    uint8_t reserved[1024];     // Reserved space
} tbos_superblock_v2_t;

/**
 * @brief File System Implementation (Composition over inheritance)
 */
typedef struct tbos_filesystem_v2 {
    tbos_base_t base;

    // Dependencies (Dependency Injection)
    tbos_block_device_interface_t* block_device;
    tbos_inode_manager_interface_t* inode_manager;
    tbos_block_allocator_interface_t* block_allocator;
    tbos_directory_manager_interface_t* directory_manager;
    tbos_path_resolver_interface_t* path_resolver;
    tbos_file_cache_interface_t* cache;

    // Core data
    tbos_superblock_v2_t* superblock;
    tbos_memory_pool_t* inode_pool;
    tbos_memory_pool_t* block_pool;

    // File handles (using Object Pool pattern)
    tbos_file_handle_t* file_handles;
    size_t max_file_handles;
    uint32_t next_handle_id;

    // State
    bool mounted;
    bool read_only;
    pthread_rwlock_t fs_lock;   // Reader-writer lock for concurrency

    // Configuration (Strategy Pattern)
    struct {
        size_t cache_size;
        bool enable_journaling;
        bool enable_compression;
        uint32_t sync_interval;
    } config;
} tbos_filesystem_v2_t;

/**
 * @brief File System Factory (Factory Pattern + Abstract Factory)
 */
typedef struct {
    tbos_filesystem_v2_t* (*create_filesystem)(const char* type, const char* device);
    tbos_block_device_interface_t* (*create_block_device)(const char* device_path);
    tbos_inode_manager_interface_t* (*create_inode_manager)(size_t max_inodes);
    tbos_block_allocator_interface_t* (*create_block_allocator)(size_t total_blocks);
    tbos_directory_manager_interface_t* (*create_directory_manager)(void);
    tbos_path_resolver_interface_t* (*create_path_resolver)(void);
    tbos_file_cache_interface_t* (*create_file_cache)(size_t cache_size);
} tbos_filesystem_factory_t;

// Factory functions (following DRY principle)
tbos_filesystem_factory_t* tbos_get_filesystem_factory(void);
tbos_filesystem_v2_t* tbos_create_ext2_filesystem(const char* device);
tbos_filesystem_v2_t* tbos_create_simple_filesystem(const char* device);

// Implementation of filesystem interface (Adapter Pattern)
typedef struct tbos_filesystem_adapter {
    tbos_filesystem_interface_t interface;  // Interface implementation
    tbos_filesystem_v2_t* filesystem;       // Actual implementation
} tbos_filesystem_adapter_t;

tbos_filesystem_adapter_t* tbos_filesystem_adapter_create(tbos_filesystem_v2_t* fs);

// High-level operations (Facade Pattern)
typedef struct {
    tbos_result_t (*mount)(const char* device, const char* mount_point);
    tbos_result_t (*unmount)(const char* mount_point);
    tbos_result_t (*format)(const char* device, const char* fs_type);
    tbos_result_t (*check)(const char* device, bool repair);
    tbos_result_t (*get_info)(const char* mount_point);
} tbos_fs_manager_t;

extern tbos_fs_manager_t* g_tbos_fs_manager;

// Utility functions following DRY principle
tbos_result_t tbos_validate_filesystem_params(const char* device, const char* mount_point);
tbos_result_t tbos_check_filesystem_permissions(const char* device);
const char* tbos_get_error_string(int error_code);

// Observer pattern for filesystem events
typedef enum {
    TBOS_FS_EVENT_MOUNT,
    TBOS_FS_EVENT_UNMOUNT,
    TBOS_FS_EVENT_FILE_CREATED,
    TBOS_FS_EVENT_FILE_DELETED,
    TBOS_FS_EVENT_SPACE_LOW,
    TBOS_FS_EVENT_ERROR
} tbos_fs_event_type_t;

typedef struct {
    void (*on_filesystem_event)(tbos_fs_event_type_t event, const char* path, void* data);
} tbos_fs_observer_t;

tbos_result_t tbos_register_fs_observer(tbos_fs_observer_t* observer);
tbos_result_t tbos_unregister_fs_observer(tbos_fs_observer_t* observer);

#endif // TBOS_FILESYSTEM_V2_H