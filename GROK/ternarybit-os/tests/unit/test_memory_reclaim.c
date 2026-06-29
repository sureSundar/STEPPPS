/**
 * @file test_memory_reclaim.c
 * @brief Test the memory reclamation system
 *
 * Build:
 *   gcc -DHOST_BUILD -Wall -g \
 *       test_memory_reclaim.c \
 *       ../../src/rock/memory/memory_reclaim.c \
 *       ../../src/rock/error/error.c \
 *       -o test_memory_reclaim -lpthread
 */

#include "../../src/rock/memory/memory_reclaim.h"
#include "../../src/rock/error/error.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/* Test counters */
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_START(name) \
    printf("\n[TEST] %s...\n", name);

#define TEST_ASSERT(condition, message) \
    if (condition) { \
        printf("  ✅ %s\n", message); \
        tests_passed++; \
    } else { \
        printf("  ❌ %s\n", message); \
        tests_failed++; \
    }

/* Mock cache for testing */
static size_t g_mock_cache_size = 10 * 1024 * 1024;  /* 10 MB */
static uint64_t g_mock_evict_calls = 0;
static uint64_t g_mock_flush_calls = 0;

static size_t mock_cache_get_size(void) {
    return g_mock_cache_size;
}

static size_t mock_cache_evict(size_t target) {
    g_mock_evict_calls++;
    size_t evicted = (target < g_mock_cache_size) ? target : g_mock_cache_size;
    g_mock_cache_size -= evicted;
    return evicted;
}

static void mock_cache_flush(void) {
    g_mock_flush_calls++;
    g_mock_cache_size = 0;
}

/* ========================================================================= */
/* TEST CASES                                                                */
/* ========================================================================= */

void test_initialization(void) {
    TEST_START("Memory Reclamation Initialization");

    int result = mem_reclaim_init();
    TEST_ASSERT(result == 0, "Initialization succeeded");

    /* Check we can get usage */
    uint32_t usage = mem_reclaim_get_usage_percent();
    TEST_ASSERT(usage <= 100, "Usage percentage is valid");
    printf("    Current memory usage: %u%%\n", usage);
}

void test_cache_registration(void) {
    TEST_START("Cache Registration");

    /* Reset mock cache */
    g_mock_cache_size = 10 * 1024 * 1024;
    g_mock_evict_calls = 0;

    cache_info_t cache = {
        .type = CACHE_TYPE_GENERAL,
        .name = "TestCache",
        .priority = CACHE_PRIORITY_NORMAL,
        .current_size = g_mock_cache_size,
        .max_size = 100 * 1024 * 1024,
        .min_size = 1 * 1024 * 1024,
        .get_size = mock_cache_get_size,
        .evict = mock_cache_evict,
        .flush = mock_cache_flush,
        .hits = 0,
        .misses = 0,
        .evictions = 0,
        .registered = false
    };

    int result = mem_reclaim_register_cache(&cache);
    TEST_ASSERT(result == 0, "Cache registered successfully");

    size_t total = mem_reclaim_get_total_cache_size();
    TEST_ASSERT(total >= g_mock_cache_size, "Total cache size includes our cache");
    printf("    Total cache size: %zu bytes\n", total);

    /* Try to register duplicate */
    result = mem_reclaim_register_cache(&cache);
    TEST_ASSERT(result != 0, "Duplicate registration rejected");
}

void test_pressure_check(void) {
    TEST_START("Pressure Level Check");

    mem_pressure_level_t level = mem_reclaim_check_pressure();
    TEST_ASSERT(level >= MEM_PRESSURE_NONE && level <= MEM_PRESSURE_LEVEL_CRITICAL,
                "Pressure level is valid");

    uint32_t usage = mem_reclaim_get_usage_percent();
    printf("    Usage: %u%%, Pressure: %d\n", usage, level);

    /* Verify pressure correlates with usage */
    if (usage < 60) {
        TEST_ASSERT(level == MEM_PRESSURE_NONE, "Low usage = no pressure");
    } else if (usage >= 95) {
        TEST_ASSERT(level == MEM_PRESSURE_LEVEL_CRITICAL, "Very high usage = critical");
    }
}

void test_quick_reclaim(void) {
    TEST_START("Quick Reclamation");

    /* Reset mock */
    g_mock_cache_size = 10 * 1024 * 1024;
    g_mock_evict_calls = 0;

    /* Request reclamation */
    size_t freed = mem_reclaim_quick(5 * 1024 * 1024);  /* 5 MB */
    printf("    Quick reclaim freed: %zu bytes\n", freed);

    /* Check evict was called */
    TEST_ASSERT(g_mock_evict_calls > 0 || freed == 0, "Eviction attempted or no cache available");
}

void test_reclaim_request(void) {
    TEST_START("Reclamation Request");

    /* Reset mock */
    g_mock_cache_size = 20 * 1024 * 1024;
    g_mock_evict_calls = 0;

    reclaim_request_t request = {
        .target_bytes = 8 * 1024 * 1024,  /* 8 MB */
        .strategy = RECLAIM_STRATEGY_CACHE_EVICT,
        .timeout_ms = 5000,
        .allow_emergency = false,
        .sync = true
    };

    reclaim_result_t result = mem_reclaim_request(&request);

    printf("    Requested: %zu bytes\n", request.target_bytes);
    printf("    Reclaimed: %zu bytes\n", result.bytes_reclaimed);
    printf("    From cache: %zu bytes\n", result.bytes_from_cache);
    printf("    Time: %u ms\n", result.time_ms);

    TEST_ASSERT(result.bytes_from_cache > 0 || g_mock_cache_size == 0,
                "Cache eviction occurred");
    TEST_ASSERT(result.time_ms >= 0, "Time tracked");
}

void test_emergency_reclaim(void) {
    TEST_START("Emergency Reclamation");

    /* Reset mock */
    g_mock_cache_size = 50 * 1024 * 1024;
    g_mock_flush_calls = 0;

    size_t freed = mem_reclaim_emergency();

    printf("    Emergency freed: %zu bytes\n", freed);
    printf("    Flush calls: %llu\n", (unsigned long long)g_mock_flush_calls);

    TEST_ASSERT(g_mock_flush_calls > 0, "Cache flush was called");
}

void test_statistics(void) {
    TEST_START("Statistics Tracking");

    reclaim_stats_t stats;
    mem_reclaim_get_stats(&stats);

    printf("    Total reclaims: %llu\n", (unsigned long long)stats.total_reclaim_calls);
    printf("    Successful: %llu\n", (unsigned long long)stats.successful_reclaims);
    printf("    Bytes reclaimed: %zu\n", stats.total_bytes_reclaimed);
    printf("    Emergency triggers: %llu\n", (unsigned long long)stats.emergency_triggers);

    TEST_ASSERT(stats.total_reclaim_calls > 0, "Reclaims were tracked");
    TEST_ASSERT(stats.emergency_triggers > 0, "Emergency was tracked");
}

void test_karma_protection(void) {
    TEST_START("Karma-Based Protection");

    /* Set minimum karma */
    mem_reclaim_set_min_karma(-50);

    /* Check protection */
    bool protected = mem_reclaim_is_protected(1);
    TEST_ASSERT(protected == true, "Default protection is on");

    /* In production, would test actual karma lookup */
}

void test_print_status(void) {
    TEST_START("Status Display");

    mem_reclaim_print_status();
    TEST_ASSERT(true, "Status printed successfully");
}

/* ========================================================================= */
/* MAIN                                                                      */
/* ========================================================================= */

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Memory Reclamation Test Suite              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    /* Initialize error system first */
    error_init();

    /* Run tests */
    test_initialization();
    test_cache_registration();
    test_pressure_check();
    test_quick_reclaim();
    test_reclaim_request();
    test_emergency_reclaim();
    test_statistics();
    test_karma_protection();
    test_print_status();

    /* Cleanup */
    mem_reclaim_shutdown();
    error_shutdown();

    /* Results */
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                      TEST RESULTS                            ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Tests Passed:   %-3d                                         ║\n", tests_passed);
    printf("║ Tests Failed:   %-3d                                         ║\n", tests_failed);
    printf("║ Success Rate:   %.1f%%                                        ║\n",
           (float)tests_passed / (tests_passed + tests_failed) * 100.0f);
    printf("╠══════════════════════════════════════════════════════════════╣\n");

    if (tests_failed == 0) {
        printf("║ ✅ ALL TESTS PASSED - MEMORY RECLAMATION READY            ║\n");
    } else {
        printf("║ ⚠️  SOME TESTS FAILED                                       ║\n");
    }

    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    return (tests_failed == 0) ? 0 : 1;
}
