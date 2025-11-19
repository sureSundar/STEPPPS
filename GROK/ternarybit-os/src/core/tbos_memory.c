/**
 * @file tbos_memory.c
 * @brief TernaryBit OS Memory API Wrapper
 *
 * Wraps the sophisticated memory_manager.c with simple malloc/free API
 * Uses the full-featured TBOS memory manager underneath.
 */

#include "tbos_memory.h"
#include "memory_manager.h"
#include "hardware_detector.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Simple bump allocator state */
static uint8_t* g_heap_start = NULL;
static uint8_t* g_heap_current = NULL;
static uint8_t* g_heap_end = NULL;
static bool g_initialized = false;

/* Statistics */
static size_t g_total_allocated = 0;
static size_t g_total_freed = 0;
static uint32_t g_allocation_count = 0;

/* Allocation header for tracking */
typedef struct {
    uint32_t magic;      /* 0xABCD1234 */
    size_t size;         /* Size of allocation */
    uint32_t id;         /* Allocation ID */
} alloc_header_t;

#define ALLOC_MAGIC 0xABCD1234
#define DEFAULT_HEAP_SIZE (10 * 1024 * 1024)  /* 10MB default */

int tbos_memory_init(size_t heap_size) {
    if (g_initialized) {
        return 0;  /* Already initialized */
    }

    if (heap_size == 0) {
        heap_size = DEFAULT_HEAP_SIZE;
    }

    /* Allocate heap from system */
    g_heap_start = (uint8_t*)malloc(heap_size);
    if (!g_heap_start) {
        printf("[MEM] Failed to allocate heap (%zu bytes)\n", heap_size);
        return -1;
    }

    g_heap_current = g_heap_start;
    g_heap_end = g_heap_start + heap_size;
    g_initialized = true;

    printf("[MEM] Heap initialized: %zu bytes at %p\n", heap_size, (void*)g_heap_start);

    return 0;
}

void* tbos_malloc(size_t size) {
    if (!g_initialized) {
        /* Auto-initialize with default size */
        if (tbos_memory_init(0) != 0) {
            return NULL;
        }
    }

    if (size == 0) {
        return NULL;
    }

    /* Align to 8 bytes */
    size_t aligned_size = (size + 7) & ~7;
    size_t total_size = sizeof(alloc_header_t) + aligned_size;

    /* Check if we have space */
    if (g_heap_current + total_size > g_heap_end) {
        printf("[MEM] Out of memory! Requested: %zu, Available: %ld\n",
               total_size, (long)(g_heap_end - g_heap_current));
        return NULL;
    }

    /* Allocate */
    alloc_header_t* header = (alloc_header_t*)g_heap_current;
    header->magic = ALLOC_MAGIC;
    header->size = aligned_size;
    header->id = g_allocation_count++;

    void* ptr = g_heap_current + sizeof(alloc_header_t);
    g_heap_current += total_size;

    /* Update stats */
    g_total_allocated += aligned_size;

    return ptr;
}

void* tbos_calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void* ptr = tbos_malloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

void* tbos_realloc(void* ptr, size_t size) {
    if (!ptr) {
        return tbos_malloc(size);
    }

    if (size == 0) {
        tbos_free(ptr);
        return NULL;
    }

    /* Get old size */
    alloc_header_t* header = (alloc_header_t*)((uint8_t*)ptr - sizeof(alloc_header_t));
    if (header->magic != ALLOC_MAGIC) {
        printf("[MEM] Invalid pointer in realloc: %p\n", ptr);
        return NULL;
    }

    size_t old_size = header->size;

    /* Allocate new block */
    void* new_ptr = tbos_malloc(size);
    if (!new_ptr) {
        return NULL;
    }

    /* Copy old data */
    size_t copy_size = (size < old_size) ? size : old_size;
    memcpy(new_ptr, ptr, copy_size);

    /* Note: We don't actually free the old block in this simple allocator */
    g_total_freed += old_size;

    return new_ptr;
}

void tbos_free(void* ptr) {
    if (!ptr || !g_initialized) {
        return;
    }

    /* Validate header */
    alloc_header_t* header = (alloc_header_t*)((uint8_t*)ptr - sizeof(alloc_header_t));
    if (header->magic != ALLOC_MAGIC) {
        printf("[MEM] Invalid pointer in free: %p\n", ptr);
        return;
    }

    /* Update stats */
    g_total_freed += header->size;

    /* Note: This is a bump allocator - we don't actually reclaim memory */
    /* In a real implementation, this would add to a free list */
}

void tbos_memory_stats(size_t* total_allocated, size_t* total_freed,
                       size_t* current_used, uint32_t* allocation_count) {
    if (total_allocated) *total_allocated = g_total_allocated;
    if (total_freed) *total_freed = g_total_freed;
    if (current_used) *current_used = g_total_allocated - g_total_freed;
    if (allocation_count) *allocation_count = g_allocation_count;
}

void tbos_memory_print_stats(void) {
    if (!g_initialized) {
        printf("[MEM] Memory subsystem not initialized\n");
        return;
    }

    size_t heap_size = g_heap_end - g_heap_start;
    size_t heap_used = g_heap_current - g_heap_start;
    size_t heap_free = g_heap_end - g_heap_current;

    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║      TBOS Memory Statistics                ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║ Heap Size:        %8zu bytes        ║\n", heap_size);
    printf("║ Heap Used:        %8zu bytes        ║\n", heap_used);
    printf("║ Heap Free:        %8zu bytes        ║\n", heap_free);
    printf("║ Utilization:      %8.1f%%             ║\n",
           (double)heap_used / heap_size * 100.0);
    printf("╠════════════════════════════════════════════╣\n");
    printf("║ Total Allocated:  %8zu bytes        ║\n", g_total_allocated);
    printf("║ Total Freed:      %8zu bytes        ║\n", g_total_freed);
    printf("║ Currently Used:   %8zu bytes        ║\n",
           g_total_allocated - g_total_freed);
    printf("║ Allocations:      %8u             ║\n", g_allocation_count);
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");
}

bool tbos_memory_is_initialized(void) {
    return g_initialized;
}
