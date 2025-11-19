/*
 * TBOS Compression Module - Unit Tests
 * Tests PXFS compression integration with TBOS
 */

#include "../../src/core/compression/tbos_compression.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========================================================================= */
/* TEST UTILITIES                                                            */
/* ========================================================================= */

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("❌ FAILED: %s\n", message); \
            return -1; \
        } else { \
            printf("✅ PASSED: %s\n", message); \
        } \
    } while (0)

/* ========================================================================= */
/* TEST CASES                                                                */
/* ========================================================================= */

int test_initialization(void) {
    printf("\n[TEST] Compression Initialization\n");

    int result = tbos_compression_init();
    TEST_ASSERT(result == 0, "Compression subsystem initializes successfully");

    /* Initialize again - should be idempotent */
    result = tbos_compression_init();
    TEST_ASSERT(result == 0, "Re-initialization is safe");

    return 0;
}

int test_raw_compression(void) {
    printf("\n[TEST] RAW Mode Compression\n");

    const char* test_data = "Hello, TernaryBit OS!";
    size_t test_len = strlen(test_data);

    uint8_t compressed[256];
    size_t compressed_len;

    int result = tbos_compress((const uint8_t*)test_data, test_len,
                               compressed, &compressed_len,
                               PXFS_MODE_RAW);

    TEST_ASSERT(result == 0, "RAW compression succeeds");
    TEST_ASSERT(compressed_len > 0, "Compressed data has non-zero length");

    /* Decompress and verify */
    uint8_t decompressed[256];
    size_t decompressed_len;

    result = tbos_decompress(compressed, compressed_len,
                            decompressed, &decompressed_len);

    TEST_ASSERT(result == 0, "RAW decompression succeeds");
    TEST_ASSERT(decompressed_len == test_len, "Decompressed length matches original");
    TEST_ASSERT(memcmp(test_data, decompressed, test_len) == 0,
                "Decompressed data matches original");

    printf("   Original size: %zu bytes\n", test_len);
    printf("   Compressed size: %zu bytes\n", compressed_len);
    printf("   Ratio: %.2fx\n", (double)test_len / (double)compressed_len);

    return 0;
}

int test_dense_compression(void) {
    printf("\n[TEST] DENSE Mode Compression\n");

    uint8_t test_data[1024];
    for (int i = 0; i < 1024; i++) {
        test_data[i] = i & 0xFF;  /* Sequential pattern */
    }

    uint8_t compressed[2048];
    size_t compressed_len;

    int result = tbos_compress(test_data, sizeof(test_data),
                               compressed, &compressed_len,
                               PXFS_MODE_DENSE);

    TEST_ASSERT(result == 0, "DENSE compression succeeds");
    TEST_ASSERT(compressed_len < sizeof(test_data), "DENSE achieves compression");

    printf("   Original size: %zu bytes\n", sizeof(test_data));
    printf("   Compressed size: %zu bytes\n", compressed_len);
    printf("   Ratio: %.2fx\n", (double)sizeof(test_data) / (double)compressed_len);

    return 0;
}

int test_mode_recommendation(void) {
    printf("\n[TEST] Mode Recommendation\n");

    /* Test 1: Highly repetitive data -> should recommend QUANTUM */
    uint8_t repetitive[1000];
    memset(repetitive, 'A', sizeof(repetitive));

    pxfs_mode_t mode1 = tbos_compression_recommend_mode(repetitive, sizeof(repetitive));
    TEST_ASSERT(mode1 == PXFS_MODE_QUANTUM, "Recommends QUANTUM for repetitive data");

    /* Test 2: Random data -> should recommend DENSE or RAW */
    uint8_t random[1000];
    for (int i = 0; i < 1000; i++) {
        random[i] = (i * 7919) & 0xFF;  /* Pseudo-random */
    }

    pxfs_mode_t mode2 = tbos_compression_recommend_mode(random, sizeof(random));
    TEST_ASSERT(mode2 == PXFS_MODE_DENSE || mode2 == PXFS_MODE_RAW,
                "Recommends DENSE/RAW for random data");

    /* Test 3: Small data -> should recommend RAW */
    uint8_t small[50];
    pxfs_mode_t mode3 = tbos_compression_recommend_mode(small, sizeof(small));
    TEST_ASSERT(mode3 == PXFS_MODE_RAW, "Recommends RAW for small data");

    return 0;
}

int test_compression_statistics(void) {
    printf("\n[TEST] Compression Statistics\n");

    /* Reset statistics */
    tbos_compression_reset_stats();

    /* Perform some compressions */
    const char* test1 = "Test data 1";
    const char* test2 = "Test data 2 with more content";
    uint8_t compressed[256];
    size_t compressed_len;

    tbos_compress((const uint8_t*)test1, strlen(test1),
                  compressed, &compressed_len, PXFS_MODE_RAW);
    tbos_compress((const uint8_t*)test2, strlen(test2),
                  compressed, &compressed_len, PXFS_MODE_RAW);

    /* Get statistics */
    tbos_compression_stats_t stats;
    tbos_compression_get_stats(&stats);

    TEST_ASSERT(stats.compression_calls == 2, "Statistics track compression calls");
    TEST_ASSERT(stats.total_bytes_in > 0, "Statistics track input bytes");
    TEST_ASSERT(stats.total_bytes_out > 0, "Statistics track output bytes");
    TEST_ASSERT(stats.avg_compression_ratio > 0, "Compression ratio calculated");

    printf("   Total compressions: %lu\n", stats.compression_calls);
    printf("   Total input: %lu bytes\n", stats.total_bytes_in);
    printf("   Total output: %lu bytes\n", stats.total_bytes_out);
    printf("   Average ratio: %.2fx\n", stats.avg_compression_ratio);

    return 0;
}

int test_steppps_compression(void) {
    printf("\n[TEST] STEPPPS-Aligned Compression\n");

    const char* test_data = "STEPPPS Mother-Arcing Protocol Data";
    size_t test_len = strlen(test_data);

    uint8_t compressed[256];
    size_t compressed_len;

    int result = tbos_steppps_compress((const uint8_t*)test_data, test_len,
                                       compressed, &compressed_len);

    TEST_ASSERT(result == 0, "STEPPPS compression succeeds");

    /* Decompress */
    uint8_t decompressed[256];
    size_t decompressed_len;

    result = tbos_steppps_decompress(compressed, compressed_len,
                                     decompressed, &decompressed_len);

    TEST_ASSERT(result == 0, "STEPPPS decompression succeeds");
    TEST_ASSERT(memcmp(test_data, decompressed, test_len) == 0,
                "STEPPPS roundtrip preserves data");

    return 0;
}

int test_consciousness_aware_compression(void) {
    printf("\n[TEST] Consciousness-Aware Compression\n");

    const char* test_data = "Consciousness Level Data";
    size_t test_len = strlen(test_data);

    uint8_t compressed_high[256];
    uint8_t compressed_low[256];
    size_t compressed_high_len, compressed_low_len;

    /* High consciousness compression */
    int result = tbos_conscious_compress((const uint8_t*)test_data, test_len,
                                         250,  /* High consciousness */
                                         compressed_high, &compressed_high_len);
    TEST_ASSERT(result == 0, "High consciousness compression succeeds");

    /* Low consciousness compression */
    result = tbos_conscious_compress((const uint8_t*)test_data, test_len,
                                     50,  /* Low consciousness */
                                     compressed_low, &compressed_low_len);
    TEST_ASSERT(result == 0, "Low consciousness compression succeeds");

    printf("   High consciousness size: %zu bytes\n", compressed_high_len);
    printf("   Low consciousness size: %zu bytes\n", compressed_low_len);

    return 0;
}

int test_max_size_calculation(void) {
    printf("\n[TEST] Maximum Size Calculation\n");

    size_t input_size = 1000;

    size_t max_raw = tbos_compression_max_size(input_size, PXFS_MODE_RAW);
    size_t max_dense = tbos_compression_max_size(input_size, PXFS_MODE_DENSE);
    size_t max_quantum = tbos_compression_max_size(input_size, PXFS_MODE_QUANTUM);

    TEST_ASSERT(max_raw > 0, "RAW max size calculated");
    TEST_ASSERT(max_dense > 0, "DENSE max size calculated");
    TEST_ASSERT(max_quantum > 0, "QUANTUM max size calculated");
    TEST_ASSERT(max_quantum < max_raw, "QUANTUM requires less space than RAW");

    printf("   For %zu bytes input:\n", input_size);
    printf("   RAW max: %zu pixels\n", max_raw);
    printf("   DENSE max: %zu pixels\n", max_dense);
    printf("   QUANTUM max: %zu pixels\n", max_quantum);

    return 0;
}

/* ========================================================================= */
/* MAIN TEST RUNNER                                                          */
/* ========================================================================= */

int main(void) {
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  TBOS COMPRESSION MODULE - COMPREHENSIVE TEST SUITE      ║\n");
    printf("║  PXFS (Pixel File System) Integration Tests             ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");

    int total_tests = 0;
    int failed_tests = 0;

    #define RUN_TEST(test_func) \
        do { \
            total_tests++; \
            if (test_func() != 0) { \
                failed_tests++; \
            } \
        } while (0)

    RUN_TEST(test_initialization);
    RUN_TEST(test_raw_compression);
    RUN_TEST(test_dense_compression);
    RUN_TEST(test_mode_recommendation);
    RUN_TEST(test_compression_statistics);
    RUN_TEST(test_steppps_compression);
    RUN_TEST(test_consciousness_aware_compression);
    RUN_TEST(test_max_size_calculation);

    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║  TEST SUMMARY                                            ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("  Total tests: %d\n", total_tests);
    printf("  Passed: %d\n", total_tests - failed_tests);
    printf("  Failed: %d\n", failed_tests);

    if (failed_tests == 0) {
        printf("\n🎉 ALL TESTS PASSED! Compression module is operational.\n");
        return 0;
    } else {
        printf("\n❌ SOME TESTS FAILED. Review output above.\n");
        return 1;
    }
}
