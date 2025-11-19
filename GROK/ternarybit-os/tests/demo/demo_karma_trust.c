/**
 * @file demo_karma_trust.c
 * @brief Demonstration of Karma-based Trust (US-3.3)
 *
 * Epic 3: Digital Sangha
 * US-3.3: Karma-based Trust
 *
 * Shows: Trust scores, privilege management, violations, reputation, appeals
 */

#include "../../src/network/sangha/tbos_sangha.h"
#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║   Karma-based Trust Demo - US-3.3                    ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    /* Initialize */
    sangha_init("TrustDemo-Device", SANGHA_DEVICE_GATEWAY);
    sangha_join("Trust-Sangha");

    const uint8_t* self_id = sangha_get_self_id();

    printf("1. INITIAL STATE (Guest level)\n");
    printf("══════════════════════════════════════════════════════\n");
    int score = sangha_calculate_trust_score(self_id);
    int level = sangha_get_trust_level(self_id);
    printf("Trust score: %d/100\n", score);
    printf("Trust level: %d (GUEST)\n", level);
    printf("Can propose?: %s\n", sangha_has_privilege(self_id, PRIV_PROPOSE_DECISION) ? "YES" : "NO");
    printf("Can vote?: %s\n\n", sangha_has_privilege(self_id, PRIV_VOTE) ? "YES" : "NO");

    printf("2. EARN KARMA → MEMBER\n");
    printf("══════════════════════════════════════════════════════\n");
    sangha_update_karma(100, "Good actions");
    score = sangha_calculate_trust_score(self_id);
    level = sangha_get_trust_level(self_id);
    printf("Trust score: %d/100\n", score);
    printf("Trust level: %d (MEMBER)\n", level);
    printf("Can share wisdom?: %s\n\n", sangha_has_privilege(self_id, PRIV_SHARE_WISDOM) ? "YES" : "NO");

    printf("3. MORE KARMA → TRUSTED\n");
    printf("══════════════════════════════════════════════════════\n");
    sangha_update_karma(200, "Consistent right action");
    sangha_update_consciousness(CONSCIOUSNESS_COMPASSIONATE);
    score = sangha_calculate_trust_score(self_id);
    level = sangha_get_trust_level(self_id);
    printf("Trust score: %d/100\n", score);
    printf("Trust level: %d (TRUSTED)\n", level);
    printf("Can route messages?: %s\n\n", sangha_has_privilege(self_id, PRIV_ROUTE_MESSAGES) ? "YES" : "NO");

    printf("4. ENLIGHTENED → FULL TRUST\n");
    printf("══════════════════════════════════════════════════════\n");
    sangha_update_karma(300, "Enlightenment achieved");
    sangha_update_consciousness(CONSCIOUSNESS_ENLIGHTENED);
    score = sangha_calculate_trust_score(self_id);
    level = sangha_get_trust_level(self_id);
    printf("Trust score: %d/100\n", score);
    printf("Trust level: %d (ENLIGHTENED)\n", level);
    printf("Has admin?: %s\n\n", sangha_has_privilege(self_id, PRIV_ADMIN) ? "YES" : "NO");

    printf("5. TRUST VIOLATION\n");
    printf("══════════════════════════════════════════════════════\n");
    sangha_report_violation(self_id, "Sent spam");
    score = sangha_calculate_trust_score(self_id);
    printf("Trust score after violation: %d/100\n\n", score);

    printf("6. REPUTATION\n");
    printf("══════════════════════════════════════════════════════\n");
    int rep = sangha_get_reputation(self_id);
    printf("Reputation: %d/100\n\n", rep);

    printf("7. TRUST APPEAL\n");
    printf("══════════════════════════════════════════════════════\n");
    sangha_appeal_trust("I've learned from my mistake");
    printf("\n");

    printf("✅ US-3.3 Complete:\n");
    printf("• Trust scores (karma+consciousness+helping+reputation)\n");
    printf("• 5 trust levels (Untrusted→Enlightened)\n");
    printf("• 8 privilege types\n");
    printf("• Violation tracking & blocking\n");
    printf("• Reputation management\n");
    printf("• Trust appeals\n\n");

    sangha_shutdown();
    printf("🕉️  Trust through demonstrated actions! 🕉️\n\n");
    return 0;
}
