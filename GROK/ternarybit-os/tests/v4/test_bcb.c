// TEST_DEPS:
/**
 * @file test_bcb.c
 * @brief Boot Capability Block (BCB) Unit Tests
 *
 * Tests BCB initialization, validation, checksum, and HAL selection.
 *
 * Traceability: v4.0/ROADMAP.md V4-007
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "tbos/bcb.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) printf("\n[TEST] %s...\n", name);
#define ASSERT(cond, msg) \
    if (cond) { \
        printf("  ✅ %s\n", msg); \
        tests_passed++; \
    } else { \
        printf("  ❌ %s\n", msg); \
        tests_failed++; \
    }

/* ========================================================================= */
/* TEST CASES                                                                 */
/* ========================================================================= */

void test_bcb_structure_size(void) {
    TEST("BCB Structure Size");

    ASSERT(sizeof(tbos_bcb_v1_t) == BCB_SIZE,
           "BCB size is exactly 128 bytes");

    ASSERT(sizeof(tbos_bcb_v1_t) == 128,
           "BCB size matches constant");
}

void test_bcb_init(void) {
    TEST("BCB Initialization");

    tbos_bcb_v1_t bcb;
    bcb_init(&bcb);

    ASSERT(bcb.magic == BCB_MAGIC,
           "Magic is TBBC");

    ASSERT(bcb.version == BCB_VERSION_CURRENT,
           "Version is current");

    ASSERT(bcb.length == BCB_SIZE - 8,
           "Length field correct");

    ASSERT(bcb.stage_flags == 0,
           "Stage flags initialized to 0");
}

void test_bcb_validation(void) {
    TEST("BCB Validation");

    tbos_bcb_v1_t bcb;
    bcb_init(&bcb);

    ASSERT(bcb_validate(&bcb) == true,
           "Initialized BCB is valid");

    ASSERT(BCB_IS_VALID(&bcb),
           "BCB_IS_VALID macro works");

    /* Test invalid magic */
    bcb.magic = 0x12345678;
    ASSERT(bcb_validate(&bcb) == false,
           "Invalid magic detected");

    /* Restore and test invalid version */
    bcb.magic = BCB_MAGIC;
    bcb.version = 0;
    ASSERT(bcb_validate(&bcb) == false,
           "Invalid version detected");
}

void test_bcb_checksum(void) {
    TEST("BCB Checksum");

    tbos_bcb_v1_t bcb;
    bcb_init(&bcb);

    /* Set some data */
    bcb.stage_flags = BCB_FLAG_BIOS | BCB_FLAG_VGA_CONSOLE;
    bcb.mem_total_bytes = 64 * 1024 * 1024;

    /* Compute checksum */
    bcb_compute_checksum(&bcb);

    ASSERT(bcb.bcb_checksum != 0,
           "Checksum is non-zero");

    ASSERT(bcb_verify_checksum(&bcb) == true,
           "Checksum verification passes");

    /* Modify data and verify checksum fails */
    bcb.mem_total_bytes = 128 * 1024 * 1024;
    ASSERT(bcb_verify_checksum(&bcb) == false,
           "Modified data fails checksum");

    /* Recompute and verify */
    bcb_compute_checksum(&bcb);
    ASSERT(bcb_verify_checksum(&bcb) == true,
           "Recomputed checksum passes");
}

void test_bcb_flags(void) {
    TEST("BCB Stage Flags");

    tbos_bcb_v1_t bcb;
    bcb_init(&bcb);

    bcb.stage_flags = BCB_FLAG_BIOS | BCB_FLAG_VGA_CONSOLE | BCB_FLAG_64BIT;

    ASSERT(BCB_HAS_FLAG(&bcb, BCB_FLAG_BIOS),
           "BIOS flag detected");

    ASSERT(BCB_HAS_FLAG(&bcb, BCB_FLAG_VGA_CONSOLE),
           "VGA console flag detected");

    ASSERT(BCB_HAS_FLAG(&bcb, BCB_FLAG_64BIT),
           "64-bit flag detected");

    ASSERT(!BCB_HAS_FLAG(&bcb, BCB_FLAG_HOSTED),
           "Hosted flag not present");

    ASSERT(!BCB_HAS_FLAG(&bcb, BCB_FLAG_UEFI),
           "UEFI flag not present");
}

void test_bcb_hal_selection(void) {
    TEST("BCB HAL Type Selection");

    tbos_bcb_v1_t bcb;

    /* Test BIOS */
    bcb_init(&bcb);
    bcb.stage_flags = BCB_FLAG_BIOS;
    ASSERT(bcb_get_hal_type(&bcb) == BCB_HAL_BARE_METAL_BIOS,
           "BIOS HAL selected");

    /* Test UEFI */
    bcb_init(&bcb);
    bcb.stage_flags = BCB_FLAG_UEFI;
    ASSERT(bcb_get_hal_type(&bcb) == BCB_HAL_BARE_METAL_UEFI,
           "UEFI HAL selected");

    /* Test Hosted */
    bcb_init(&bcb);
    bcb.stage_flags = BCB_FLAG_HOSTED;
    ASSERT(bcb_get_hal_type(&bcb) == BCB_HAL_HOSTED_LINUX,
           "Hosted HAL selected");

    /* Test Virtualized */
    bcb_init(&bcb);
    bcb.stage_flags = BCB_FLAG_VIRTUALIZED;
    ASSERT(bcb_get_hal_type(&bcb) == BCB_HAL_VIRTUALIZED,
           "Virtualized HAL selected");

    /* Test WASM */
    bcb_init(&bcb);
    bcb.stage_flags = BCB_FLAG_WASM;
    ASSERT(bcb_get_hal_type(&bcb) == BCB_HAL_WASM,
           "WASM HAL selected");

    /* Test priority: WASM > Hosted > Virtualized > UEFI > BIOS */
    bcb_init(&bcb);
    bcb.stage_flags = BCB_FLAG_BIOS | BCB_FLAG_HOSTED;
    ASSERT(bcb_get_hal_type(&bcb) == BCB_HAL_HOSTED_LINUX,
           "Hosted takes priority over BIOS");
}

void test_bcb_memory_format(void) {
    TEST("BCB Memory Formatting");

    char buf[16];

    /* Test GB */
    bcb_format_memory(4ULL * 1024 * 1024 * 1024, buf);
    ASSERT(buf[3] == 'G' && buf[4] == 'B',
           "GB formatting works");

    /* Test MB */
    bcb_format_memory(512 * 1024 * 1024, buf);
    ASSERT(buf[4] == 'M' && buf[5] == 'B',
           "MB formatting works");

    /* Test KB */
    bcb_format_memory(640 * 1024, buf);
    ASSERT(buf[4] == 'K' && buf[5] == 'B',
           "KB formatting works");
}

void test_bcb_macros(void) {
    TEST("BCB Macros");

    tbos_bcb_v1_t bcb;
    bcb_init(&bcb);
    bcb.stage_flags = BCB_FLAG_HOSTED;
    bcb.host_api_ptr = 0x12345678;

    ASSERT(BCB_IS_HOSTED(&bcb),
           "BCB_IS_HOSTED macro works");

    ASSERT(!BCB_IS_BIOS(&bcb),
           "BCB_IS_BIOS macro works (false case)");

    ASSERT(!BCB_IS_UEFI(&bcb),
           "BCB_IS_UEFI macro works (false case)");

    /* Test host API retrieval */
    tbos_host_api_t* api = BCB_GET_HOST_API(&bcb);
    ASSERT(api == (tbos_host_api_t*)0x12345678,
           "BCB_GET_HOST_API returns correct pointer");

    /* Test non-hosted */
    bcb.stage_flags = BCB_FLAG_BIOS;
    api = BCB_GET_HOST_API(&bcb);
    ASSERT(api == NULL,
           "BCB_GET_HOST_API returns NULL for non-hosted");
}

/* ========================================================================= */
/* MAIN                                                                       */
/* ========================================================================= */

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   Boot Capability Block (BCB) Unit Tests                     ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    test_bcb_structure_size();
    test_bcb_init();
    test_bcb_validation();
    test_bcb_checksum();
    test_bcb_flags();
    test_bcb_hal_selection();
    test_bcb_memory_format();
    test_bcb_macros();

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   Test Results                                               ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Tests Passed: %-3d                                           ║\n", tests_passed);
    printf("║ Tests Failed: %-3d                                           ║\n", tests_failed);
    printf("╠══════════════════════════════════════════════════════════════╣\n");

    if (tests_failed == 0) {
        printf("║ ✅ ALL TESTS PASSED                                         ║\n");
    } else {
        printf("║ ❌ SOME TESTS FAILED                                        ║\n");
    }

    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    return tests_failed == 0 ? 0 : 1;
}
