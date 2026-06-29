/**
 * @file tbos_tier_memory.c
 * @brief TernaryBit OS - Tier-Adaptive Memory Implementation
 *
 * Implements memory management that adapts to processor tier.
 *
 * @version 1.0 (Production)
 * @date 2025-11-03
 */

#include "tbos_tier_memory.h"
#include <stdio.h>
#include <string.h>

#ifndef TBOS_MEM_STATIC_ONLY
#include <stdlib.h>
#endif

/* ========================================================================= */
/* STATIC POOL FOR TIER 0/1 (NO HEAP)                                        */
/* ========================================================================= */

#ifdef TBOS_MEM_STATIC_ONLY
/* Static memory pool for 8-bit systems */
static uint8_t g_static_pool[TBOS_MEM_POOL_SIZE * TBOS_MEM_POOL_COUNT];
static uint8_t g_pool_bitmap[(TBOS_MEM_POOL_SIZE * TBOS_MEM_POOL_COUNT) / 8 + 1];
#endif

/* ========================================================================= */
/* GLOBAL STATE                                                              */
/* ========================================================================= */

static bool g_mem_initialized = false;
static tbos_mem_stats_t g_stats;
static tbos_mem_pool_t g_pools[TBOS_MEM_POOL_COUNT];
static uint32_t g_pool_count = 0;

/* Lock for thread safety (only on 32-bit+) */
#if TBOS_ARCH_BITS >= 32 && !defined(TBOS_MEM_STATIC_ONLY)
#include <pthread.h>
static pthread_mutex_t g_mem_mutex = PTHREAD_MUTEX_INITIALIZER;
#define MEM_LOCK()   pthread_mutex_lock(&g_mem_mutex)
#define MEM_UNLOCK() pthread_mutex_unlock(&g_mem_mutex)
#else
#define MEM_LOCK()   ((void)0)
#define MEM_UNLOCK() ((void)0)
#endif

/* ========================================================================= */
/* HELPER FUNCTIONS                                                          */
/* ========================================================================= */

/* Align size up to alignment boundary */
static inline tbos_mem_size_t align_up(tbos_mem_size_t size, uint32_t align) {
    return (size + align - 1) & ~(align - 1);
}

/* Check if power of 2 */
static inline bool is_power_of_2(uint32_t x) {
    return x && !(x & (x - 1));
}

/* ========================================================================= */
/* STATIC POOL ALLOCATOR (Tier 0-1)                                          */
/* ========================================================================= */

#ifdef TBOS_MEM_STATIC_ONLY

static void* static_alloc(tbos_mem_size_t size) {
    if (size > TBOS_MEM_MAX_ALLOC) {
        return NULL;
    }

    /* Round up to block size */
    tbos_mem_size_t aligned_size = align_up(size + sizeof(tbos_mem_block_t),
                                             TBOS_MEM_ALIGNMENT);

    /* Search bitmap for free blocks */
    uint32_t blocks_needed = (aligned_size + TBOS_MEM_ALIGNMENT - 1) /
                             TBOS_MEM_ALIGNMENT;
    uint32_t total_blocks = sizeof(g_static_pool) / TBOS_MEM_ALIGNMENT;

    uint32_t consecutive = 0;
    uint32_t start_block = 0;

    for (uint32_t i = 0; i < total_blocks; i++) {
        uint32_t byte_idx = i / 8;
        uint32_t bit_idx = i % 8;

        if (!(g_pool_bitmap[byte_idx] & (1 << bit_idx))) {
            /* Free block */
            if (consecutive == 0) {
                start_block = i;
            }
            consecutive++;

            if (consecutive >= blocks_needed) {
                /* Found enough consecutive blocks */
                for (uint32_t j = start_block; j < start_block + blocks_needed; j++) {
                    uint32_t b = j / 8;
                    uint32_t bit = j % 8;
                    g_pool_bitmap[b] |= (1 << bit);
                }

                uint8_t *ptr = g_static_pool + (start_block * TBOS_MEM_ALIGNMENT);
                tbos_mem_block_t *block = (tbos_mem_block_t*)ptr;
                block->size = size;
                block->flags = TBOS_MEM_FLAG_USED | TBOS_MEM_FLAG_POOLED;
                block->pool_id = 0;

                g_stats.total_allocated += size;
                g_stats.current_usage += size;
                g_stats.allocation_count++;
                g_stats.pool_hits++;

                if (g_stats.current_usage > g_stats.peak_usage) {
                    g_stats.peak_usage = g_stats.current_usage;
                }

                return ptr + sizeof(tbos_mem_block_t);
            }
        } else {
            consecutive = 0;
        }
    }

    g_stats.failures++;
    return NULL;
}

static void static_free(void *ptr) {
    if (!ptr) return;

    uint8_t *raw_ptr = (uint8_t*)ptr - sizeof(tbos_mem_block_t);
    tbos_mem_block_t *block = (tbos_mem_block_t*)raw_ptr;

    if (!(block->flags & TBOS_MEM_FLAG_USED)) {
        return;  /* Double free */
    }

    tbos_mem_size_t size = block->size;
    tbos_mem_size_t aligned_size = align_up(size + sizeof(tbos_mem_block_t),
                                             TBOS_MEM_ALIGNMENT);
    uint32_t blocks_used = aligned_size / TBOS_MEM_ALIGNMENT;

    /* Calculate block index */
    uint32_t start_block = (raw_ptr - g_static_pool) / TBOS_MEM_ALIGNMENT;

    /* Clear bitmap */
    for (uint32_t i = start_block; i < start_block + blocks_used; i++) {
        uint32_t byte_idx = i / 8;
        uint32_t bit_idx = i % 8;
        g_pool_bitmap[byte_idx] &= ~(1 << bit_idx);
    }

    block->flags = 0;
    g_stats.total_freed += size;
    g_stats.current_usage -= size;
    g_stats.free_count++;
}

#endif /* TBOS_MEM_STATIC_ONLY */

/* ========================================================================= */
/* HEAP ALLOCATOR (Tier 2+)                                                  */
/* ========================================================================= */

#if defined(TBOS_MEM_BASIC_HEAP) || defined(TBOS_MEM_FULL_HEAP)

static void* heap_alloc(tbos_mem_size_t size) {
    if (size > TBOS_MEM_MAX_ALLOC) {
        g_stats.failures++;
        return NULL;
    }

    /* Add header overhead */
    tbos_mem_size_t total = size + sizeof(tbos_mem_block_t);
    total = align_up(total, TBOS_MEM_ALIGNMENT);

    void *raw = malloc(total);
    if (!raw) {
        g_stats.failures++;
        return NULL;
    }

    tbos_mem_block_t *block = (tbos_mem_block_t*)raw;
    block->size = size;
    block->flags = TBOS_MEM_FLAG_USED;
    block->pool_id = 0xFF;  /* Heap indicator */
#if TBOS_ARCH_BITS >= 32
    block->magic = TBOS_MEM_MAGIC;
    block->next = NULL;
#endif

    g_stats.total_allocated += size;
    g_stats.current_usage += size;
    g_stats.allocation_count++;
    g_stats.heap_hits++;

    if (g_stats.current_usage > g_stats.peak_usage) {
        g_stats.peak_usage = g_stats.current_usage;
    }

    return (uint8_t*)raw + sizeof(tbos_mem_block_t);
}

static void heap_free(void *ptr) {
    if (!ptr) return;

    tbos_mem_block_t *block = (tbos_mem_block_t*)((uint8_t*)ptr -
                                                   sizeof(tbos_mem_block_t));

#if TBOS_ARCH_BITS >= 32
    /* Validate magic */
    if (block->magic != TBOS_MEM_MAGIC) {
        fprintf(stderr, "[MEM] Invalid free: bad magic at %p\n", ptr);
        return;
    }
#endif

    if (!(block->flags & TBOS_MEM_FLAG_USED)) {
        fprintf(stderr, "[MEM] Double free detected at %p\n", ptr);
        return;
    }

    tbos_mem_size_t size = block->size;
    block->flags = 0;
#if TBOS_ARCH_BITS >= 32
    block->magic = 0;
#endif

    g_stats.total_freed += size;
    g_stats.current_usage -= size;
    g_stats.free_count++;

    free(block);
}

static void* heap_realloc(void *ptr, tbos_mem_size_t new_size) {
    if (!ptr) {
        return heap_alloc(new_size);
    }

    if (new_size == 0) {
        heap_free(ptr);
        return NULL;
    }

    tbos_mem_block_t *block = (tbos_mem_block_t*)((uint8_t*)ptr -
                                                   sizeof(tbos_mem_block_t));
    tbos_mem_size_t old_size = block->size;

    /* If shrinking or same size, just update */
    if (new_size <= old_size) {
        g_stats.current_usage -= (old_size - new_size);
        block->size = new_size;
        return ptr;
    }

    /* Allocate new block */
    void *new_ptr = heap_alloc(new_size);
    if (!new_ptr) {
        return NULL;
    }

    /* Copy old data */
    memcpy(new_ptr, ptr, old_size);

    /* Free old block */
    heap_free(ptr);

    return new_ptr;
}

#endif /* HEAP */

/* ========================================================================= */
/* ALIGNED ALLOCATOR                                                         */
/* ========================================================================= */

void* tbos_tier_alloc_aligned(tbos_mem_size_t size, uint32_t alignment) {
    if (!is_power_of_2(alignment) || alignment < TBOS_MEM_ALIGNMENT) {
        alignment = TBOS_MEM_ALIGNMENT;
    }

#ifdef TBOS_MEM_STATIC_ONLY
    /* Static pool - alignment is limited */
    return static_alloc(size);
#else
    /* Allocate with extra space for alignment */
    tbos_mem_size_t total = size + alignment + sizeof(void*);
    void *raw = tbos_tier_alloc(total);
    if (!raw) return NULL;

    /* Align pointer */
    uintptr_t addr = (uintptr_t)raw + sizeof(void*);
    uintptr_t aligned = (addr + alignment - 1) & ~(alignment - 1);

    /* Store original pointer before aligned address */
    ((void**)aligned)[-1] = raw;

    return (void*)aligned;
#endif
}

void tbos_tier_free_aligned(void *ptr) {
    if (!ptr) return;

#ifdef TBOS_MEM_STATIC_ONLY
    static_free(ptr);
#else
    /* Retrieve original pointer */
    void *raw = ((void**)ptr)[-1];
    tbos_tier_free(raw);
#endif
}

/* ========================================================================= */
/* PUBLIC API                                                                */
/* ========================================================================= */

int tbos_tier_mem_init(void) {
    if (g_mem_initialized) {
        return 0;
    }

    memset(&g_stats, 0, sizeof(g_stats));
    memset(g_pools, 0, sizeof(g_pools));

#ifdef TBOS_MEM_STATIC_ONLY
    memset(g_static_pool, 0, sizeof(g_static_pool));
    memset(g_pool_bitmap, 0, sizeof(g_pool_bitmap));
    printf("[MEM] Tier 0/1: Static pool initialized (%u bytes)\n",
           (unsigned)sizeof(g_static_pool));
#elif defined(TBOS_MEM_SMALL_HEAP)
    printf("[MEM] Tier 1: Small heap initialized\n");
#elif defined(TBOS_MEM_BASIC_HEAP)
    printf("[MEM] Tier 2: Basic heap initialized\n");
#elif defined(TBOS_MEM_FULL_HEAP)
    #ifdef TBOS_MEM_NUMA_AWARE
    printf("[MEM] Tier %d: Full heap with NUMA support initialized\n",
           TBOS_ARCH_TIER);
    #else
    printf("[MEM] Tier 3: Full heap initialized\n");
    #endif
#endif

    g_mem_initialized = true;
    return 0;
}

void tbos_tier_mem_shutdown(void) {
    if (!g_mem_initialized) return;

    printf("[MEM] Shutdown: allocated=%llu, freed=%llu, current=%llu\n",
           (unsigned long long)g_stats.total_allocated,
           (unsigned long long)g_stats.total_freed,
           (unsigned long long)g_stats.current_usage);

    g_mem_initialized = false;
}

void* tbos_tier_alloc(tbos_mem_size_t size) {
    if (!g_mem_initialized) {
        tbos_tier_mem_init();
    }

    if (size == 0) {
        return NULL;
    }

    MEM_LOCK();
    void *ptr;

#ifdef TBOS_MEM_STATIC_ONLY
    ptr = static_alloc(size);
#else
    ptr = heap_alloc(size);
#endif

    MEM_UNLOCK();
    return ptr;
}

void* tbos_tier_calloc(tbos_mem_size_t count, tbos_mem_size_t size) {
    tbos_mem_size_t total = count * size;
    if (count != 0 && total / count != size) {
        /* Overflow */
        return NULL;
    }

    void *ptr = tbos_tier_alloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

void* tbos_tier_realloc(void *ptr, tbos_mem_size_t new_size) {
    if (!g_mem_initialized) {
        tbos_tier_mem_init();
    }

    MEM_LOCK();
    void *result;

#ifdef TBOS_MEM_STATIC_ONLY
    /* For static pool, allocate new and copy */
    if (!ptr) {
        result = static_alloc(new_size);
    } else if (new_size == 0) {
        static_free(ptr);
        result = NULL;
    } else {
        result = static_alloc(new_size);
        if (result) {
            tbos_mem_block_t *block = (tbos_mem_block_t*)((uint8_t*)ptr -
                                                          sizeof(tbos_mem_block_t));
            tbos_mem_size_t copy_size = block->size < new_size ? block->size : new_size;
            memcpy(result, ptr, copy_size);
            static_free(ptr);
        }
    }
#else
    result = heap_realloc(ptr, new_size);
#endif

    MEM_UNLOCK();
    return result;
}

void tbos_tier_free(void *ptr) {
    if (!ptr || !g_mem_initialized) return;

    MEM_LOCK();

#ifdef TBOS_MEM_STATIC_ONLY
    static_free(ptr);
#else
    heap_free(ptr);
#endif

    MEM_UNLOCK();
}

#ifdef TBOS_MEM_NUMA_AWARE
void* tbos_tier_alloc_numa(tbos_mem_size_t size, int node) {
    /* NUMA-aware allocation */
    (void)node;  /* TODO: Implement with numa_alloc_onnode() */
    return tbos_tier_alloc(size);
}
#endif

void tbos_tier_mem_stats(tbos_mem_stats_t *stats) {
    if (stats) {
        MEM_LOCK();
        *stats = g_stats;
        MEM_UNLOCK();
    }
}

tbos_mem_size_t tbos_tier_mem_available(void) {
#ifdef TBOS_MEM_STATIC_ONLY
    return sizeof(g_static_pool) - g_stats.current_usage;
#else
    /* For heap systems, return estimate */
    const tbos_arch_info_t *info = tbos_arch_get_info();
    return info->available_memory;
#endif
}

tbos_mem_size_t tbos_tier_mem_gc(void) {
#if TBOS_ARCH_BITS >= 32 && defined(__GLIBC__)
    /* Trim heap on glibc systems */
    malloc_trim(0);
    return 0;  /* Can't know exact amount freed */
#else
    return 0;
#endif
}

void tbos_tier_mem_print_status(void) {
    tbos_mem_stats_t stats;
    tbos_tier_mem_stats(&stats);

    const char* tier_names[] = {
        "Calculator", "Embedded", "Retro", "Desktop", "Server", "Supercomputer"
    };

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║      TernaryBit OS - Tier-Adaptive Memory Status             ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Processor Tier:  %-42s ║\n", tier_names[TBOS_ARCH_TIER]);
    printf("║ Memory Model:    ");
#ifdef TBOS_MEM_STATIC_ONLY
    printf("%-42s ║\n", "Static Pool (no heap)");
#elif defined(TBOS_MEM_SMALL_HEAP)
    printf("%-42s ║\n", "Small Heap");
#elif defined(TBOS_MEM_BASIC_HEAP)
    printf("%-42s ║\n", "Basic Heap");
#elif defined(TBOS_MEM_FULL_HEAP)
    #ifdef TBOS_MEM_NUMA_AWARE
    printf("%-42s ║\n", "Full Heap + NUMA");
    #else
    printf("%-42s ║\n", "Full Heap");
    #endif
#endif
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Total Allocated: %-39llu B  ║\n",
           (unsigned long long)stats.total_allocated);
    printf("║ Total Freed:     %-39llu B  ║\n",
           (unsigned long long)stats.total_freed);
    printf("║ Current Usage:   %-39llu B  ║\n",
           (unsigned long long)stats.current_usage);
    printf("║ Peak Usage:      %-39llu B  ║\n",
           (unsigned long long)stats.peak_usage);
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Allocations:     %-42u ║\n", stats.allocation_count);
    printf("║ Frees:           %-42u ║\n", stats.free_count);
    printf("║ Pool Hits:       %-42u ║\n", stats.pool_hits);
    printf("║ Heap Hits:       %-42u ║\n", stats.heap_hits);
    printf("║ Failures:        %-42u ║\n", stats.failures);
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
}
