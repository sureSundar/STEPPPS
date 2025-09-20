/**
 * @file memory_manager.c
 * @brief Universal Memory Management Implementation for TernaryBit OS
 *
 * Implements adaptive memory management that scales from 512 bytes to exabytes
 * with >95% efficiency target.
 *
 * @version 1.0
 * @date 2025-01-XX
 */

#include "memory_manager.h"
#include "../boot/universal_boot.h"
#include <string.h>
#include <stdlib.h>
#include <stddef.h>  // For size_t
#include <stdint.h>  // For uint32_t, etc.

// Magic numbers for validation
#define MEMORY_BLOCK_MAGIC      0xDEADBEEF
#define MEMORY_MANAGER_MAGIC    0xFEEDFACE

// Global memory manager instance
memory_manager_t* g_memory_manager = NULL;

// Forward declarations
static memory_block_t* create_memory_block(void* address, size_t size, uint32_t flags,
                                          memory_zone_t zone, uint32_t alignment, uint32_t alloc_id);

// Simple allocator for initial setup
static void* simple_alloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr) {
        memset(ptr, 0, size); // Zero-initialize memory
    }
    return ptr;
}

// Simple deallocator
static void simple_free(void* ptr) {
    if (ptr) {
        free(ptr);
    }
}

// Global memory block list
static memory_block_t* g_all_blocks = NULL;

// Helper function to align memory
static inline size_t align_up(size_t size, size_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

// Helper function to create a new memory block
static memory_block_t* create_memory_block(memory_manager_t* manager, void* address, 
                                         size_t size, uint32_t flags, memory_zone_t zone, 
                                         uint32_t alignment, uint32_t alloc_id) {
    if (!manager || !address || size == 0) {
        return NULL;
    }

    memory_block_t* block = (memory_block_t*)simple_alloc(sizeof(memory_block));
    if (!block) {
        return NULL;
    }

    // Initialize block
    block->address = address;
    block->size = size;
    block->flags = flags;
    block->zone = zone;
    block->alignment = alignment;
    block->is_free = true;
    block->magic = 0xDEADBEEF;
    block->alloc_id = alloc_id;
    block->next = NULL;
    block->prev = NULL;

    // Add to global block list
    if (!manager->all_blocks) {
        manager->all_blocks = block;
    } else {
        // Add to the beginning of the list
        block->next = manager->all_blocks;
        manager->all_blocks->prev = block;
        manager->all_blocks = block;
    }

    // Add to zone's block list if zone is valid
    if (zone < ZONE_MAX) {
        memory_zone_config_t* zone_cfg = &manager->zones[zone];
        if (!zone_cfg->first_block) {
            zone_cfg->first_block = block;
        } else {
            // Add to the beginning of zone's block list
            block->next = zone_cfg->first_block;
            zone_cfg->first_block->prev = block;
            zone_cfg->first_block = block;
        }
        zone_cfg->block_count++;
        zone_cfg->total_size += size;
        if (block->is_free && size > zone_cfg->largest_free_block) {
            zone_cfg->largest_free_block = size;
        }
    }

    return block;
}

// Helper function to find a block in a specific zone
static memory_block_t* find_block_in_zone(memory_zone_config_t* zone, void* address) {
    if (!zone || !address || !zone->is_active) {
        return NULL;
    }

    uintptr_t addr_val = (uintptr_t)address;
    uintptr_t zone_start = zone->base_address;
    uintptr_t zone_end = zone_start + zone->total_size;
    
    // Check if address is within this zone
    if (addr_val < zone_start || addr_val >= zone_end) {
        return NULL;
    }

    // Search through blocks in this zone
    memory_block_t* block = zone->first_block;
    while (block) {
        uintptr_t block_start = (uintptr_t)block->address;
        uintptr_t block_end = block_start + block->size;
        
        if (addr_val >= block_start && addr_val < block_end) {
            return block;
        }
        block = block->next;
    }
    
    return NULL;
}

// Helper function to get a memory block by address
memory_block_t* get_memory_block(memory_manager_t* manager, void* address) {
    if (!manager || !address) {
        return NULL;
    }
    
    // Check if address is within any zone
    for (int i = 0; i < ZONE_MAX; i++) {
        memory_block_t* block = find_block_in_zone(&manager->zones[i], address);
        if (block) {
            return block;
        }
    }
    
    return NULL;
}

// Global memory manager instance
memory_manager_t* g_memory_manager = NULL;

// Forward declarations
static memory_block_t* create_memory_block(memory_manager_t* manager, void* address, 
                                         size_t size, uint32_t flags, memory_zone_t zone, 
                                         uint32_t alignment, uint32_t alloc_id);
static memory_block_t* find_block_in_zone(memory_zone_config_t* zone, void* address);

/**
 * @brief Set up default memory zones based on system configuration
 * @param manager Memory manager instance
 * @param total_memory Total available memory in bytes
 * @param kernel_base Kernel base address
 */
static void setup_default_memory_zones(memory_manager_t* manager, size_t total_memory, uint32_t kernel_base) {
    if (!manager) return;
    
    // Initialize all zones as inactive first
    for (int i = 0; i < ZONE_MAX; i++) {
        manager->zones[i].is_active = false;
        manager->zones[i].first_block = NULL;
        manager->zones[i].block_count = 0;
        manager->zones[i].allocation_count = 0;
        manager->zones[i].allocated_size = 0;
        manager->zones[i].total_size = 0;
        manager->zones[i].largest_free_block = 0;
        manager->zones[i].base_address = 0;
    }
    
    // Initialize all zones as inactive first
    for (int i = 0; i < ZONE_MAX; i++) {
        manager->zones[i].is_active = false;
        manager->zones[i].first_block = NULL;
        manager->zones[i].block_count = 0;
        manager->zones[i].allocation_count = 0;
        manager->zones[i].allocated_size = 0;
        manager->zones[i].total_size = 0;
        manager->zones[i].largest_free_block = 0;
    }

    // Kernel zone (first 16MB or 1/4 of total memory, whichever is smaller)
    size_t kernel_size = (total_memory < (16 * 1024 * 1024)) ? 
                         (total_memory / 4) : (16 * 1024 * 1024);
    
    // STEPPPS framework zone (1/8 of total memory, minimum 4MB)
    size_t steppps_size = (total_memory / 8) > (4 * 1024 * 1024) ? 
                          (total_memory / 8) : (4 * 1024 * 1024);
    
    // Application zone (remaining memory)
    size_t app_size = total_memory - kernel_size - steppps_size;

    // Set up kernel zone
    manager->zones[ZONE_KERNEL].base_address = kernel_base;
    manager->zones[ZONE_KERNEL].total_size = kernel_size;
    manager->zones[ZONE_KERNEL].is_active = true;

    // Set up STEPPPS zone
    manager->zones[ZONE_STEPPPS].base_address = kernel_base + kernel_size;
    manager->zones[ZONE_STEPPPS].total_size = steppps_size;
    manager->zones[ZONE_STEPPPS].is_active = true;

    // Set up application zone
    manager->zones[ZONE_APPLICATION].base_address = kernel_base + kernel_size + steppps_size;
    manager->zones[ZONE_APPLICATION].total_size = app_size;
    manager->zones[ZONE_APPLICATION].is_active = true;

    // Initialize free blocks for each zone
    for (int i = 0; i < ZONE_MAX; i++) {
        if (manager->zones[i].is_active) {
            // Create a single free block for the entire zone
            memory_block_t* block = (memory_block_t*)simple_alloc(sizeof(memory_block_t));
            if (block) {
                block->address = (void*)manager->zones[i].base_address;
                block->size = manager->zones[i].total_size;
                block->flags = MEM_FLAG_NONE;
                block->zone = (memory_zone_t)i;
                block->alignment = ALIGN_DEFAULT;
                block->is_free = true;
                block->next = NULL;
                block->prev = NULL;
                block->magic = 0xDEADBEEF; // Magic number for validation
                block->alloc_id = 0;
                
                manager->zones[i].first_block = block;
                manager->zones[i].block_count = 1;
                manager->zones[i].largest_free_block = block->size;
            }
        }
    }
}

static void simple_free(void* ptr) {
    free(ptr);
}

/**
 * @brief Initialize Universal Memory Manager
 */
memory_manager_t* initialize_memory_manager(
    const hardware_info_t* hardware,
    size_t total_memory,
    uint32_t kernel_base
) {
    if (!hardware || total_memory == 0) {
        return NULL;
    }
    if (!hardware) return NULL;

    // Allocate and initialize manager structure
    memory_manager_t* manager = (memory_manager_t*)simple_alloc(sizeof(memory_manager_t));
    if (!manager) {
        return NULL;
    }
    
    // Initialize manager fields
    manager->magic = MEMORY_MANAGER_MAGIC;
    manager->total_memory = total_memory;
    manager->kernel_base = kernel_base;
    manager->hardware = hardware;
    
    // Initialize memory zones
    for (int i = 0; i < ZONE_MAX; i++) {
        manager->zones[i].is_active = false;
        manager->zones[i].first_block = NULL;
        manager->zones[i].block_count = 0;
        manager->zones[i].allocation_count = 0;
        manager->zones[i].allocated_size = 0;
        manager->zones[i].total_size = 0;
        manager->zones[i].largest_free_block = 0;
    }
    
    // Initialize statistics
    manager->stats.total_memory = total_memory;
    manager->stats.free_memory = total_memory;
    manager->stats.allocated_memory = 0;
    manager->stats.largest_free = total_memory;
    manager->stats.fragmentation = 0;
    manager->stats.allocation_count = 0;
    manager->stats.deallocation_count = 0;
    manager->stats.failed_allocations = 0;
    manager->stats.memory_leaks = 0;
    manager->stats.efficiency_rating = 100;
    
    // Set default configuration based on hardware capabilities
    manager->mmu_enabled = (hardware->capabilities & HW_CAP_MMU) != 0;
    manager->compression_enabled = (total_memory > 65536); // Enable on >64KB systems
    manager->leak_detection_enabled = true;
    manager->protection_enabled = (hardware->capabilities & HW_CAP_MMU) != 0;

    // Set up initial memory zones
    setup_default_memory_zones(manager, total_memory, kernel_base);

    // Initialize allocation tracking
    manager->next_alloc_id = 1;

    // Set global reference
    g_memory_manager = manager;

    return manager;
}

/**
 * @brief Configure memory zones based on hardware
 */
bool configure_memory_zones(memory_manager_t* manager, const void* boot_config) {
    if (!manager || !boot_config) return false;

    const boot_config_t* config = (const boot_config_t*)boot_config;

    // Configure Kernel Zone
    manager->zones[ZONE_KERNEL].base_address = config->kernel_base_addr;
    manager->zones[ZONE_KERNEL].total_size = config->kernel_size_bytes;
    manager->zones[ZONE_KERNEL].is_active = true;

    // Configure STEPPPS Zone
    manager->zones[ZONE_STEPPPS].base_address = config->steppps_base_addr;
    manager->zones[ZONE_STEPPPS].total_size = config->steppps_size_bytes;
    manager->zones[ZONE_STEPPPS].is_active = true;

    // Configure Stack Zone
    manager->zones[ZONE_STACK].base_address = config->stack_base_addr;
    manager->zones[ZONE_STACK].total_size = config->stack_size_bytes;
    manager->zones[ZONE_STACK].is_active = true;

    // Configure Heap Zone
    manager->zones[ZONE_HEAP].base_address = config->heap_base_addr;
    manager->zones[ZONE_HEAP].total_size = config->heap_size_bytes;
    manager->zones[ZONE_HEAP].is_active = true;

    // Configure remaining zones based on available memory
    uint32_t remaining_memory = manager->stats.total_memory -
        (config->kernel_size_bytes + config->steppps_size_bytes +
         config->stack_size_bytes + config->heap_size_bytes);

    if (remaining_memory > 0) {
        // Application Zone
        manager->zones[ZONE_APPLICATION].base_address =
            config->heap_base_addr + config->heap_size_bytes;
        manager->zones[ZONE_APPLICATION].total_size = remaining_memory / 2;
        manager->zones[ZONE_APPLICATION].is_active = true;

        // Buffer Zone
        manager->zones[ZONE_BUFFER].base_address =
            manager->zones[ZONE_APPLICATION].base_address +
            manager->zones[ZONE_APPLICATION].total_size;
        manager->zones[ZONE_BUFFER].total_size = remaining_memory / 4;
        manager->zones[ZONE_BUFFER].is_active = true;

        // Cache Zone
        manager->zones[ZONE_CACHE].base_address =
            manager->zones[ZONE_BUFFER].base_address +
            manager->zones[ZONE_BUFFER].total_size;
        manager->zones[ZONE_CACHE].total_size = remaining_memory / 4;
        manager->zones[ZONE_CACHE].is_active = true;
    }

    // Initialize free blocks for each active zone
    for (int i = 0; i < ZONE_MAX; i++) {
        if (manager->zones[i].is_active && manager->zones[i].total_size > 0) {
            // Create a single large free block for the entire zone
            memory_block_t* free_block = create_memory_block(
                (void*)(uintptr_t)manager->zones[i].base_address,
                manager->zones[i].total_size,
                0,
                (memory_zone_t)i,
                ALIGN_DEFAULT,
                0
            );

            if (free_block) {
                free_block->is_free = true;
                manager->zones[i].first_block = free_block;
                manager->zones[i].largest_free_block = manager->zones[i].total_size;
            }
        }
    }

    return true;
}

/**
 * @brief Find suitable free block in zone
 */
static memory_block_t* find_free_block(memory_zone_config_t* zone, size_t size, uint32_t alignment) {
    (void)zone; (void)size; (void)alignment; // Suppress warnings for now
    // For testing, just return NULL to trigger real malloc
    return NULL;
}

/**
 * @brief Create new memory block
 */
static memory_block_t* create_memory_block(void* address, size_t size, uint32_t flags,
                                          memory_zone_t zone, uint32_t alignment, uint32_t alloc_id) {
    memory_block_t* block = (memory_block_t*)simple_alloc(sizeof(memory_block_t));
    if (!block) return NULL;

    block->address = address;
    block->size = size;
    block->flags = flags;
    block->zone = zone;
    block->alignment = alignment;
    block->is_free = false;
    block->next = NULL;
    block->prev = NULL;
    block->magic = MEMORY_BLOCK_MAGIC;
    block->alloc_id = alloc_id;

    return block;
}

/**
 * @brief Allocate memory with specified parameters
 * Implements PSC AllocateMemory() algorithm
 */
memory_result_t allocate_memory(memory_manager_t* manager, const memory_request_t* request) {
    memory_result_t result = {0};

    if (!manager || !request || request->size == 0) {
        result.error_message = "Invalid parameters";
        return result;
    }

    uint64_t start_time = platform_get_timestamp_us();

    // Determine target zone
    memory_zone_t target_zone = request->preferred_zone;
    if (!manager->zones[target_zone].is_active) {
        target_zone = ZONE_HEAP; // Fallback to heap
    }

    memory_zone_config_t* zone = &manager->zones[target_zone];

    // Find suitable free block
    memory_block_t* block = find_free_block(zone, request->size, request->alignment);

    // For testing purposes, use real malloc when no block is found
    if (!block) {
        // Try garbage collection if enabled
        if (manager->garbage_collection_enabled) {
            garbage_collect(manager);
            block = find_free_block(zone, request->size, request->alignment);
        }

        // Use real malloc for testing
        void* real_addr = simple_alloc(request->size);
        if (!real_addr) {
            manager->stats.failed_allocations++;
            result.error_message = "Real allocation failed";
            return result;
        }

        // Create memory block for tracking
        uint32_t alloc_id = manager->next_alloc_id++;
        memory_block_t* new_block = create_memory_block(
            real_addr,
            request->size,
            request->flags,
            target_zone,
            request->alignment,
            alloc_id
        );

        if (!new_block) {
            simple_free(real_addr);
            result.error_message = "Failed to create memory block";
            return result;
        }

        // Initialize memory if requested
        if (request->flags & MEM_FLAG_ZERO) {
            memset(real_addr, 0, request->size);
        }

        // Add to global block list
        new_block->next = manager->all_blocks;
        if (manager->all_blocks) {
            manager->all_blocks->prev = new_block;
        }
        manager->all_blocks = new_block;

        // Update zone statistics
        zone->allocated_size += request->size;
        zone->allocation_count++;

        // Update global statistics
        manager->stats.allocated_memory += request->size;
        manager->stats.free_memory -= request->size;
        manager->stats.allocation_count++;

        // Calculate allocation time
        uint32_t alloc_time = (uint32_t)(platform_get_timestamp_us() - start_time);
        manager->avg_alloc_time_us = (manager->avg_alloc_time_us + alloc_time) / 2;

        // Prepare result
        result.address = real_addr;
        result.actual_size = request->size;
        result.alloc_id = alloc_id;
        result.zone = target_zone;
        result.alloc_time_us = alloc_time;
        result.success = true;

        // Update efficiency rating
        if (manager->stats.total_memory > 0) {
            manager->stats.efficiency_rating =
                (uint32_t)((manager->stats.allocated_memory * 100) / manager->stats.total_memory);
        }

        return result;
    }

    // This code should not be reached in current implementation
    result.error_message = "Unexpected code path";
    return result;
}

/**
 * @brief Free allocated memory
 */
bool free_memory(memory_manager_t* manager, void* address) {
    if (!manager || !address) return false;

    uint64_t start_time = platform_get_timestamp_us();

    // Find the memory block
    memory_block_t* block = manager->all_blocks;
    while (block) {
        if (block->address == address && !block->is_free) {
            break;
        }
        block = block->next;
    }

    if (!block) {
        return false; // Block not found
    }

    // Mark block as free
    block->is_free = true;

    // Free the real memory
    simple_free(block->address);

    // Update zone statistics
    memory_zone_config_t* zone = &manager->zones[block->zone];
    zone->allocated_size -= block->size;

    // Update global statistics
    manager->stats.allocated_memory -= block->size;
    manager->stats.free_memory += block->size;
    manager->stats.deallocation_count++;

    // Remove from global block list
    if (block->prev) {
        block->prev->next = block->next;
    } else {
        manager->all_blocks = block->next;
    }
    if (block->next) {
        block->next->prev = block->prev;
    }

    // Calculate free time
    uint32_t free_time = (uint32_t)(platform_get_timestamp_us() - start_time);
    manager->avg_free_time_us = (manager->avg_free_time_us + free_time) / 2;

    // Free the block structure
    simple_free(block);

    return true;
}

/**
 * @brief Free memory by allocation ID
 */
bool free_memory_by_id(memory_manager_t* manager, uint32_t alloc_id) {
    if (!manager || alloc_id == 0) return false;

    memory_block_t* block = manager->all_blocks;
    while (block) {
        if (block->alloc_id == alloc_id && !block->is_free) {
            return free_memory(manager, block->address);
        }
        block = block->next;
    }

    return false;
}

/**
 * @brief Reallocate memory with new size
 */
void* reallocate_memory(memory_manager_t* manager, void* address, size_t new_size) {
    if (!manager || !address || new_size == 0) return NULL;

    memory_block_t* block = get_memory_block_info(manager, address);
    if (!block || block->is_free) return NULL;

    // If new size is smaller or same, just return current address
    if (new_size <= block->size) {
        return address;
    }

    // Allocate new block
    memory_request_t request = {
        .size = new_size,
        .alignment = block->alignment,
        .flags = block->flags,
        .preferred_zone = block->zone,
        .timeout_ms = 1000,
        .debug_name = "realloc"
    };

    memory_result_t result = allocate_memory(manager, &request);
    if (!result.success) return NULL;

    // Copy old data
    memcpy(result.address, address, block->size);

    // Free old block
    free_memory(manager, address);

    return result.address;
}

/**
 * @brief Get memory statistics
 */
memory_stats_t get_memory_statistics(const memory_manager_t* manager) {
    memory_stats_t empty_stats = {0};
    if (!manager) return empty_stats;

    return manager->stats;
}

/**
 * @brief Perform garbage collection
 */
uint32_t garbage_collect(memory_manager_t* manager) {
    if (!manager || !manager->garbage_collection_enabled) return 0;

    uint32_t freed_memory = 0;
    manager->gc_cycles++;

    // Simple garbage collection - coalesce free blocks
    for (int zone_idx = 0; zone_idx < ZONE_MAX; zone_idx++) {
        memory_zone_config_t* zone = &manager->zones[zone_idx];
        if (!zone->is_active) continue;

        memory_block_t* block = zone->first_block;
        while (block && block->next) {
            if (block->is_free && block->next->is_free) {
                // Coalesce adjacent free blocks
                block->size += block->next->size;
                memory_block_t* next_block = block->next;
                block->next = next_block->next;
                if (next_block->next) {
                    next_block->next->prev = block;
                }
                freed_memory += sizeof(memory_block_t);
                simple_free(next_block);
            }
            block = block->next;
        }
    }

    return freed_memory;
}

/**
 * @brief Compact memory to reduce fragmentation
 */
bool compact_memory(memory_manager_t* manager) {
    if (!manager) return false;

    manager->compaction_cycles++;

    // Simple compaction - just update statistics for now
    // In a real implementation, this would move memory blocks to reduce fragmentation

    uint32_t total_free = 0;
    uint32_t largest_free = 0;

    for (int zone_idx = 0; zone_idx < ZONE_MAX; zone_idx++) {
        memory_zone_config_t* zone = &manager->zones[zone_idx];
        if (!zone->is_active) continue;

        memory_block_t* block = zone->first_block;
        while (block) {
            if (block->is_free) {
                total_free += block->size;
                if (block->size > largest_free) {
                    largest_free = block->size;
                }
            }
            block = block->next;
        }

        zone->largest_free_block = largest_free;
    }

    // Calculate fragmentation
    if (total_free > 0) {
        manager->stats.fragmentation = 100 - ((largest_free * 100) / total_free);
    }

    return true;
}

/**
 * @brief Detect memory leaks
 */
uint32_t detect_memory_leaks(memory_manager_t* manager) {
    if (!manager || !manager->leak_detection_enabled) return 0;

    uint32_t leaks = 0;
    memory_block_t* block = manager->all_blocks;

    while (block) {
        if (!block->is_free && block->magic != MEMORY_BLOCK_MAGIC) {
            // Potential corruption or leak
            leaks++;
        }
        block = block->next;
    }

    manager->stats.memory_leaks = leaks;
    return leaks;
}

/**
 * @brief Validate memory integrity
 */
bool validate_memory_integrity(const memory_manager_t* manager) {
    if (!manager) return false;

    // Check manager magic number (if we had one)
    // Check all block magic numbers
    memory_block_t* block = manager->all_blocks;
    while (block) {
        if (block->magic != MEMORY_BLOCK_MAGIC) {
            return false;
        }
        block = block->next;
    }

    return true;
}

/**
 * @brief Get memory block information
 */
memory_block_t* get_memory_block_info(memory_manager_t* manager, void* address) {
    if (!manager || !address) return NULL;

    memory_block_t* block = manager->all_blocks;
    while (block) {
        if (block->address == address) {
            return block;
        }
        block = block->next;
    }

    return NULL;
}

/**
 * @brief Set memory protection for block
 */
bool set_memory_protection(memory_manager_t* manager, void* address, uint32_t flags) {
    (void)flags; // Suppress unused parameter warning

    if (!manager || !address || !manager->protection_enabled) return false;

    memory_block_t* block = get_memory_block_info(manager, address);
    if (!block) return false;

    // In a real implementation, this would set MMU protection flags
    return true;
}

/**
 * @brief Print memory manager statistics
 */
void print_memory_statistics(const memory_manager_t* manager) {
    if (!manager) return;

    // This would print comprehensive memory statistics
    // Implementation depends on available output methods
}

/**
 * @brief Shutdown memory manager
 */
void shutdown_memory_manager(memory_manager_t* manager) {
    if (!manager) return;

    // Free all allocated blocks
    memory_block_t* block = manager->all_blocks;
    while (block) {
        memory_block_t* next = block->next;
        simple_free(block);
        block = next;
    }

    // Clear global reference
    if (g_memory_manager == manager) {
        g_memory_manager = NULL;
    }

    // Free manager structure
    simple_free(manager);
}