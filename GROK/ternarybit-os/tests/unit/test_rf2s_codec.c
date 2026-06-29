/*
 * RF2S Codec Unit Tests
 * Tests frequency parsing and path canonicalization
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* Host build stubs */
#ifdef HOST_BUILD
#include <errno.h>
#define EINVAL 22
#define ENOSPC 28
#define ENOMEM 12

void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
void* memset(void* s, int c, size_t n);
void* memcpy(void* dest, const void* src, size_t n);
size_t strlen(const char* s);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
char* strchr(const char* s, int c);
int snprintf(char* str, size_t size, const char* format, ...);
#endif

#include "fs/rf2s_codec.h"

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing %s... ", #name); \
    tests_run++; \
    test_##name(); \
    tests_passed++; \
    printf("PASS\n"); \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf("FAIL: %s != %s (%lld != %lld)\n", #a, #b, (long long)(a), (long long)(b)); \
        return; \
    } \
} while(0)

#define ASSERT_STR_EQ(a, b) do { \
    if (strcmp((a), (b)) != 0) { \
        printf("FAIL: %s != %s (\"%s\" != \"%s\")\n", #a, #b, (a), (b)); \
        return; \
    } \
} while(0)

/* Test frequency parsing - basic Hz */
TEST(parse_hz_basic) {
    rf2s_freq_t freq;
    int rc = rf2s_parse_frequency("432", &freq);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(freq.frequency_hz, 432);
    ASSERT_EQ(freq.original_unit, RF2S_UNIT_HZ);
}

/* Test frequency parsing - Hz suffix */
TEST(parse_hz_suffix) {
    rf2s_freq_t freq;
    int rc = rf2s_parse_frequency("432Hz", &freq);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(freq.frequency_hz, 432);
    ASSERT_EQ(freq.original_unit, RF2S_UNIT_HZ);
}

/* Test frequency parsing - kHz */
TEST(parse_khz) {
    rf2s_freq_t freq;
    int rc = rf2s_parse_frequency("432kHz", &freq);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(freq.frequency_hz, 432000);
    ASSERT_EQ(freq.original_unit, RF2S_UNIT_KHZ);
}

/* Test frequency parsing - kHz decimal */
TEST(parse_khz_decimal) {
    rf2s_freq_t freq;
    int rc = rf2s_parse_frequency("432.5kHz", &freq);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(freq.frequency_hz, 432500);
    ASSERT_EQ(freq.original_unit, RF2S_UNIT_KHZ);
}

/* Test frequency parsing - MHz */
TEST(parse_mhz) {
    rf2s_freq_t freq;
    int rc = rf2s_parse_frequency("432MHz", &freq);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(freq.frequency_hz, 432000000);
    ASSERT_EQ(freq.original_unit, RF2S_UNIT_MHZ);
}

/* Test frequency parsing - MHz decimal */
TEST(parse_mhz_decimal) {
    rf2s_freq_t freq;
    int rc = rf2s_parse_frequency("2.4MHz", &freq);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(freq.frequency_hz, 2400000);
    ASSERT_EQ(freq.original_unit, RF2S_UNIT_MHZ);
}

/* Test frequency parsing - GHz */
TEST(parse_ghz) {
    rf2s_freq_t freq;
    int rc = rf2s_parse_frequency("2GHz", &freq);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(freq.frequency_hz, 2000000000ULL);
    ASSERT_EQ(freq.original_unit, RF2S_UNIT_GHZ);
}

/* Test frequency parsing - GHz decimal */
TEST(parse_ghz_decimal) {
    rf2s_freq_t freq;
    int rc = rf2s_parse_frequency("2.4GHz", &freq);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(freq.frequency_hz, 2400000000ULL);
    ASSERT_EQ(freq.original_unit, RF2S_UNIT_GHZ);
}

/* Test frequency parsing - case insensitive */
TEST(parse_case_insensitive) {
    rf2s_freq_t freq;

    int rc = rf2s_parse_frequency("432MHZ", &freq);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(freq.frequency_hz, 432000000);

    rc = rf2s_parse_frequency("432mhz", &freq);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(freq.frequency_hz, 432000000);
}

/* Test rf2s_is_frequency */
TEST(is_frequency) {
    ASSERT_EQ(rf2s_is_frequency("432"), 1);
    ASSERT_EQ(rf2s_is_frequency("432Hz"), 1);
    ASSERT_EQ(rf2s_is_frequency("432MHz"), 1);
    ASSERT_EQ(rf2s_is_frequency("2.4GHz"), 1);
    ASSERT_EQ(rf2s_is_frequency("sensor.dat"), 0);
    ASSERT_EQ(rf2s_is_frequency("hello"), 0);
    ASSERT_EQ(rf2s_is_frequency(""), 0);
}

/* Test full path parsing */
TEST(parse_path) {
    rf2s_path_t path;
    int rc = rf2s_parse("/432MHz/sensor/data.bin", &path);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(path.frequency.frequency_hz, 432000000);
    ASSERT_EQ(path.component_count, 2);
    ASSERT_STR_EQ(path.components[0], "sensor");
    ASSERT_STR_EQ(path.components[1], "data.bin");
    rf2s_free(&path);
}

/* Test canonical path conversion */
TEST(to_canonical) {
    rf2s_path_t path;
    int rc = rf2s_parse("/432MHz/sensor/data.bin", &path);
    ASSERT_EQ(rc, 0);

    char canonical[256];
    rc = rf2s_to_canonical(&path, canonical, sizeof(canonical));
    ASSERT_EQ(rc, 0);
    ASSERT_STR_EQ(canonical, "/rf2s/432000000/sensor/data.bin");

    rf2s_free(&path);
}

/* Test canonical path with custom base */
TEST(to_canonical_with_base) {
    rf2s_path_t path;
    int rc = rf2s_parse("/2.4GHz/wifi/beacon", &path);
    ASSERT_EQ(rc, 0);

    char canonical[256];
    rc = rf2s_to_canonical_with_base(&path, "/mnt/radio", canonical, sizeof(canonical));
    ASSERT_EQ(rc, 0);
    ASSERT_STR_EQ(canonical, "/mnt/radio/2400000000/wifi/beacon");

    rf2s_free(&path);
}

/* Test frequency formatting */
TEST(format_frequency) {
    char buffer[64];

    rf2s_format_frequency(432, buffer, sizeof(buffer));
    ASSERT_STR_EQ(buffer, "432 Hz");

    rf2s_format_frequency(432000, buffer, sizeof(buffer));
    ASSERT_STR_EQ(buffer, "432 kHz");

    rf2s_format_frequency(432000000, buffer, sizeof(buffer));
    ASSERT_STR_EQ(buffer, "432 MHz");

    rf2s_format_frequency(2400000000ULL, buffer, sizeof(buffer));
    ASSERT_STR_EQ(buffer, "2400 MHz");
}

/* Test normalized string in rf2s_freq_t */
TEST(normalized_string) {
    rf2s_freq_t freq;
    rf2s_parse_frequency("432MHz", &freq);
    ASSERT_STR_EQ(freq.normalized, "432000000");

    rf2s_parse_frequency("2.4GHz", &freq);
    ASSERT_STR_EQ(freq.normalized, "2400000000");
}

int main(void) {
    printf("RF2S Codec Unit Tests\n");
    printf("=====================\n\n");

    printf("Frequency Parsing Tests:\n");
    RUN_TEST(parse_hz_basic);
    RUN_TEST(parse_hz_suffix);
    RUN_TEST(parse_khz);
    RUN_TEST(parse_khz_decimal);
    RUN_TEST(parse_mhz);
    RUN_TEST(parse_mhz_decimal);
    RUN_TEST(parse_ghz);
    RUN_TEST(parse_ghz_decimal);
    RUN_TEST(parse_case_insensitive);

    printf("\nFrequency Detection Tests:\n");
    RUN_TEST(is_frequency);

    printf("\nPath Parsing Tests:\n");
    RUN_TEST(parse_path);

    printf("\nCanonicalization Tests:\n");
    RUN_TEST(to_canonical);
    RUN_TEST(to_canonical_with_base);

    printf("\nFormatting Tests:\n");
    RUN_TEST(format_frequency);
    RUN_TEST(normalized_string);

    printf("\n=====================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}
