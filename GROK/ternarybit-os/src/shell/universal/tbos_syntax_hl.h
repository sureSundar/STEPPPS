/**
 * @file tbos_syntax_hl.h
 * @brief TBOS Syntax Highlighting
 *
 * Provides real-time syntax highlighting for shell input:
 * - Valid commands (green)
 * - Invalid commands (red)
 * - File paths (blue)
 * - Options/flags (magenta)
 * - Strings (yellow)
 * - Variables (cyan)
 * - Operators (bold white)
 */

#ifndef TBOS_SYNTAX_HL_H
#define TBOS_SYNTAX_HL_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * HIGHLIGHT TYPES
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Highlight category
 */
typedef enum {
    HL_DEFAULT = 0,  /* Normal text (white) */
    HL_COMMAND,      /* Valid command (green) */
    HL_INVALID,      /* Invalid command (red) */
    HL_ARGUMENT,     /* Arguments (default) */
    HL_PATH,         /* File paths (blue) */
    HL_STRING,       /* Quoted strings (yellow) */
    HL_OPTION,       /* Options like --help (magenta) */
    HL_VARIABLE,     /* $VARIABLES (cyan bright) */
    HL_OPERATOR,     /* | > < & ; (white bold) */
    HL_COMMENT,      /* # comments (gray) */
    HL_NUMBER,       /* Numbers (cyan) */
    HL_ERROR         /* Syntax errors (red bg) */
} tbos_hl_type_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * HIGHLIGHT SPAN
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief A span of highlighted text
 */
typedef struct {
    size_t start;         /* Start position (inclusive) */
    size_t end;           /* End position (exclusive) */
    tbos_hl_type_t type;  /* Highlight type */
} tbos_hl_span_t;

/**
 * @brief Highlighting result
 */
typedef struct {
    tbos_hl_span_t* spans;  /* Array of spans */
    size_t count;           /* Number of spans */
    size_t capacity;        /* Allocated capacity */
} tbos_hl_result_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * HIGHLIGHTING API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize syntax highlighter
 */
void tbos_highlight_init(void);

/**
 * @brief Shutdown syntax highlighter
 */
void tbos_highlight_shutdown(void);

/**
 * @brief Analyze line and produce highlighting spans
 *
 * @param line  Line to highlight
 * @return Highlight result (caller must free with tbos_highlight_free)
 */
tbos_hl_result_t* tbos_highlight(const char* line);

/**
 * @brief Free highlighting result
 *
 * @param hl  Result to free
 */
void tbos_highlight_free(tbos_hl_result_t* hl);

/**
 * @brief Render line with highlighting
 *
 * Outputs ANSI escape codes for colors.
 *
 * @param line  Original line
 * @param hl    Highlighting result (or NULL for no highlighting)
 */
void tbos_highlight_render(const char* line, const tbos_hl_result_t* hl);

/**
 * @brief Get ANSI color code for highlight type
 *
 * @param type  Highlight type
 * @return ANSI escape sequence
 */
const char* tbos_highlight_color(tbos_hl_type_t type);

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND VALIDATION
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Check if string is a valid command
 *
 * @param cmd  Command name to check
 * @return true if valid, false otherwise
 */
bool tbos_highlight_is_valid_command(const char* cmd);

/**
 * @brief Check if string looks like a path
 *
 * @param str  String to check
 * @return true if looks like a path
 */
bool tbos_highlight_is_path(const char* str);

/**
 * @brief Check if string is an option flag
 *
 * @param str  String to check
 * @return true if looks like -flag or --flag
 */
bool tbos_highlight_is_option(const char* str);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_SYNTAX_HL_H */
