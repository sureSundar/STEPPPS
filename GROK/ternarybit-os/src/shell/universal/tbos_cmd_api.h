/**
 * @file tbos_cmd_api.h
 * @brief TBOS Command API - Clean interface for command execution
 *
 * This API separates command logic from UI rendering. Commands return
 * structured results that can be rendered as ANSI (TUI) or JSON (Web).
 */

#ifndef TBOS_CMD_API_H
#define TBOS_CMD_API_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * OUTPUT FORMAT
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    TBOS_FMT_ANSI,      /* Terminal with ANSI colors (default) */
    TBOS_FMT_PLAIN,     /* Plain text, no colors */
    TBOS_FMT_JSON,      /* JSON structured output */
    TBOS_FMT_HTML       /* HTML with CSS classes */
} tbos_output_format_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * RESULT TYPES
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    TBOS_OK = 0,
    TBOS_ERROR,
    TBOS_NOT_FOUND,
    TBOS_PERMISSION_DENIED,
    TBOS_INVALID_ARGS
} tbos_status_t;

typedef enum {
    TBOS_TYPE_TEXT,     /* Plain text output */
    TBOS_TYPE_TABLE,    /* Tabular data (ls, ps) */
    TBOS_TYPE_LIST,     /* List of items */
    TBOS_TYPE_TREE,     /* Tree structure */
    TBOS_TYPE_KEYVAL,   /* Key-value pairs */
    TBOS_TYPE_PROGRESS, /* Progress indicator */
    TBOS_TYPE_EMPTY     /* No output */
} tbos_api_result_type_t;

/* Table cell for structured output */
typedef struct {
    char* value;
    char* color;        /* Optional: "red", "green", "blue", etc. */
    bool bold;
} tbos_cell_t;

/* Table row */
typedef struct {
    tbos_cell_t* cells;
    size_t count;
} tbos_row_t;

/* Structured result */
typedef struct {
    tbos_status_t status;
    tbos_api_result_type_t type;

    /* Text output (for TBOS_TYPE_TEXT) */
    char* text;

    /* Table output (for TBOS_TYPE_TABLE) */
    char** headers;
    size_t header_count;
    tbos_row_t* rows;
    size_t row_count;

    /* List output (for TBOS_TYPE_LIST) */
    char** items;
    size_t item_count;

    /* Key-value output (for TBOS_TYPE_KEYVAL) */
    char** keys;
    char** values;
    size_t kv_count;

    /* Error message if status != TBOS_OK */
    char* error;
} tbos_api_result_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND CONTEXT
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    tbos_output_format_t format;
    char* cwd;              /* Current working directory */
    char** env;             /* Environment variables */
    size_t env_count;
    void* user_data;        /* For custom handlers */
} tbos_api_context_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND HANDLER TYPE
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef tbos_api_result_t* (*tbos_cmd_handler_t)(
    int argc,
    char** argv,
    tbos_api_context_t* ctx
);

/* ═══════════════════════════════════════════════════════════════════════════
 * API FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize the command API
 */
int tbos_cmd_init(void);

/**
 * @brief Shutdown the command API
 */
void tbos_cmd_shutdown(void);

/**
 * @brief Execute a command line and get structured result
 * @param line Command line to execute
 * @param ctx Execution context (format, cwd, etc.)
 * @return Result structure (caller must free with tbos_result_free)
 */
tbos_api_result_t* tbos_cmd_execute(const char* line, tbos_api_context_t* ctx);

/**
 * @brief Execute with argc/argv
 */
tbos_api_result_t* tbos_cmd_execute_argv(int argc, char** argv, tbos_api_context_t* ctx);

/**
 * @brief Free a result structure
 */
void tbos_result_free(tbos_api_result_t* result);

/**
 * @brief Render result to string in specified format
 * @param result Result to render
 * @param format Output format
 * @return Allocated string (caller must free)
 */
char* tbos_result_render(tbos_api_result_t* result, tbos_output_format_t format);

/* ═══════════════════════════════════════════════════════════════════════════
 * RESULT BUILDERS (for command implementations)
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Create a text result
 */
tbos_api_result_t* tbos_api_result_text(const char* text);

/**
 * @brief Create an error result
 */
tbos_api_result_t* tbos_result_error(tbos_status_t status, const char* message);

/**
 * @brief Create an empty/success result
 */
tbos_api_result_t* tbos_result_ok(void);

/**
 * @brief Create a table result
 */
tbos_api_result_t* tbos_api_result_table(char** headers, size_t header_count);

/**
 * @brief Add a row to a table result
 */
int tbos_result_add_row(tbos_api_result_t* result, tbos_cell_t* cells, size_t count);

/**
 * @brief Create a list result
 */
tbos_api_result_t* tbos_result_list(void);

/**
 * @brief Add item to list result
 */
int tbos_result_add_item(tbos_api_result_t* result, const char* item);

/**
 * @brief Create a key-value result
 */
tbos_api_result_t* tbos_result_keyval(void);

/**
 * @brief Add key-value pair
 */
int tbos_result_add_kv(tbos_api_result_t* result, const char* key, const char* value);

/* ═══════════════════════════════════════════════════════════════════════════
 * CONTEXT HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Create a default context
 */
tbos_api_context_t* tbos_api_context_create(tbos_output_format_t format);

/**
 * @brief Free a context
 */
void tbos_context_free(tbos_api_context_t* ctx);

/**
 * @brief Get format from environment (TBOS_OUTPUT_FORMAT)
 */
tbos_output_format_t tbos_format_from_env(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_CMD_API_H */
