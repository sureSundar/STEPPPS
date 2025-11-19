/**
 * @file tbos_baremetal_stubs.c
 * @brief Bare-metal stubs for TBOS components that normally use hosted features
 */

#include <stdint.h>
#include <stdbool.h>

/* Forward declarations for HAL types */
typedef struct hal_hardware_info hal_hardware_info_t;

/* Lifecycle stubs */
void tbos_lifecycle_genesis(const char* persona_id, const hal_hardware_info_t* info) {
    (void)persona_id;
    (void)info;
    /* No-op in bare-metal */
}

void tbos_lifecycle_record_command(const char* cmdline, int result) {
    (void)cmdline;
    (void)result;
    /* No-op in bare-metal */
}

bool tbos_lifecycle_should_shutdown(const char** reason_out) {
    (void)reason_out;
    return false; /* Never auto-shutdown in bare-metal */
}

void tbos_lifecycle_session_end(void) {
    /* No-op in bare-metal */
}

bool tbos_lifecycle_rebirth_pending(const char** persona_out, const char** reason_out) {
    (void)persona_out;
    (void)reason_out;
    return false; /* No rebirth in bare-metal */
}

void tbos_lifecycle_clear_rebirth(void) {
    /* No-op in bare-metal */
}

void tbos_lifecycle_schedule_rebirth(void) {
    /* No-op in bare-metal */
}

/* Karma ledger stubs */
typedef int karma_score_t;

void tbos_karma_ledger_init(void) {
    /* No-op in bare-metal */
}

void tbos_karma_ledger_log(const char* persona, karma_score_t delta, const char* reason) {
    (void)persona;
    (void)delta;
    (void)reason;
    /* No-op in bare-metal */
}

karma_score_t tbos_karma_ledger_get_total(const char* persona) {
    (void)persona;
    return 0; /* No persistence in bare-metal */
}

void tbos_karma_ledger_shutdown(void) {
    /* No-op in bare-metal */
}
