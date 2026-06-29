/**
 * @file memory_reclaim.h
 * @brief TernaryBit OS - Memory Reclamation System
 *
 * Production-level memory reclamation with:
 * - Memory pressure detection
 * - Cache eviction
 * - Garbage collection triggers
 * - Emergency memory release
 * - Karma-aware prioritization
 *
 * @version 1.0 (Production)
 * @date 2025-11-03
 */

#ifndef TBOS_MEMORY_RECLAIM_H
#define TBOS_MEMORY_RECLAIM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* CONSTANTS                                                                 */
/* ========================================================================= */

/* Memory pressure thresholds (percentage) */
#define MEM_PRESSURE_LOW        60   /* Below this: no action */
#define MEM_PRESSURE_MEDIUM     75   /* Soft reclamation */
#define MEM_PRESSURE_HIGH       85   /* Aggressive reclamation */
#define MEM_PRESSURE_CRITICAL   95   /* Emergency reclamation */

/* Cache priorities (higher = evict last) */
#define CACHE_PRIORITY_LOW      1
#define CACHE_PRIORITY_NORMAL   5
#define CACHE_PRIORITY_HIGH     10
#define CACHE_PRIORITY_CRITICAL 100  /* Never evict */

/* Reclamation targets */
#define RECLAIM_TARGET_DEFAULT  (10 * 1024 * 1024)  /* 10 MB */
#define RECLAIM_MIN_CHUNK       (4 * 1024)           /* 4 KB */

/* ========================================================================= */
/* TYPES                                                                     */
/* ========================================================================= */

/**
 * @brief Memory pressure levels
 */
typedef enum {
    MEM_PRESSURE_NONE = 0,       /* No memory pressure */
    MEM_PRESSURE_LEVEL_LOW,      /* Light pressure, monitoring */
    MEM_PRESSURE_LEVEL_MEDIUM,   /* Moderate, start soft reclaim */
    MEM_PRESSURE_LEVEL_HIGH,     /* High, aggressive reclaim */
    MEM_PRESSURE_LEVEL_CRITICAL  /* Critical, emergency mode */
} mem_pressure_level_t;

/**
 * @brief Reclamation strategies
 */
typedef enum {
    RECLAIM_STRATEGY_NONE = 0,
    RECLAIM_STRATEGY_CACHE_TRIM,     /* Trim caches to target size */
    RECLAIM_STRATEGY_CACHE_EVICT,    /* Evict low-priority cache entries */
    RECLAIM_STRATEGY_GC_SOFT,        /* Soft garbage collection */
    RECLAIM_STRATEGY_GC_AGGRESSIVE,  /* Aggressive garbage collection */
    RECLAIM_STRATEGY_COMPACT,        /* Memory compaction */
    RECLAIM_STRATEGY_EMERGENCY,      /* Emergency release (kill low-karma) */
    RECLAIM_STRATEGY_ALL             /* All strategies */
} reclaim_strategy_t;

/**
 * @brief Cache type for eviction
 */
typedef enum {
    CACHE_TYPE_FILESYSTEM = 0,   /* Filesystem cache */
    CACHE_TYPE_NETWORK,          /* Network buffers */
    CACHE_TYPE_COMPRESSION,      /* Compression buffers */
    CACHE_TYPE_PXFS,             /* PXFS pixel cache */
    CACHE_TYPE_DNS,              /* DNS cache */
    CACHE_TYPE_GENERAL,          /* General purpose cache */
    CACHE_TYPE_COUNT
} cache_type_t;

/**
 * @brief Registered cache information
 */
typedef struct {
    cache_type_t type;
    const char *name;
    uint32_t priority;               /* Higher = evict last */
    size_t current_size;             /* Current size in bytes */
    size_t max_size;                 /* Maximum allowed size */
    size_t min_size;                 /* Minimum to keep */

    /* Callbacks */
    size_t (*get_size)(void);        /* Get current cache size */
    size_t (*evict)(size_t target);  /* Evict entries, return bytes freed */
    void (*flush)(void);             /* Flush entire cache */

    /* Statistics */
    uint64_t hits;
    uint64_t misses;
    uint64_t evictions;

    bool registered;
} cache_info_t;

/**
 * @brief Memory reclamation request
 */
typedef struct {
    size_t target_bytes;             /* Bytes to reclaim */
    reclaim_strategy_t strategy;     /* Strategy to use */
    uint32_t timeout_ms;             /* Timeout for operation */
    bool allow_emergency;            /* Allow emergency measures */
    bool sync;                       /* Synchronous operation */
} reclaim_request_t;

/**
 * @brief Memory reclamation result
 */
typedef struct {
    size_t bytes_reclaimed;          /* Bytes actually reclaimed */
    size_t bytes_from_cache;         /* Bytes from cache eviction */
    size_t bytes_from_gc;            /* Bytes from garbage collection */
    size_t bytes_from_compact;       /* Bytes from compaction */
    uint32_t time_ms;                /* Time taken */
    bool success;                    /* Target met */
    const char *message;             /* Status message */
} reclaim_result_t;

/**
 * @brief Memory reclamation statistics
 */
typedef struct {
    /* Pressure tracking */
    mem_pressure_level_t current_pressure;
    uint64_t pressure_events[5];     /* Events per pressure level */
    uint64_t time_in_pressure_ms[5]; /* Time spent in each level */

    /* Reclamation statistics */
    uint64_t total_reclaim_calls;
    uint64_t successful_reclaims;
    uint64_t failed_reclaims;
    size_t total_bytes_reclaimed;
    size_t total_bytes_from_cache;
    size_t total_bytes_from_gc;

    /* Cache statistics */
    size_t total_cache_size;
    size_t total_cache_evictions;

    /* Emergency statistics */
    uint64_t emergency_triggers;
    uint64_t processes_killed;       /* Low-karma processes terminated */

    /* Timing */
    uint32_t avg_reclaim_time_ms;
    uint32_t max_reclaim_time_ms;
} reclaim_stats_t;

/**
 * @brief Memory pressure callback type
 */
typedef void (*pressure_callback_t)(mem_pressure_level_t level, size_t available, size_t total);

/* ========================================================================= */
/* INITIALIZATION                                                            */
/* ========================================================================= */

/**
 * @brief Initialize memory reclamation system
 * @return 0 on success
 */
int mem_reclaim_init(void);

/**
 * @brief Shutdown memory reclamation system
 */
void mem_reclaim_shutdown(void);

/* ========================================================================= */
/* CACHE MANAGEMENT                                                          */
/* ========================================================================= */

/**
 * @brief Register a cache for management
 * @param info Cache information
 * @return 0 on success
 */
int mem_reclaim_register_cache(cache_info_t *info);

/**
 * @brief Unregister a cache
 * @param type Cache type to unregister
 * @return 0 on success
 */
int mem_reclaim_unregister_cache(cache_type_t type);

/**
 * @brief Update cache size (call when cache grows/shrinks)
 * @param type Cache type
 * @param size Current size
 */
void mem_reclaim_update_cache_size(cache_type_t type, size_t size);

/**
 * @brief Get total cache memory used
 * @return Total bytes in all caches
 */
size_t mem_reclaim_get_total_cache_size(void);

/* ========================================================================= */
/* PRESSURE MONITORING                                                       */
/* ========================================================================= */

/**
 * @brief Check current memory pressure level
 * @return Current pressure level
 */
mem_pressure_level_t mem_reclaim_check_pressure(void);

/**
 * @brief Get memory usage percentage
 * @return Usage as 0-100
 */
uint32_t mem_reclaim_get_usage_percent(void);

/**
 * @brief Set pressure callback
 * @param callback Function to call on pressure change
 */
void mem_reclaim_set_pressure_callback(pressure_callback_t callback);

/**
 * @brief Manually trigger pressure check
 */
void mem_reclaim_trigger_check(void);

/* ========================================================================= */
/* RECLAMATION                                                               */
/* ========================================================================= */

/**
 * @brief Request memory reclamation
 * @param request Reclamation parameters
 * @return Result of reclamation
 */
reclaim_result_t mem_reclaim_request(const reclaim_request_t *request);

/**
 * @brief Quick reclaim - try to free some memory
 * @param target_bytes Target bytes to free
 * @return Bytes actually freed
 */
size_t mem_reclaim_quick(size_t target_bytes);

/**
 * @brief Emergency reclamation - aggressive measures
 * @return Bytes freed
 */
size_t mem_reclaim_emergency(void);

/**
 * @brief Trigger garbage collection
 * @param aggressive Use aggressive mode
 * @return Bytes collected
 */
size_t mem_reclaim_gc(bool aggressive);

/**
 * @brief Compact memory to reduce fragmentation
 * @return Bytes recovered
 */
size_t mem_reclaim_compact(void);

/* ========================================================================= */
/* KARMA-AWARE RECLAMATION                                                   */
/* ========================================================================= */

/**
 * @brief Set minimum karma for process protection
 * @param min_karma Processes below this may be killed in emergency
 */
void mem_reclaim_set_min_karma(int64_t min_karma);

/**
 * @brief Check if a process is protected from reclamation
 * @param pid Process ID
 * @return true if protected
 */
bool mem_reclaim_is_protected(uint32_t pid);

/* ========================================================================= */
/* STATISTICS                                                                */
/* ========================================================================= */

/**
 * @brief Get reclamation statistics
 * @param stats Output statistics
 */
void mem_reclaim_get_stats(reclaim_stats_t *stats);

/**
 * @brief Reset statistics
 */
void mem_reclaim_reset_stats(void);

/**
 * @brief Print reclamation status
 */
void mem_reclaim_print_status(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_MEMORY_RECLAIM_H */
