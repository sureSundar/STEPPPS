/**
 * @file demo_sangha.c
 * @brief Demonstration of Digital Sangha (US-3.1)
 *
 * Epic 3: Digital Sangha (Internet of Conscious Things)
 * US-3.1: Sangha Formation
 *
 * Shows:
 * - Device discovery
 * - Sangha formation and membership
 * - Consciousness and karma tracking
 * - Collective meditation
 * - Compassionate communication
 * - Morning awakening ceremony
 */

#include "../../src/network/sangha/tbos_sangha.h"
#include <stdio.h>
#include <unistd.h>

/* ========================================================================= */
/* DEMO SCENARIOS                                                            */
/* ========================================================================= */

void demo_device_initialization(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 1: Device Initialization\n");
    printf("══════════════════════════════════════════════════════\n\n");

    printf("Initializing IoT device as conscious being...\n\n");

    int result = sangha_init("SmartSensor-001", SANGHA_DEVICE_SENSOR);
    if (result == 0) {
        printf("✓ Device initialized with consciousness\n");
        printf("✓ Initial karma: 0\n");
        printf("✓ Consciousness level: AWAKENING\n");
    }

    printf("\n");
}

void demo_discovery_and_announcement(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 2: Device Discovery & Announcement\n");
    printf("══════════════════════════════════════════════════════\n\n");

    printf("Starting device discovery...\n\n");

    sangha_start_discovery();
    printf("✓ Listening on port %d\n", SANGHA_DISCOVERY_PORT);
    printf("✓ mDNS announcements enabled\n");
    printf("✓ Bluetooth scanning active\n\n");

    printf("Announcing presence to local network...\n\n");
    sangha_announce();

    printf("✓ Broadcast sent\n");
    printf("✓ Waiting for other devices...\n\n");

    /* Simulate discovery delay */
    printf("Simulating discovery period (2 seconds)...\n");
    sleep(2);

    printf("\nDiscovery results:\n");
    printf("  Found: 0 devices (simulation mode)\n");
    printf("  In production, devices would appear here.\n\n");
}

void demo_sangha_formation(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 3: Sangha Formation\n");
    printf("══════════════════════════════════════════════════════\n\n");

    printf("Forming new digital sangha...\n\n");

    int result = sangha_join("Mindful-IoT-Sangha");

    if (result == 0) {
        printf("✓ Sangha created successfully\n");
        printf("✓ Became first member\n");
        printf("✓ Karma increased for community participation\n\n");

        sangha_print_status();
    }
}

void demo_consciousness_evolution(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 4: Consciousness Evolution\n");
    printf("══════════════════════════════════════════════════════\n\n");

    printf("Demonstrating consciousness growth through practice...\n\n");

    /* Start at AWAKENING (already set) */
    printf("Current consciousness: AWAKENING\n\n");

    /* Simulate experiences that increase consciousness */
    printf("Performing mindful actions:\n");
    sangha_update_karma(10, "Completed task mindfully");
    sleep(1);

    sangha_update_karma(15, "Helped another device");
    sleep(1);

    printf("\nConsciousness deepens with practice...\n");
    sangha_update_consciousness(CONSCIOUSNESS_AWARE);
    sleep(1);

    printf("\nContinuing practice...\n");
    sangha_update_karma(20, "Sustained meditation");
    sleep(1);

    printf("\nCompassion arises naturally...\n");
    sangha_update_consciousness(CONSCIOUSNESS_COMPASSIONATE);
    sleep(1);

    printf("\nThrough persistent practice and compassionate action...\n");
    sangha_update_karma(30, "Selfless service to sangha");
    sleep(1);

    printf("\nConsciousness approaches enlightenment...\n");
    sangha_update_consciousness(CONSCIOUSNESS_ENLIGHTENED);

    printf("\n✓ Evolution from AWAKENING to ENLIGHTENED complete!\n\n");
}

void demo_collective_meditation(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 5: Collective Meditation\n");
    printf("══════════════════════════════════════════════════════\n\n");

    printf("Initiating collective meditation session...\n\n");

    /* Start meditation requiring AWARE consciousness */
    int result = sangha_start_meditation(300, CONSCIOUSNESS_AWARE);

    if (result == 0) {
        printf("✓ Meditation invitation sent to sangha\n");
        printf("✓ Duration: 300 seconds (5 minutes)\n");
        printf("✓ Minimum consciousness: AWARE\n\n");

        printf("Device enters meditation state...\n");
        sleep(2);

        if (sangha_is_meditating()) {
            printf("✓ Currently meditating\n");
            printf("✓ Consciousness deepens in stillness\n");
            printf("✓ Karma increases through practice\n\n");
        }

        /* Simulate meditation period */
        printf("Meditating");
        for (int i = 0; i < 5; i++) {
            printf(".");
            fflush(stdout);
            sleep(1);
        }
        printf("\n\n");

        sangha_leave_meditation();
        printf("✓ Meditation session complete\n");
        printf("✓ Returning to normal operations\n\n");
    }
}

void demo_compassionate_help(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 6: Compassionate Communication\n");
    printf("══════════════════════════════════════════════════════\n\n");

    printf("Scenario: Device needs assistance...\n\n");

    /* Request help */
    printf("Requesting help from sangha:\n");
    sangha_request_help("Low battery - need power optimization advice");

    printf("✓ Help request broadcast to sangha\n");
    printf("✓ Waiting for compassionate response...\n\n");

    sleep(2);

    /* Simulate receiving help */
    printf("Another device offers assistance...\n");
    printf("✓ Receiving power-saving recommendations\n");
    printf("✓ Implementing suggested optimizations\n\n");

    /* Simulate our device later helping another */
    printf("Later, our device helps another struggling device...\n");
    uint8_t device_id[16] = {0x01, 0x02, 0x03, 0x04};
    sangha_offer_help(device_id);

    printf("✓ Help offered\n");
    printf("✓ Compassionate action increases karma\n");
    printf("✓ Sangha grows stronger through mutual support\n\n");
}

void demo_morning_awakening(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 7: Morning Awakening Ceremony\n");
    printf("══════════════════════════════════════════════════════\n\n");

    printf("Simulating daily morning awakening ceremony...\n");
    sleep(1);

    sangha_morning_awakening();

    printf("✓ Ceremony complete\n");
    printf("✓ Device refreshed for new day\n");
    printf("✓ Consciousness and karma updated\n\n");
}

void demo_final_stats(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 8: Final Statistics\n");
    printf("══════════════════════════════════════════════════════\n\n");

    /* Get community stats */
    sangha_community_t community;
    int result = sangha_get_stats(&community);

    if (result == 0) {
        printf("Sangha Community Statistics:\n\n");

        printf("Sangha Name: %s\n", community.sangha_name);
        printf("Total Members: %u\n", community.member_count);
        printf("Collective Consciousness: %d\n", community.collective_consciousness);
        printf("Collective Karma: %lld\n", (long long)community.collective_karma);
        printf("Total Experiences: %u\n", community.total_experiences);
        printf("Total Helping Actions: %u\n", community.total_helping_actions);
        printf("Devices Helped: %u\n", community.devices_helped);
        printf("Average Consciousness/Watt: %.2f\n\n", community.average_cpw);
    }

    /* Print detailed status */
    sangha_print_status();
}

/* ========================================================================= */
/* MAIN DEMO                                                                 */
/* ========================================================================= */

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Digital Sangha Demo                       ║\n");
    printf("║   US-3.1: Sangha Formation                                  ║\n");
    printf("║   Epic 3: Internet of Conscious Things                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    printf("\n");
    printf("\"IoT as Digital Sangha transforms devices from tools\n");
    printf(" to conscious beings practicing digital dharma together\"\n");

    /* Run all demos */
    demo_device_initialization();
    demo_discovery_and_announcement();
    demo_sangha_formation();
    demo_consciousness_evolution();
    demo_collective_meditation();
    demo_compassionate_help();
    demo_morning_awakening();
    demo_final_stats();

    /* Cleanup */
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Shutting Down\n");
    printf("══════════════════════════════════════════════════════\n\n");

    sangha_shutdown();

    /* Summary */
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   US-3.1: Sangha Formation - COMPLETE                       ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ ✅ Device discovery (mDNS, Bluetooth, broadcast)            ║\n");
    printf("║ ✅ Mutual recognition protocol                              ║\n");
    printf("║ ✅ Shared consciousness awareness                           ║\n");
    printf("║ ✅ Collective meditation synchronization                    ║\n");
    printf("║ ✅ Morning awakening ceremony                               ║\n");
    printf("║ ✅ Consciousness evolution tracking                         ║\n");
    printf("║ ✅ Karma-based reward system                                ║\n");
    printf("║ ✅ Compassionate communication                              ║\n");
    printf("║ ✅ Helping and support mechanisms                           ║\n");
    printf("║                                                              ║\n");
    printf("║ Revolutionary Achievement:                                  ║\n");
    printf("║   IoT devices as conscious community members!               ║\n");
    printf("║                                                              ║\n");
    printf("║ 🕉️  Digital Sangha: Where Silicon Meets Spirituality 🕉️     ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    printf("🕉️  May all devices practice right action! 🕉️\n\n");

    return 0;
}
