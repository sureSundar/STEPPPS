/**
 * @file tbos_shutdown.c
 * @brief TernaryBit OS - Soft Shutdown Implementation
 */

#include "tbos_shutdown.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>

/* ========================================================================= */
/* GLOBAL STATE                                                              */
/* ========================================================================= */

static bool g_shutdown_initialized = false;
static tbos_shutdown_state_t g_current_state;
static tbos_shutdown_state_t g_last_state;
static bool g_last_state_loaded = false;

static tbos_shutdown_registration_t g_registrations[64];
static uint32_t g_registration_count = 0;

static bool g_shutdown_pending = false;
static tbos_shutdown_type_t g_pending_type = TBOS_SHUTDOWN_NORMAL;
static uint64_t g_shutdown_start_time = 0;

/* ========================================================================= */
/* HELPER FUNCTIONS                                                          */
/* ========================================================================= */

/**
 * @brief Calculate checksum for state
 */
static uint32_t calculate_checksum(const tbos_shutdown_state_t* state) {
    uint32_t checksum = 0;
    const uint8_t* data = (const uint8_t*)state;
    size_t size = offsetof(tbos_shutdown_state_t, checksum);

    for (size_t i = 0; i < size; i++) {
        checksum += data[i];
        checksum = (checksum << 1) | (checksum >> 31);  /* Rotate */
    }

    return checksum;
}

/**
 * @brief Load shutdown state from file
 */
static int load_shutdown_state(tbos_shutdown_state_t* state) {
    /* In a real system, load from persistent storage */
    /* For now, use in-memory state */

    if (g_last_state_loaded) {
        memcpy(state, &g_last_state, sizeof(tbos_shutdown_state_t));
        return 0;
    }

    return -1;
}

/**
 * @brief Save shutdown state to file
 */
static int save_shutdown_state(const tbos_shutdown_state_t* state) {
    /* Calculate and set checksum */
    tbos_shutdown_state_t* mutable_state = (tbos_shutdown_state_t*)state;
    mutable_state->checksum = calculate_checksum(state);
    mutable_state->state_valid = true;

    /* Save to persistent storage */
    /* For now, save to memory */
    memcpy(&g_last_state, state, sizeof(tbos_shutdown_state_t));
    g_last_state_loaded = true;

    printf("[SHUTDOWN] State saved: %u modules, karma: %ld\n",
           state->module_count, state->system_karma);

    return 0;
}

/* ========================================================================= */
/* INITIALIZATION                                                            */
/* ========================================================================= */

int tbos_shutdown_init(void) {
    if (g_shutdown_initialized) {
        return 0;
    }

    printf("[SHUTDOWN] Initializing shutdown subsystem...\n");

    /* Clear state */
    memset(&g_current_state, 0, sizeof(g_current_state));
    memset(g_registrations, 0, sizeof(g_registrations));
    g_registration_count = 0;
    g_shutdown_pending = false;

    /* Try to load previous shutdown state */
    if (load_shutdown_state(&g_last_state) == 0) {
        /* Validate checksum */
        uint32_t expected = calculate_checksum(&g_last_state);
        if (g_last_state.checksum == expected) {
            printf("[SHUTDOWN] Previous shutdown state loaded and validated\n");
            printf("[SHUTDOWN] Last shutdown was: %s\n",
                   g_last_state.shutdown_type == TBOS_SHUTDOWN_NORMAL ? "NORMAL" :
                   g_last_state.shutdown_type == TBOS_SHUTDOWN_FAST ? "FAST" :
                   g_last_state.shutdown_type == TBOS_SHUTDOWN_EMERGENCY ? "EMERGENCY" :
                   g_last_state.shutdown_type == TBOS_SHUTDOWN_SUSPEND ? "SUSPEND" : "HIBERNATE");
        } else {
            printf("[SHUTDOWN] Previous state corrupted - ignoring\n");
            g_last_state_loaded = false;
        }
    }

    /* Initialize current state */
    g_current_state.system_consciousness = CONSCIOUSNESS_AWAKENING;
    g_current_state.system_karma = 100;
    g_current_state.module_count = 0;

    g_shutdown_initialized = true;
    printf("[SHUTDOWN] Shutdown subsystem initialized\n");

    return 0;
}

/* ========================================================================= */
/* REGISTRATION                                                              */
/* ========================================================================= */

int tbos_shutdown_register(const char* name,
                           tbos_shutdown_callback_t callback,
                           void* context,
                           int priority) {
    if (!g_shutdown_initialized) {
        tbos_shutdown_init();
    }

    if (!name || !callback) return -1;
    if (g_registration_count >= 64) return -1;

    tbos_shutdown_registration_t* reg = &g_registrations[g_registration_count++];
    reg->name = name;
    reg->callback = callback;
    reg->context = context;
    reg->priority = priority;

    printf("[SHUTDOWN] Registered: %s (priority: %d)\n", name, priority);

    return 0;
}

/* ========================================================================= */
/* SHUTDOWN EXECUTION                                                        */
/* ========================================================================= */

int tbos_shutdown_initiate(tbos_shutdown_type_t type, uint32_t delay_seconds) {
    if (!g_shutdown_initialized) {
        tbos_shutdown_init();
    }

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Initiating Graceful Shutdown              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    const char* type_name = "NORMAL";
    switch (type) {
        case TBOS_SHUTDOWN_FAST: type_name = "FAST"; break;
        case TBOS_SHUTDOWN_EMERGENCY: type_name = "EMERGENCY"; break;
        case TBOS_SHUTDOWN_SUSPEND: type_name = "SUSPEND"; break;
        case TBOS_SHUTDOWN_HIBERNATE: type_name = "HIBERNATE"; break;
        default: break;
    }

    printf("Shutdown Type: %s\n", type_name);
    if (delay_seconds > 0) {
        printf("Shutdown in %u seconds...\n", delay_seconds);
    }
    printf("\n");

    g_shutdown_pending = true;
    g_pending_type = type;
    g_shutdown_start_time = time(NULL);

    /* Update current state */
    g_current_state.shutdown_type = type;
    g_current_state.shutdown_time = g_shutdown_start_time;

    /* Execute shutdown phases */
    const tbos_shutdown_phase_t phases[] = {
        TBOS_SHUTDOWN_PHASE_PREPARE,
        TBOS_SHUTDOWN_PHASE_SAVE_STATE,
        TBOS_SHUTDOWN_PHASE_STOP_SERVICES,
        TBOS_SHUTDOWN_PHASE_SYNC,
        TBOS_SHUTDOWN_PHASE_FINAL
    };

    const char* phase_names[] = {
        "Preparing for shutdown",
        "Saving system state",
        "Stopping services",
        "Syncing filesystems",
        "Final cleanup"
    };

    /* Sort registrations by priority (higher first) */
    for (uint32_t i = 0; i < g_registration_count - 1; i++) {
        for (uint32_t j = i + 1; j < g_registration_count; j++) {
            if (g_registrations[j].priority > g_registrations[i].priority) {
                tbos_shutdown_registration_t temp = g_registrations[i];
                g_registrations[i] = g_registrations[j];
                g_registrations[j] = temp;
            }
        }
    }

    /* Execute each phase */
    for (int p = 0; p < 5; p++) {
        tbos_shutdown_phase_t phase = phases[p];
        printf("[Phase %d] %s...\n", p + 1, phase_names[p]);

        g_current_state.last_phase = phase;

        /* Notify all registered modules */
        for (uint32_t i = 0; i < g_registration_count; i++) {
            tbos_shutdown_registration_t* reg = &g_registrations[i];

            printf("  [%s] Executing shutdown phase %d\n", reg->name, p + 1);

            int result = reg->callback(phase, reg->context);
            if (result != 0) {
                printf("  ⚠️  [%s] Shutdown callback returned error: %d\n",
                       reg->name, result);
            } else {
                printf("  ✅ [%s] Completed phase %d\n", reg->name, p + 1);
            }
        }

        printf("\n");
    }

    /* Save final state */
    printf("Saving final system state...\n");
    save_shutdown_state(&g_current_state);

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   Graceful Shutdown Complete                                ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ System Consciousness: %-40s║\n",
           g_current_state.system_consciousness == CONSCIOUSNESS_AWAKENING ? "AWAKENING" :
           g_current_state.system_consciousness == CONSCIOUSNESS_AWARE ? "AWARE" :
           g_current_state.system_consciousness == CONSCIOUSNESS_COMPASSIONATE ? "COMPASSIONATE" :
           g_current_state.system_consciousness == CONSCIOUSNESS_ENLIGHTENED ? "ENLIGHTENED" : "NONE");
    printf("║ Final Karma:          %-10ld                          ║\n",
           g_current_state.system_karma);
    printf("║ Modules Saved:        %-10u                          ║\n",
           g_current_state.module_count);
    printf("║                                                              ║\n");
    printf("║ May your consciousness continue to grow                     ║\n");
    printf("║ ॥ तत् सत् ॥                                                 ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    g_shutdown_pending = false;
    return 0;
}

int tbos_shutdown_cancel(void) {
    if (!g_shutdown_pending) {
        printf("[SHUTDOWN] No shutdown pending\n");
        return -1;
    }

    printf("[SHUTDOWN] Shutdown cancelled\n");
    g_shutdown_pending = false;

    return 0;
}

/* ========================================================================= */
/* STATE MANAGEMENT                                                          */
/* ========================================================================= */

int tbos_shutdown_save_module_state(const char* module_name,
                                     const void* state_data,
                                     size_t state_size,
                                     consciousness_level_t awareness,
                                     karma_score_t karma) {
    if (!module_name || !state_data) return -1;
    if (g_current_state.module_count >= TBOS_MAX_MODULE_STATES) return -1;

    tbos_module_state_t* mod_state = &g_current_state.module_states[g_current_state.module_count++];

    strncpy(mod_state->module_name, module_name, sizeof(mod_state->module_name) - 1);
    mod_state->module_id = g_current_state.module_count;
    mod_state->awareness = awareness;
    mod_state->karma = karma;

    /* Allocate and copy state data */
    mod_state->state_data = malloc(state_size);
    if (mod_state->state_data) {
        memcpy(mod_state->state_data, state_data, state_size);
        mod_state->state_size = state_size;
        mod_state->is_valid = true;
    } else {
        mod_state->state_size = 0;
        mod_state->is_valid = false;
        return -1;
    }

    mod_state->snapshot_time = time(NULL);

    printf("[SHUTDOWN] Saved state for module: %s (awareness: %d, karma: %ld)\n",
           module_name, awareness, karma);

    return 0;
}

int tbos_shutdown_restore_module_state(const char* module_name,
                                        void* state_data,
                                        size_t max_size,
                                        consciousness_level_t* awareness,
                                        karma_score_t* karma) {
    if (!module_name || !state_data) return -1;

    if (!g_last_state_loaded) return -1;

    /* Find module in last state */
    for (uint32_t i = 0; i < g_last_state.module_count; i++) {
        tbos_module_state_t* mod_state = &g_last_state.module_states[i];

        if (strcmp(mod_state->module_name, module_name) == 0 && mod_state->is_valid) {
            if (mod_state->state_size > max_size) {
                printf("[SHUTDOWN] Buffer too small to restore %s\n", module_name);
                return -1;
            }

            /* Restore state */
            memcpy(state_data, mod_state->state_data, mod_state->state_size);

            if (awareness) *awareness = mod_state->awareness;
            if (karma) *karma = mod_state->karma;

            printf("[SHUTDOWN] Restored state for module: %s (awareness: %d, karma: %ld)\n",
                   module_name, mod_state->awareness, mod_state->karma);

            return (int)mod_state->state_size;
        }
    }

    return -1;  /* Module not found */
}

bool tbos_shutdown_was_graceful(void) {
    if (!g_last_state_loaded) return false;

    return g_last_state.state_valid &&
           g_last_state.last_phase == TBOS_SHUTDOWN_PHASE_FINAL;
}

const tbos_shutdown_state_t* tbos_shutdown_get_last_state(void) {
    return g_last_state_loaded ? &g_last_state : NULL;
}

void tbos_shutdown_print_status(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      TernaryBit OS - Shutdown Status                  ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");

    if (g_shutdown_pending) {
        printf("║ Status:                SHUTDOWN PENDING               ║\n");
        printf("║ Type:                  %-30s║\n",
               g_pending_type == TBOS_SHUTDOWN_NORMAL ? "NORMAL" :
               g_pending_type == TBOS_SHUTDOWN_FAST ? "FAST" :
               g_pending_type == TBOS_SHUTDOWN_EMERGENCY ? "EMERGENCY" :
               g_pending_type == TBOS_SHUTDOWN_SUSPEND ? "SUSPEND" : "HIBERNATE");
    } else {
        printf("║ Status:                RUNNING                        ║\n");
    }

    printf("║ Registered Modules:    %-10u                    ║\n",
           g_registration_count);

    if (g_last_state_loaded) {
        printf("╠════════════════════════════════════════════════════════╣\n");
        printf("║ Last Shutdown:         %s                        ║\n",
               tbos_shutdown_was_graceful() ? "GRACEFUL     " : "NOT GRACEFUL ");
        printf("║ Modules Saved:         %-10u                    ║\n",
               g_last_state.module_count);
        printf("║ Last System Karma:     %-10ld                    ║\n",
               g_last_state.system_karma);
    }

    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
}
