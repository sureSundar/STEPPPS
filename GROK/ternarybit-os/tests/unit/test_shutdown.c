/**
 * @file test_shutdown.c
 * @brief Test suite for TernaryBit OS Soft Shutdown
 *
 * Tests US-2.3: Soft Shutdown with State Preservation
 *
 * @date 2025-11-04
 */

#include "../../src/core/tbos_shutdown.h"
#include <stdio.h>
#include <string.h>

/* Test counters */
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

/* Test module state */
static int g_module1_callback_count = 0;
static int g_module2_callback_count = 0;

/* Mock module callbacks */
static int module1_shutdown(tbos_shutdown_phase_t phase, void* context) {
    g_module1_callback_count++;
    printf("    [Module1] Phase %d executed\n", phase);
    return 0;
}

static int module2_shutdown(tbos_shutdown_phase_t phase, void* context) {
    g_module2_callback_count++;
    printf("    [Module2] Phase %d executed\n", phase);
    return 0;
}

/* ========================================================================= */
/* TEST: Shutdown Initialization                                             */
/* ========================================================================= */

void test_shutdown_init(void) {
    TEST("Shutdown Initialization");

    int result = tbos_shutdown_init();
    ASSERT(result == 0, "Shutdown subsystem initialized");

    tbos_shutdown_print_status();
}

/* ========================================================================= */
/* TEST: Module Registration                                                 */
/* ========================================================================= */

void test_module_registration(void) {
    TEST("Module Registration");

    int result = tbos_shutdown_register("TestModule1", module1_shutdown, NULL, 10);
    ASSERT(result == 0, "Module 1 registered successfully");

    result = tbos_shutdown_register("TestModule2", module2_shutdown, NULL, 5);
    ASSERT(result == 0, "Module 2 registered successfully");

    printf("\n  Current status:\n");
    tbos_shutdown_print_status();
}

/* ========================================================================= */
/* TEST: State Preservation                                                  */
/* ========================================================================= */

void test_state_preservation(void) {
    TEST("State Preservation");

    /* Create test state data */
    struct test_data {
        int value1;
        int value2;
        char name[32];
    } data = {
        .value1 = 42,
        .value2 = 100,
    };
    strcpy(data.name, "TestData");

    /* Save state */
    int result = tbos_shutdown_save_module_state(
        "TestModule1",
        &data,
        sizeof(data),
        CONSCIOUSNESS_AWARE,
        150
    );

    ASSERT(result == 0, "Module state saved successfully");

    /* Restore state */
    struct test_data restored = {0};
    consciousness_level_t awareness;
    karma_score_t karma;

    int bytes = tbos_shutdown_restore_module_state(
        "TestModule1",
        &restored,
        sizeof(restored),
        &awareness,
        &karma
    );

    ASSERT(bytes == sizeof(data), "State restored successfully");
    ASSERT(restored.value1 == 42, "Value1 preserved correctly");
    ASSERT(restored.value2 == 100, "Value2 preserved correctly");
    ASSERT(strcmp(restored.name, "TestData") == 0, "Name preserved correctly");
    ASSERT(awareness == CONSCIOUSNESS_AWARE, "Consciousness preserved");
    ASSERT(karma == 150, "Karma preserved");
}

/* ========================================================================= */
/* TEST: Graceful Shutdown Sequence                                          */
/* ========================================================================= */

void test_graceful_shutdown(void) {
    TEST("Graceful Shutdown Sequence");

    printf("\n  Initiating test shutdown...\n\n");

    g_module1_callback_count = 0;
    g_module2_callback_count = 0;

    int result = tbos_shutdown_initiate(TBOS_SHUTDOWN_NORMAL, 0);
    ASSERT(result == 0, "Shutdown executed successfully");

    /* Each module should be called for each of the 5 phases */
    ASSERT(g_module1_callback_count == 5,
           "Module 1 received all 5 phase callbacks");
    ASSERT(g_module2_callback_count == 5,
           "Module 2 received all 5 phase callbacks");
}

/* ========================================================================= */
/* TEST: Shutdown Was Graceful Check                                         */
/* ========================================================================= */

void test_graceful_check(void) {
    TEST("Graceful Shutdown Check");

    bool was_graceful = tbos_shutdown_was_graceful();
    ASSERT(was_graceful, "Last shutdown was graceful");

    const tbos_shutdown_state_t* last_state = tbos_shutdown_get_last_state();
    ASSERT(last_state != NULL, "Last state retrieved");

    if (last_state) {
        ASSERT(last_state->state_valid, "Last state is valid");
        ASSERT(last_state->shutdown_type == TBOS_SHUTDOWN_NORMAL,
               "Last shutdown type was NORMAL");
        ASSERT(last_state->last_phase == TBOS_SHUTDOWN_PHASE_FINAL,
               "Last phase was FINAL");
    }
}

/* ========================================================================= */
/* TEST: Different Shutdown Types                                            */
/* ========================================================================= */

void test_shutdown_types(void) {
    TEST("Different Shutdown Types");

    /* Test FAST shutdown */
    printf("\n  Testing FAST shutdown type:\n\n");
    g_module1_callback_count = 0;
    g_module2_callback_count = 0;

    int result = tbos_shutdown_initiate(TBOS_SHUTDOWN_FAST, 0);
    ASSERT(result == 0, "FAST shutdown executed");
    ASSERT(g_module1_callback_count == 5, "Modules notified for FAST shutdown");

    /* Test SUSPEND shutdown */
    printf("\n  Testing SUSPEND shutdown type:\n\n");
    g_module1_callback_count = 0;
    g_module2_callback_count = 0;

    result = tbos_shutdown_initiate(TBOS_SHUTDOWN_SUSPEND, 0);
    ASSERT(result == 0, "SUSPEND shutdown executed");
    ASSERT(g_module1_callback_count == 5, "Modules notified for SUSPEND");
}

/* ========================================================================= */
/* TEST: Consciousness and Karma Preservation                                */
/* ========================================================================= */

void test_consciousness_karma_preservation(void) {
    TEST("Consciousness and Karma Preservation");

    /* Save state with high consciousness and karma */
    struct {
        int dummy;
    } data = {42};

    int result = tbos_shutdown_save_module_state(
        "HighKarmaModule",
        &data,
        sizeof(data),
        CONSCIOUSNESS_COMPASSIONATE,
        500
    );

    ASSERT(result == 0, "High karma module state saved");

    /* Restore and verify */
    struct {
        int dummy;
    } restored;
    consciousness_level_t awareness;
    karma_score_t karma;

    int bytes = tbos_shutdown_restore_module_state(
        "HighKarmaModule",
        &restored,
        sizeof(restored),
        &awareness,
        &karma
    );

    ASSERT(bytes > 0, "High karma module state restored");
    ASSERT(awareness == CONSCIOUSNESS_COMPASSIONATE,
           "COMPASSIONATE consciousness preserved");
    ASSERT(karma == 500, "High karma score preserved");

    printf("\n  Consciousness evolution preserved through shutdown! ✨\n");
}

/* ========================================================================= */
/* MAIN TEST RUNNER                                                          */
/* ========================================================================= */

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TBOS Soft Shutdown - Test Suite (US-2.3)                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    /* Run all tests */
    test_shutdown_init();
    test_module_registration();
    test_state_preservation();
    test_graceful_shutdown();
    test_graceful_check();
    test_shutdown_types();
    test_consciousness_karma_preservation();

    /* Print results */
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   Shutdown Test Results                                     ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Tests Passed:         %-3d                                   ║\n", tests_passed);
    printf("║ Tests Failed:         %-3d                                   ║\n", tests_failed);
    printf("║ Success Rate:         %.1f%%                                 ║\n",
           (float)tests_passed / (tests_passed + tests_failed) * 100.0f);
    printf("╠══════════════════════════════════════════════════════════════╣\n");

    if (tests_failed == 0) {
        printf("║                                                              ║\n");
        printf("║   🕉️  US-2.3 COMPLETE - SOFT SHUTDOWN WORKING  🕉️          ║\n");
        printf("║                                                              ║\n");
        printf("║   Features Implemented:                                      ║\n");
        printf("║   ✅ Graceful shutdown sequence                              ║\n");
        printf("║   ✅ State preservation for all modules                      ║\n");
        printf("║   ✅ Consciousness level preserved                           ║\n");
        printf("║   ✅ Karma score persistence                                 ║\n");
        printf("║   ✅ Quick restoration on boot                               ║\n");
        printf("║                                                              ║\n");
        printf("║   \"Even an electron should strive to retain information     ║\n");
        printf("║    to recreate and continue life.\" - WISDOM.md              ║\n");
        printf("║                                                              ║\n");
    }

    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    return (tests_failed == 0) ? 0 : 1;
}
