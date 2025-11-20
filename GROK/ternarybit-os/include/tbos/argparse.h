/**
 * @file argparse.h
 * @brief POSIX-compatible argument parser for TBOS commands
 *
 * Provides getopt-style argument parsing with support for:
 * - Short flags: -l, -a, -h
 * - Combined flags: -lah
 * - Flags with values: -n 10
 * - Long options: --help, --version
 * - Positional arguments
 * - Standard help generation
 */

#ifndef TBOS_ARGPARSE_H
#define TBOS_ARGPARSE_H

#include <stddef.h>
#include <stdbool.h>

/* Maximum arguments that can be parsed */
#define MAX_PARSED_ARGS 32
#define MAX_POSITIONAL_ARGS 16
#define MAX_FLAG_LEN 32
#define MAX_VALUE_LEN 256

/**
 * Parsed argument structure
 */
typedef struct {
    char flag[MAX_FLAG_LEN];        /* Flag name (e.g., "l", "help", "n") */
    bool has_value;                  /* Whether flag has an associated value */
    char value[MAX_VALUE_LEN];      /* Value if has_value is true */
    bool is_long;                    /* true for --flag, false for -f */
} parsed_arg_t;

/**
 * Argument parser result
 */
typedef struct {
    parsed_arg_t flags[MAX_PARSED_ARGS];      /* Parsed flags */
    int flag_count;                            /* Number of flags */

    char* positional[MAX_POSITIONAL_ARGS];    /* Positional arguments */
    int positional_count;                      /* Number of positional args */

    int error;                                 /* 0 on success, error code otherwise */
    char error_msg[128];                       /* Error message if error != 0 */
} argparse_result_t;

/**
 * Flag specification for command
 */
typedef struct {
    char short_flag;        /* Short flag character (e.g., 'l' for -l) */
    const char* long_flag;  /* Long flag name (e.g., "help" for --help) */
    bool takes_value;       /* Whether flag requires a value */
    const char* description;/* Help text for this flag */
} flag_spec_t;

/**
 * Parse command arguments
 *
 * @param args Raw argument string from shell
 * @param result Output structure to store parsed results
 * @return 0 on success, -1 on error
 */
int argparse_parse(const char* args, argparse_result_t* result);

/**
 * Check if a flag was provided
 *
 * @param result Parsed result structure
 * @param flag_name Flag to check (e.g., "l", "help")
 * @return true if flag was provided
 */
bool argparse_has_flag(const argparse_result_t* result, const char* flag_name);

/**
 * Get value of a flag
 *
 * @param result Parsed result structure
 * @param flag_name Flag name
 * @return Value string, or NULL if flag not found or has no value
 */
const char* argparse_get_value(const argparse_result_t* result, const char* flag_name);

/**
 * Get positional argument by index
 *
 * @param result Parsed result structure
 * @param index Index of positional argument (0-based)
 * @return Positional argument string, or NULL if index out of range
 */
const char* argparse_get_positional(const argparse_result_t* result, int index);

/**
 * Print help message for command
 *
 * @param cmd_name Command name
 * @param description Command description
 * @param usage Usage string (e.g., "[OPTIONS] FILE")
 * @param specs Array of flag specifications
 * @param spec_count Number of flag specs
 */
void argparse_print_help(const char* cmd_name,
                        const char* description,
                        const char* usage,
                        const flag_spec_t* specs,
                        int spec_count);

/**
 * Initialize result structure
 *
 * @param result Result structure to initialize
 */
void argparse_init(argparse_result_t* result);

/**
 * Free any dynamically allocated memory in result
 *
 * @param result Result structure to clean up
 */
void argparse_cleanup(argparse_result_t* result);

/* Error codes */
#define ARGPARSE_SUCCESS        0
#define ARGPARSE_ERROR_UNKNOWN_FLAG   -1
#define ARGPARSE_ERROR_MISSING_VALUE  -2
#define ARGPARSE_ERROR_TOO_MANY_ARGS  -3
#define ARGPARSE_ERROR_INVALID_SYNTAX -4

#endif /* TBOS_ARGPARSE_H */
