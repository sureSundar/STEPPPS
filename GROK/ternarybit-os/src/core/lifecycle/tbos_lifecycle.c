#define _POSIX_C_SOURCE 200809L

#include "tbos_lifecycle.h"
#include "../shell/tbos_shell.h"
#include "../steppps/tbos_steppps.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct {
    const char* persona;
    uint32_t max_session_sec;
    uint32_t idle_limit_sec;
    int32_t min_karma;
} tbos_lifecycle_policy_t;

static const tbos_lifecycle_policy_t g_policies[] = {
    { "calc_4k",        600,   90,   0  },
    { "mobile_64k",     1800,  180, -5  },
    { "desktop_512m",   3600,  240, -15 },
    { "super_1g",       5400,  360, -25 },
    { "network_sangha", 3600,  180, -10 },
    { "fs_universal",   3600,  240, -20 },
};

static const tbos_lifecycle_policy_t g_default_policy = {
    "default", 1800, 180, -25
};

static const tbos_lifecycle_policy_t* g_active_policy = NULL;
static char g_current_persona[64] = "default";
static uint64_t g_session_start = 0;
static uint64_t g_last_activity = 0;
static uint32_t g_command_counter = 0;
static int g_shutdown_forced = 0;
static char g_shutdown_reason[160];
static char g_rebirth_persona[64];
static char g_rebirth_reason[160];
static int g_rebirth_pending = 0;

static uint64_t lifecycle_now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec;
}

static const tbos_lifecycle_policy_t* lifecycle_select_policy(const char* persona) {
    if (!persona) {
        return &g_default_policy;
    }
    for (size_t i = 0; i < sizeof(g_policies)/sizeof(g_policies[0]); ++i) {
        if (strcmp(g_policies[i].persona, persona) == 0) {
            return &g_policies[i];
        }
    }
    return &g_default_policy;
}

static void lifecycle_emit_event(const char* description, steppps_event_type_t type, karma_score_t karma) {
    steppps_time_t now;
    steppps_time_get(&now);
    steppps_event_t event = {
        .event_id = 0,
        .type = type,
        .timestamp_us = now.uptime_us,
        .consciousness = CONSCIOUSNESS_AWARE,
        .karma_impact = karma
    };
    snprintf(event.description, sizeof(event.description), "%s", description);
    steppps_event_emit(&event);
}

void tbos_lifecycle_genesis(const char* persona_id, const hal_hardware_info_t* info) {
    if (persona_id && persona_id[0] != '\0') {
        strncpy(g_current_persona, persona_id, sizeof(g_current_persona) - 1);
        g_current_persona[sizeof(g_current_persona) - 1] = '\0';
    } else {
        strncpy(g_current_persona, "default", sizeof(g_current_persona) - 1);
        g_current_persona[sizeof(g_current_persona) - 1] = '\0';
    }

    g_active_policy = lifecycle_select_policy(g_current_persona);
    g_session_start = lifecycle_now_seconds();
    g_last_activity = g_session_start;
    g_command_counter = 0;
    g_shutdown_forced = 0;
    g_rebirth_pending = 0;
    g_shutdown_reason[0] = '\0';
    g_rebirth_persona[0] = '\0';
    g_rebirth_reason[0] = '\0';

    printf("[Lifecycle] Genesis persona=%s profile=%d RAM=%uB idle=%us session=%us\n",
           g_current_persona,
           info ? info->profile : HAL_PROFILE_STANDARD,
           info ? info->ram_bytes : 0,
           g_active_policy->idle_limit_sec,
           g_active_policy->max_session_sec);

    lifecycle_emit_event("Lifecycle genesis", STEPPPS_EVENT_SYSTEM, 5);
}

void tbos_lifecycle_record_command(const char* cmdline, int result) {
    (void)cmdline;
    (void)result;
    g_last_activity = lifecycle_now_seconds();
    g_command_counter++;
}

bool tbos_lifecycle_should_shutdown(const char** reason_out) {
    if (!g_active_policy) {
        return false;
    }

    if (g_shutdown_forced) {
        if (reason_out) {
            *reason_out = g_shutdown_reason;
        }
        return true;
    }

    const uint64_t now = lifecycle_now_seconds();

    if (g_active_policy->idle_limit_sec &&
        now >= g_last_activity + g_active_policy->idle_limit_sec) {
        snprintf(g_shutdown_reason, sizeof(g_shutdown_reason),
                 "Auto shutdown (%s idle for %lus)",
                 g_current_persona,
                 (unsigned long)(now - g_last_activity));
        g_shutdown_forced = 1;
    } else if (g_active_policy->max_session_sec &&
               now >= g_session_start + g_active_policy->max_session_sec) {
        snprintf(g_shutdown_reason, sizeof(g_shutdown_reason),
                 "Auto shutdown (%s session exceeded %lus)",
                 g_current_persona,
                 (unsigned long)g_active_policy->max_session_sec);
        g_shutdown_forced = 1;
    } else if (g_active_policy->min_karma != 0) {
        tbos_shell_session_t* session = tbos_shell_get_session();
        if (session && session->user_karma <= g_active_policy->min_karma) {
            snprintf(g_shutdown_reason, sizeof(g_shutdown_reason),
                     "Auto shutdown (%s karma %ld <= %d)",
                     g_current_persona,
                     session->user_karma,
                     g_active_policy->min_karma);
            g_shutdown_forced = 1;
        }
    }

    if (g_shutdown_forced) {
        lifecycle_emit_event(g_shutdown_reason, STEPPPS_EVENT_SYSTEM, -5);
        tbos_lifecycle_schedule_rebirth();
        if (reason_out) {
            *reason_out = g_shutdown_reason;
        }
        return true;
    }

    if (reason_out) {
        *reason_out = NULL;
    }
    return false;
}

void tbos_lifecycle_session_end(void) {
    if (!g_active_policy) {
        return;
    }
    tbos_shell_session_t* session = tbos_shell_get_session();
    printf("[Lifecycle] Session end persona=%s commands=%u karma=%ld\n",
           g_current_persona,
           g_command_counter,
           session ? session->user_karma : 0);
    lifecycle_emit_event("Lifecycle demise", STEPPPS_EVENT_SYSTEM, -1);
    g_active_policy = NULL;
}

static const char* lifecycle_rebirth_for_reason(const char* reason) {
    if (!reason) {
        return g_current_persona;
    }
    if (strstr(reason, "karma")) {
        return "network_sangha";
    }
    if (strstr(reason, "idle")) {
        return g_current_persona;
    }
    if (strstr(reason, "session")) {
        return "fs_universal";
    }
    return g_current_persona;
}

bool tbos_lifecycle_rebirth_pending(const char** persona_out, const char** reason_out) {
    if (!g_rebirth_pending) {
        if (persona_out) *persona_out = NULL;
        if (reason_out) *reason_out = NULL;
        return false;
    }
    if (persona_out) *persona_out = g_rebirth_persona;
    if (reason_out) *reason_out = g_rebirth_reason;
    return true;
}

void tbos_lifecycle_clear_rebirth(void) {
    g_rebirth_pending = 0;
    g_rebirth_persona[0] = '\0';
    g_rebirth_reason[0] = '\0';
}

void tbos_lifecycle_schedule_rebirth(void) {
    if (!g_shutdown_forced || g_rebirth_pending) {
        return;
    }
    const char* target = lifecycle_rebirth_for_reason(g_shutdown_reason);
    strncpy(g_rebirth_persona, target, sizeof(g_rebirth_persona) - 1);
    g_rebirth_persona[sizeof(g_rebirth_persona) - 1] = '\0';
    snprintf(g_rebirth_reason, sizeof(g_rebirth_reason),
             "Rebirth triggered after: %.100s", g_shutdown_reason);
    g_rebirth_pending = 1;
}
