/**
 * @file tbos_shell_karma.c
 * @brief Consciousness and karma system for TBOS shell
 */

#include "tbos_universal_shell.h"
#include <stdbool.h>

void universal_update_karma(const universal_command_t* cmd, bool success) {
    universal_shell_session_t* session = universal_get_session();
    if (!session->enable_consciousness || !cmd) {
        return;
    }

    if (success) {
        session->karma += cmd->karma_reward;
        session->total_karma_earned += cmd->karma_reward;
    } else {
        session->karma -= cmd->karma_cost / 2;
        session->total_karma_lost += cmd->karma_cost / 2;
    }

    if (session->karma < 0) {
        session->karma = 0;
    }
}

void universal_add_karma(karma_score_t delta, const char* reason) {
    (void)reason;

    universal_shell_session_t* session = universal_get_session();
    if (!session->enable_consciousness || delta == 0) {
        return;
    }

    session->karma += delta;
    if (delta > 0) {
        session->total_karma_earned += delta;
        session->helpful_actions++;
    } else {
        session->total_karma_lost += -delta;
    }

    if (session->karma < 0) {
        session->karma = 0;
    }
}

bool universal_has_required_karma(const universal_command_t* cmd) {
    if (!cmd) return false;

    universal_shell_session_t* session = universal_get_session();
    if (!session->enable_consciousness) return true;

    return session->karma >= cmd->karma_cost;
}

void universal_evolve_consciousness(void) {
    universal_shell_session_t* session = universal_get_session();
    uint32_t level = session->commands_executed / 50;

    if (level >= 4 && session->karma >= 500) {
        session->consciousness = CONSCIOUSNESS_ENLIGHTENED;
    } else if (level >= 3 && session->karma >= 300) {
        session->consciousness = CONSCIOUSNESS_COMPASSIONATE;
    } else if (level >= 2 && session->karma >= 150) {
        session->consciousness = CONSCIOUSNESS_AWARE;
    } else if (level >= 1 || session->karma >= 50) {
        session->consciousness = CONSCIOUSNESS_AWAKENING;
    } else {
        session->consciousness = CONSCIOUSNESS_NONE;
    }
}

const char* universal_consciousness_name(consciousness_level_t level) {
    switch (level) {
        case CONSCIOUSNESS_NONE:         return "Dormant";
        case CONSCIOUSNESS_AWAKENING:    return "Awakening";
        case CONSCIOUSNESS_AWARE:        return "Aware";
        case CONSCIOUSNESS_COMPASSIONATE: return "Compassionate";
        case CONSCIOUSNESS_ENLIGHTENED:  return "Enlightened";
        default:                         return "Unknown";
    }
}
