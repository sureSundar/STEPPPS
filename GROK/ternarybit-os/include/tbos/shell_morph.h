/**
 * @file shell_morph.h
 * @brief TernaryBit OS - Shell Morphing Framework
 *
 * Allows TBOS to morph into different shells (sh, bash, zsh, cmd, etc.)
 * while maintaining TBOS core features and VFS layer.
 *
 * Architecture:
 * - Core TBOS shell (always available)
 * - Pluggable shell interpreters
 * - Shared VFS/filesystem layer
 * - Seamless switching with ch-sh command
 */

#ifndef TBOS_SHELL_MORPH_H
#define TBOS_SHELL_MORPH_H

#include <stddef.h>
#include <stdbool.h>

/* Shell types supported */
typedef enum {
    SHELL_TBOS = 0,      /* TernaryBit OS native shell */
    SHELL_SH,            /* POSIX sh (Bourne shell) */
    SHELL_BASH,          /* Bourne Again Shell */
    SHELL_ZSH,           /* Z Shell */
    SHELL_KSH,           /* Korn Shell */
    SHELL_TCSH,          /* TC Shell */
    SHELL_DASH,          /* Debian Almquist Shell */
    SHELL_CMD,           /* Windows CMD emulation */
    SHELL_POWERSHELL,    /* PowerShell emulation */
    SHELL_FISH,          /* Friendly Interactive Shell */
    SHELL_MAX
} shell_type_t;

/* Shell interpreter interface */
typedef struct shell_interpreter {
    const char* name;              /* Shell name (e.g., "sh", "bash") */
    const char* prompt;            /* Default prompt (e.g., "$ ", "bash$ ") */
    shell_type_t type;             /* Shell type ID */

    /* Lifecycle callbacks */
    int (*init)(void);             /* Initialize shell interpreter */
    void (*cleanup)(void);         /* Cleanup on exit */

    /* Command execution */
    int (*execute)(const char* cmdline);  /* Execute command line */
    int (*parse)(const char* cmdline, char** argv, int* argc);  /* Parse command */

    /* Features */
    bool supports_pipes;           /* Supports | pipes */
    bool supports_redirection;     /* Supports > < >> */
    bool supports_variables;       /* Supports $VAR */
    bool supports_scripting;       /* Supports script files */
    bool supports_job_control;     /* Supports bg/fg/jobs */
} shell_interpreter_t;

/* Shell morphing API */

/**
 * Initialize shell morphing framework
 */
void shell_morph_init(void);

/**
 * Register a shell interpreter
 * @param interp Shell interpreter to register
 * @return 0 on success, negative on error
 */
int shell_morph_register(const shell_interpreter_t* interp);

/**
 * Switch to a different shell
 * @param shell_name Name of shell to switch to (e.g., "sh", "bash")
 * @return 0 on success, negative on error
 */
int shell_morph_switch(const char* shell_name);

/**
 * Get current active shell
 * @return Pointer to current shell interpreter
 */
const shell_interpreter_t* shell_morph_current(void);

/**
 * List available shells
 * @param buffer Buffer to write list to
 * @param buflen Buffer length
 * @return 0 on success, negative on error
 */
int shell_morph_list(char* buffer, size_t buflen);

/**
 * Execute command in current shell
 * @param cmdline Command line to execute
 * @return Exit code
 */
int shell_morph_execute(const char* cmdline);

/* Built-in shell interpreters */
extern const shell_interpreter_t shell_tbos_interpreter;
extern const shell_interpreter_t shell_sh_interpreter;
extern const shell_interpreter_t shell_bash_interpreter;

#endif /* TBOS_SHELL_MORPH_H */
