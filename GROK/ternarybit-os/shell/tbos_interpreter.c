/**
 * @file tbos_interpreter.c
 * @brief TernaryBit OS Native Shell Interpreter
 *
 * Wraps the TBOS native shell as a shell_interpreter_t for the morphing framework
 */

#include "tbos/shell_morph.h"
#include "tbos/libc.h"

/* External kernel functions */
extern void kernel_print(const char* str);
extern int shell_execute_command(const char* cmdline);

/* Forward declarations */
static int tbos_init(void);
static void tbos_cleanup(void);
static int tbos_execute(const char* cmdline);
static int tbos_parse(const char* cmdline, char** argv, int* argc);

/* The TBOS interpreter instance */
const shell_interpreter_t shell_tbos_interpreter = {
    .name = "tbos",
    .prompt = "tbos> ",
    .type = SHELL_TBOS,
    .init = tbos_init,
    .cleanup = tbos_cleanup,
    .execute = tbos_execute,
    .parse = tbos_parse,
    .supports_pipes = true,
    .supports_redirection = true,
    .supports_variables = true,
    .supports_scripting = true,
    .supports_job_control = false
};

static int tbos_init(void) {
    kernel_print("TernaryBit OS Shell v3.0 - Revolutionary Bootloader Edition\n");
    kernel_print("Type 'help' for command list or 'ch-sh sh' to switch to POSIX sh\n");
    return 0;
}

static void tbos_cleanup(void) {
    /* Nothing to clean up for TBOS native shell */
    return;
}

static int tbos_execute(const char* cmdline) {
    /* Delegate to TBOS native command execution */
    return shell_execute_command(cmdline);
}

static int tbos_parse(const char* cmdline, char** argv, int* argc) {
    if (!cmdline || !argv || !argc) return -1;

    *argc = 0;
    static char buffer[1024];
    strncpy(buffer, cmdline, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* p = buffer;

    while (*p && *argc < 63) {
        /* Skip leading whitespace */
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        /* Start of argument */
        argv[(*argc)++] = p;

        /* Find end of argument */
        while (*p && *p != ' ' && *p != '\t') p++;

        if (*p) {
            *p = '\0';
            p++;
        }
    }

    argv[*argc] = NULL;
    return 0;
}
