/**
 * @file argparse.c
 * @brief POSIX-compatible argument parser implementation
 */

#include "tbos/argparse.h"
#include "tbos/libc.h"

/* External kernel function */
extern void kernel_print(const char* str);

void argparse_init(argparse_result_t* result) {
    if (!result) return;

    memset(result, 0, sizeof(*result));
    result->flag_count = 0;
    result->positional_count = 0;
    result->error = ARGPARSE_SUCCESS;
    result->error_msg[0] = '\0';
}

void argparse_cleanup(argparse_result_t* result) {
    /* Nothing to free in bare-metal implementation */
    /* Positional args point to static buffer */
    (void)result;
}

static bool is_flag(const char* arg) {
    return arg && arg[0] == '-' && arg[1] != '\0';
}

static bool is_long_flag(const char* arg) {
    return arg && arg[0] == '-' && arg[1] == '-' && arg[2] != '\0';
}

static int add_flag(argparse_result_t* result, const char* flag_name,
                   bool is_long, bool has_value, const char* value) {
    if (result->flag_count >= MAX_PARSED_ARGS) {
        result->error = ARGPARSE_ERROR_TOO_MANY_ARGS;
        strncpy(result->error_msg, "Too many arguments", sizeof(result->error_msg) - 1);
        return -1;
    }

    parsed_arg_t* arg = &result->flags[result->flag_count++];
    strncpy(arg->flag, flag_name, MAX_FLAG_LEN - 1);
    arg->flag[MAX_FLAG_LEN - 1] = '\0';
    arg->is_long = is_long;
    arg->has_value = has_value;

    if (has_value && value) {
        strncpy(arg->value, value, MAX_VALUE_LEN - 1);
        arg->value[MAX_VALUE_LEN - 1] = '\0';
    } else {
        arg->value[0] = '\0';
    }

    return 0;
}

static int add_positional(argparse_result_t* result, char* arg) {
    if (result->positional_count >= MAX_POSITIONAL_ARGS) {
        result->error = ARGPARSE_ERROR_TOO_MANY_ARGS;
        strncpy(result->error_msg, "Too many positional arguments", sizeof(result->error_msg) - 1);
        return -1;
    }

    result->positional[result->positional_count++] = arg;
    return 0;
}

int argparse_parse(const char* args, argparse_result_t* result) {
    if (!result) return -1;

    argparse_init(result);

    if (!args || !*args) {
        return 0;  /* Empty args is valid */
    }

    /* Make a mutable copy of args */
    static char buffer[1024];
    strncpy(buffer, args, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    /* Tokenize the argument string */
    char* tokens[MAX_PARSED_ARGS + MAX_POSITIONAL_ARGS];
    int token_count = 0;

    char* p = buffer;
    char* token_start = NULL;
    bool in_quote = false;
    char quote_char = '\0';

    while (*p) {
        /* Skip leading whitespace */
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        /* Start of token */
        token_start = p;

        /* Handle quotes */
        if (*p == '"' || *p == '\'') {
            quote_char = *p;
            in_quote = true;
            p++;
            token_start = p;  /* Start after quote */

            /* Find closing quote */
            while (*p && *p != quote_char) p++;
            if (*p == quote_char) {
                *p = '\0';  /* Terminate token */
                p++;
                in_quote = false;
            }
        } else {
            /* Regular token - find next space */
            while (*p && *p != ' ' && *p != '\t') p++;
            if (*p) {
                *p = '\0';
                p++;
            }
        }

        if (token_count < (int)(sizeof(tokens) / sizeof(tokens[0]))) {
            tokens[token_count++] = token_start;
        }
    }

    if (in_quote) {
        result->error = ARGPARSE_ERROR_INVALID_SYNTAX;
        strncpy(result->error_msg, "Unterminated quoted argument", sizeof(result->error_msg) - 1);
        result->error_msg[sizeof(result->error_msg) - 1] = '\0';
        return -1;
    }

    /* Parse tokens into flags and positional arguments */
    for (int i = 0; i < token_count; i++) {
        char* token = tokens[i];

        if (is_long_flag(token)) {
            /* Long flag: --flag or --flag=value */
            char* flag_name = token + 2;  /* Skip -- */
            char* eq = strchr(flag_name, '=');

            if (eq) {
                /* --flag=value */
                *eq = '\0';
                char* value = eq + 1;
                if (add_flag(result, flag_name, true, true, value) != 0) {
                    return -1;
                }
            } else {
                /* Check if next token is the value (not a flag) */
                bool has_value = false;
                char* value = NULL;

                if (i + 1 < token_count && !is_flag(tokens[i + 1])) {
                    has_value = true;
                    value = tokens[i + 1];
                    i++;  /* Skip next token */
                }

                if (add_flag(result, flag_name, true, has_value, value) != 0) {
                    return -1;
                }
            }
        } else if (is_flag(token) && !is_long_flag(token)) {
            /* Short flag(s): -l or -lah */
            char* flags = token + 1;  /* Skip - */

            /* Process each character as a separate flag */
            while (*flags) {
                char flag_name[2] = {*flags, '\0'};

                /* Check if this flag takes a value */
                /* For now, assume single-char flags don't take values unless they're the last one */
                bool might_take_value = (*(flags + 1) == '\0');  /* Last flag in group */
                bool has_value = false;
                char* value = NULL;

                if (might_take_value && i + 1 < token_count && !is_flag(tokens[i + 1])) {
                    has_value = true;
                    value = tokens[i + 1];
                    i++;  /* Skip next token */
                }

                if (add_flag(result, flag_name, false, has_value, value) != 0) {
                    return -1;
                }

                flags++;
            }
        } else {
            /* Positional argument */
            if (add_positional(result, token) != 0) {
                return -1;
            }
        }
    }

    return 0;
}

bool argparse_has_flag(const argparse_result_t* result, const char* flag_name) {
    if (!result || !flag_name) return false;

    for (int i = 0; i < result->flag_count; i++) {
        if (strcmp(result->flags[i].flag, flag_name) == 0) {
            return true;
        }
    }

    return false;
}

const char* argparse_get_value(const argparse_result_t* result, const char* flag_name) {
    if (!result || !flag_name) return NULL;

    for (int i = 0; i < result->flag_count; i++) {
        if (strcmp(result->flags[i].flag, flag_name) == 0) {
            if (result->flags[i].has_value) {
                return result->flags[i].value;
            }
            return NULL;
        }
    }

    return NULL;
}

const char* argparse_get_positional(const argparse_result_t* result, int index) {
    if (!result || index < 0 || index >= result->positional_count) {
        return NULL;
    }

    return result->positional[index];
}

void argparse_print_help(const char* cmd_name,
                        const char* description,
                        const char* usage,
                        const flag_spec_t* specs,
                        int spec_count) {
    kernel_print("\n");
    kernel_print(cmd_name);
    kernel_print(" - ");
    kernel_print(description);
    kernel_print("\n\nUsage: ");
    kernel_print(cmd_name);
    kernel_print(" ");
    kernel_print(usage);
    kernel_print("\n\nOptions:\n");

    for (int i = 0; i < spec_count; i++) {
        const flag_spec_t* spec = &specs[i];

        kernel_print("  ");

        /* Print short flag */
        if (spec->short_flag) {
            kernel_print("-");
            char flag_str[2] = {spec->short_flag, '\0'};
            kernel_print(flag_str);

            if (spec->takes_value) {
                kernel_print(" <value>");
            }

            if (spec->long_flag) {
                kernel_print(", ");
            } else {
                kernel_print("  ");
            }
        } else {
            kernel_print("      ");
        }

        /* Print long flag */
        if (spec->long_flag) {
            kernel_print("--");
            kernel_print(spec->long_flag);

            if (spec->takes_value) {
                kernel_print("=<value>");
            }
        }

        /* Align descriptions */
        kernel_print("\n      ");
        kernel_print(spec->description);
        kernel_print("\n");
    }

    kernel_print("\n");
}
