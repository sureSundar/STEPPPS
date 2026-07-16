/**
 * @file tbos_shell_parser.c
 * @brief Command line parsing for TBOS shell
 */

#include "tbos_universal_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int universal_parse_command(const char* cmdline, int* argc, char*** argv) {
    if (!cmdline || !argc || !argv) {
        return -1;
    }

    *argc = 0;
    *argv = NULL;

    /* Allocate temporary argv array */
    char** args = calloc(UNIVERSAL_SHELL_MAX_ARGS + 1, sizeof(char*));
    if (!args) {
        return -1;
    }

    int count = 0;
    const char* p = cmdline;

    while (*p && count < UNIVERSAL_SHELL_MAX_ARGS) {
        /* Skip whitespace */
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        /* Parse token */
        char token[4096];
        size_t len = 0;
        char quote = '\0';
        bool escaped = false;

        while (*p) {
            if (escaped) {
                switch (*p) {
                    case 'n': token[len++] = '\n'; break;
                    case 't': token[len++] = '\t'; break;
                    case 'r': token[len++] = '\r'; break;
                    default:  token[len++] = *p;   break;
                }
                escaped = false;
                p++;
            } else if (*p == '\\') {
                escaped = true;
                p++;
            } else if (*p == quote) {
                quote = '\0';
                p++;
            } else if (!quote && (*p == '"' || *p == '\'')) {
                quote = *p++;
            } else if (!quote && (*p == ' ' || *p == '\t')) {
                break;
            } else {
                if (len < sizeof(token) - 1) {
                    token[len++] = *p;
                }
                p++;
            }
        }

        if (len > 0) {
            token[len] = '\0';
            args[count] = strdup(token);
            if (!args[count]) {
                for (int j = 0; j < count; j++) free(args[j]);
                free(args);
                return -1;
            }
            count++;
        }
    }

    if (count == 0) {
        free(args);
        args = NULL;
    } else {
        args[count] = NULL;
    }

    *argc = count;
    *argv = args;
    return 0;
}

void universal_free_argv(int argc, char** argv) {
    if (!argv) return;
    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);
}
