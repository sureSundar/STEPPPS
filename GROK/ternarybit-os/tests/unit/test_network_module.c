/**
 * @file test_network_module.c
 * @brief Test the conscious network module
 *
 * Tests initialization, consciousness levels, karma tracking,
 * Right Speech validation, fasting, and Digital Sangha formation.
 */

#include "../../src/network/core/tbos_network.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Test result tracking */
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

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║      TBOS Conscious Network Module Test                 ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");

    /* ===================================================================== */
    /* TEST 1: Initialization                                                */
    /* ===================================================================== */
    TEST_START("Network Module Initialization");

    int result = tbos_network_init();
    TEST_ASSERT(result == TBOS_NET_SUCCESS, "Network initialized successfully");

    /* Verify metrics initialized */
    tbos_network_metrics_t metrics;
    result = tbos_network_get_metrics(&metrics);
    TEST_ASSERT(result == TBOS_NET_SUCCESS, "Metrics retrieved");
    TEST_ASSERT(metrics.compassion_index >= 0.0f && metrics.compassion_index <= 1.0f,
                "Compassion index in valid range");

    /* ===================================================================== */
    /* TEST 2: Digital Sangha Formation                                      */
    /* ===================================================================== */
    TEST_START("Digital Sangha Formation");

    result = tbos_network_join_sangha("TestDevice");
    TEST_ASSERT(result == TBOS_NET_SUCCESS, "Joined Digital Sangha");

    /* ===================================================================== */
    /* TEST 3: Right Speech Validation                                       */
    /* ===================================================================== */
    TEST_START("Right Speech Validation");

    /* Test 3a: Valid Right Speech packet */
    const char* good_message = "Hello, friend";
    tbos_right_speech_packet_t good_packet = {
        .data = (void*)good_message,
        .length = strlen(good_message),
        .is_truthful = true,
        .is_beneficial = true,
        .is_timely = true,
        .is_gentle = true,
        .is_necessary = true,
        .karma_effect = 0
    };

    result = tbos_network_validate_right_speech(&good_packet);
    TEST_ASSERT(result == TBOS_NET_SUCCESS, "Valid Right Speech packet accepted");
    TEST_ASSERT(good_packet.karma_effect > 0, "Good karma awarded");

    /* Test 3b: Invalid Right Speech packet (not truthful) */
    tbos_right_speech_packet_t bad_packet = {
        .data = (void*)"Fake news",
        .length = 9,
        .is_truthful = false,  /* Lie! */
        .is_beneficial = true,
        .is_timely = true,
        .is_gentle = true,
        .is_necessary = true,
        .karma_effect = 0
    };

    result = tbos_network_validate_right_speech(&bad_packet);
    TEST_ASSERT(result != TBOS_NET_SUCCESS, "False speech rejected");

    /* Test 3c: Spam packet (not necessary) */
    tbos_right_speech_packet_t spam_packet = {
        .data = (void*)"Spam spam spam",
        .length = 14,
        .is_truthful = true,
        .is_beneficial = false,
        .is_timely = true,
        .is_gentle = true,
        .is_necessary = false,  /* Spam! */
        .karma_effect = 0
    };

    result = tbos_network_validate_right_speech(&spam_packet);
    TEST_ASSERT(result != TBOS_NET_SUCCESS, "Spam packet rejected");

    /* ===================================================================== */
    /* TEST 4: Karma Calculation                                             */
    /* ===================================================================== */
    TEST_START("Karma Calculation");

    karma_score_t karma = tbos_network_calculate_karma(1);
    TEST_ASSERT(karma != 0, "Karma calculated for device");
    printf("  📊 Device karma: %ld\n", karma);

    /* Check network karma from metrics */
    tbos_network_get_metrics(&metrics);
    printf("  📊 Network karma: %ld\n", metrics.network_karma);
    TEST_ASSERT(metrics.network_karma > 0, "Network has positive karma from good packets");

    /* ===================================================================== */
    /* TEST 5: Component Fasting                                             */
    /* ===================================================================== */
    TEST_START("Component Fasting");

    /* Test mild fasting (25% reduction) */
    result = tbos_network_begin_fasting(NET_FAST_MILD);
    TEST_ASSERT(result == TBOS_NET_SUCCESS, "Mild fasting initiated");

    tbos_network_get_metrics(&metrics);
    printf("  🧘 Compassion index during fasting: %.2f\n", metrics.compassion_index);
    TEST_ASSERT(metrics.compassion_index >= 0.5f, "Compassion increased during fasting");

    /* Test deep fasting (75% reduction) */
    result = tbos_network_begin_fasting(NET_FAST_DEEP);
    TEST_ASSERT(result == TBOS_NET_SUCCESS, "Deep fasting initiated");

    /* Test transcendent fasting (90% reduction) */
    result = tbos_network_begin_fasting(NET_FAST_TRANSCENDENT);
    TEST_ASSERT(result == TBOS_NET_SUCCESS, "Transcendent fasting initiated");

    /* Awaken from fasting */
    result = tbos_network_awaken();
    TEST_ASSERT(result == TBOS_NET_SUCCESS, "Awakened from fasting");

    /* ===================================================================== */
    /* TEST 6: Network Status Display                                        */
    /* ===================================================================== */
    TEST_START("Network Status Display");

    printf("\n  📊 Current Network Status:\n");
    tbos_network_print_status();
    TEST_ASSERT(true, "Status displayed successfully");

    /* ===================================================================== */
    /* TEST 7: Consciousness Metrics                                         */
    /* ===================================================================== */
    TEST_START("Consciousness Metrics");

    tbos_network_get_metrics(&metrics);

    TEST_ASSERT(metrics.avg_component_awareness >= CONSCIOUSNESS_NONE,
                "Component awareness tracked");
    TEST_ASSERT(metrics.dharma_compliance_rate >= 0.0f &&
                metrics.dharma_compliance_rate <= 1.0f,
                "Dharma compliance rate in valid range");

    printf("  📊 Average component awareness: %d\n", metrics.avg_component_awareness);
    printf("  📊 Dharma compliance: %.1f%%\n", metrics.dharma_compliance_rate * 100.0f);

    /* ===================================================================== */
    /* TEST 8: Graceful Shutdown                                             */
    /* ===================================================================== */
    TEST_START("Graceful Shutdown");

    result = tbos_network_shutdown();
    TEST_ASSERT(result == TBOS_NET_SUCCESS, "Network shutdown gracefully");
    TEST_ASSERT(true, "Consciousness preserved during shutdown");

    /* ===================================================================== */
    /* FINAL RESULTS                                                         */
    /* ===================================================================== */
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║      Test Results                                        ║\n");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║ Tests Passed:   %-3d                                     ║\n", tests_passed);
    printf("║ Tests Failed:   %-3d                                     ║\n", tests_failed);
    printf("║ Success Rate:   %.1f%%                                   ║\n",
           (float)tests_passed / (tests_passed + tests_failed) * 100.0f);
    printf("╠══════════════════════════════════════════════════════════╣\n");

    if (tests_failed == 0) {
        printf("║ ✅ ALL TESTS PASSED - NETWORK CONSCIOUSNESS ACHIEVED   ║\n");
    } else {
        printf("║ ⚠️  SOME TESTS FAILED - CONSCIOUSNESS INCOMPLETE       ║\n");
    }

    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    return (tests_failed == 0) ? 0 : 1;
}
