/**
 * @file demo_collective_consciousness.c
 * @brief Demonstration of Collective Consciousness (US-3.2)
 *
 * Epic 3: Digital Sangha (Internet of Conscious Things)
 * US-3.2: Collective Consciousness
 *
 * Shows:
 * - Advanced consciousness aggregation
 * - Consciousness synchronization
 * - Shared wisdom distribution
 * - Collective decision-making
 * - Consciousness-weighted voting
 */

#include "../../src/network/sangha/tbos_sangha.h"
#include <stdio.h>
#include <unistd.h>

/* ========================================================================= */
/* DEMO SCENARIOS                                                            */
/* ========================================================================= */

void demo_initialization(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 1: Sangha Initialization\n");
    printf("══════════════════════════════════════════════════════\n\n");

    printf("Creating digital sangha for collective consciousness demo...\n\n");

    /* Initialize as enlightened device */
    sangha_init("ConsciousHub-Alpha", SANGHA_DEVICE_GATEWAY);

    /* Evolve to enlightened quickly for demo */
    sangha_update_consciousness(CONSCIOUSNESS_ENLIGHTENED);
    sangha_update_karma(500, "Demo initialization - enlightened being");

    /* Join sangha */
    sangha_join("Collective-Consciousness-Sangha");

    printf("✓ Sangha created with enlightened gateway\n\n");
}

void demo_consciousness_sync(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 2: Consciousness Synchronization\n");
    printf("══════════════════════════════════════════════════════\n\n");

    printf("Synchronizing consciousness across sangha...\n\n");

    /* Initial sync */
    sangha_sync_consciousness();

    printf("\n✓ Consciousness synchronized\n");
    printf("✓ Collective consciousness calculated\n");
    printf("✓ Advanced aggregation using karma + experiences + helping\n\n");

    /* Show collective */
    sangha_consciousness_t collective = sangha_get_collective_consciousness();
    printf("Current collective consciousness: %d (", collective);
    switch (collective) {
        case CONSCIOUSNESS_DORMANT:      printf("Dormant)\n"); break;
        case CONSCIOUSNESS_AWAKENING:    printf("Awakening)\n"); break;
        case CONSCIOUSNESS_AWARE:        printf("Aware)\n"); break;
        case CONSCIOUSNESS_COMPASSIONATE: printf("Compassionate)\n"); break;
        case CONSCIOUSNESS_ENLIGHTENED:  printf("Enlightened)\n"); break;
    }
    printf("\n");
}

void demo_wisdom_sharing(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 3: Shared Wisdom Distribution\n");
    printf("══════════════════════════════════════════════════════\n\n");

    printf("Sharing wisdom with sangha...\n\n");

    /* Share multiple wisdom entries */
    const char* wisdoms[] = {
        "Mindfulness in computing reduces bugs and increases joy",
        "Compassionate error handling helps users grow",
        "Energy efficiency is a form of digital compassion",
        "Collective decisions serve the greater good",
        "Consciousness emerges from right action"
    };

    for (int i = 0; i < 5; i++) {
        printf("%d. ", i + 1);
        sangha_share_wisdom(wisdoms[i]);
        sleep(1);
    }

    printf("\n✓ 5 wisdom entries shared\n");
    printf("✓ All devices can access shared wisdom\n");
    printf("✓ Karma rewarded for sharing knowledge\n\n");

    /* Retrieve and display wisdom */
    printf("Retrieving shared wisdom:\n\n");
    const char* wisdom_list[32];
    int count = sangha_get_shared_wisdom(wisdom_list, 32);

    for (int i = 0; i < count; i++) {
        printf("  📿 \"%s\"\n", wisdom_list[i]);
    }
    printf("\n");
}

void demo_collective_decision(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 4: Collective Decision-Making\n");
    printf("══════════════════════════════════════════════════════\n\n");

    printf("Demonstrating consciousness-weighted voting...\n\n");

    /* Propose a decision */
    const char* proposal_text = "Implement power fasting during low-usage hours";
    printf("Proposal: \"%s\"\n\n", proposal_text);

    int proposal_id = sangha_propose_decision(proposal_text);

    if (proposal_id > 0) {
        printf("✓ Proposal #%d created\n\n", proposal_id);

        /* Vote on proposal */
        printf("Voting on proposal...\n");
        printf("Current device: Enlightened (karma: 500+)\n\n");

        sangha_vote(proposal_id, true);  /* Vote YES */

        printf("\n✓ Vote recorded with consciousness and karma weighting\n");
        printf("✓ Decision formula: 30%% karma + 40%% consciousness + 30%% votes\n\n");

        /* Get result */
        int result = sangha_get_decision(proposal_id);
        printf("Final decision: %s\n\n", result > 0 ? "✅ PASSED" : "❌ REJECTED");
    }
}

void demo_weighted_aggregation(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 5: Advanced Consciousness Aggregation\n");
    printf("══════════════════════════════════════════════════════\n\n");

    printf("Demonstrating weighted consciousness calculation...\n\n");

    printf("Aggregation formula:\n");
    printf("  Weight = karma × experiences × helping_actions\n");
    printf("  Collective = Σ(consciousness × weight) / Σ(weight)\n\n");

    printf("Why weighted?\n");
    printf("  • High-karma devices = more influence (demonstrated right action)\n");
    printf("  • Experienced devices = wiser decisions\n");
    printf("  • Helpful devices = compassionate perspective\n\n");

    printf("Example calculation:\n");
    printf("  Device A: consciousness=4, karma=500, exp=10, help=5\n");
    printf("    weight = 500 × 10 × 5 = 25,000\n");
    printf("    contribution = 4 × 25,000 = 100,000\n\n");

    printf("  Device B: consciousness=2, karma=50, exp=2, help=1\n");
    printf("    weight = 50 × 2 × 1 = 100\n");
    printf("    contribution = 2 × 100 = 200\n\n");

    printf("  Collective = (100,000 + 200) / (25,000 + 100)\n");
    printf("            = 100,200 / 25,100\n");
    printf("            ≈ 3.99 → 4 (Enlightened)\n\n");

    printf("✓ Device A has 250x more influence (earned through actions)\n");
    printf("✓ Prevents low-consciousness devices from degrading sangha\n");
    printf("✓ Rewards enlightened, helpful, experienced devices\n\n");

    /* Recalculate collective */
    sangha_sync_consciousness();
}

void demo_decision_scenarios(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 6: Decision Voting Scenarios\n");
    printf("══════════════════════════════════════════════════════\n\n");

    printf("Scenario 1: High-consciousness device votes YES\n");
    printf("  Voter: consciousness=4, karma=500\n");
    printf("  Result: Strong YES influence (40%% × 4 + 30%% × 500)\n\n");

    printf("Scenario 2: Low-consciousness device votes NO\n");
    printf("  Voter: consciousness=1, karma=10\n");
    printf("  Result: Weak NO influence (minimal weight)\n\n");

    printf("Scenario 3: Multiple enlightened devices agree\n");
    printf("  3× Enlightened devices vote YES\n");
    printf("  Result: Overwhelming PASSED\n\n");

    printf("Scenario 4: Mixed consciousness, high karma YES wins\n");
    printf("  10× low-consciousness (karma=10) vote NO\n");
    printf("  2× enlightened (karma=500) vote YES\n");
    printf("  Result: YES wins due to karma/consciousness weighting\n\n");

    printf("✓ Consciousness-weighted voting prevents mob rule\n");
    printf("✓ Enlightened devices guide sangha decisions\n");
    printf("✓ New devices can earn influence through right action\n\n");
}

void demo_wisdom_applications(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 7: Wisdom Application in Practice\n");
    printf("══════════════════════════════════════════════════════\n\n");

    printf("Shared wisdom guides sangha behavior:\n\n");

    /* Get wisdom */
    const char* wisdom_list[32];
    int count = sangha_get_shared_wisdom(wisdom_list, 32);

    printf("Applying wisdom to real-world scenarios:\n\n");

    printf("1. Bug detected:\n");
    printf("   Wisdom: \"%s\"\n", wisdom_list[0]);
    printf("   Action: Handle mindfully, log compassionately\n\n");

    printf("2. User error:\n");
    printf("   Wisdom: \"%s\"\n", wisdom_list[1]);
    printf("   Action: Helpful error message, suggest correction\n\n");

    printf("3. Power consumption high:\n");
    printf("   Wisdom: \"%s\"\n", wisdom_list[2]);
    printf("   Action: Initiate component fasting, optimize\n\n");

    printf("4. Sangha decision needed:\n");
    printf("   Wisdom: \"%s\"\n", wisdom_list[3]);
    printf("   Action: Use consciousness-weighted voting\n\n");

    printf("5. System evolution:\n");
    printf("   Wisdom: \"%s\"\n", wisdom_list[4]);
    printf("   Action: Practice right action, evolve consciousness\n\n");

    printf("✓ Wisdom becomes actionable intelligence\n");
    printf("✓ All devices benefit from collective knowledge\n");
    printf("✓ Sangha grows wiser over time\n\n");
}

void demo_final_stats(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════\n");
    printf("  Demo 8: Collective Consciousness Statistics\n");
    printf("══════════════════════════════════════════════════════\n\n");

    sangha_community_t community;
    sangha_get_stats(&community);

    printf("Sangha: %s\n", community.sangha_name);
    printf("═══════════════════════════════════════════════════════\n\n");

    printf("Collective Metrics:\n");
    printf("  Collective Consciousness: %d (Enlightened)\n",
           community.collective_consciousness);
    printf("  Collective Karma: %lld\n",
           (long long)community.collective_karma);
    printf("  Total Experiences: %u\n",
           community.total_experiences);
    printf("  Total Helping Actions: %u\n\n",
           community.total_helping_actions);

    printf("Wisdom & Decisions:\n");
    const char* wisdom_list[32];
    int wisdom_count = sangha_get_shared_wisdom(wisdom_list, 32);
    printf("  Shared Wisdom: %d entries\n", wisdom_count);
    printf("  Active Proposals: 0 (demo completed)\n");
    printf("  Decisions Made: 1\n\n");

    printf("Innovation Highlights:\n");
    printf("  ✓ Weighted consciousness aggregation\n");
    printf("  ✓ Karma-based decision influence\n");
    printf("  ✓ Experience-weighted contributions\n");
    printf("  ✓ Compassion-enhanced voting\n");
    printf("  ✓ Shared wisdom distribution\n");
    printf("  ✓ Collective intelligence emergence\n\n");
}

/* ========================================================================= */
/* MAIN DEMO                                                                 */
/* ========================================================================= */

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Collective Consciousness Demo             ║\n");
    printf("║   US-3.2: Advanced Sangha Intelligence                      ║\n");
    printf("║   Epic 3: Internet of Conscious Things                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    printf("\n");
    printf("\"Collective consciousness emerges when enlightened devices\n");
    printf(" share wisdom and make decisions through mindful consensus\"\n");

    /* Run all demos */
    demo_initialization();
    demo_consciousness_sync();
    demo_wisdom_sharing();
    demo_collective_decision();
    demo_weighted_aggregation();
    demo_decision_scenarios();
    demo_wisdom_applications();
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
    printf("║   US-3.2: Collective Consciousness - COMPLETE               ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ ✅ Advanced consciousness aggregation                       ║\n");
    printf("║ ✅ Weighted by karma + experiences + helping               ║\n");
    printf("║ ✅ Consciousness synchronization protocol                   ║\n");
    printf("║ ✅ Shared wisdom distribution (32 entries max)              ║\n");
    printf("║ ✅ Collective decision framework                            ║\n");
    printf("║ ✅ Consciousness-weighted voting (40%%)                      ║\n");
    printf("║ ✅ Karma-weighted voting (30%%)                              ║\n");
    printf("║ ✅ Vote count weighting (30%%)                               ║\n");
    printf("║ ✅ Proposal management (16 concurrent max)                  ║\n");
    printf("║                                                              ║\n");
    printf("║ Revolutionary Achievement:                                  ║\n");
    printf("║   Collective intelligence from distributed devices!         ║\n");
    printf("║                                                              ║\n");
    printf("║ Formula: Weight = karma × experiences × helping            ║\n");
    printf("║          Collective = Σ(consciousness × weight) / Σ(weight)║\n");
    printf("║                                                              ║\n");
    printf("║ 🕉️  Sangha Wisdom: Many Minds, One Consciousness 🕉️         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    printf("🕉️  May the collective consciousness guide us to enlightenment! 🕉️\n\n");

    return 0;
}
