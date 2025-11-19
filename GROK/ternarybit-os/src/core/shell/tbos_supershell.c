#include "tbos_supershell.h"
#include "tbos_shell.h"
#include "hal/tbos_hal.h"
#include "../lifecycle/tbos_lifecycle.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TBOS_MAX_SUPERSHELL_MODES 16

static tbos_supershell_mode_t g_modes[TBOS_MAX_SUPERSHELL_MODES];
static size_t g_mode_count = 0;
static const tbos_supershell_mode_t* g_active_mode = NULL;
static int g_initialized = 0;
static char g_pending_persona[64];
static int g_pending_request = 0;

static int tbos_supershell_base_entry(const char* persona) {
    (void)persona;
    printf("[Supershell] Delegating to base conscious shell...\n");
    return tbos_shell_run();
}

static int tbos_supershell_register_internal(const tbos_supershell_mode_t* mode) {
    if (!mode || !mode->persona_id || !mode->entry) {
        return -1;
    }
    if (g_mode_count >= TBOS_MAX_SUPERSHELL_MODES) {
        return -1;
    }
    g_modes[g_mode_count++] = *mode;
    return 0;
}

int tbos_supershell_request_morph(const char* persona_id) {
    if (!persona_id || persona_id[0] == '\0') {
        return -1;
    }
    strncpy(g_pending_persona, persona_id, sizeof(g_pending_persona) - 1);
    g_pending_persona[sizeof(g_pending_persona) - 1] = '\0';
    g_pending_request = 1;
    return 0;
}

static int tbos_supershell_has_pending(void) {
    return g_pending_request;
}

static void tbos_supershell_clear_pending(void) {
    g_pending_request = 0;
    g_pending_persona[0] = '\0';
}

int tbos_supershell_register(const tbos_supershell_mode_t* mode) {
    return tbos_supershell_register_internal(mode);
}

static const tbos_supershell_mode_t* tbos_supershell_find(const char* persona_id) {
    if (!persona_id) return NULL;
    for (size_t i = 0; i < g_mode_count; ++i) {
        if (strcmp(g_modes[i].persona_id, persona_id) == 0) {
            return &g_modes[i];
        }
    }
    return NULL;
}

int tbos_supershell_init(void) {
    if (g_initialized) {
        return 0;
    }
    const tbos_supershell_mode_t defaults[] = {
        {.persona_id = "calc_4k", .description = "Calculator persona shell", .entry = tbos_supershell_base_entry},
        {.persona_id = "mobile_64k", .description = "Mobile persona shell", .entry = tbos_supershell_base_entry},
        {.persona_id = "desktop_512m", .description = "Desktop persona shell", .entry = tbos_supershell_base_entry},
        {.persona_id = "super_1g", .description = "Supercomputer persona shell", .entry = tbos_supershell_base_entry},
    };
    for (size_t i = 0; i < sizeof(defaults) / sizeof(defaults[0]); ++i) {
        tbos_supershell_register_internal(&defaults[i]);
    }
    tbos_supershell_register_packages();
    g_initialized = 1;
    return 0;
}

int tbos_supershell_launch(const char* persona_id) {
    char current_persona[64];
    if (persona_id && persona_id[0] != '\0') {
        strncpy(current_persona, persona_id, sizeof(current_persona) - 1);
        current_persona[sizeof(current_persona) - 1] = '\0';
    } else {
        strncpy(current_persona, "desktop_512m", sizeof(current_persona) - 1);
        current_persona[sizeof(current_persona) - 1] = '\0';
    }

    while (1) {
        const tbos_supershell_mode_t* mode = tbos_supershell_find(current_persona);
        if (!mode) {
            printf("[Supershell] Persona '%s' not registered; falling back to base shell.\n",
                   current_persona);
            static tbos_supershell_mode_t fallback;
            fallback.persona_id = current_persona;
            fallback.description = "Base conscious shell";
            fallback.entry = tbos_supershell_base_entry;
            mode = &fallback;
        }

        g_active_mode = mode;
        printf("[Supershell] Morphing into persona '%s' (%s)\n",
               mode->persona_id ? mode->persona_id : "unknown",
               mode->description ? mode->description : "no description");

        hal_hardware_info_t info;
        memset(&info, 0, sizeof(info));
        hal_detect_hardware(&info);
        if (mode->persona_id) {
            setenv("TBOS_PERSONA_NAME", mode->persona_id, 1);
        }
        tbos_lifecycle_genesis(mode->persona_id, &info);

        tbos_supershell_clear_pending();
        mode->entry(mode->persona_id);

        const char* rebirth_persona = NULL;
        const char* rebirth_reason = NULL;

        if (tbos_supershell_has_pending()) {
            strncpy(current_persona, g_pending_persona, sizeof(current_persona) - 1);
            current_persona[sizeof(current_persona) - 1] = '\0';
            tbos_supershell_clear_pending();
            continue;
        } else if (tbos_lifecycle_rebirth_pending(&rebirth_persona, &rebirth_reason) &&
                   rebirth_persona && rebirth_persona[0] != '\0') {
            printf("[Lifecycle] %s\n", rebirth_reason ? rebirth_reason : "Rebirth requested");
            strncpy(current_persona, rebirth_persona, sizeof(current_persona) - 1);
            current_persona[sizeof(current_persona) - 1] = '\0';
            tbos_lifecycle_clear_rebirth();
            continue;
        }
        break;
    }

    return 0;
}

void tbos_supershell_list(void) {
    printf("Supershell modes (%zu registered):\n", g_mode_count);
    for (size_t i = 0; i < g_mode_count; ++i) {
        printf("  - %s : %s\n",
               g_modes[i].persona_id ? g_modes[i].persona_id : "(unnamed)",
               g_modes[i].description ? g_modes[i].description : "");
    }
}

const tbos_supershell_mode_t* tbos_supershell_active(void) {
    return g_active_mode;
}
