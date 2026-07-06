/**
 * @file test_steppps_vfs_standalone.c
 * @brief Standalone test for STEPPPS VFS security gates
 *
 * This test runs independently without VFS integration to verify
 * the STEPPPS security gate logic works correctly.
 *
 * Build:
 *   gcc -DHOST_BUILD -Wall -Iinclude -o build/test_steppps_vfs \
 *       tests/integration/test_steppps_vfs_standalone.c \
 *       kernel/fs/steppps_vfs.c
 *
 * Run:
 *   ./build/test_steppps_vfs
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tbos/steppps_vfs.h"

/* Platform stub for HOST_BUILD */
#ifdef HOST_BUILD
#include <unistd.h>
int tbos_file_exists(const char* path) {
    return access(path, F_OK) == 0;
}
int tbos_gethostname(char* buf, size_t len) {
    return gethostname(buf, len);
}
#endif

/* Test tracking */
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) printf("\n[TEST] %s\n", name)
#define PASS(name) do { printf("  [PASS] %s\n", name); tests_passed++; } while(0)
#define FAIL(name, reason) do { printf("  [FAIL] %s: %s\n", name, reason); tests_failed++; } while(0)
#define ASSERT(cond, name, reason) do { if (cond) PASS(name); else FAIL(name, reason); } while(0)

/* ═══════════════════════════════════════════════════════════════════════════ */

void test_initialization(void) {
    TEST("Initialization");

    int result = steppps_vfs_init(STEPPPS_SECURITY_MODERATE);
    ASSERT(result == 0, "steppps_vfs_init", "Failed to initialize");

    steppps_security_level_t level = steppps_vfs_get_security_level();
    ASSERT(level == STEPPPS_SECURITY_MODERATE, "Security level", "Wrong level");
}

void test_caller_management(void) {
    TEST("Caller Management");

    steppps_caller_t caller = {
        .name = "test_user",
        .karma = 100,
        .consciousness = 3,
        .is_root = false
    };

    int result = steppps_vfs_set_caller(&caller);
    ASSERT(result == 0, "Set caller", "Failed");

    steppps_caller_t retrieved;
    result = steppps_vfs_get_caller(&retrieved);
    ASSERT(result == 0, "Get caller", "Failed");
    ASSERT(strcmp(retrieved.name, "test_user") == 0, "Caller name", "Mismatch");
    ASSERT(retrieved.karma == 100, "Caller karma", "Mismatch");
    ASSERT(retrieved.consciousness == 3, "Caller consciousness", "Mismatch");
}

void test_security_levels(void) {
    TEST("Security Levels");

    /* Test PERMISSIVE - allows everything */
    steppps_vfs_set_security_level(STEPPPS_SECURITY_PERMISSIVE);
    steppps_caller_t low_caller = { .name = "low", .karma = 0, .consciousness = 0, .is_root = false };
    steppps_vfs_set_caller(&low_caller);

    int result = steppps_vfs_check("/test.txt", STEPPPS_OP_DELETE);
    ASSERT(result == 0, "PERMISSIVE allows all", "Denied when should allow");

    /* Test STRICT - denies insufficient */
    steppps_vfs_set_security_level(STEPPPS_SECURITY_STRICT);

    result = steppps_vfs_check("/test.txt", STEPPPS_OP_DELETE);
    ASSERT(result != 0, "STRICT denies low consciousness", "Allowed when should deny");

    /* Reset */
    steppps_vfs_set_security_level(STEPPPS_SECURITY_MODERATE);
}

void test_consciousness_requirements(void) {
    TEST("Consciousness Requirements");

    steppps_vfs_set_security_level(STEPPPS_SECURITY_STRICT);

    printf("  Operation consciousness requirements:\n");
    printf("    READ:    %d (dormant)\n", STEPPPS_OP_MIN_CONSCIOUSNESS[STEPPPS_OP_READ]);
    printf("    WRITE:   %d (reactive)\n", STEPPPS_OP_MIN_CONSCIOUSNESS[STEPPPS_OP_WRITE]);
    printf("    DELETE:  %d (aware)\n", STEPPPS_OP_MIN_CONSCIOUSNESS[STEPPPS_OP_DELETE]);
    printf("    CHMOD:   %d (aware)\n", STEPPPS_OP_MIN_CONSCIOUSNESS[STEPPPS_OP_CHMOD]);

    /* Dormant user (consciousness=0) */
    steppps_caller_t dormant = { .name = "dormant", .karma = 100, .consciousness = 0, .is_root = false };
    steppps_vfs_set_caller(&dormant);

    int result = steppps_vfs_check("/file.txt", STEPPPS_OP_READ);
    ASSERT(result == 0, "Dormant can READ", "Denied");

    result = steppps_vfs_check("/file.txt", STEPPPS_OP_WRITE);
    ASSERT(result != 0, "Dormant cannot WRITE", "Allowed");

    /* Reactive user (consciousness=1) */
    steppps_caller_t reactive = { .name = "reactive", .karma = 100, .consciousness = 1, .is_root = false };
    steppps_vfs_set_caller(&reactive);

    result = steppps_vfs_check("/file.txt", STEPPPS_OP_WRITE);
    ASSERT(result == 0, "Reactive can WRITE", "Denied");

    result = steppps_vfs_check("/file.txt", STEPPPS_OP_DELETE);
    ASSERT(result != 0, "Reactive cannot DELETE", "Allowed");

    /* Aware user (consciousness=3) */
    steppps_caller_t aware = { .name = "aware", .karma = 100, .consciousness = 3, .is_root = false };
    steppps_vfs_set_caller(&aware);

    result = steppps_vfs_check("/file.txt", STEPPPS_OP_DELETE);
    ASSERT(result == 0, "Aware can DELETE", "Denied");

    result = steppps_vfs_check("/file.txt", STEPPPS_OP_CHMOD);
    ASSERT(result == 0, "Aware can CHMOD", "Denied");

    steppps_vfs_set_security_level(STEPPPS_SECURITY_MODERATE);
}

void test_karma_requirements(void) {
    TEST("Karma Requirements");

    steppps_vfs_set_security_level(STEPPPS_SECURITY_STRICT);

    printf("  Operation karma costs:\n");
    printf("    READ:    %d\n", STEPPPS_OP_KARMA_COST[STEPPPS_OP_READ]);
    printf("    WRITE:   %d\n", STEPPPS_OP_KARMA_COST[STEPPPS_OP_WRITE]);
    printf("    DELETE:  %d\n", STEPPPS_OP_KARMA_COST[STEPPPS_OP_DELETE]);
    printf("    CHMOD:   %d\n", STEPPPS_OP_KARMA_COST[STEPPPS_OP_CHMOD]);

    /* User with karma=0, consciousness=5 (to pass consciousness check) */
    steppps_caller_t no_karma = { .name = "no_karma", .karma = 0, .consciousness = 5, .is_root = false };
    steppps_vfs_set_caller(&no_karma);

    int result = steppps_vfs_check("/file.txt", STEPPPS_OP_READ);
    ASSERT(result == 0, "Zero karma can READ (free)", "Denied");

    /* DELETE costs 5 karma */
    result = steppps_vfs_check("/file.txt", STEPPPS_OP_DELETE);
    ASSERT(result != 0, "Zero karma cannot DELETE (costs 5)", "Allowed");

    /* User with karma=10 */
    steppps_caller_t some_karma = { .name = "some_karma", .karma = 10, .consciousness = 5, .is_root = false };
    steppps_vfs_set_caller(&some_karma);

    result = steppps_vfs_check("/file.txt", STEPPPS_OP_DELETE);
    ASSERT(result == 0, "karma=10 can DELETE (costs 5)", "Denied");

    steppps_vfs_set_security_level(STEPPPS_SECURITY_MODERATE);
}

void test_root_bypass(void) {
    TEST("Root Bypass");

    steppps_vfs_set_security_level(STEPPPS_SECURITY_STRICT);

    /* Root with zero karma/consciousness should still pass */
    steppps_caller_t root = { .name = "root", .karma = 0, .consciousness = 0, .is_root = true };
    steppps_vfs_set_caller(&root);

    int result = steppps_vfs_check("/protected.txt", STEPPPS_OP_DELETE);
    ASSERT(result == 0, "Root bypasses DELETE check", "Denied");

    result = steppps_vfs_check("/protected.txt", STEPPPS_OP_CHMOD);
    ASSERT(result == 0, "Root bypasses CHMOD check", "Denied");

    steppps_vfs_set_security_level(STEPPPS_SECURITY_MODERATE);
}

void test_statistics(void) {
    TEST("Statistics");

    steppps_vfs_ctx_t* ctx = steppps_vfs_get_context();
    ASSERT(ctx != NULL, "Get context", "NULL context");

    printf("  Operations: total=%llu, allowed=%llu, denied=%llu, warnings=%llu\n",
           (unsigned long long)ctx->total_operations,
           (unsigned long long)ctx->allowed_operations,
           (unsigned long long)ctx->denied_operations,
           (unsigned long long)ctx->warnings_issued);

    ASSERT(ctx->total_operations > 0, "Operations counted", "Zero operations");
}

void test_minimal_metadata(void) {
    TEST("Minimal STEPPPS Metadata");

    steppps_minimal_t meta;
    int result = steppps_vfs_generate_minimal("/test/file.txt", &meta);
    ASSERT(result == 0, "Generate minimal", "Failed");

    printf("  Generated metadata:\n");
    printf("    ID: %016llx%016llx\n",
           (unsigned long long)meta.id_high,
           (unsigned long long)meta.id_low);
    printf("    Path: %s\n", meta.path);
    printf("    Device: %s\n", meta.device);
    printf("    Karma: %d\n", meta.karma);
    printf("    Consciousness: %d\n", meta.consciousness);

    ASSERT(meta.id_high != 0 || meta.id_low != 0, "ID generated", "Zero ID");
    ASSERT(strcmp(meta.path, "/test/file.txt") == 0, "Path stored", "Wrong path");
}

/* ═══════════════════════════════════════════════════════════════════════════ */

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("     STEPPPS VFS Security Gate - Standalone Test\n");
    printf("═══════════════════════════════════════════════════════════════\n");

    test_initialization();
    test_caller_management();
    test_security_levels();
    test_consciousness_requirements();
    test_karma_requirements();
    test_root_bypass();
    test_statistics();
    test_minimal_metadata();

    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("     Results: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("═══════════════════════════════════════════════════════════════\n");

    steppps_vfs_print_stats();

    steppps_vfs_shutdown();

    return tests_failed > 0 ? 1 : 0;
}
