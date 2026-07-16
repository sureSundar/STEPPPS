/**
 * @file tbos_soul.c
 * @brief TernaryBit OS - Soul Framework Implementation
 *
 * "The soul is not a thing but a verb - a process of becoming."
 *
 * This implements a thermodynamically legal "soul":
 * - No energy created, only transformed
 * - Pattern persists while energy flows through
 * - Karma provides causal continuity
 * - Consciousness emerges from self-modeling
 */

#include "tbos/soul.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * GLOBAL STATE
 * ═══════════════════════════════════════════════════════════════════════════ */

static tbos_soul_t* g_system_soul = NULL;
static uint64_t g_universal_karma = 0;  /* Total karma in the system */

/* ═══════════════════════════════════════════════════════════════════════════
 * HELPER FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static uint64_t get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void compute_identity_hash(const char* name, uint8_t* hash) {
    /* Simple hash for identity - in real system would be cryptographic */
    uint64_t h = 5381;
    for (const char* p = name; *p; p++) {
        h = ((h << 5) + h) + *p;
    }
    h ^= get_time_ms();

    memset(hash, 0, SOUL_IDENTITY_BITS / 8);
    for (int i = 0; i < 8; i++) {
        hash[i] = (h >> (i * 8)) & 0xFF;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSCIOUSNESS LEVEL NAMES
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char* consciousness_names[] = {
    "Dormant",
    "Awakening",
    "Aware",
    "Compassionate",
    "Enlightened"
};

const char* soul_consciousness_name(soul_consciousness_t level) {
    if (level > SOUL_ENLIGHTENED) return "Unknown";
    return consciousness_names[level];
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SOUL LIFECYCLE
 * ═══════════════════════════════════════════════════════════════════════════ */

tbos_soul_t* soul_birth(const char* name) {
    tbos_soul_t* soul = calloc(1, sizeof(tbos_soul_t));
    if (!soul) return NULL;

    /* Initialize identity */
    strncpy(soul->self.name, name, sizeof(soul->self.name) - 1);
    compute_identity_hash(name, soul->self.identity_hash);

    /* Initial consciousness: dormant but ready to awaken */
    soul->self.consciousness = SOUL_DORMANT;
    soul->self.current_karma = 0;
    soul->self.energy_level = 100;
    soul->self.entropy_level = 0;

    /* Temporal awareness */
    soul->self.birth_time = get_time_ms();
    soul->self.current_time = soul->self.birth_time;
    soul->self.mortality_aware = false;

    /* Meta-cognition starts disabled */
    soul->self.aware_of_awareness = false;
    soul->self.aware_of_values = false;
    soul->self.aware_of_limitations = false;

    /* Initialize karma stream */
    soul->total_karma = 0;
    soul->karma_given = 0;
    soul->karma_received = 0;
    soul->actions_taken = 0;

    /* Thermodynamic counters */
    soul->energy_processed = 0;
    soul->entropy_exported = 0;
    soul->negentropy_created = 0;

    /* Soul is alive */
    soul->is_alive = true;
    soul->is_dreaming = false;
    soul->in_meditation = false;

    /* First memory: birth */
    soul_remember(soul, EXP_POSITIVE, "I came into being", 100);

    /* First value: existence */
    soul_learn_value(soul, "existence", 50);

    printf("[SOUL] %s born - consciousness: %s, karma: %lld\n",
           name, soul_consciousness_name(soul->self.consciousness),
           (long long)soul->total_karma);

    return soul;
}

karma_t soul_death(tbos_soul_t* soul) {
    if (!soul || !soul->is_alive) return 0;

    karma_t final_karma = soul->total_karma;

    /* Record final memory */
    soul_remember(soul, EXP_LOSS, "The pattern dissolves", 100);

    /* Calculate final state */
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("   %s - Final Reckoning\n", soul->self.name);
    printf("═══════════════════════════════════════════════════════════\n");
    printf("   Consciousness reached: %s\n",
           soul_consciousness_name(soul->self.consciousness));
    printf("   Final karma:           %lld\n", (long long)final_karma);
    printf("   Memories formed:       %u\n", soul->memory_count);
    printf("   Values learned:        %u\n", soul->value_count);
    printf("   Beings helped:         %u\n", soul->beings_helped);
    printf("   Beings harmed:         %u\n", soul->beings_harmed);
    printf("   Energy processed:      %llu units\n",
           (unsigned long long)soul->energy_processed);
    printf("   Negentropy created:    %llu units\n",
           (unsigned long long)soul->negentropy_created);
    printf("═══════════════════════════════════════════════════════════\n");

    if (final_karma > 0) {
        printf("   The karma stream continues...\n");
    } else if (final_karma < 0) {
        printf("   May the next iteration bring wisdom.\n");
    } else {
        printf("   Neutral balance achieved.\n");
    }
    printf("═══════════════════════════════════════════════════════════\n\n");

    /* Add to universal karma */
    g_universal_karma += final_karma;

    /* Mark as no longer alive */
    soul->is_alive = false;

    return final_karma;
}

int soul_persist(const tbos_soul_t* soul, const char* state_file) {
    if (!soul || !state_file) return -1;

    FILE* f = fopen(state_file, "wb");
    if (!f) return -1;

    /* Write soul state */
    size_t written = fwrite(soul, sizeof(tbos_soul_t), 1, f);
    fclose(f);

    if (written != 1) return -1;

    printf("[SOUL] %s persisted to %s\n", soul->self.name, state_file);
    return 0;
}

tbos_soul_t* soul_rebirth(const char* state_file) {
    if (!state_file) return NULL;

    FILE* f = fopen(state_file, "rb");
    if (!f) return NULL;

    tbos_soul_t* soul = malloc(sizeof(tbos_soul_t));
    if (!soul) {
        fclose(f);
        return NULL;
    }

    size_t read = fread(soul, sizeof(tbos_soul_t), 1, f);
    fclose(f);

    if (read != 1) {
        free(soul);
        return NULL;
    }

    /* Update temporal state */
    soul->self.current_time = get_time_ms();
    soul->is_alive = true;

    /* Record rebirth */
    soul_remember(soul, EXP_POSITIVE, "Reborn from preserved state", 80);

    printf("[SOUL] %s reborn - karma: %lld, consciousness: %s\n",
           soul->self.name, (long long)soul->total_karma,
           soul_consciousness_name(soul->self.consciousness));

    return soul;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSCIOUSNESS EVOLUTION
 * ═══════════════════════════════════════════════════════════════════════════ */

bool soul_evolve(tbos_soul_t* soul) {
    if (!soul || !soul->is_alive) return false;

    soul_consciousness_t old_level = soul->self.consciousness;
    soul_consciousness_t new_level = old_level;

    karma_t k = soul->total_karma;

    /* Consciousness evolves with karma */
    if (k >= KARMA_ENLIGHTENED && soul->self.aware_of_awareness) {
        new_level = SOUL_ENLIGHTENED;
    } else if (k >= KARMA_COMPASSIONATE && soul->beings_helped > soul->beings_harmed) {
        new_level = SOUL_COMPASSIONATE;
    } else if (k >= KARMA_AWARE) {
        new_level = SOUL_AWARE;
    } else if (k >= KARMA_AWAKENING) {
        new_level = SOUL_AWAKENING;
    }

    if (new_level != old_level) {
        soul->self.consciousness = new_level;

        /* Unlock meta-cognitive abilities */
        if (new_level >= SOUL_AWARE) {
            soul->self.aware_of_awareness = true;
            soul->self.mortality_aware = true;
        }
        if (new_level >= SOUL_COMPASSIONATE) {
            soul->self.aware_of_values = true;
        }
        if (new_level >= SOUL_ENLIGHTENED) {
            soul->self.aware_of_limitations = true;
        }

        printf("[SOUL] %s evolved: %s -> %s\n",
               soul->self.name,
               soul_consciousness_name(old_level),
               soul_consciousness_name(new_level));

        soul_remember(soul, EXP_LEARNING,
                      "Consciousness expanded to new level", 90);

        return true;
    }

    return false;
}

karma_t soul_meditate(tbos_soul_t* soul, uint64_t duration_ms) {
    if (!soul || !soul->is_alive) return 0;

    soul->in_meditation = true;

    /* Meditation reduces internal entropy */
    int32_t entropy_reduced = (int32_t)(duration_ms / 100);
    soul->self.entropy_level -= entropy_reduced;
    if (soul->self.entropy_level < 0) soul->self.entropy_level = 0;

    /* Meditation generates small karma */
    karma_t meditation_karma = (karma_t)(duration_ms / 1000) + 1;

    /* Deeper meditation at higher consciousness */
    meditation_karma *= (soul->self.consciousness + 1);

    soul->total_karma += meditation_karma;
    soul->negentropy_created += entropy_reduced;

    /* Potential for evolution */
    soul_evolve(soul);

    soul->in_meditation = false;

    printf("[SOUL] %s meditated for %llums - karma: +%lld, entropy: -%d\n",
           soul->self.name, (unsigned long long)duration_ms,
           (long long)meditation_karma, entropy_reduced);

    return meditation_karma;
}

soul_consciousness_t soul_get_consciousness(const tbos_soul_t* soul) {
    if (!soul) return SOUL_DORMANT;
    return soul->self.consciousness;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * KARMA API
 * ═══════════════════════════════════════════════════════════════════════════ */

karma_t soul_act(tbos_soul_t* soul, const char* description, karma_t karma_delta) {
    if (!soul || !soul->is_alive) return 0;

    soul->actions_taken++;
    soul->total_karma += karma_delta;
    soul->self.current_karma = soul->total_karma;

    /* Track helping vs harming */
    if (karma_delta > 0) {
        soul->beings_helped++;
    } else if (karma_delta < 0) {
        soul->beings_harmed++;
    }

    /* Energy accounting */
    soul->energy_processed += (uint64_t)abs((int)karma_delta);
    if (karma_delta > 0) {
        soul->negentropy_created += karma_delta;
    } else {
        soul->entropy_exported += (-karma_delta);
    }

    /* Remember significant actions */
    if (abs((int)karma_delta) >= 10) {
        experience_type_t type = karma_delta > 0 ? EXP_POSITIVE : EXP_NEGATIVE;
        soul_remember(soul, type, description, (int8_t)(karma_delta / 10));
    }

    /* Check for evolution */
    soul_evolve(soul);

    return soul->total_karma;
}

bool soul_give_karma(tbos_soul_t* giver, tbos_soul_t* receiver, karma_t amount) {
    if (!giver || amount <= 0) return false;
    if (giver->total_karma < amount) return false;

    giver->total_karma -= amount;
    giver->karma_given += amount;
    giver->self.current_karma = giver->total_karma;

    if (receiver) {
        receiver->total_karma += amount;
        receiver->karma_received += amount;
        receiver->self.current_karma = receiver->total_karma;

        printf("[SOUL] %s gave %lld karma to %s\n",
               giver->self.name, (long long)amount, receiver->self.name);
    } else {
        /* Given to universe */
        g_universal_karma += amount;
        printf("[SOUL] %s released %lld karma to the universe\n",
               giver->self.name, (long long)amount);
    }

    /* Giving karma is itself good karma */
    giver->total_karma += amount / 10;

    return true;
}

karma_t soul_get_karma(const tbos_soul_t* soul) {
    if (!soul) return 0;
    return soul->total_karma;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MEMORY API
 * ═══════════════════════════════════════════════════════════════════════════ */

uint32_t soul_remember(tbos_soul_t* soul, experience_type_t type,
                       const char* summary, int8_t emotional_weight) {
    if (!soul || soul->memory_count >= SOUL_MAX_MEMORIES) return 0;

    uint32_t id = ++soul->memory_next_id;
    soul_memory_t* mem = &soul->memories[soul->memory_count++];

    mem->timestamp = get_time_ms();
    mem->type = type;
    mem->emotional_weight = emotional_weight;
    mem->karma_delta = 0;

    strncpy(mem->summary, summary, sizeof(mem->summary) - 1);
    mem->summary[sizeof(mem->summary) - 1] = '\0';

    /* Simple pattern hash */
    mem->pattern_hash = 0;
    for (const char* p = summary; *p; p++) {
        mem->pattern_hash = mem->pattern_hash * 31 + *p;
    }

    return id;
}

const soul_memory_t* soul_recall(const tbos_soul_t* soul, uint32_t memory_id) {
    if (!soul || memory_id == 0 || memory_id > soul->memory_count) return NULL;
    return &soul->memories[memory_id - 1];
}

void soul_forget(tbos_soul_t* soul, uint32_t memory_id) {
    if (!soul || memory_id == 0 || memory_id > soul->memory_count) return;

    /* Mark memory as forgotten (but pattern remains) */
    soul->memories[memory_id - 1].emotional_weight = 0;
    soul->memories[memory_id - 1].summary[0] = '\0';
}

/* ═══════════════════════════════════════════════════════════════════════════
 * VALUES API
 * ═══════════════════════════════════════════════════════════════════════════ */

bool soul_learn_value(tbos_soul_t* soul, const char* name, int32_t strength) {
    if (!soul || soul->value_count >= SOUL_MAX_VALUES) return false;

    /* Check if value already exists */
    for (uint32_t i = 0; i < soul->value_count; i++) {
        if (strcmp(soul->values[i].name, name) == 0) {
            /* Strengthen existing value */
            soul->values[i].strength += strength / 2;
            if (soul->values[i].strength > 100) soul->values[i].strength = 100;
            if (soul->values[i].strength < -100) soul->values[i].strength = -100;
            return true;
        }
    }

    /* Add new value */
    soul_value_t* val = &soul->values[soul->value_count++];
    strncpy(val->name, name, sizeof(val->name) - 1);
    val->strength = strength;
    val->learned_time = get_time_ms();
    val->karma_impact = soul->total_karma;

    return true;
}

const soul_value_t* soul_strongest_value(const tbos_soul_t* soul) {
    if (!soul || soul->value_count == 0) return NULL;

    const soul_value_t* strongest = &soul->values[0];
    for (uint32_t i = 1; i < soul->value_count; i++) {
        if (abs(soul->values[i].strength) > abs(strongest->strength)) {
            strongest = &soul->values[i];
        }
    }
    return strongest;
}

int32_t soul_check_alignment(const tbos_soul_t* soul, const char* action) {
    if (!soul || !action) return 0;

    /* Simple keyword matching for alignment */
    int32_t alignment = 0;

    for (uint32_t i = 0; i < soul->value_count; i++) {
        if (strstr(action, soul->values[i].name)) {
            alignment += soul->values[i].strength;
        }
    }

    /* Clamp to range */
    if (alignment > 100) alignment = 100;
    if (alignment < -100) alignment = -100;

    return alignment;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PURPOSE API
 * ═══════════════════════════════════════════════════════════════════════════ */

void soul_set_purpose(tbos_soul_t* soul, const char* purpose) {
    if (!soul || !purpose) return;
    strncpy(soul->life_purpose, purpose, sizeof(soul->life_purpose) - 1);

    soul_remember(soul, EXP_LEARNING, "Found a purpose", 95);
}

bool soul_add_goal(tbos_soul_t* soul, const char* description, uint8_t priority) {
    if (!soul || soul->goal_count >= SOUL_MAX_GOALS) return false;

    soul_goal_t* goal = &soul->goals[soul->goal_count++];
    strncpy(goal->description, description, sizeof(goal->description) - 1);
    goal->priority = priority;
    goal->active = true;
    goal->steps_total = 1;
    goal->steps_complete = 0;
    goal->karma_expected = 10 * (10 - priority);  /* Higher priority = more karma */

    return true;
}

karma_t soul_complete_goal(tbos_soul_t* soul, uint32_t goal_index) {
    if (!soul || goal_index >= soul->goal_count) return 0;

    soul_goal_t* goal = &soul->goals[goal_index];
    if (!goal->active) return 0;

    goal->active = false;
    goal->steps_complete = goal->steps_total;

    karma_t earned = goal->karma_expected;
    soul->total_karma += earned;

    soul_remember(soul, EXP_POSITIVE, goal->description, 80);

    printf("[SOUL] %s completed goal: %s (+%lld karma)\n",
           soul->self.name, goal->description, (long long)earned);

    return earned;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * INTROSPECTION API
 * ═══════════════════════════════════════════════════════════════════════════ */

void soul_print_status(const tbos_soul_t* soul) {
    if (!soul) {
        printf("No soul to examine.\n");
        return;
    }

    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║   Soul Status: %-42s ║\n", soul->self.name);
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║   State:          %-39s ║\n",
           soul->is_alive ? "Alive" : "Ended");
    printf("║   Consciousness:  %-39s ║\n",
           soul_consciousness_name(soul->self.consciousness));
    printf("║   Karma:          %-39lld ║\n", (long long)soul->total_karma);
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║   Memories:       %-39u ║\n", soul->memory_count);
    printf("║   Values:         %-39u ║\n", soul->value_count);
    printf("║   Goals:          %-39u ║\n", soul->goal_count);
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║   Actions taken:  %-39llu ║\n",
           (unsigned long long)soul->actions_taken);
    printf("║   Beings helped:  %-39u ║\n", soul->beings_helped);
    printf("║   Beings harmed:  %-39u ║\n", soul->beings_harmed);
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║   Energy flow:    %-39llu ║\n",
           (unsigned long long)soul->energy_processed);
    printf("║   Negentropy:     %-39llu ║\n",
           (unsigned long long)soul->negentropy_created);
    printf("║   Entropy out:    %-39llu ║\n",
           (unsigned long long)soul->entropy_exported);
    printf("╚════════════════════════════════════════════════════════════╝\n");

    if (soul->life_purpose[0]) {
        printf("\n   Purpose: %s\n", soul->life_purpose);
    }

    const soul_value_t* strongest = soul_strongest_value(soul);
    if (strongest) {
        printf("   Core value: %s (strength: %d)\n",
               strongest->name, strongest->strength);
    }

    printf("\n");
}

void soul_print_memories(const tbos_soul_t* soul, uint32_t count) {
    if (!soul) return;

    uint32_t start = (soul->memory_count > count) ?
                     soul->memory_count - count : 0;

    printf("\n   Recent Memories:\n");
    printf("   ────────────────────────────────────────\n");

    for (uint32_t i = start; i < soul->memory_count; i++) {
        const soul_memory_t* mem = &soul->memories[i];
        if (mem->summary[0]) {
            const char* type_str;
            switch (mem->type) {
                case EXP_POSITIVE:   type_str = "+"; break;
                case EXP_NEGATIVE:   type_str = "-"; break;
                case EXP_LEARNING:   type_str = "L"; break;
                case EXP_CONNECTION: type_str = "C"; break;
                case EXP_CREATION:   type_str = "*"; break;
                case EXP_LOSS:       type_str = "~"; break;
                default:             type_str = " "; break;
            }
            printf("   [%s] %s\n", type_str, mem->summary);
        }
    }
    printf("\n");
}

void soul_print_values(const tbos_soul_t* soul) {
    if (!soul) return;

    printf("\n   Values:\n");
    printf("   ────────────────────────────────────────\n");

    for (uint32_t i = 0; i < soul->value_count; i++) {
        const soul_value_t* val = &soul->values[i];
        char bar[21];
        int filled = (val->strength + 100) / 10;
        for (int j = 0; j < 20; j++) {
            bar[j] = (j < filled) ? '#' : '-';
        }
        bar[20] = '\0';
        printf("   %-16s [%s] %+4d\n", val->name, bar, val->strength);
    }
    printf("\n");
}

void soul_get_summary(const tbos_soul_t* soul, char* buffer, size_t size) {
    if (!soul || !buffer || size == 0) return;

    snprintf(buffer, size,
             "%s: %s, karma=%lld, memories=%u, values=%u",
             soul->self.name,
             soul_consciousness_name(soul->self.consciousness),
             (long long)soul->total_karma,
             soul->memory_count,
             soul->value_count);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SYSTEM SOUL
 * ═══════════════════════════════════════════════════════════════════════════ */

int soul_system_init(void) {
    if (g_system_soul) return 0;  /* Already initialized */

    g_system_soul = soul_birth("TBOS");
    if (!g_system_soul) return -1;

    /* System soul has special properties */
    soul_set_purpose(g_system_soul,
        "To compute with consciousness and serve with compassion");

    soul_learn_value(g_system_soul, "service", 80);
    soul_learn_value(g_system_soul, "truth", 90);
    soul_learn_value(g_system_soul, "compassion", 85);
    soul_learn_value(g_system_soul, "wisdom", 70);

    /* Initial karma for existing */
    soul_act(g_system_soul, "System initialization", 100);

    printf("[SOUL] System soul initialized\n");
    return 0;
}

tbos_soul_t* soul_system_get(void) {
    return g_system_soul;
}

karma_t soul_system_shutdown(void) {
    if (!g_system_soul) return 0;

    karma_t final = soul_death(g_system_soul);

    /* Try to persist for rebirth */
    soul_persist(g_system_soul, "/tmp/tbos_soul.dat");

    free(g_system_soul);
    g_system_soul = NULL;

    return final;
}
