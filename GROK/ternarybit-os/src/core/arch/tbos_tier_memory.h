/**
 * @file tbos_tier_memory.h
 * @brief TernaryBit OS - Tier-Adaptive Memory Management
 *
 * Automatically selects optimal memory strategy based on processor tier:
 * - Tier 0 (Calculator): Static pools only, no heap
 * - Tier 1 (Embedded): Small heap with static fallback
 * - Tier 2 (Retro): Basic heap with pools
 * - Tier 3 (Desktop): Full heap with caching
 * - Tier 4 (Server): NUMA-aware allocation
 * - Tier 5 (Supercomputer): Distributed memory support
 *
 * @version 1.0 (Production)
 * @date 2025-11-03
 */

#ifndef TBOS_TIER_MEMORY_H
#define TBOS_TIER_MEMORY_H

#include "tbos_arch.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* TIER-SPECIFIC CONFIGURATION                                               */
/* ========================================================================= */

#if TBOS_ARCH_TIER == 0  /* Calculator: 8-bit, <1MB */
    #define TBOS_MEM_STATIC_ONLY    1
    #define TBOS_MEM_POOL_SIZE      256      /* 256 bytes */
    #define TBOS_MEM_MAX_ALLOC      64       /* Max single allocation */
    #define TBOS_MEM_ALIGNMENT      1
    #define TBOS_MEM_POOL_COUNT     4
    typedef uint8_t tbos_mem_size_t;

#elif TBOS_ARCH_TIER == 1  /* Embedded: 16-bit, 1-64KB */
    #define TBOS_MEM_SMALL_HEAP     1
    #define TBOS_MEM_POOL_SIZE      4096     /* 4KB */
    #define TBOS_MEM_MAX_ALLOC      1024     /* 1KB max */
    #define TBOS_MEM_ALIGNMENT      2
    #define TBOS_MEM_POOL_COUNT     8
    typedef uint16_t tbos_mem_size_t;

#elif TBOS_ARCH_TIER == 2  /* Retro: 32-bit, 64MB-1GB */
    #define TBOS_MEM_BASIC_HEAP     1
    #define TBOS_MEM_POOL_SIZE      65536    /* 64KB */
    #define TBOS_MEM_MAX_ALLOC      (16*1024*1024)  /* 16MB max */
    #define TBOS_MEM_ALIGNMENT      4
    #define TBOS_MEM_POOL_COUNT     16
    typedef uint32_t tbos_mem_size_t;

#elif TBOS_ARCH_TIER == 3  /* Desktop: 64-bit, 1-16GB */
    #define TBOS_MEM_FULL_HEAP      1
    #define TBOS_MEM_POOL_SIZE      (1024*1024)  /* 1MB */
    #define TBOS_MEM_MAX_ALLOC      (256*1024*1024)  /* 256MB max */
    #define TBOS_MEM_ALIGNMENT      8
    #define TBOS_MEM_POOL_COUNT     32
    #define TBOS_MEM_CACHE_ENABLED  1
    typedef uint64_t tbos_mem_size_t;

#elif TBOS_ARCH_TIER == 4  /* Server: 64-bit, 16-256GB */
    #define TBOS_MEM_FULL_HEAP      1
    #define TBOS_MEM_NUMA_AWARE     1
    #define TBOS_MEM_POOL_SIZE      (4*1024*1024)  /* 4MB */
    #define TBOS_MEM_MAX_ALLOC      (1024*1024*1024ULL)  /* 1GB max */
    #define TBOS_MEM_ALIGNMENT      64  /* Cache line */
    #define TBOS_MEM_POOL_COUNT     64
    #define TBOS_MEM_CACHE_ENABLED  1
    #define TBOS_MEM_HUGE_PAGES     1
    typedef uint64_t tbos_mem_size_t;

#else  /* Supercomputer: >256GB */
    #define TBOS_MEM_FULL_HEAP      1
    #define TBOS_MEM_NUMA_AWARE     1
    #define TBOS_MEM_DISTRIBUTED    1
    #define TBOS_MEM_POOL_SIZE      (16*1024*1024)  /* 16MB */
    #define TBOS_MEM_MAX_ALLOC      (16ULL*1024*1024*1024)  /* 16GB max */
    #define TBOS_MEM_ALIGNMENT      64
    #define TBOS_MEM_POOL_COUNT     128
    #define TBOS_MEM_CACHE_ENABLED  1
    #define TBOS_MEM_HUGE_PAGES     1
    typedef uint64_t tbos_mem_size_t;
#endif

/* ========================================================================= */
/* MEMORY BLOCK HEADER                                                       */
/* ========================================================================= */

typedef struct tbos_mem_block {
    tbos_mem_size_t size;           /* Size of this block */
    uint8_t flags;                  /* Block flags */
    uint8_t pool_id;                /* Pool this came from */
#if TBOS_ARCH_BITS >= 32
    uint16_t magic;                 /* Magic number for validation */
    struct tbos_mem_block *next;    /* Next free block (in free list) */
#endif
} tbos_mem_block_t;

/* Block flags */
#define TBOS_MEM_FLAG_USED      0x01
#define TBOS_MEM_FLAG_POOLED    0x02
#define TBOS_MEM_FLAG_ALIGNED   0x04
#define TBOS_MEM_FLAG_ZEROED    0x08
#define TBOS_MEM_FLAG_LOCKED    0x10
#define TBOS_MEM_FLAG_DMA       0x20

/* Magic number for validation */
#define TBOS_MEM_MAGIC          0xTB05

/* ========================================================================= */
/* MEMORY POOL                                                               */
/* ========================================================================= */

typedef struct {
    uint8_t *base;                  /* Base address of pool */
    tbos_mem_size_t size;           /* Total size */
    tbos_mem_size_t used;           /* Used bytes */
    tbos_mem_size_t block_size;     /* Block size (for fixed pools) */
    uint32_t total_blocks;          /* Total blocks */
    uint32_t free_blocks;           /* Free blocks */
    uint8_t *bitmap;                /* Allocation bitmap (if fixed) */
    tbos_mem_block_t *free_list;    /* Free list (if variable) */
    bool is_fixed;                  /* Fixed or variable size blocks */
#ifdef TBOS_MEM_NUMA_AWARE
    uint32_t numa_node;             /* NUMA node this pool belongs to */
#endif
} tbos_mem_pool_t;

/* ========================================================================= */
/* MEMORY STATISTICS                                                         */
/* ========================================================================= */

typedef struct {
    tbos_mem_size_t total_allocated;
    tbos_mem_size_t total_freed;
    tbos_mem_size_t current_usage;
    tbos_mem_size_t peak_usage;
    uint32_t allocation_count;
    uint32_t free_count;
    uint32_t pool_hits;
    uint32_t heap_hits;
    uint32_t failures;
#ifdef TBOS_MEM_NUMA_AWARE
    tbos_mem_size_t per_node_usage[16];
#endif
} tbos_mem_stats_t;

/* ========================================================================= */
/* API FUNCTIONS                                                             */
/* ========================================================================= */

/**
 * @brief Initialize tier-adaptive memory system
 * @return 0 on success
 */
int tbos_tier_mem_init(void);

/**
 * @brief Shutdown memory system
 */
void tbos_tier_mem_shutdown(void);

/**
 * @brief Allocate memory
 * @param size Size in bytes
 * @return Pointer to memory or NULL
 */
void* tbos_tier_alloc(tbos_mem_size_t size);

/**
 * @brief Allocate zeroed memory
 * @param count Number of elements
 * @param size Size of each element
 * @return Pointer to zeroed memory or NULL
 */
void* tbos_tier_calloc(tbos_mem_size_t count, tbos_mem_size_t size);

/**
 * @brief Reallocate memory
 * @param ptr Original pointer
 * @param new_size New size
 * @return Pointer to new memory or NULL
 */
void* tbos_tier_realloc(void *ptr, tbos_mem_size_t new_size);

/**
 * @brief Free memory
 * @param ptr Pointer to free
 */
void tbos_tier_free(void *ptr);

/**
 * @brief Allocate aligned memory
 * @param size Size in bytes
 * @param alignment Alignment (must be power of 2)
 * @return Aligned pointer or NULL
 */
void* tbos_tier_alloc_aligned(tbos_mem_size_t size, uint32_t alignment);

/**
 * @brief Free aligned memory
 * @param ptr Aligned pointer
 */
void tbos_tier_free_aligned(void *ptr);

#ifdef TBOS_MEM_NUMA_AWARE
/**
 * @brief Allocate on specific NUMA node
 * @param size Size in bytes
 * @param node NUMA node (or -1 for local)
 * @return Pointer to memory or NULL
 */
void* tbos_tier_alloc_numa(tbos_mem_size_t size, int node);
#endif

/**
 * @brief Get memory statistics
 * @param stats Output statistics
 */
void tbos_tier_mem_stats(tbos_mem_stats_t *stats);

/**
 * @brief Print memory status
 */
void tbos_tier_mem_print_status(void);

/**
 * @brief Get available memory
 * @return Available bytes
 */
tbos_mem_size_t tbos_tier_mem_available(void);

/**
 * @brief Trigger garbage collection (if supported)
 * @return Bytes freed
 */
tbos_mem_size_t tbos_tier_mem_gc(void);

/* ========================================================================= */
/* CONVENIENCE MACROS                                                        */
/* ========================================================================= */

/* Use tier-adaptive allocator as default */
#define TBOS_ALLOC(size)        tbos_tier_alloc(size)
#define TBOS_CALLOC(n, size)    tbos_tier_calloc(n, size)
#define TBOS_REALLOC(ptr, size) tbos_tier_realloc(ptr, size)
#define TBOS_FREE(ptr)          tbos_tier_free(ptr)

/* Typed allocation */
#define TBOS_NEW(type)          ((type*)tbos_tier_calloc(1, sizeof(type)))
#define TBOS_NEW_ARRAY(type, n) ((type*)tbos_tier_calloc(n, sizeof(type)))
#define TBOS_DELETE(ptr)        tbos_tier_free(ptr)

#ifdef __cplusplus
}
#endif

#endif /* TBOS_TIER_MEMORY_H */
