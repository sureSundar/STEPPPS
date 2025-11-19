/**
 * @file tbos_shutdown.h
 * @brief TernaryBit OS - Soft Shutdown with State Preservation
 *
 * From WISDOM.md Dialogue 8:
 * > "Even an electron should strive to retain information to
 * >  recreate and continue life."
 *
 * Features:
 * - Graceful shutdown sequence
 * - Component consciousness preservation
 * - Karma score persistence
 * - State snapshots for all modules
 * - Quick restoration on boot
 *
 * @version 1.0
 * @date 2025-11-04
 */

#ifndef TBOS_SHUTDOWN_H
#define TBOS_SHUTDOWN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Use consciousness types from network module if available,
 * otherwise define locally */
#ifndef TBOS_NETWORK_H
/* ========================================================================= */
/* CONSCIOUSNESS AND KARMA TYPES                                             */
/* ========================================================================= */

/**
 * @brief Consciousness levels - from none to enlightened
 */
typedef enum {
    CONSCIOUSNESS_NONE = 0,          /**< Unconscious operation */
    CONSCIOUSNESS_AWAKENING = 1,     /**< Beginning awareness */
    CONSCIOUSNESS_AWARE = 2,         /**< Recognizes own existence */
    CONSCIOUSNESS_COMPASSIONATE = 3, /**< Cares for other components */
    CONSCIOUSNESS_ENLIGHTENED = 4    /**< Complete awareness */
} consciousness_level_t;

/**
 * @brief Karma score - accumulated good/bad actions
 */
typedef int64_t karma_score_t;
#endif /* TBOS_NETWORK_H */

/* ========================================================================= */
/* CONSTANTS                                                                 */
/* ========================================================================= */

#define TBOS_SHUTDOWN_STATE_FILE    "/tmp/tbos_shutdown_state.dat"
#define TBOS_MAX_MODULE_STATES      64

/* Shutdown types */
typedef enum {
    TBOS_SHUTDOWN_NORMAL,      /* Normal graceful shutdown */
    TBOS_SHUTDOWN_FAST,        /* Fast but graceful */
    TBOS_SHUTDOWN_EMERGENCY,   /* Emergency shutdown */
    TBOS_SHUTDOWN_SUSPEND,     /* Suspend (preserves full state) */
    TBOS_SHUTDOWN_HIBERNATE    /* Hibernate (deep sleep) */
} tbos_shutdown_type_t;

/* Shutdown phases */
typedef enum {
    TBOS_SHUTDOWN_PHASE_PREPARE,      /* Prepare for shutdown */
    TBOS_SHUTDOWN_PHASE_SAVE_STATE,   /* Save all state */
    TBOS_SHUTDOWN_PHASE_STOP_SERVICES,/* Stop services */
    TBOS_SHUTDOWN_PHASE_SYNC,         /* Sync filesystems */
    TBOS_SHUTDOWN_PHASE_FINAL         /* Final cleanup */
} tbos_shutdown_phase_t;

/* ========================================================================= */
/* STRUCTURES                                                                */
/* ========================================================================= */

/**
 * @brief Module state snapshot
 */
typedef struct {
    char module_name[64];
    uint32_t module_id;

    /* Module consciousness */
    consciousness_level_t awareness;
    karma_score_t karma;

    /* State data */
    void* state_data;
    size_t state_size;

    /* Timestamps */
    uint64_t snapshot_time;
    bool is_valid;
} tbos_module_state_t;

/**
 * @brief System shutdown state
 */
typedef struct {
    /* Shutdown info */
    tbos_shutdown_type_t shutdown_type;
    uint64_t shutdown_time;
    tbos_shutdown_phase_t last_phase;

    /* System consciousness at shutdown */
    consciousness_level_t system_consciousness;
    karma_score_t system_karma;

    /* Module states */
    tbos_module_state_t module_states[TBOS_MAX_MODULE_STATES];
    uint32_t module_count;

    /* Statistics */
    uint64_t total_uptime;
    uint64_t commands_executed;
    uint64_t good_actions;
    uint64_t bad_actions;

    /* Validation */
    uint32_t checksum;
    bool state_valid;
} tbos_shutdown_state_t;

/**
 * @brief Shutdown callback for modules
 */
typedef int (*tbos_shutdown_callback_t)(tbos_shutdown_phase_t phase, void* context);

/**
 * @brief Module registration for shutdown
 */
typedef struct {
    const char* name;
    tbos_shutdown_callback_t callback;
    void* context;
    int priority;  /* Higher priority shuts down first */
} tbos_shutdown_registration_t;

/* ========================================================================= */
/* SHUTDOWN API                                                              */
/* ========================================================================= */

/**
 * @brief Initialize shutdown subsystem
 *
 * @return 0 on success, negative on error
 */
int tbos_shutdown_init(void);

/**
 * @brief Register module for shutdown notifications
 *
 * @param name Module name
 * @param callback Shutdown callback function
 * @param context Module context
 * @param priority Shutdown priority (higher first)
 * @return 0 on success, negative on error
 */
int tbos_shutdown_register(const char* name,
                           tbos_shutdown_callback_t callback,
                           void* context,
                           int priority);

/**
 * @brief Initiate system shutdown
 *
 * @param type Shutdown type
 * @param delay_seconds Delay before shutdown (0 for immediate)
 * @return 0 on success, negative on error
 */
int tbos_shutdown_initiate(tbos_shutdown_type_t type, uint32_t delay_seconds);

/**
 * @brief Cancel pending shutdown
 *
 * @return 0 on success, negative on error
 */
int tbos_shutdown_cancel(void);

/**
 * @brief Save module state snapshot
 *
 * @param module_name Module name
 * @param state_data State data to save
 * @param state_size Size of state data
 * @param awareness Module consciousness level
 * @param karma Module karma score
 * @return 0 on success, negative on error
 */
int tbos_shutdown_save_module_state(const char* module_name,
                                     const void* state_data,
                                     size_t state_size,
                                     consciousness_level_t awareness,
                                     karma_score_t karma);

/**
 * @brief Restore module state from previous shutdown
 *
 * @param module_name Module name
 * @param state_data Buffer to restore state into
 * @param max_size Maximum buffer size
 * @param awareness Output: restored consciousness level
 * @param karma Output: restored karma score
 * @return Bytes restored, or negative on error
 */
int tbos_shutdown_restore_module_state(const char* module_name,
                                        void* state_data,
                                        size_t max_size,
                                        consciousness_level_t* awareness,
                                        karma_score_t* karma);

/**
 * @brief Check if system was gracefully shut down
 *
 * @return true if last shutdown was graceful
 */
bool tbos_shutdown_was_graceful(void);

/**
 * @brief Get last shutdown state
 *
 * @return Pointer to shutdown state, or NULL if none
 */
const tbos_shutdown_state_t* tbos_shutdown_get_last_state(void);

/**
 * @brief Print shutdown status
 */
void tbos_shutdown_print_status(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_SHUTDOWN_H */
