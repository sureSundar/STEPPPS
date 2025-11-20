/**
 * @file shell_morph.c
 * @brief Shell Morphing Framework Implementation
 */

#include "tbos/shell_morph.h"
#include "tbos/libc.h"

#define MAX_SHELLS 16

/* Global state */
static const shell_interpreter_t* g_shells[MAX_SHELLS];
static int g_shell_count = 0;
static const shell_interpreter_t* g_current_shell = NULL;

/* External kernel print function */
extern void kernel_print(const char* str);

void shell_morph_init(void) {
    g_shell_count = 0;
    g_current_shell = NULL;
    memset(g_shells, 0, sizeof(g_shells));
}

int shell_morph_register(const shell_interpreter_t* interp) {
    if (!interp || !interp->name || !interp->execute) {
        return -1;  /* Invalid interpreter */
    }

    if (g_shell_count >= MAX_SHELLS) {
        return -2;  /* Too many shells */
    }

    /* Check for duplicates */
    for (int i = 0; i < g_shell_count; i++) {
        if (strcmp(g_shells[i]->name, interp->name) == 0) {
            return -3;  /* Already registered */
        }
    }

    g_shells[g_shell_count++] = interp;

    /* If no current shell, make this the default */
    if (!g_current_shell) {
        g_current_shell = interp;
        if (interp->init) {
            interp->init();
        }
    }

    return 0;
}

int shell_morph_switch(const char* shell_name) {
    if (!shell_name) {
        return -1;
    }

    /* Find the requested shell */
    const shell_interpreter_t* new_shell = NULL;
    for (int i = 0; i < g_shell_count; i++) {
        if (strcmp(g_shells[i]->name, shell_name) == 0) {
            new_shell = g_shells[i];
            break;
        }
    }

    if (!new_shell) {
        return -2;  /* Shell not found */
    }

    /* Cleanup current shell */
    if (g_current_shell && g_current_shell->cleanup) {
        g_current_shell->cleanup();
    }

    /* Switch to new shell */
    g_current_shell = new_shell;

    /* Initialize new shell */
    if (g_current_shell->init) {
        g_current_shell->init();
    }

    return 0;
}

const shell_interpreter_t* shell_morph_current(void) {
    return g_current_shell;
}

int shell_morph_list(char* buffer, size_t buflen) {
    if (!buffer || buflen == 0) {
        return -1;
    }

    size_t offset = 0;
    for (int i = 0; i < g_shell_count; i++) {
        const char* name = g_shells[i]->name;
        size_t name_len = strlen(name);

        if (offset + name_len + 2 >= buflen) {
            break;  /* Not enough space */
        }

        memcpy(buffer + offset, name, name_len);
        offset += name_len;

        /* Add marker for current shell */
        if (g_shells[i] == g_current_shell) {
            buffer[offset++] = '*';
        }

        buffer[offset++] = ' ';
    }

    if (offset > 0) {
        buffer[offset - 1] = '\0';  /* Replace last space with null */
    } else {
        buffer[0] = '\0';
    }

    return 0;
}

int shell_morph_execute(const char* cmdline) {
    if (!g_current_shell || !g_current_shell->execute) {
        return -1;
    }

    return g_current_shell->execute(cmdline);
}
