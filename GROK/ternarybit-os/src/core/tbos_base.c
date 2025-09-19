/**
 * @file tbos_base.c
 * @brief TernaryBit OS Base Implementation
 */

#include "tbos_base.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Global logger instance
tbos_logger_t* g_tbos_logger = NULL;

// Result creation utilities
tbos_result_t tbos_create_success_result(void* data, size_t data_size) {
    tbos_result_t result = {
        .success = true,
        .error_code = 0,
        .error_message = NULL,
        .data = data,
        .data_size = data_size
    };
    return result;
}

tbos_result_t tbos_create_error_result(int error_code, const char* error_message) {
    tbos_result_t result = {
        .success = false,
        .error_code = error_code,
        .error_message = error_message,
        .data = NULL,
        .data_size = 0
    };
    return result;
}

// Memory pool implementation
tbos_memory_pool_t* tbos_memory_pool_create(size_t block_size, size_t num_blocks) {
    tbos_memory_pool_t* pool = malloc(sizeof(tbos_memory_pool_t));
    if (!pool) return NULL;
    
    pool->block_size = block_size;
    pool->total_blocks = num_blocks;
    pool->free_blocks = num_blocks;
    
    // Allocate pool memory
    pool->pool_memory = malloc(block_size * num_blocks);
    if (!pool->pool_memory) {
        free(pool);
        return NULL;
    }
    
    // Allocate bitmap
    size_t bitmap_size = (num_blocks + 7) / 8;
    pool->free_bitmap = malloc(bitmap_size);
    if (!pool->free_bitmap) {
        free(pool->pool_memory);
        free(pool);
        return NULL;
    }
    
    // Initialize bitmap (all blocks free)
    memset(pool->free_bitmap, 0xFF, bitmap_size);
    
    return pool;
}

void tbos_memory_pool_destroy(tbos_memory_pool_t* pool) {
    if (pool) {
        free(pool->free_bitmap);
        free(pool->pool_memory);
        free(pool);
    }
}
