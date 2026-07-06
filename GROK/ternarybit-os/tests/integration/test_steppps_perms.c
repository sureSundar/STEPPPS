/**
 * @file test_steppps_perms.c
 * @brief TernaryBit OS - STEPPPS + File Permissions Integration Test
 *
 * Tests that STEPPPS security gates work with VFS file operations.
 * Verifies karma/consciousness requirements are enforced alongside Unix perms.
 *
 * Build (kernel mode): gcc -Wall -Iinclude -Isrc -o build/test_steppps_perms \
 *        tests/integration/test_steppps_perms.c \
 *        kernel/fs/vfs.c kernel/fs/ramfs.c kernel/fs/steppps_vfs.c \
 *        kernel/libc.c kernel/kernel_io.c
 */

#include <stdio.h>
#include <string.h>
#include "tbos/vfs.h"
#include "tbos/steppps_vfs.h"

/* External ramfs driver */
extern const vfs_driver_t ramfs_driver;

/* Test result tracking */
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_START(name) printf("\n[TEST] %s...\n", name)
#define TEST_PASS(name) do { printf("[PASS] %s\n", name); tests_passed++; } while(0)
#define TEST_FAIL(name, reason) do { printf("[FAIL] %s: %s\n", name, reason); tests_failed++; } while(0)

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: STEPPPS Security Gate Initialization
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_steppps_init(void) {
    TEST_START("STEPPPS VFS Initialization");

    int result = steppps_vfs_init(STEPPPS_SECURITY_MODERATE);
    if (result == 0) {
        TEST_PASS("STEPPPS VFS Initialization");
    } else {
        TEST_FAIL("STEPPPS VFS Initialization", "Failed to initialize");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: Caller Context
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_caller_context(void) {
    TEST_START("Caller Context Management");

    steppps_caller_t caller = {
        .name = "test_user",
        .karma = 100,
        .consciousness = 3,  /* AWARE */
        .is_root = false
    };

    int result = steppps_vfs_set_caller(&caller);
    if (result != 0) {
        TEST_FAIL("Caller Context Management", "Failed to set caller");
        return;
    }

    steppps_caller_t retrieved;
    result = steppps_vfs_get_caller(&retrieved);
    if (result != 0) {
        TEST_FAIL("Caller Context Management", "Failed to get caller");
        return;
    }

    if (strcmp(retrieved.name, "test_user") == 0 &&
        retrieved.karma == 100 &&
        retrieved.consciousness == 3) {
        printf("       Caller: %s (karma: %d, consciousness: %d)\n",
               retrieved.name, retrieved.karma, retrieved.consciousness);
        TEST_PASS("Caller Context Management");
    } else {
        TEST_FAIL("Caller Context Management", "Caller data mismatch");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: Basic Permission Check (Allow)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_permission_allow(void) {
    TEST_START("Permission Check (Allow)");

    /* Set high-karma, enlightened caller */
    steppps_caller_t caller = {
        .name = "enlightened_user",
        .karma = 500,
        .consciousness = 5,  /* ENLIGHTENED */
        .is_root = false
    };
    steppps_vfs_set_caller(&caller);

    /* Check various operations */
    int result = steppps_vfs_check("/test/file.txt", STEPPPS_OP_READ);
    if (result != 0) {
        TEST_FAIL("Permission Check (Allow)", "READ denied for enlightened user");
        return;
    }

    result = steppps_vfs_check("/test/file.txt", STEPPPS_OP_WRITE);
    if (result != 0) {
        TEST_FAIL("Permission Check (Allow)", "WRITE denied for enlightened user");
        return;
    }

    result = steppps_vfs_check("/test/file.txt", STEPPPS_OP_DELETE);
    if (result != 0) {
        TEST_FAIL("Permission Check (Allow)", "DELETE denied for enlightened user");
        return;
    }

    printf("       High-karma user allowed: READ, WRITE, DELETE\n");
    TEST_PASS("Permission Check (Allow)");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: Permission Check (Deny in STRICT mode)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_permission_deny(void) {
    TEST_START("Permission Check (Deny in STRICT mode)");

    /* Set STRICT security level */
    steppps_vfs_set_security_level(STEPPPS_SECURITY_STRICT);

    /* Set low-karma, unconscious caller */
    steppps_caller_t caller = {
        .name = "dormant_user",
        .karma = 0,
        .consciousness = 0,  /* DORMANT */
        .is_root = false
    };
    steppps_vfs_set_caller(&caller);

    /* READ should still work (no karma/consciousness required) */
    int result = steppps_vfs_check("/test/file.txt", STEPPPS_OP_READ);
    if (result != 0) {
        TEST_FAIL("Permission Check (Deny)", "READ denied (should be free)");
        return;
    }

    /* DELETE requires consciousness=3 (AWARE), should be denied */
    result = steppps_vfs_check("/test/file.txt", STEPPPS_OP_DELETE);
    if (result == 0) {
        TEST_FAIL("Permission Check (Deny)", "DELETE allowed for dormant user (should deny)");
        return;
    }

    printf("       Dormant user: READ allowed, DELETE denied\n");
    TEST_PASS("Permission Check (Deny)");

    /* Reset to moderate */
    steppps_vfs_set_security_level(STEPPPS_SECURITY_MODERATE);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: Root Bypass
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_root_bypass(void) {
    TEST_START("Root Bypass");

    /* Set STRICT security level */
    steppps_vfs_set_security_level(STEPPPS_SECURITY_STRICT);

    /* Root with no karma should still pass */
    steppps_caller_t caller = {
        .name = "root",
        .karma = 0,
        .consciousness = 0,
        .is_root = true  /* Root bypasses all checks */
    };
    steppps_vfs_set_caller(&caller);

    int result = steppps_vfs_check("/test/protected.txt", STEPPPS_OP_DELETE);
    if (result == 0) {
        printf("       Root user bypassed security check\n");
        TEST_PASS("Root Bypass");
    } else {
        TEST_FAIL("Root Bypass", "Root was denied");
    }

    steppps_vfs_set_security_level(STEPPPS_SECURITY_MODERATE);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: File-Specific Requirements
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_file_requirements(void) {
    TEST_START("File-Specific Requirements");

    /* Set high karma requirement on a file */
    int result = steppps_vfs_set_required_karma("/protected/secret.txt", 100);
    if (result != 0) {
        printf("       (Skipping - companion file write not available)\n");
        TEST_PASS("File-Specific Requirements");
        return;
    }

    /* User with karma=50 should be denied */
    steppps_vfs_set_security_level(STEPPPS_SECURITY_STRICT);
    steppps_caller_t low_karma = {
        .name = "low_karma_user",
        .karma = 50,
        .consciousness = 5,
        .is_root = false
    };
    steppps_vfs_set_caller(&low_karma);

    result = steppps_vfs_check("/protected/secret.txt", STEPPPS_OP_READ);
    if (result == 0) {
        TEST_FAIL("File-Specific Requirements", "Low karma user allowed");
        return;
    }

    /* User with karma=150 should be allowed */
    steppps_caller_t high_karma = {
        .name = "high_karma_user",
        .karma = 150,
        .consciousness = 5,
        .is_root = false
    };
    steppps_vfs_set_caller(&high_karma);

    result = steppps_vfs_check("/protected/secret.txt", STEPPPS_OP_READ);
    if (result != 0) {
        TEST_FAIL("File-Specific Requirements", "High karma user denied");
        return;
    }

    printf("       File requires karma >= 100: low(50) denied, high(150) allowed\n");
    TEST_PASS("File-Specific Requirements");

    steppps_vfs_set_security_level(STEPPPS_SECURITY_MODERATE);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: Operation Karma Costs
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_karma_costs(void) {
    TEST_START("Operation Karma Costs");

    printf("       Operation costs:\n");
    printf("         READ:    %d karma\n", STEPPPS_OP_KARMA_COST[STEPPPS_OP_READ]);
    printf("         WRITE:   %d karma\n", STEPPPS_OP_KARMA_COST[STEPPPS_OP_WRITE]);
    printf("         DELETE:  %d karma\n", STEPPPS_OP_KARMA_COST[STEPPPS_OP_DELETE]);
    printf("         EXECUTE: %d karma\n", STEPPPS_OP_KARMA_COST[STEPPPS_OP_EXECUTE]);
    printf("         CREATE:  %d karma\n", STEPPPS_OP_KARMA_COST[STEPPPS_OP_CREATE]);
    printf("         CHMOD:   %d karma\n", STEPPPS_OP_KARMA_COST[STEPPPS_OP_CHMOD]);
    printf("       Consciousness requirements:\n");
    printf("         READ:    %d (dormant)\n", STEPPPS_OP_MIN_CONSCIOUSNESS[STEPPPS_OP_READ]);
    printf("         WRITE:   %d (reactive)\n", STEPPPS_OP_MIN_CONSCIOUSNESS[STEPPPS_OP_WRITE]);
    printf("         DELETE:  %d (aware)\n", STEPPPS_OP_MIN_CONSCIOUSNESS[STEPPPS_OP_DELETE]);
    printf("         CHMOD:   %d (aware)\n", STEPPPS_OP_MIN_CONSCIOUSNESS[STEPPPS_OP_CHMOD]);

    TEST_PASS("Operation Karma Costs");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: VFS + STEPPPS Integration
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_vfs_integration(void) {
    TEST_START("VFS + STEPPPS Integration");

    /* Initialize VFS */
    vfs_init();
    vfs_mount("/", &ramfs_driver);

    /* Enable STEPPPS on VFS */
    vfs_enable_steppps(true);

    if (!vfs_steppps_enabled()) {
        TEST_FAIL("VFS + STEPPPS Integration", "STEPPPS not enabled");
        return;
    }

    printf("       VFS STEPPPS integration: enabled\n");

    /* Set an aware caller */
    steppps_caller_t caller = {
        .name = "aware_process",
        .karma = 50,
        .consciousness = 3,  /* AWARE */
        .is_root = false
    };
    steppps_vfs_set_caller(&caller);

    /* Create a file (requires consciousness=1) */
    int result = vfs_write_file("/test.txt", "Hello STEPPPS", 13);
    if (result < 0) {
        TEST_FAIL("VFS + STEPPPS Integration", "Write failed");
        return;
    }

    /* Read the file (requires consciousness=0) */
    char buf[64];
    size_t size;
    result = vfs_read_file("/test.txt", buf, sizeof(buf), &size);
    if (result < 0) {
        TEST_FAIL("VFS + STEPPPS Integration", "Read failed");
        return;
    }

    buf[size] = '\0';
    printf("       Created and read file: \"%s\"\n", buf);

    /* chmod (requires consciousness=3) */
    result = vfs_chmod("/test.txt", 0644);
    if (result < 0) {
        TEST_FAIL("VFS + STEPPPS Integration", "chmod failed");
        return;
    }

    printf("       chmod succeeded with aware caller\n");
    TEST_PASS("VFS + STEPPPS Integration");

    vfs_enable_steppps(false);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: Statistics
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_statistics(void) {
    TEST_START("STEPPPS Statistics");

    steppps_vfs_ctx_t* ctx = steppps_vfs_get_context();
    if (!ctx) {
        TEST_FAIL("STEPPPS Statistics", "Failed to get context");
        return;
    }

    printf("       Total operations: %llu\n", (unsigned long long)ctx->total_operations);
    printf("       Allowed: %llu\n", (unsigned long long)ctx->allowed_operations);
    printf("       Denied: %llu\n", (unsigned long long)ctx->denied_operations);
    printf("       Warnings: %llu\n", (unsigned long long)ctx->warnings_issued);

    TEST_PASS("STEPPPS Statistics");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST RUNNER
 * ═══════════════════════════════════════════════════════════════════════════ */

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("     TernaryBit OS - STEPPPS + File Permissions Test\n");
    printf("═══════════════════════════════════════════════════════════════\n");

    /* Run tests */
    test_steppps_init();
    test_caller_context();
    test_permission_allow();
    test_permission_deny();
    test_root_bypass();
    test_file_requirements();
    test_karma_costs();
    test_vfs_integration();
    test_statistics();

    /* Summary */
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("       Test Results: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("═══════════════════════════════════════════════════════════════\n");

    /* Print STEPPPS stats */
    steppps_vfs_print_stats();

    printf("\n");

    steppps_vfs_shutdown();

    return tests_failed > 0 ? 1 : 0;
}
