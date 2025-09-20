/**
 * @file memory_manager.h
 * @brief Universal Memory Management System for TernaryBit OS
 *
 * Traceability:
 * - PRD: PR-021 (Performance Requirements)
 * - FRD: FR-UHS-002 (Memory Adaptation)
 * - NFRD: NFR-PFM-005 (Memory Utilization Efficiency)
 * - HLD: Memory Manager Component
 * - LLD: memory_layout_t structure
 * - PSC: AllocateMemory() algorithm
 *
 * @version 1.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "hardware_detector.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Memory allocation flags
#define MEM_FLAG_NONE           0x00000000
#define MEM_FLAG_ZERO           0x00000001  // Zero memory on allocation
#define MEM_FLAG_CRITICAL       0x00000002  // Critical allocation (never fail)
#define MEM_FLAG_TEMPORARY      0x00000004  // Temporary allocation
#define MEM_FLAG_CACHEABLE      0x00000008  // Can be cached/swapped
#define MEM_FLAG_EXECUTABLE     0x00000010  // Memory will contain executable code
#define MEM_FLAG_READONLY       0x00000020  // Read-only memory
#define MEM_FLAG_SHARED         0x00000040  // Shared memory
#define MEM_FLAG_LOCKED         0x00000080  // Lock in physical memory

// Memory alignment constants
#define ALIGN_BYTE              1
#define ALIGN_WORD              2
#define ALIGN_DWORD             4
#define ALIGN_QWORD             8
#define ALIGN_PAGE              4096
#define ALIGN_DEFAULT           ALIGN_QWORD

// Memory zones for different device types
typedef enum {
    ZONE_KERNEL = 0,        // Kernel memory
    ZONE_STEPPPS = 1,       // STEPPPS framework memory
    ZONE_APPLICATION = 2,   // Application memory
    ZONE_BUFFER = 3,        // I/O buffer memory
    ZONE_CACHE = 4,         // Cache memory
    ZONE_HEAP = 5,          // General heap memory
    ZONE_STACK = 6,         // Stack memory
    ZONE_MAX = 7
} memory_zone_t;

// Memory block structure
typedef struct memory_block {
    void* address;              // Block start address
    size_t size;               // Block size in bytes
    uint32_t flags;            // Allocation flags
    memory_zone_t zone;        // Memory zone
    uint32_t alignment;        // Memory alignment
    bool is_free;              // Free/allocated status
    struct memory_block* next; // Next block in list
    struct memory_block* prev; // Previous block in list
    uint32_t magic;            // Magic number for validation
    uint32_t alloc_id;         // Allocation identifier
} memory_block_t;

// Memory zone configuration
typedef struct {
    uint32_t base_address;     // Zone base address
    size_t total_size;         // Total zone size
    size_t allocated_size;     // Currently allocated
    size_t largest_free_block; // Largest free block
    uint32_t block_count;      // Number of blocks
    uint32_t allocation_count; // Number of allocations
    memory_block_t* first_block; // First block in zone
    bool is_active;            // Zone is active
} memory_zone_config_t;

// Memory statistics
typedef struct {
    size_t total_memory;       // Total system memory
    size_t allocated_memory;   // Currently allocated
    size_t free_memory;        // Available memory
    size_t largest_free;       // Largest free block
    size_t fragmentation;      // Memory fragmentation
    uint32_t allocation_count; // Total allocations
    uint32_t deallocation_count; // Total deallocations
    uint32_t failed_allocations; // Failed allocation count
    uint32_t memory_leaks;     // Detected memory leaks
    uint32_t efficiency_rating; // Memory efficiency (0-100%)
} memory_stats_t;

// Magic numbers for validation
#define MEMORY_MANAGER_MAGIC    0xFEEDFACE

// Universal Memory Manager
typedef struct {
    // Validation
    uint32_t magic;             // Magic number for validation
    
    // Memory information
    size_t total_memory;        // Total available memory
    uint32_t kernel_base;       // Kernel base address
    
    // Hardware information
    const hardware_info_t* hardware;

    // Memory zones
    memory_zone_config_t zones[ZONE_MAX];

    // Global statistics
    memory_stats_t stats;

    // Manager configuration
    bool mmu_enabled;          // MMU support
    bool compression_enabled;  // Memory compression
    bool garbage_collection_enabled; // Automatic GC
    bool leak_detection_enabled; // Memory leak detection

    // Performance tracking
    uint32_t avg_alloc_time_us; // Average allocation time
    uint32_t avg_free_time_us;  // Average free time
    uint32_t gc_cycles;         // Garbage collection cycles
    uint32_t compaction_cycles; // Memory compaction cycles

    // Memory protection
    bool protection_enabled;    // Memory protection active
    uint32_t access_violations; // Access violation count

    // Allocation tracking
    uint32_t next_alloc_id;    // Next allocation ID
    memory_block_t* all_blocks; // All allocated blocks

} memory_manager_t;

// Memory allocation request
typedef struct {
    size_t size;               // Requested size
    uint32_t alignment;        // Required alignment
    uint32_t flags;            // Allocation flags
    memory_zone_t preferred_zone; // Preferred memory zone
    uint32_t timeout_ms;       // Allocation timeout
    const char* debug_name;    // Debug name for tracking
} memory_request_t;

// Memory allocation result
typedef struct {
    void* address;             // Allocated memory address
    size_t actual_size;        // Actual allocated size
    uint32_t alloc_id;         // Allocation identifier
    memory_zone_t zone;        // Allocated zone
    uint32_t alloc_time_us;    // Allocation time
    bool success;              // Allocation success
    const char* error_message; // Error message if failed
} memory_result_t;

// Function prototypes

/**
 * @brief Initialize Universal Memory Manager
 * @param hardware Hardware information
 * @param total_memory Total available memory
 * @param kernel_base Kernel memory base address
 * @return memory_manager_t* Initialized memory manager
 */
memory_manager_t* initialize_memory_manager(
    const hardware_info_t* hardware,
    size_t total_memory,
    uint32_t kernel_base
);

/**
 * @brief Configure memory zones based on hardware
 * @param manager Memory manager
 * @param boot_config Boot configuration
 * @return bool Success/failure
 */
bool configure_memory_zones(
    memory_manager_t* manager,
    const void* boot_config  // boot_config_t from universal_boot.h
);

/**
 * @brief Allocate memory with specified parameters
 * @param manager Memory manager
 * @param request Allocation request
 * @return memory_result_t Allocation result
 *
 * Implements PSC AllocateMemory() algorithm
 * Target: >95% efficiency (NFRD NFR-PFM-005)
 */
memory_result_t allocate_memory(
    memory_manager_t* manager,
    const memory_request_t* request
);

/**
 * @brief Free allocated memory
 * @param manager Memory manager
 * @param address Memory address to free
 * @return bool Success/failure
 */
bool free_memory(memory_manager_t* manager, void* address);

/**
 * @brief Free memory by allocation ID
 * @param manager Memory manager
 * @param alloc_id Allocation identifier
 * @return bool Success/failure
 */
bool free_memory_by_id(memory_manager_t* manager, uint32_t alloc_id);

/**
 * @brief Reallocate memory with new size
 * @param manager Memory manager
 * @param address Current memory address
 * @param new_size New size required
 * @return void* New memory address or NULL
 */
void* reallocate_memory(memory_manager_t* manager, void* address, size_t new_size);

/**
 * @brief Get memory statistics
 * @param manager Memory manager
 * @return memory_stats_t Current memory statistics
 */
memory_stats_t get_memory_statistics(const memory_manager_t* manager);

/**
 * @brief Perform garbage collection
 * @param manager Memory manager
 * @return uint32_t Amount of memory freed
 */
uint32_t garbage_collect(memory_manager_t* manager);

/**
 * @brief Compact memory to reduce fragmentation
 * @param manager Memory manager
 * @return bool Success/failure
 */
bool compact_memory(memory_manager_t* manager);

/**
 * @brief Detect memory leaks
 * @param manager Memory manager
 * @return uint32_t Number of leaks detected
 */
uint32_t detect_memory_leaks(memory_manager_t* manager);

/**
 * @brief Validate memory integrity
 * @param manager Memory manager
 * @return bool Memory integrity check result
 */
bool validate_memory_integrity(const memory_manager_t* manager);

/**
 * @brief Get memory block information
 * @param manager Memory manager
 * @param address Memory address
 * @return memory_block_t* Block information or NULL
 */
memory_block_t* get_memory_block_info(memory_manager_t* manager, void* address);

/**
 * @brief Set memory protection for block
 * @param manager Memory manager
 * @param address Memory address
 * @param flags Protection flags
 * @return bool Success/failure
 */
bool set_memory_protection(memory_manager_t* manager, void* address, uint32_t flags);

/**
 * @brief Print memory manager statistics
 * @param manager Memory manager
 */
void print_memory_statistics(const memory_manager_t* manager);

/**
 * @brief Shutdown memory manager
 * @param manager Memory manager
 */
void shutdown_memory_manager(memory_manager_t* manager);

// Convenience macros for common allocations
#define MALLOC(manager, alloc_size) \
    allocate_memory(manager, &(memory_request_t){ \
        .size = alloc_size, \
        .alignment = ALIGN_DEFAULT, \
        .flags = MEM_FLAG_ZERO, \
        .preferred_zone = ZONE_HEAP, \
        .timeout_ms = 1000, \
        .debug_name = __func__ \
    }).address

#define CALLOC(manager, count, elem_size) \
    allocate_memory(manager, &(memory_request_t){ \
        .size = (count) * (elem_size), \
        .alignment = ALIGN_DEFAULT, \
        .flags = MEM_FLAG_ZERO, \
        .preferred_zone = ZONE_HEAP, \
        .timeout_ms = 1000, \
        .debug_name = __func__ \
    }).address

#define FREE(manager, ptr) free_memory(manager, ptr)

// Global memory manager instance
extern memory_manager_t* g_memory_manager;

#endif // MEMORY_MANAGER_H