#ifndef TBOS_SHELL_H
#define TBOS_SHELL_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    SHELL_STATUS_OK = 0,
    SHELL_STATUS_UNKNOWN_COMMAND = 127,
    SHELL_STATUS_INVALID_ARGUMENT = 128,
    SHELL_STATUS_COMMAND_TOO_LONG = 129
} shell_status_t;

/* Initialize the shared TBOS command runtime. */
void shell_init(void);

/* Run the interactive keyboard/serial frontend. */
void shell_loop(void);

/*
 * Execute one command through the shared TBOS runtime.
 *
 * This entry point is intentionally independent of keyboard input so hosted
 * tests and the GUI Terminal can invoke the same command implementation.
 */
int shell_execute_command(const char* cmdline);

/* Read-only runtime state for status surfaces and tests. */
const char* shell_current_path(void);
uint32_t shell_commands_executed(void);
int32_t shell_user_karma(void);
uint8_t shell_consciousness_level(void);

#endif // TBOS_SHELL_H
