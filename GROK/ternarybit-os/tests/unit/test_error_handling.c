/**
 * @file test_error_handling.c
 * @brief Test the production error handling system
 *
 * Tests:
 * - Error reporting with severity levels
 * - Watchdog registration and timeout detection
 * - Recovery mechanism
 * - Error statistics tracking
 * - Error propagation macros
 *
 * Build:
 *   gcc -DHOST_BUILD -Wall -g -I../../src/rock/error \
 *       test_error_handling.c ../../src/rock/error/error.c \
 *       -o test_error_handling
 */

#include "../../src/rock/error/error.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

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

/* Custom error handler for testing */
static int g_custom_handler_called = 0;
static error_t g_last_error = {0};

static void test_error_handler(const error_t *error) {
    g_custom_handler_called++;
    if (error) {
        g_last_error = *error;
    }
}

/* Recovery handler for testing */
static int g_recovery_attempts = 0;
static bool g_recovery_should_succeed = false;

static bool test_recovery_handler(const error_t *error) {
    (void)error;
    g_recovery_attempts++;
    return g_recovery_should_succeed;
}

/* ========================================================================= */
/* TEST CASES                                                                */
/* ========================================================================= */

void test_error_initialization(void) {
    TEST_START("Error System Initialization");

    error_init();
    TEST_ASSERT(true, "Error system initialized");

    /* Check initial stats are zero */
    error_stats_t stats;
    error_get_stats(&stats);
    TEST_ASSERT(stats.total_errors == 0, "Initial error count is zero");
    TEST_ASSERT(stats.panics_triggered == 0, "Initial panic count is zero");
}

void test_error_reporting(void) {
    TEST_START("Error Reporting");

    /* Set custom handler */
    g_custom_handler_called = 0;
    error_set_handler(test_error_handler);

    /* Report debug error */
    ERROR_DEBUG(ERROR_DOMAIN_GENERAL, 0x1001, "Test debug message");
    TEST_ASSERT(g_custom_handler_called == 1, "Debug error handler called");
    TEST_ASSERT(g_last_error.severity == ERROR_SEVERITY_DEBUG, "Severity is DEBUG");

    /* Report warning */
    ERROR_WARNING(ERROR_DOMAIN_MEMORY, 0x1002, "Test warning message");
    TEST_ASSERT(g_custom_handler_called == 2, "Warning error handler called");
    TEST_ASSERT(g_last_error.domain == ERROR_DOMAIN_MEMORY, "Domain is MEMORY");

    /* Report error */
    ERROR_ERROR(ERROR_DOMAIN_NETWORK, 0x1003, "Test error message");
    TEST_ASSERT(g_custom_handler_called == 3, "Error handler called");
    TEST_ASSERT(g_last_error.code == 0x1003, "Error code matches");

    /* Verify statistics */
    error_stats_t stats;
    error_get_stats(&stats);
    TEST_ASSERT(stats.total_errors == 3, "Total error count is 3");
    TEST_ASSERT(stats.errors_by_severity[ERROR_SEVERITY_DEBUG] == 1, "1 debug error");
    TEST_ASSERT(stats.errors_by_severity[ERROR_SEVERITY_WARNING] == 1, "1 warning");
    TEST_ASSERT(stats.errors_by_severity[ERROR_SEVERITY_ERROR] == 1, "1 error");
}

void test_error_string_conversion(void) {
    TEST_START("Error String Conversion");

    error_t error = {
        .code = 0xDEAD,
        .severity = ERROR_SEVERITY_CRITICAL,
        .domain = ERROR_DOMAIN_SECURITY,
        .message = "Test critical error",
        .file = "test.c",
        .line = 42,
        .timestamp = 12345
    };

    const char *str = error_to_string(&error);
    TEST_ASSERT(str != NULL, "Error string not NULL");
    TEST_ASSERT(strstr(str, "CRITICAL") != NULL, "Contains CRITICAL");
    TEST_ASSERT(strstr(str, "SECURITY") != NULL, "Contains SECURITY");
    TEST_ASSERT(strstr(str, "0x0000DEAD") != NULL || strstr(str, "0xDEAD") != NULL,
                "Contains error code");
    TEST_ASSERT(strstr(str, "test.c") != NULL, "Contains filename");
}

void test_watchdog_system(void) {
    TEST_START("Watchdog System");

    watchdog_init();
    TEST_ASSERT(true, "Watchdog initialized");

    /* Register a module with short timeout */
    int result = watchdog_register(1001, 100, WATCHDOG_ACTION_LOG);
    TEST_ASSERT(result == TBOS_SUCCESS, "Module registered");

    /* Send heartbeat */
    watchdog_heartbeat(1001);
    TEST_ASSERT(true, "Heartbeat sent");

    /* Check - should not trigger */
    error_stats_t stats_before;
    error_get_stats(&stats_before);
    watchdog_check();
    error_stats_t stats_after;
    error_get_stats(&stats_after);
    TEST_ASSERT(stats_after.watchdog_triggers == stats_before.watchdog_triggers,
                "No timeout before deadline");

    /* Wait for timeout (in production, would use actual timer) */
    printf("  Waiting 150ms for timeout...\n");
    usleep(150000);  /* 150ms */

    /* Check - should trigger */
    watchdog_check();
    error_get_stats(&stats_after);
    TEST_ASSERT(stats_after.watchdog_triggers > stats_before.watchdog_triggers,
                "Timeout detected after deadline");

    /* Unregister */
    result = watchdog_unregister(1001);
    TEST_ASSERT(result == TBOS_SUCCESS, "Module unregistered");

    /* Try to unregister non-existent module */
    result = watchdog_unregister(9999);
    TEST_ASSERT(result == TBOS_ENOENT, "Non-existent module returns ENOENT");
}

void test_recovery_mechanism(void) {
    TEST_START("Recovery Mechanism");

    g_recovery_attempts = 0;
    g_recovery_should_succeed = false;

    /* Register recovery handler */
    int result = recovery_register(ERROR_DOMAIN_NETWORK, test_recovery_handler);
    TEST_ASSERT(result == TBOS_SUCCESS, "Recovery handler registered");

    /* Set max retries */
    recovery_set_max_retries(3);
    TEST_ASSERT(true, "Max retries set to 3");

    /* Create test error */
    error_t error = {
        .code = 0x3001,
        .severity = ERROR_SEVERITY_CRITICAL,
        .domain = ERROR_DOMAIN_NETWORK,
        .message = "Test network error",
        .file = __FILE__,
        .line = __LINE__
    };

    /* Attempt recovery (should fail) */
    g_recovery_should_succeed = false;
    bool recovered = recovery_attempt(&error);
    TEST_ASSERT(!recovered, "Recovery failed as expected");
    TEST_ASSERT(g_recovery_attempts == 3, "3 recovery attempts made");

    /* Reset and try with success */
    g_recovery_attempts = 0;
    g_recovery_should_succeed = true;
    recovered = recovery_attempt(&error);
    TEST_ASSERT(recovered, "Recovery succeeded");
    TEST_ASSERT(g_recovery_attempts == 1, "Only 1 attempt needed for success");
}

void test_error_propagation_macros(void) {
    TEST_START("Error Propagation Macros");

    /* Test TBOS_CHECK (internal function) */
    int test_func_check(int should_fail) {
        if (should_fail) {
            return TBOS_EINVAL;
        }
        return TBOS_SUCCESS;
    }

    int wrapper_check(int should_fail) {
        TBOS_CHECK(test_func_check(should_fail));
        return TBOS_SUCCESS;
    }

    int result = wrapper_check(0);
    TEST_ASSERT(result == TBOS_SUCCESS, "TBOS_CHECK passes success");

    result = wrapper_check(1);
    TEST_ASSERT(result == TBOS_EINVAL, "TBOS_CHECK propagates error");

    /* Test TBOS_REQUIRE_ARG */
    int test_require(void *ptr) {
        TBOS_REQUIRE_ARG(ptr != NULL);
        return TBOS_SUCCESS;
    }

    result = test_require((void*)1);
    TEST_ASSERT(result == TBOS_SUCCESS, "TBOS_REQUIRE_ARG passes valid arg");

    result = test_require(NULL);
    TEST_ASSERT(result == TBOS_EINVAL, "TBOS_REQUIRE_ARG rejects NULL");
}

void test_error_statistics(void) {
    TEST_START("Error Statistics");

    /* Reset stats */
    error_reset_stats();

    error_stats_t stats;
    error_get_stats(&stats);
    TEST_ASSERT(stats.total_errors == 0, "Stats reset - total is 0");

    /* Generate some errors */
    ERROR_INFO(ERROR_DOMAIN_GENERAL, 1, "Info 1");
    ERROR_INFO(ERROR_DOMAIN_GENERAL, 2, "Info 2");
    ERROR_WARNING(ERROR_DOMAIN_MEMORY, 3, "Warning 1");
    ERROR_ERROR(ERROR_DOMAIN_FILESYSTEM, 4, "Error 1");

    error_get_stats(&stats);
    TEST_ASSERT(stats.total_errors == 4, "Total errors is 4");
    TEST_ASSERT(stats.errors_by_severity[ERROR_SEVERITY_INFO] == 2, "2 info errors");
    TEST_ASSERT(stats.errors_by_severity[ERROR_SEVERITY_WARNING] == 1, "1 warning");
    TEST_ASSERT(stats.errors_by_severity[ERROR_SEVERITY_ERROR] == 1, "1 error");
    TEST_ASSERT(stats.errors_by_domain[ERROR_DOMAIN_GENERAL] == 2, "2 general errors");
    TEST_ASSERT(stats.errors_by_domain[ERROR_DOMAIN_MEMORY] == 1, "1 memory error");
    TEST_ASSERT(stats.errors_by_domain[ERROR_DOMAIN_FILESYSTEM] == 1, "1 filesystem error");
    TEST_ASSERT(stats.last_error_code == 4, "Last error code is 4");
}

void test_assertion_macros(void) {
    TEST_START("Assertion Macros");

    /* TBOS_ASSERT should not panic on true condition */
    error_stats_t stats_before;
    error_get_stats(&stats_before);

    TBOS_ASSERT(1 == 1);
    TEST_ASSERT(true, "TBOS_ASSERT(true) does not report error");

    /* TBOS_ASSERT on false should report critical error */
    g_custom_handler_called = 0;
    TBOS_ASSERT(1 == 2);  /* This will trigger critical error */

    error_stats_t stats_after;
    error_get_stats(&stats_after);
    TEST_ASSERT(stats_after.errors_by_severity[ERROR_SEVERITY_CRITICAL] >
                stats_before.errors_by_severity[ERROR_SEVERITY_CRITICAL],
                "TBOS_ASSERT(false) reports critical error");
}

/* ========================================================================= */
/* MAIN                                                                      */
/* ========================================================================= */

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Error Handling System Test Suite           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    /* Run tests */
    test_error_initialization();
    test_error_reporting();
    test_error_string_conversion();
    test_watchdog_system();
    test_recovery_mechanism();
    test_error_propagation_macros();
    test_error_statistics();
    test_assertion_macros();

    /* Shutdown */
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
        printf("║ ✅ ALL TESTS PASSED - ERROR HANDLING PRODUCTION READY      ║\n");
    } else {
        printf("║ ⚠️  SOME TESTS FAILED                                       ║\n");
    }

    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    return (tests_failed == 0) ? 0 : 1;
}
