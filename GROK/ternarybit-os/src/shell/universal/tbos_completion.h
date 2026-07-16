/**
 * @file tbos_completion.h
 * @brief TBOS Tab Completion Engine
 *
 * Provides context-aware completion for:
 * - Shell commands
 * - File and directory paths
 * - Command-specific arguments
 * - Environment variables
 */

#ifndef TBOS_COMPLETION_H
#define TBOS_COMPLETION_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * COMPLETION TYPES
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Completion context types
 */
typedef enum {
    COMPLETE_COMMAND,    /* Shell command names */
    COMPLETE_PATH,       /* File/directory paths */
    COMPLETE_VARIABLE,   /* Environment variables ($VAR) */
    COMPLETE_OPTION,     /* Command options (--help, -v) */
    COMPLETE_ARGUMENT,   /* Command-specific arguments */
    COMPLETE_CUSTOM      /* Custom provider completions */
} tbos_complete_type_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * COMPLETION RESULT
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Completion result structure
 */
typedef struct {
    char** matches;         /* Array of matching strings */
    size_t count;           /* Number of matches */
    size_t selected;        /* Currently selected index (for cycling) */
    char* common_prefix;    /* Longest common prefix of all matches */
    tbos_complete_type_t type;  /* Type of completion */
    size_t replace_start;   /* Start position in line to replace */
    size_t replace_len;     /* Length of text to replace */
} tbos_completions_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * COMPLETION PROVIDER CALLBACK
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Custom completion provider function
 *
 * @param cmd      Command name (first word)
 * @param partial  Partial text being completed
 * @param argnum   Argument number (0 = first arg after command)
 * @return Completion result (caller frees)
 */
typedef tbos_completions_t* (*tbos_complete_provider_t)(
    const char* cmd,
    const char* partial,
    int argnum
);

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN COMPLETION API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize completion engine
 */
void tbos_complete_init(void);

/**
 * @brief Shutdown completion engine
 */
void tbos_complete_shutdown(void);

/**
 * @brief Complete at cursor position
 *
 * Determines context (command, path, etc.) and returns matches.
 *
 * @param line    Full line being edited
 * @param cursor  Cursor position in line
 * @return Completion result (caller must free with tbos_completions_free)
 */
tbos_completions_t* tbos_complete(const char* line, size_t cursor);

/**
 * @brief Free completion result
 *
 * @param c  Completion result to free
 */
void tbos_completions_free(tbos_completions_t* c);

/**
 * @brief Get next match (cycle forward)
 *
 * @param c  Completion result
 * @return Next match string or NULL if empty
 */
const char* tbos_complete_next(tbos_completions_t* c);

/**
 * @brief Get previous match (cycle backward)
 *
 * @param c  Completion result
 * @return Previous match string or NULL if empty
 */
const char* tbos_complete_prev(tbos_completions_t* c);

/**
 * @brief Get currently selected match
 *
 * @param c  Completion result
 * @return Selected match or NULL
 */
const char* tbos_complete_selected(const tbos_completions_t* c);

/* ═══════════════════════════════════════════════════════════════════════════
 * PROVIDER REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Register custom completion provider for a command
 *
 * @param cmd       Command name
 * @param provider  Provider function
 * @return 0 on success, -1 on error
 */
int tbos_complete_register(const char* cmd, tbos_complete_provider_t provider);

/**
 * @brief Unregister completion provider
 *
 * @param cmd  Command name
 */
void tbos_complete_unregister(const char* cmd);

/* ═══════════════════════════════════════════════════════════════════════════
 * BUILT-IN COMPLETERS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Complete shell command names
 *
 * @param partial  Partial command name
 * @return Completion result
 */
tbos_completions_t* tbos_complete_commands(const char* partial);

/**
 * @brief Complete file/directory paths
 *
 * @param partial  Partial path
 * @return Completion result
 */
tbos_completions_t* tbos_complete_paths(const char* partial);

/**
 * @brief Complete environment variables
 *
 * @param partial  Partial variable name (without $)
 * @return Completion result
 */
tbos_completions_t* tbos_complete_variables(const char* partial);

/* ═══════════════════════════════════════════════════════════════════════════
 * DISPLAY HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Display completion matches in columns
 *
 * @param c           Completion result
 * @param term_width  Terminal width for column formatting
 */
void tbos_complete_display(const tbos_completions_t* c, int term_width);

/**
 * @brief Check if completions should be displayed
 *
 * Returns true if there are multiple matches that can't be
 * auto-completed to a common prefix.
 *
 * @param c  Completion result
 * @return true if display needed
 */
bool tbos_complete_needs_display(const tbos_completions_t* c);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_COMPLETION_H */
