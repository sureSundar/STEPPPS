// TBOS PXFS Core - Sprint 19
// 🕉️ Sabarigiri Vaasa Saranam 🕉️
// Pixel eXchange File System - Content-addressed storage

#include <stdint.h>
#include <stddef.h>

// External functions
extern void kernel_print(const char* str);
extern void kernel_print_hex(uint32_t value);
extern void* kmalloc(size_t size);
extern void kfree(void* ptr);

// PXFS Constants
#define PXFS_MAGIC 0x50584653      // "PXFS"
#define PXFS_VERSION 1
#define PXFS_BLOCK_SIZE 4096
#define PXFS_MAX_NAME 256
#define PXFS_MAX_FILES 1024
#define PXFS_HASH_SIZE 32          // SHA-256 size

// PXFS Structures
typedef struct {
    uint32_t magic;                // PXFS magic number
    uint32_t version;              // Filesystem version
    uint32_t block_size;           // Block size (4096)
    uint32_t total_blocks;         // Total filesystem blocks
    uint32_t free_blocks;          // Free blocks available
    uint32_t file_count;           // Number of files
    uint8_t  volume_id[16];        // Volume identifier
    char     volume_name[64];      // Volume name
    uint32_t created_time;         // Creation timestamp
    uint32_t modified_time;        // Last modification
} __attribute__((packed)) pxfs_superblock_t;

typedef struct {
    uint8_t  hash[PXFS_HASH_SIZE]; // Content hash (SHA-256)
    char     name[PXFS_MAX_NAME];  // Human-readable name
    uint32_t size;                 // File size in bytes
    uint32_t blocks;               // Number of blocks
    uint32_t block_list[16];       // Block pointers (max 64KB files)
    uint32_t created_time;         // Creation time
    uint32_t accessed_time;        // Last access time
    uint32_t modified_time;        // Last modification
    uint16_t permissions;          // File permissions
    uint16_t type;                 // File type (regular, dir, etc.)
    uint32_t checksum;             // Integrity checksum
} __attribute__((packed)) pxfs_inode_t;

typedef struct {
    char     name[PXFS_MAX_NAME];  // Entry name
    uint32_t inode;                // Inode number
    uint16_t type;                 // Entry type
    uint16_t reserved;             // Reserved
} __attribute__((packed)) pxfs_dirent_t;

// Forward declarations
uint32_t pxfs_alloc_block(void);
void pxfs_free_block(uint32_t block_num);
void pxfs_create_root_dir(void);

// PXFS Global State
static pxfs_superblock_t* superblock = NULL;
static pxfs_inode_t* inode_table = NULL;
static uint8_t* block_bitmap = NULL;
static uint32_t fs_base_addr = 0x200000;  // Start at 2MB

// Hash function (simplified SHA-256-like)
uint32_t pxfs_hash_simple(const void* data, size_t len) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint32_t hash = 0x811C9DC5;  // FNV offset basis

    for (size_t i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= 0x01000193;       // FNV prime
    }

    return hash;
}

// Initialize PXFS
int pxfs_init(void) {
    kernel_print("[PXFS] Initializing Pixel eXchange File System...\n");

    // Allocate superblock
    superblock = (pxfs_superblock_t*)kmalloc(sizeof(pxfs_superblock_t));
    if (!superblock) {
        kernel_print("[PXFS] ERROR: Cannot allocate superblock\n");
        return -1;
    }

    // Initialize superblock
    superblock->magic = PXFS_MAGIC;
    superblock->version = PXFS_VERSION;
    superblock->block_size = PXFS_BLOCK_SIZE;
    superblock->total_blocks = 1024;      // 4MB filesystem
    superblock->free_blocks = 1000;       // Reserve some for metadata
    superblock->file_count = 0;

    // Set volume info
    const char* vol_name = "TBOS-Sacred-Volume";
    for (int i = 0; i < 64 && vol_name[i]; i++) {
        superblock->volume_name[i] = vol_name[i];
    }

    // Allocate inode table
    inode_table = (pxfs_inode_t*)kmalloc(sizeof(pxfs_inode_t) * PXFS_MAX_FILES);
    if (!inode_table) {
        kernel_print("[PXFS] ERROR: Cannot allocate inode table\n");
        return -1;
    }

    // Clear inode table
    for (int i = 0; i < PXFS_MAX_FILES; i++) {
        inode_table[i].size = 0;
        inode_table[i].blocks = 0;
        inode_table[i].name[0] = '\0';
    }

    // Allocate block bitmap
    block_bitmap = (uint8_t*)kmalloc(superblock->total_blocks / 8);
    if (!block_bitmap) {
        kernel_print("[PXFS] ERROR: Cannot allocate block bitmap\n");
        return -1;
    }

    // Clear bitmap (all blocks free)
    for (uint32_t i = 0; i < superblock->total_blocks / 8; i++) {
        block_bitmap[i] = 0;
    }

    // Create root directory
    pxfs_create_root_dir();

    kernel_print("[PXFS] Initialized successfully!\n");
    kernel_print("  Magic: 0x");
    kernel_print_hex(superblock->magic);
    kernel_print("\n  Blocks: ");
    kernel_print_hex(superblock->total_blocks);
    kernel_print("\n  Volume: ");
    kernel_print(superblock->volume_name);
    kernel_print("\n");

    return 0;
}

// Create root directory
void pxfs_create_root_dir(void) {
    pxfs_inode_t* root = &inode_table[0];

    // Set root directory properties
    root->name[0] = '/';
    root->name[1] = '\0';
    root->size = sizeof(pxfs_dirent_t) * 2;  // . and .. entries
    root->blocks = 1;
    root->block_list[0] = pxfs_alloc_block();
    root->type = 1;  // Directory type
    root->permissions = 0755;

    // Create . and .. entries
    pxfs_dirent_t* entries = (pxfs_dirent_t*)(fs_base_addr + root->block_list[0] * PXFS_BLOCK_SIZE);

    // . entry (self)
    entries[0].name[0] = '.';
    entries[0].name[1] = '\0';
    entries[0].inode = 0;
    entries[0].type = 1;

    // .. entry (parent, same as self for root)
    entries[1].name[0] = '.';
    entries[1].name[1] = '.';
    entries[1].name[2] = '\0';
    entries[1].inode = 0;
    entries[1].type = 1;

    superblock->file_count = 1;
}

// Allocate a block
uint32_t pxfs_alloc_block(void) {
    for (uint32_t i = 0; i < superblock->total_blocks; i++) {
        uint32_t byte_idx = i / 8;
        uint32_t bit_idx = i % 8;

        if (!(block_bitmap[byte_idx] & (1 << bit_idx))) {
            // Block is free, allocate it
            block_bitmap[byte_idx] |= (1 << bit_idx);
            superblock->free_blocks--;
            return i;
        }
    }

    return 0xFFFFFFFF;  // No free blocks
}

// Free a block
void pxfs_free_block(uint32_t block_num) {
    if (block_num >= superblock->total_blocks) return;

    uint32_t byte_idx = block_num / 8;
    uint32_t bit_idx = block_num % 8;

    block_bitmap[byte_idx] &= ~(1 << bit_idx);
    superblock->free_blocks++;
}

// Create a file
int pxfs_create_file(const char* name, const void* data, size_t size) {
    if (superblock->file_count >= PXFS_MAX_FILES) {
        kernel_print("[PXFS] ERROR: Maximum files reached\n");
        return -1;
    }

    // Find free inode
    uint32_t inode_num = 0;
    for (uint32_t i = 1; i < PXFS_MAX_FILES; i++) {  // Skip root
        if (inode_table[i].size == 0) {
            inode_num = i;
            break;
        }
    }

    if (inode_num == 0) {
        kernel_print("[PXFS] ERROR: No free inodes\n");
        return -1;
    }

    pxfs_inode_t* inode = &inode_table[inode_num];

    // Set file properties
    for (int i = 0; i < PXFS_MAX_NAME && name[i]; i++) {
        inode->name[i] = name[i];
    }
    inode->size = size;
    inode->blocks = (size + PXFS_BLOCK_SIZE - 1) / PXFS_BLOCK_SIZE;
    inode->type = 0;  // Regular file
    inode->permissions = 0644;

    // Calculate content hash
    uint32_t hash = pxfs_hash_simple(data, size);
    *((uint32_t*)inode->hash) = hash;

    // Allocate blocks and write data
    const uint8_t* src = (const uint8_t*)data;
    for (uint32_t i = 0; i < inode->blocks; i++) {
        uint32_t block = pxfs_alloc_block();
        if (block == 0xFFFFFFFF) {
            kernel_print("[PXFS] ERROR: Cannot allocate block\n");
            return -1;
        }

        inode->block_list[i] = block;

        // Copy data to block
        uint8_t* dest = (uint8_t*)(fs_base_addr + block * PXFS_BLOCK_SIZE);
        size_t copy_size = (size > PXFS_BLOCK_SIZE) ? PXFS_BLOCK_SIZE : size;

        for (size_t j = 0; j < copy_size; j++) {
            dest[j] = src[j];
        }

        src += copy_size;
        size -= copy_size;
    }

    superblock->file_count++;

    kernel_print("[PXFS] Created file: ");
    kernel_print(name);
    kernel_print(" (");
    kernel_print_hex(inode->size);
    kernel_print(" bytes)\n");

    return inode_num;
}

// Read a file
int pxfs_read_file(const char* name, void* buffer, size_t max_size) {
    // Find file by name
    for (uint32_t i = 0; i < PXFS_MAX_FILES; i++) {
        if (inode_table[i].size > 0) {
            // Compare names
            int match = 1;
            for (int j = 0; j < PXFS_MAX_NAME; j++) {
                if (inode_table[i].name[j] != name[j]) {
                    match = 0;
                    break;
                }
                if (name[j] == '\0') break;
            }

            if (match) {
                // Found file, read it
                pxfs_inode_t* inode = &inode_table[i];
                size_t to_read = (inode->size < max_size) ? inode->size : max_size;

                uint8_t* dest = (uint8_t*)buffer;
                size_t remaining = to_read;

                for (uint32_t block_idx = 0; block_idx < inode->blocks && remaining > 0; block_idx++) {
                    uint8_t* src = (uint8_t*)(fs_base_addr + inode->block_list[block_idx] * PXFS_BLOCK_SIZE);
                    size_t copy_size = (remaining > PXFS_BLOCK_SIZE) ? PXFS_BLOCK_SIZE : remaining;

                    for (size_t j = 0; j < copy_size; j++) {
                        dest[j] = src[j];
                    }

                    dest += copy_size;
                    remaining -= copy_size;
                }

                return to_read;
            }
        }
    }

    return -1;  // File not found
}

// List files
void pxfs_list_files(void) {
    kernel_print("\n=== PXFS File Listing ===\n");
    kernel_print("Name                Size      Hash\n");
    kernel_print("-----------------------------------\n");

    for (uint32_t i = 0; i < PXFS_MAX_FILES; i++) {
        if (inode_table[i].size > 0) {
            kernel_print(inode_table[i].name);
            kernel_print("    ");
            kernel_print_hex(inode_table[i].size);
            kernel_print("    0x");
            kernel_print_hex(*((uint32_t*)inode_table[i].hash));
            kernel_print("\n");
        }
    }

    kernel_print("Total files: ");
    kernel_print_hex(superblock->file_count);
    kernel_print("\nFree blocks: ");
    kernel_print_hex(superblock->free_blocks);
    kernel_print("\n");
}

// Get filesystem info
void pxfs_info(void) {
    kernel_print("\n=== PXFS Information ===\n");
    kernel_print("Magic: 0x");
    kernel_print_hex(superblock->magic);
    kernel_print("\nVersion: ");
    kernel_print_hex(superblock->version);
    kernel_print("\nBlock Size: ");
    kernel_print_hex(superblock->block_size);
    kernel_print("\nTotal Blocks: ");
    kernel_print_hex(superblock->total_blocks);
    kernel_print("\nFree Blocks: ");
    kernel_print_hex(superblock->free_blocks);
    kernel_print("\nFiles: ");
    kernel_print_hex(superblock->file_count);
    kernel_print("\nVolume: ");
    kernel_print(superblock->volume_name);
    kernel_print("\n");
}