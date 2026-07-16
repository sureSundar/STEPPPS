/**
 * @file test_pxfs.c
 * @brief PXFS Integration Tests
 *
 * Tests for PXFS compression, decompression, and filesystem operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "pxfs.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST UTILITIES
 * ═══════════════════════════════════════════════════════════════════════════ */

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        tests_run++; \
        printf("  [TEST] %s... ", #name); \
        fflush(stdout); \
    } while(0)

#define PASS() \
    do { \
        tests_passed++; \
        printf("\033[32mPASS\033[0m\n"); \
    } while(0)

#define FAIL(msg) \
    do { \
        printf("\033[31mFAIL\033[0m: %s\n", msg); \
    } while(0)

#define ASSERT_EQ(a, b, msg) \
    do { \
        if ((a) != (b)) { \
            FAIL(msg); \
            return; \
        } \
    } while(0)

#define ASSERT_TRUE(cond, msg) \
    do { \
        if (!(cond)) { \
            FAIL(msg); \
            return; \
        } \
    } while(0)

/* ═══════════════════════════════════════════════════════════════════════════
 * COMPRESSION TESTS
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_compress_raw(void) {
    TEST(compress_raw);

    uint8_t data[] = "Hello, PXFS World!";
    uint8_t compressed[256];
    uint8_t decompressed[256];
    size_t comp_len, decomp_len;

    int ret = pxfs_compress(data, sizeof(data), compressed, &comp_len, PXFS_MODE_RAW);
    ASSERT_EQ(ret, 0, "compress failed");
    ASSERT_TRUE(compressed[0] == 0x50, "bad magic");
    ASSERT_TRUE(compressed[1] == PXFS_MODE_RAW, "bad mode");

    ret = pxfs_decompress(compressed, comp_len, decompressed, &decomp_len);
    ASSERT_EQ(ret, 0, "decompress failed");
    ASSERT_TRUE(memcmp(data, decompressed, sizeof(data)) == 0, "data mismatch");

    PASS();
}

static void test_compress_zeros(void) {
    TEST(compress_zeros_quantum);

    uint8_t data[4096];
    memset(data, 0, sizeof(data));

    uint8_t compressed[4200];
    uint8_t decompressed[4200];
    size_t comp_len, decomp_len;

    int ret = pxfs_compress(data, sizeof(data), compressed, &comp_len, PXFS_MODE_AUTO);
    ASSERT_EQ(ret, 0, "compress failed");

    /* Should achieve excellent compression */
    float ratio = pxfs_compression_ratio(sizeof(data), comp_len);
    ASSERT_TRUE(ratio >= 100.0f, "compression ratio too low");

    ret = pxfs_decompress(compressed, comp_len, decompressed, &decomp_len);
    ASSERT_EQ(ret, 0, "decompress failed");
    ASSERT_EQ(decomp_len, sizeof(data), "size mismatch");
    ASSERT_TRUE(memcmp(data, decompressed, sizeof(data)) == 0, "data mismatch");

    PASS();
}

static void test_compress_ones(void) {
    TEST(compress_ones_quantum);

    uint8_t data[4096];
    memset(data, 0xFF, sizeof(data));

    uint8_t compressed[4200];
    uint8_t decompressed[4200];
    size_t comp_len, decomp_len;

    int ret = pxfs_compress(data, sizeof(data), compressed, &comp_len, PXFS_MODE_QUANTUM);
    ASSERT_EQ(ret, 0, "compress failed");

    float ratio = pxfs_compression_ratio(sizeof(data), comp_len);
    ASSERT_TRUE(ratio >= 100.0f, "compression ratio too low");

    ret = pxfs_decompress(compressed, comp_len, decompressed, &decomp_len);
    ASSERT_EQ(ret, 0, "decompress failed");
    ASSERT_TRUE(memcmp(data, decompressed, sizeof(data)) == 0, "data mismatch");

    PASS();
}

static void test_compress_repeat(void) {
    TEST(compress_repeat_quantum);

    uint8_t data[4096];
    memset(data, 0x42, sizeof(data));

    uint8_t compressed[4200];
    uint8_t decompressed[4200];
    size_t comp_len, decomp_len;

    int ret = pxfs_compress(data, sizeof(data), compressed, &comp_len, PXFS_MODE_QUANTUM);
    ASSERT_EQ(ret, 0, "compress failed");

    float ratio = pxfs_compression_ratio(sizeof(data), comp_len);
    ASSERT_TRUE(ratio >= 100.0f, "compression ratio too low");

    ret = pxfs_decompress(compressed, comp_len, decompressed, &decomp_len);
    ASSERT_EQ(ret, 0, "decompress failed");
    ASSERT_TRUE(memcmp(data, decompressed, sizeof(data)) == 0, "data mismatch");

    PASS();
}

static void test_compress_sequence(void) {
    TEST(compress_sequence_quantum);

    uint8_t data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = (uint8_t)i;
    }

    uint8_t compressed[300];
    uint8_t decompressed[300];
    size_t comp_len, decomp_len;

    int ret = pxfs_compress(data, sizeof(data), compressed, &comp_len, PXFS_MODE_QUANTUM);
    ASSERT_EQ(ret, 0, "compress failed");

    float ratio = pxfs_compression_ratio(sizeof(data), comp_len);
    ASSERT_TRUE(ratio >= 10.0f, "compression ratio too low");

    ret = pxfs_decompress(compressed, comp_len, decompressed, &decomp_len);
    ASSERT_EQ(ret, 0, "decompress failed");
    ASSERT_TRUE(memcmp(data, decompressed, sizeof(data)) == 0, "data mismatch");

    PASS();
}

static void test_compress_rle(void) {
    TEST(compress_rle_fractal);

    /* Create RLE-friendly data */
    uint8_t data[1024];
    size_t pos = 0;
    for (int i = 0; i < 16; i++) {
        memset(data + pos, 'A' + i, 64);
        pos += 64;
    }

    uint8_t compressed[1100];
    uint8_t decompressed[1100];
    size_t comp_len, decomp_len;

    int ret = pxfs_compress(data, sizeof(data), compressed, &comp_len, PXFS_MODE_FRACTAL);
    ASSERT_EQ(ret, 0, "compress failed");

    float ratio = pxfs_compression_ratio(sizeof(data), comp_len);
    ASSERT_TRUE(ratio >= 2.0f, "compression ratio too low");

    ret = pxfs_decompress(compressed, comp_len, decompressed, &decomp_len);
    ASSERT_EQ(ret, 0, "decompress failed");
    ASSERT_TRUE(memcmp(data, decompressed, sizeof(data)) == 0, "data mismatch");

    PASS();
}

static void test_compress_random(void) {
    TEST(compress_random_fallback);

    /* Random data should fall back to raw */
    uint8_t data[1024];
    srand(12345);
    for (size_t i = 0; i < sizeof(data); i++) {
        data[i] = (uint8_t)(rand() & 0xFF);
    }

    uint8_t compressed[1100];
    uint8_t decompressed[1100];
    size_t comp_len, decomp_len;

    int ret = pxfs_compress(data, sizeof(data), compressed, &comp_len, PXFS_MODE_AUTO);
    ASSERT_EQ(ret, 0, "compress failed");

    /* Random data shouldn't compress much */
    float ratio = pxfs_compression_ratio(sizeof(data), comp_len);
    ASSERT_TRUE(ratio >= 0.9f, "unexpected compression");

    ret = pxfs_decompress(compressed, comp_len, decompressed, &decomp_len);
    ASSERT_EQ(ret, 0, "decompress failed");
    ASSERT_TRUE(memcmp(data, decompressed, sizeof(data)) == 0, "data mismatch");

    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PIXEL ENCODING TESTS
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_pixel_encode(void) {
    TEST(pixel_encode_decode);

    uint8_t data[] = "RGB";  /* 3 bytes = 1 pixel */
    pxfs_pixel_t pixels[16];
    size_t pixel_count;

    int ret = pxfs_encode_pixels(data, 3, pixels, &pixel_count, PXFS_MODE_RAW);
    ASSERT_EQ(ret, 0, "encode failed");
    ASSERT_EQ(pixel_count, 1, "wrong pixel count");
    ASSERT_EQ(pixels[0].r, 'R', "wrong R");
    ASSERT_EQ(pixels[0].g, 'G', "wrong G");
    ASSERT_EQ(pixels[0].b, 'B', "wrong B");

    uint8_t decoded[16];
    size_t decoded_len;
    ret = pxfs_decode_pixels(pixels, pixel_count, decoded, &decoded_len, PXFS_MODE_RAW);
    ASSERT_EQ(ret, 0, "decode failed");
    ASSERT_TRUE(memcmp(data, decoded, 3) == 0, "data mismatch");

    PASS();
}

static void test_pixel_padding(void) {
    TEST(pixel_encode_padding);

    /* 5 bytes = 2 pixels (with padding) */
    uint8_t data[] = "Hello";
    pxfs_pixel_t pixels[16];
    size_t pixel_count;

    int ret = pxfs_encode_pixels(data, 5, pixels, &pixel_count, PXFS_MODE_RAW);
    ASSERT_EQ(ret, 0, "encode failed");
    ASSERT_EQ(pixel_count, 2, "wrong pixel count");
    ASSERT_EQ(pixels[0].r, 'H', "wrong R");
    ASSERT_EQ(pixels[0].g, 'e', "wrong G");
    ASSERT_EQ(pixels[0].b, 'l', "wrong B");
    ASSERT_EQ(pixels[1].r, 'l', "wrong R2");
    ASSERT_EQ(pixels[1].g, 'o', "wrong G2");
    ASSERT_EQ(pixels[1].b, 0, "wrong B2 padding");

    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PATH CONVERSION TESTS
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_path_to_canonical(void) {
    TEST(path_to_canonical);

    char out[256];
    int ret = pxfs_path_to_canonical("{255,0,0}docs{255,0,0}file.txt", out, sizeof(out));
    ASSERT_EQ(ret, 0, "conversion failed");
    /* Result should be something like /pxfs/FF0000/docs/file.txt */
    ASSERT_TRUE(strstr(out, "pxfs") != NULL, "missing pxfs");

    PASS();
}

static void test_path_from_canonical(void) {
    TEST(path_from_canonical);

    char out[256];
    int ret = pxfs_path_from_canonical("/pxfs/FF0000/docs/file.txt", 255, 0, 0, out, sizeof(out));
    ASSERT_EQ(ret, 0, "conversion failed");
    ASSERT_TRUE(strstr(out, "{255,0,0}") != NULL, "missing separator");

    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * ON-DISK FORMAT TESTS
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_superblock_size(void) {
    TEST(superblock_size);

    ASSERT_EQ(sizeof(pxfs_superblock_t), 512, "superblock not 512 bytes");

    PASS();
}

static void test_inode_size(void) {
    TEST(inode_size);

    ASSERT_EQ(sizeof(pxfs_inode_t), 256, "inode not 256 bytes");

    PASS();
}

static void test_pixel_size(void) {
    TEST(pixel_size);

    ASSERT_EQ(sizeof(pxfs_pixel_t), 3, "pixel not 3 bytes");

    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMPRESSION RATIO BENCHMARK
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_compression_benchmark(void) {
    TEST(compression_benchmark);

    printf("\n");
    printf("    %-30s %10s %10s %10s\n", "Data Type", "Original", "Compressed", "Ratio");
    printf("    %-30s %10s %10s %10s\n", "─────────", "────────", "──────────", "─────");

    uint8_t compressed[8192];
    size_t comp_len;

    /* All zeros */
    {
        uint8_t data[4096];
        memset(data, 0, sizeof(data));
        pxfs_compress(data, sizeof(data), compressed, &comp_len, PXFS_MODE_AUTO);
        printf("    %-30s %10zu %10zu %9.1f:1\n", "All zeros (4KB)",
               sizeof(data), comp_len, pxfs_compression_ratio(sizeof(data), comp_len));
    }

    /* All ones */
    {
        uint8_t data[4096];
        memset(data, 0xFF, sizeof(data));
        pxfs_compress(data, sizeof(data), compressed, &comp_len, PXFS_MODE_AUTO);
        printf("    %-30s %10zu %10zu %9.1f:1\n", "All ones (4KB)",
               sizeof(data), comp_len, pxfs_compression_ratio(sizeof(data), comp_len));
    }

    /* Repeating byte */
    {
        uint8_t data[4096];
        memset(data, 0x42, sizeof(data));
        pxfs_compress(data, sizeof(data), compressed, &comp_len, PXFS_MODE_AUTO);
        printf("    %-30s %10zu %10zu %9.1f:1\n", "Repeating 0x42 (4KB)",
               sizeof(data), comp_len, pxfs_compression_ratio(sizeof(data), comp_len));
    }

    /* Sequence */
    {
        uint8_t data[256];
        for (int i = 0; i < 256; i++) data[i] = (uint8_t)i;
        pxfs_compress(data, sizeof(data), compressed, &comp_len, PXFS_MODE_AUTO);
        printf("    %-30s %10zu %10zu %9.1f:1\n", "Sequence 0-255",
               sizeof(data), comp_len, pxfs_compression_ratio(sizeof(data), comp_len));
    }

    /* Text */
    {
        uint8_t data[4096];
        const char* text = "The quick brown fox jumps over the lazy dog. ";
        size_t tlen = strlen(text);
        for (size_t i = 0; i < sizeof(data); i++) {
            data[i] = text[i % tlen];
        }
        pxfs_compress(data, sizeof(data), compressed, &comp_len, PXFS_MODE_FRACTAL);
        printf("    %-30s %10zu %10zu %9.1f:1\n", "Repeating text (4KB)",
               sizeof(data), comp_len, pxfs_compression_ratio(sizeof(data), comp_len));
    }

    /* Random */
    {
        uint8_t data[1024];
        srand(54321);
        for (size_t i = 0; i < sizeof(data); i++) {
            data[i] = (uint8_t)(rand() & 0xFF);
        }
        pxfs_compress(data, sizeof(data), compressed, &comp_len, PXFS_MODE_AUTO);
        printf("    %-30s %10zu %10zu %9.1f:1\n", "Random data (1KB)",
               sizeof(data), comp_len, pxfs_compression_ratio(sizeof(data), comp_len));
    }

    printf("  ");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════════════════════════ */

int main(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║     PXFS - Pixel eXchange FileSystem Test Suite           ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("\n");

    printf("=== Compression Tests ===\n");
    test_compress_raw();
    test_compress_zeros();
    test_compress_ones();
    test_compress_repeat();
    test_compress_sequence();
    test_compress_rle();
    test_compress_random();

    printf("\n=== Pixel Encoding Tests ===\n");
    test_pixel_encode();
    test_pixel_padding();

    printf("\n=== Path Conversion Tests ===\n");
    test_path_to_canonical();
    test_path_from_canonical();

    printf("\n=== On-Disk Format Tests ===\n");
    test_superblock_size();
    test_inode_size();
    test_pixel_size();

    printf("\n=== Compression Benchmark ===\n");
    test_compression_benchmark();

    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("  Results: %d/%d tests passed", tests_passed, tests_run);
    if (tests_passed == tests_run) {
        printf(" \033[32m✓\033[0m\n");
    } else {
        printf(" \033[31m✗\033[0m\n");
    }
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
