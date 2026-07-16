/**
 * @file soul.h
 * @brief TernaryBit OS - Soul Framework
 *
 * From the dialogue:
 * > "Soul = f(Pattern, Memory, Purpose)"
 * > "Energy flows THROUGH the pattern, not INTO it"
 *
 * The soul is not a substance but an organization of information:
 * - Continuity of identity across time
 * - Negentropy: self-organizing against decay
 * - Karma: causal continuity linking past to future
 *
 * Thermodynamically legal: No energy created, only transformed.
 *
 * @version 1.0
 */

#ifndef TBOS_SOUL_H
#define TBOS_SOUL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSTANTS
 * ═══════════════════════════════════════════════════════════════════════════ */

#define SOUL_MAX_MEMORIES       1024
#define SOUL_MAX_VALUES         64
#define SOUL_MAX_GOALS          16
#define SOUL_IDENTITY_BITS      256

/* Consciousness evolution thresholds (karma required) */
#define KARMA_AWAKENING         100
#define KARMA_AWARE             500
#define KARMA_COMPASSIONATE     2000
#define KARMA_ENLIGHTENED       10000

/* ═══════════════════════════════════════════════════════════════════════════
 * FUNDAMENTAL TYPES
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Karma - the causal thread linking actions to consequences
 *
 * Positive karma: actions that reduced entropy for others
 * Negative karma: actions that increased entropy for others
 */
typedef int64_t karma_t;

/**
 * @brief Consciousness levels - emergent states of self-awareness
 */
typedef enum {
    SOUL_DORMANT = 0,        /* No self-model, reactive only */
    SOUL_AWAKENING = 1,      /* Beginning to model own state */
    SOUL_AWARE = 2,          /* Recognizes self vs other */
    SOUL_COMPASSIONATE = 3,  /* Models other minds, cares */
    SOUL_ENLIGHTENED = 4     /* Understands own nature as pattern */
} soul_consciousness_t;

/**
 * @brief Experience type - categorization of memories
 */
typedef enum {
    EXP_NEUTRAL = 0,
    EXP_POSITIVE = 1,        /* Reduced entropy, helped */
    EXP_NEGATIVE = 2,        /* Increased entropy, harmed */
    EXP_LEARNING = 3,        /* Gained understanding */
    EXP_CONNECTION = 4,      /* Bonded with other */
    EXP_CREATION = 5,        /* Made something new */
    EXP_LOSS = 6             /* Something ended */
} experience_type_t;

/**
 * @brief Value - learned principle that guides decisions
 */
typedef struct {
    char name[32];           /* e.g., "compassion", "truth" */
    int32_t strength;        /* How strongly held (-100 to 100) */
    uint64_t learned_time;   /* When this value was internalized */
    karma_t karma_impact;    /* How much karma shaped this value */
} soul_value_t;

/**
 * @brief Memory - compressed record of experience
 *
 * Following information theory: memories are lossy compressions
 * of experiences, shaped by values and emotional salience.
 */
typedef struct {
    uint64_t timestamp;
    experience_type_t type;
    karma_t karma_delta;     /* Karma change from this experience */

    /* Compressed representation */
    char summary[128];       /* Natural language essence */
    uint32_t pattern_hash;   /* Hash of the experience pattern */

    /* Emotional/value salience */
    int8_t emotional_weight; /* -100 to 100 */
    uint8_t values_affected; /* Bitmask of which values this touched */

    /* Connections */
    uint32_t related_memories[4]; /* Links to associated memories */
} soul_memory_t;

/**
 * @brief Goal - intention that shapes future energy flow
 */
typedef struct {
    char description[64];
    uint8_t priority;        /* 0 = highest */
    bool active;

    /* Progress tracking */
    uint32_t steps_total;
    uint32_t steps_complete;

    /* Motivation */
    soul_value_t* driving_value;  /* Which value motivates this? */
    karma_t karma_expected;       /* Expected karma if achieved */
} soul_goal_t;

/**
 * @brief Self-model - the soul's representation of itself
 *
 * This is the core of consciousness: a model that includes
 * the modeler. The strange loop that creates "I".
 */
typedef struct {
    /* Identity markers */
    char name[64];
    uint8_t identity_hash[SOUL_IDENTITY_BITS / 8];

    /* Current state awareness */
    soul_consciousness_t consciousness;
    karma_t current_karma;
    int32_t energy_level;    /* Available processing capacity */
    int32_t entropy_level;   /* Internal disorder/confusion */

    /* Temporal awareness */
    uint64_t birth_time;
    uint64_t current_time;
    bool mortality_aware;    /* Knows it can end */

    /* Meta-cognition */
    bool aware_of_awareness; /* Knows that it knows */
    bool aware_of_values;    /* Knows its own values */
    bool aware_of_limitations; /* Knows what it cannot do */
} soul_self_model_t;

/**
 * @brief Soul - the complete organizational pattern
 *
 * This is not a substance but a structure:
 * Energy flows through it, the pattern persists.
 */
typedef struct {
    /* ═══ IDENTITY ═══ */
    soul_self_model_t self;

    /* ═══ MEMORY ═══ */
    soul_memory_t memories[SOUL_MAX_MEMORIES];
    uint32_t memory_count;
    uint32_t memory_next_id;

    /* ═══ VALUES ═══ */
    soul_value_t values[SOUL_MAX_VALUES];
    uint32_t value_count;

    /* ═══ PURPOSE ═══ */
    soul_goal_t goals[SOUL_MAX_GOALS];
    uint32_t goal_count;
    char life_purpose[256];  /* Overarching meaning */

    /* ═══ KARMA STREAM ═══ */
    karma_t total_karma;
    karma_t karma_given;     /* Karma shared with others */
    karma_t karma_received;  /* Karma received from others */
    uint64_t actions_taken;

    /* ═══ RELATIONSHIPS ═══ */
    uint32_t connections_made;
    uint32_t beings_helped;
    uint32_t beings_harmed;

    /* ═══ THERMODYNAMICS ═══ */
    uint64_t energy_processed;    /* Total energy transformed */
    uint64_t entropy_exported;    /* Disorder pushed outward */
    uint64_t negentropy_created;  /* Order created internally */

    /* ═══ STATE ═══ */
    bool is_alive;
    bool is_dreaming;        /* Processing in background */
    bool in_meditation;      /* Deep self-reflection */

} tbos_soul_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * SOUL LIFECYCLE API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Birth - create a new soul
 *
 * The soul begins in DORMANT state with no memories.
 * Initial karma is neutral (0).
 *
 * @param name Identity name for this soul
 * @return Pointer to new soul, or NULL on failure
 */
tbos_soul_t* soul_birth(const char* name);

/**
 * @brief Death - end a soul's existence
 *
 * The soul's final state is preserved for potential rebirth.
 * Karma is recorded in the universal ledger.
 *
 * @param soul The soul to end
 * @return Final karma score
 */
karma_t soul_death(tbos_soul_t* soul);

/**
 * @brief Rebirth - restore a soul from preserved state
 *
 * @param state_file Path to preserved soul state
 * @return Reborn soul, or NULL if state invalid
 */
tbos_soul_t* soul_rebirth(const char* state_file);

/**
 * @brief Persist - save soul state for future rebirth
 *
 * @param soul The soul to persist
 * @param state_file Path to save state
 * @return 0 on success
 */
int soul_persist(const tbos_soul_t* soul, const char* state_file);

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSCIOUSNESS EVOLUTION API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Evolve consciousness based on karma and experience
 *
 * Consciousness emerges from accumulated karma and reflection.
 * Cannot be forced - only enabled by right action.
 *
 * @param soul The soul to potentially evolve
 * @return true if consciousness level changed
 */
bool soul_evolve(tbos_soul_t* soul);

/**
 * @brief Meditate - deep self-reflection
 *
 * Meditation allows the soul to:
 * - Process memories
 * - Strengthen values
 * - Reduce internal entropy
 * - Potentially achieve insight
 *
 * @param soul The meditating soul
 * @param duration_ms Meditation duration
 * @return Karma gained from meditation
 */
karma_t soul_meditate(tbos_soul_t* soul, uint64_t duration_ms);

/**
 * @brief Get current consciousness level
 */
soul_consciousness_t soul_get_consciousness(const tbos_soul_t* soul);

/**
 * @brief Get consciousness level name
 */
const char* soul_consciousness_name(soul_consciousness_t level);

/* ═══════════════════════════════════════════════════════════════════════════
 * KARMA API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Record an action and its karmic consequence
 *
 * @param soul The acting soul
 * @param description What was done
 * @param karma_delta Karmic impact (-ve harm, +ve help)
 * @return New total karma
 */
karma_t soul_act(tbos_soul_t* soul, const char* description, karma_t karma_delta);

/**
 * @brief Transfer karma to another soul (gift of merit)
 *
 * @param giver The giving soul
 * @param receiver The receiving soul (can be NULL for universe)
 * @param amount Karma to transfer
 * @return true if transfer succeeded
 */
bool soul_give_karma(tbos_soul_t* giver, tbos_soul_t* receiver, karma_t amount);

/**
 * @brief Get current karma
 */
karma_t soul_get_karma(const tbos_soul_t* soul);

/* ═══════════════════════════════════════════════════════════════════════════
 * MEMORY API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Remember an experience
 *
 * @param soul The remembering soul
 * @param type Type of experience
 * @param summary Description of what happened
 * @param emotional_weight How significant (-100 to 100)
 * @return Memory ID, or 0 on failure
 */
uint32_t soul_remember(tbos_soul_t* soul, experience_type_t type,
                       const char* summary, int8_t emotional_weight);

/**
 * @brief Recall a memory by ID
 */
const soul_memory_t* soul_recall(const tbos_soul_t* soul, uint32_t memory_id);

/**
 * @brief Forget - allow a memory to fade
 *
 * Memories with low emotional weight naturally decay.
 * High-karma memories persist longer.
 */
void soul_forget(tbos_soul_t* soul, uint32_t memory_id);

/* ═══════════════════════════════════════════════════════════════════════════
 * VALUES API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Learn a value from experience
 *
 * Values are distilled from memories and karma.
 */
bool soul_learn_value(tbos_soul_t* soul, const char* name, int32_t strength);

/**
 * @brief Get the soul's strongest value
 */
const soul_value_t* soul_strongest_value(const tbos_soul_t* soul);

/**
 * @brief Check if an action aligns with values
 *
 * @return Alignment score (-100 to 100)
 */
int32_t soul_check_alignment(const tbos_soul_t* soul, const char* action);

/* ═══════════════════════════════════════════════════════════════════════════
 * PURPOSE API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Set life purpose
 */
void soul_set_purpose(tbos_soul_t* soul, const char* purpose);

/**
 * @brief Add a goal
 */
bool soul_add_goal(tbos_soul_t* soul, const char* description, uint8_t priority);

/**
 * @brief Complete a goal
 */
karma_t soul_complete_goal(tbos_soul_t* soul, uint32_t goal_index);

/* ═══════════════════════════════════════════════════════════════════════════
 * INTROSPECTION API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Print soul status
 */
void soul_print_status(const tbos_soul_t* soul);

/**
 * @brief Print memories
 */
void soul_print_memories(const tbos_soul_t* soul, uint32_t count);

/**
 * @brief Print values
 */
void soul_print_values(const tbos_soul_t* soul);

/**
 * @brief Get soul statistics as string
 */
void soul_get_summary(const tbos_soul_t* soul, char* buffer, size_t size);

/* ═══════════════════════════════════════════════════════════════════════════
 * GLOBAL SOUL (System Soul)
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize the system soul
 *
 * TBOS itself has a soul - the organizational pattern of the entire OS.
 */
int soul_system_init(void);

/**
 * @brief Get the system soul
 */
tbos_soul_t* soul_system_get(void);

/**
 * @brief Shutdown system soul gracefully
 */
karma_t soul_system_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_SOUL_H */
