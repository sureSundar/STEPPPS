/**
 * @file tbos_memory.h
 * @brief TernaryBit OS Simple Memory API
 *
 * Provides simple malloc/free style API wrapping the complex memory manager.
 * For kernel subsystems that just need basic allocation.
 *
 * @version 1.0
 * @date 2025-11-03
 */

#ifndef TBOS_MEMORY_H
#define TBOS_MEMORY_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize TBOS memory subsystem
 * Sets up simple heap for kernel use
 *
 * @param heap_size Size of heap to create (default: 10MB)
 * @return 0 on success, -1 on failure
 */
int tbos_memory_init(size_t heap_size);

/**
 * @brief Allocate memory
 *
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
void* tbos_malloc(size_t size);

/**
 * @brief Allocate zeroed memory
 *
 * @param nmemb Number of elements
 * @param size Size of each element
 * @return Pointer to allocated memory, or NULL on failure
 */
void* tbos_calloc(size_t nmemb, size_t size);

/**
 * @brief Reallocate memory
 *
 * @param ptr Existing pointer
 * @param size New size
 * @return Pointer to reallocated memory, or NULL on failure
 */
void* tbos_realloc(void* ptr, size_t size);

/**
 * @brief Free allocated memory
 *
 * @param ptr Pointer to free
 */
void tbos_free(void* ptr);

/**
 * @brief Get memory statistics
 *
 * @param total_allocated Output: total bytes allocated
 * @param total_freed Output: total bytes freed
 * @param current_used Output: currently in use
 * @param allocation_count Output: number of allocations
 */
void tbos_memory_stats(size_t* total_allocated, size_t* total_freed,
                       size_t* current_used, uint32_t* allocation_count);

/**
 * @brief Print memory statistics
 */
void tbos_memory_print_stats(void);

/**
 * @brief Check if memory subsystem is initialized
 *
 * @return true if initialized
 */
bool tbos_memory_is_initialized(void);

#endif /* TBOS_MEMORY_H */
