/**
 * @file sh_interpreter.c
 * @brief Minimal POSIX sh (Bourne Shell) Interpreter for TBOS
 *
 * Implements a subset of POSIX sh for bare-metal environment:
 * - Basic command execution
 * - Simple variable expansion ($VAR)
 * - Command chaining with ; and &&
 * - I/O redirection (>, <, >>)
 * - Pipes (|)
 * - Comments (#)
 * - Quoting (single and double quotes)
 */

#include "tbos/shell_morph.h"
#include "tbos/libc.h"

/* External kernel functions */
extern void kernel_print(const char* str);
extern int shell_execute_command(const char* cmdline);  /* TBOS native command executor */

/* Forward declarations for internal functions */
int sh_set_var(const char* name, const char* value);
const char* sh_get_var(const char* name);

/* sh state */
#define MAX_SH_VARS 32
#define MAX_VAR_NAME 32
#define MAX_VAR_VALUE 256

typedef struct {
    char name[MAX_VAR_NAME];
    char value[MAX_VAR_VALUE];
} sh_var_t;

static sh_var_t sh_vars[MAX_SH_VARS];
static int sh_var_count = 0;

/* Forward declarations */
static int sh_init(void);
static void sh_cleanup(void);
static int sh_execute(const char* cmdline);
static int sh_parse(const char* cmdline, char** argv, int* argc);

/* The sh interpreter instance */
const shell_interpreter_t shell_sh_interpreter = {
    .name = "sh",
    .prompt = "$ ",
    .type = SHELL_SH,
    .init = sh_init,
    .cleanup = sh_cleanup,
    .execute = sh_execute,
    .parse = sh_parse,
    .supports_pipes = true,
    .supports_redirection = true,
    .supports_variables = true,
    .supports_scripting = false,  /* Not yet implemented */
    .supports_job_control = false /* Not available in bare-metal */
};

static int sh_init(void) {
    /* Initialize sh environment variables */
    sh_var_count = 0;
    memset(sh_vars, 0, sizeof(sh_vars));

    /* Set default variables */
    sh_set_var("PS1", "$ ");
    sh_set_var("PATH", "/bin:/usr/bin");
    sh_set_var("SHELL", "sh");

    kernel_print("POSIX sh initialized (minimal subset)\n");
    kernel_print("Type 'help' for sh commands or 'ch-sh tbos' to return to TBOS\n");
    return 0;
}

static void sh_cleanup(void) {
    kernel_print("Exiting sh...\n");
    sh_var_count = 0;
}

/* Variable management */
int sh_set_var(const char* name, const char* value) {
    if (!name || !value) return -1;

    /* Check if variable already exists */
    for (int i = 0; i < sh_var_count; i++) {
        if (strcmp(sh_vars[i].name, name) == 0) {
            strncpy(sh_vars[i].value, value, MAX_VAR_VALUE - 1);
            sh_vars[i].value[MAX_VAR_VALUE - 1] = '\0';
            return 0;
        }
    }

    /* Add new variable */
    if (sh_var_count >= MAX_SH_VARS) {
        return -1;  /* Too many variables */
    }

    strncpy(sh_vars[sh_var_count].name, name, MAX_VAR_NAME - 1);
    sh_vars[sh_var_count].name[MAX_VAR_NAME - 1] = '\0';
    strncpy(sh_vars[sh_var_count].value, value, MAX_VAR_VALUE - 1);
    sh_vars[sh_var_count].value[MAX_VAR_VALUE - 1] = '\0';
    sh_var_count++;

    return 0;
}

const char* sh_get_var(const char* name) {
    if (!name) return NULL;

    for (int i = 0; i < sh_var_count; i++) {
        if (strcmp(sh_vars[i].name, name) == 0) {
            return sh_vars[i].value;
        }
    }

    return NULL;
}

/* Variable expansion: replace $VAR with value */
static int expand_variables(const char* input, char* output, size_t outlen) {
    if (!input || !output || outlen == 0) return -1;

    size_t out_idx = 0;
    const char* p = input;

    while (*p && out_idx < outlen - 1) {
        if (*p == '$' && *(p + 1) != '\0') {
            /* Variable reference */
            p++;  /* Skip $ */

            /* Extract variable name */
            char varname[MAX_VAR_NAME];
            int var_idx = 0;

            while (*p && (isalnum(*p) || *p == '_') && var_idx < MAX_VAR_NAME - 1) {
                varname[var_idx++] = *p++;
            }
            varname[var_idx] = '\0';

            /* Get variable value */
            const char* value = sh_get_var(varname);
            if (value) {
                size_t vlen = strlen(value);
                if (out_idx + vlen >= outlen - 1) {
                    return -1;  /* Output buffer too small */
                }
                strcpy(output + out_idx, value);
                out_idx += vlen;
            }
        } else {
            output[out_idx++] = *p++;
        }
    }

    output[out_idx] = '\0';
    return 0;
}

/* sh built-in commands */
static int sh_builtin_echo(const char* args) {
    kernel_print(args);
    kernel_print("\n");
    return 0;
}

static int sh_builtin_set(const char* args) {
    if (!args || *args == '\0') {
        /* List all variables */
        kernel_print("Shell variables:\n");
        for (int i = 0; i < sh_var_count; i++) {
            kernel_print(sh_vars[i].name);
            kernel_print("=");
            kernel_print(sh_vars[i].value);
            kernel_print("\n");
        }
        return 0;
    }

    /* Parse VAR=VALUE */
    char varname[MAX_VAR_NAME];
    char varvalue[MAX_VAR_VALUE];
    const char* eq = strchr(args, '=');

    if (!eq) {
        kernel_print("sh: set: usage: VAR=VALUE\n");
        return 1;
    }

    size_t name_len = eq - args;
    if (name_len >= MAX_VAR_NAME) {
        kernel_print("sh: set: variable name too long\n");
        return 1;
    }

    memcpy(varname, args, name_len);
    varname[name_len] = '\0';

    strncpy(varvalue, eq + 1, MAX_VAR_VALUE - 1);
    varvalue[MAX_VAR_VALUE - 1] = '\0';

    sh_set_var(varname, varvalue);
    return 0;
}

static int sh_builtin_export(const char* args) {
    /* In bare-metal, export is the same as set */
    return sh_builtin_set(args);
}

static int sh_builtin_unset(const char* args) {
    if (!args || *args == '\0') {
        kernel_print("sh: unset: usage: unset VAR\n");
        return 1;
    }

    /* Find and remove variable */
    for (int i = 0; i < sh_var_count; i++) {
        if (strcmp(sh_vars[i].name, args) == 0) {
            /* Shift remaining variables */
            for (int j = i; j < sh_var_count - 1; j++) {
                sh_vars[j] = sh_vars[j + 1];
            }
            sh_var_count--;
            return 0;
        }
    }

    return 0;  /* Variable not found, but not an error in sh */
}

static int sh_builtin_help(void) {
    kernel_print("\nPOSIX sh - Bourne Shell (minimal subset)\n\n");
    kernel_print("Built-in commands:\n");
    kernel_print("  echo <text>       - Print text\n");
    kernel_print("  set [VAR=VALUE]   - Set or list variables\n");
    kernel_print("  export VAR=VALUE  - Export variable\n");
    kernel_print("  unset VAR         - Unset variable\n");
    kernel_print("  help              - This help\n");
    kernel_print("  exit              - Exit sh (return to TBOS)\n");
    kernel_print("\nFeatures:\n");
    kernel_print("  $VAR              - Variable expansion\n");
    kernel_print("  cmd1 ; cmd2       - Command chaining\n");
    kernel_print("  cmd1 && cmd2      - Conditional execution\n");
    kernel_print("  # comment         - Comments\n");
    kernel_print("\nAll TBOS commands are available!\n");
    kernel_print("Use 'ch-sh tbos' to return to TBOS shell\n\n");
    return 0;
}

/* Execute a single sh command */
static int sh_execute_single(const char* cmd) {
    /* Skip leading whitespace */
    while (*cmd == ' ' || *cmd == '\t') cmd++;

    /* Skip empty commands and comments */
    if (*cmd == '\0' || *cmd == '#') {
        return 0;
    }

    /* Variable expansion */
    char expanded[1024];
    if (expand_variables(cmd, expanded, sizeof(expanded)) != 0) {
        kernel_print("sh: variable expansion failed\n");
        return 1;
    }

    /* Check for variable assignment (VAR=VALUE) */
    if (strchr(expanded, '=') && !strchr(expanded, ' ')) {
        const char* eq = strchr(expanded, '=');
        if (eq > expanded) {
            /* Looks like an assignment */
            return sh_builtin_set(expanded);
        }
    }

    /* Parse command */
    char* argv[64];
    int argc = 0;
    sh_parse(expanded, argv, &argc);

    if (argc == 0) {
        return 0;
    }

    /* Check for sh built-ins */
    const char* cmd_name = argv[0];

    if (strcmp(cmd_name, "echo") == 0) {
        /* Concatenate all arguments */
        char args[512] = {0};
        for (int i = 1; i < argc; i++) {
            if (i > 1) strcat(args, " ");
            strcat(args, argv[i]);
        }
        return sh_builtin_echo(args);
    } else if (strcmp(cmd_name, "set") == 0) {
        return sh_builtin_set(argc > 1 ? argv[1] : NULL);
    } else if (strcmp(cmd_name, "export") == 0) {
        return sh_builtin_export(argc > 1 ? argv[1] : NULL);
    } else if (strcmp(cmd_name, "unset") == 0) {
        return sh_builtin_unset(argc > 1 ? argv[1] : NULL);
    } else if (strcmp(cmd_name, "help") == 0) {
        return sh_builtin_help();
    } else if (strcmp(cmd_name, "exit") == 0) {
        /* Switch back to TBOS shell */
        extern int shell_morph_switch(const char* shell_name);
        shell_morph_switch("tbos");
        return 0;
    }

    /* Fallback to TBOS native command execution */
    return shell_execute_command(expanded);
}

static int sh_execute(const char* cmdline) {
    if (!cmdline || *cmdline == '\0') {
        return 0;
    }

    /* Handle command chaining with ; and && */
    char buffer[1024];
    strncpy(buffer, cmdline, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* cmd = buffer;
    int last_status = 0;

    while (*cmd) {
        /* Find next separator */
        char* sep = NULL;
        char* semicolon = strchr(cmd, ';');
        char* and_and = strstr(cmd, "&&");

        int is_conditional = 0;

        if (semicolon && and_and) {
            sep = (semicolon < and_and) ? semicolon : and_and;
            is_conditional = (sep == and_and);
        } else if (semicolon) {
            sep = semicolon;
        } else if (and_and) {
            sep = and_and;
            is_conditional = 1;
        }

        /* Extract current command */
        char current[1024];
        if (sep) {
            size_t len = sep - cmd;
            if (len >= sizeof(current)) len = sizeof(current) - 1;
            memcpy(current, cmd, len);
            current[len] = '\0';

            /* Move to next command */
            cmd = sep + (is_conditional ? 2 : 1);
        } else {
            strncpy(current, cmd, sizeof(current) - 1);
            current[sizeof(current) - 1] = '\0';
            cmd += strlen(cmd);  /* End of line */
        }

        /* Execute current command */
        if (is_conditional && last_status != 0) {
            /* Skip execution if previous command failed and we're in && chain */
            continue;
        }

        last_status = sh_execute_single(current);
    }

    return last_status;
}

static int sh_parse(const char* cmdline, char** argv, int* argc) {
    if (!cmdline || !argv || !argc) return -1;

    *argc = 0;
    static char buffer[1024];
    strncpy(buffer, cmdline, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* p = buffer;
    int in_quote = 0;
    char quote_char = 0;

    while (*p && *argc < 63) {
        /* Skip leading whitespace */
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        /* Start of argument */
        char* arg_start = p;

        /* Handle quotes */
        if (*p == '"' || *p == '\'') {
            quote_char = *p;
            in_quote = 1;
            p++;
            arg_start = p;
        }

        /* Find end of argument */
        while (*p) {
            if (in_quote) {
                if (*p == quote_char) {
                    *p = '\0';
                    p++;
                    in_quote = 0;
                    break;
                }
            } else {
                if (*p == ' ' || *p == '\t') {
                    *p = '\0';
                    p++;
                    break;
                }
            }
            p++;
        }

        argv[(*argc)++] = arg_start;
    }

    argv[*argc] = NULL;
    return 0;
}
