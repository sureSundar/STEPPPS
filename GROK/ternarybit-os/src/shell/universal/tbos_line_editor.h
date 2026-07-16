/**
 * @file tbos_line_editor.h
 * @brief TBOS Line Editor - Readline-like line editing and history
 *
 * Provides:
 * - Line buffer management with cursor positioning
 * - Character insert/delete operations
 * - Word-level operations (delete word, etc.)
 * - Command history with navigation
 * - Reverse incremental search
 */

#ifndef TBOS_LINE_EDITOR_H
#define TBOS_LINE_EDITOR_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * LINE BUFFER
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Line buffer structure
 * Manages a single line of text with cursor position
 */
typedef struct {
    char* buffer;           /* Current line content (null-terminated) */
    size_t len;             /* Current length (excluding null) */
    size_t capacity;        /* Buffer capacity */
    size_t cursor;          /* Cursor position (0 = before first char) */
    bool insert_mode;       /* true = insert, false = overwrite */
    char* yanked;           /* Yanked (cut) text for paste */
    size_t yanked_len;      /* Length of yanked text */
} tbos_line_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND HISTORY
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Command history structure
 * Stores previous commands for navigation
 */
typedef struct {
    char** entries;         /* Array of history entries */
    size_t count;           /* Number of entries */
    size_t capacity;        /* Maximum entries */
    size_t position;        /* Current browse position */
    char* saved_line;       /* Saved current line when browsing */
    bool browsing;          /* Currently browsing history */
} tbos_history_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * LINE BUFFER FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize a line buffer
 *
 * @param line      Line structure to initialize
 * @param capacity  Initial buffer capacity
 * @return 0 on success, -1 on error
 */
int tbos_line_init(tbos_line_t* line, size_t capacity);

/**
 * @brief Free line buffer resources
 *
 * @param line  Line to free
 */
void tbos_line_free(tbos_line_t* line);

/**
 * @brief Clear line contents (keep buffer allocated)
 *
 * @param line  Line to clear
 */
void tbos_line_clear(tbos_line_t* line);

/**
 * @brief Set line content
 *
 * @param line  Line buffer
 * @param text  Text to set (copied)
 * @return 0 on success, -1 on error
 */
int tbos_line_set(tbos_line_t* line, const char* text);

/**
 * @brief Get line content as string
 *
 * @param line  Line buffer
 * @return Pointer to null-terminated string (do not free)
 */
const char* tbos_line_get(const tbos_line_t* line);

/* ═══════════════════════════════════════════════════════════════════════════
 * CHARACTER OPERATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Insert character at cursor position
 *
 * @param line  Line buffer
 * @param c     Character to insert
 * @return 0 on success, -1 if buffer full
 */
int tbos_line_insert(tbos_line_t* line, char c);

/**
 * @brief Insert string at cursor position
 *
 * @param line  Line buffer
 * @param str   String to insert
 * @return 0 on success, -1 on error
 */
int tbos_line_insert_str(tbos_line_t* line, const char* str);

/**
 * @brief Delete character before cursor (backspace)
 *
 * @param line  Line buffer
 * @return 0 on success, -1 if at beginning
 */
int tbos_line_delete(tbos_line_t* line);

/**
 * @brief Delete character at cursor (delete key)
 *
 * @param line  Line buffer
 * @return 0 on success, -1 if at end
 */
int tbos_line_delete_forward(tbos_line_t* line);

/**
 * @brief Transpose characters at cursor (Ctrl+T)
 *
 * @param line  Line buffer
 * @return 0 on success, -1 if not possible
 */
int tbos_line_transpose(tbos_line_t* line);

/* ═══════════════════════════════════════════════════════════════════════════
 * CURSOR MOVEMENT
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Move cursor left one character
 *
 * @param line  Line buffer
 * @return 0 on success, -1 if at beginning
 */
int tbos_line_cursor_left(tbos_line_t* line);

/**
 * @brief Move cursor right one character
 *
 * @param line  Line buffer
 * @return 0 on success, -1 if at end
 */
int tbos_line_cursor_right(tbos_line_t* line);

/**
 * @brief Move cursor to beginning of line
 *
 * @param line  Line buffer
 */
void tbos_line_cursor_home(tbos_line_t* line);

/**
 * @brief Move cursor to end of line
 *
 * @param line  Line buffer
 */
void tbos_line_cursor_end(tbos_line_t* line);

/**
 * @brief Move cursor to beginning of previous word
 *
 * @param line  Line buffer
 * @return 0 on success, -1 if at beginning
 */
int tbos_line_cursor_word_left(tbos_line_t* line);

/**
 * @brief Move cursor to end of next word
 *
 * @param line  Line buffer
 * @return 0 on success, -1 if at end
 */
int tbos_line_cursor_word_right(tbos_line_t* line);

/* ═══════════════════════════════════════════════════════════════════════════
 * KILL / YANK OPERATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Kill (cut) text from cursor to end of line (Ctrl+K)
 *
 * @param line  Line buffer
 * @return 0 on success
 */
int tbos_line_kill_to_end(tbos_line_t* line);

/**
 * @brief Kill text from beginning to cursor (Ctrl+U)
 *
 * @param line  Line buffer
 * @return 0 on success
 */
int tbos_line_kill_to_start(tbos_line_t* line);

/**
 * @brief Kill word before cursor (Ctrl+W)
 *
 * @param line  Line buffer
 * @return 0 on success
 */
int tbos_line_kill_word(tbos_line_t* line);

/**
 * @brief Yank (paste) previously killed text (Ctrl+Y)
 *
 * @param line  Line buffer
 * @return 0 on success, -1 if nothing to yank
 */
int tbos_line_yank(tbos_line_t* line);

/* ═══════════════════════════════════════════════════════════════════════════
 * HISTORY FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize history
 *
 * @param hist      History structure to initialize
 * @param capacity  Maximum number of entries
 * @return 0 on success, -1 on error
 */
int tbos_history_init(tbos_history_t* hist, size_t capacity);

/**
 * @brief Free history resources
 *
 * @param hist  History to free
 */
void tbos_history_free(tbos_history_t* hist);

/**
 * @brief Add entry to history
 *
 * @param hist  History structure
 * @param line  Line to add (copied, empty lines ignored)
 */
void tbos_history_add(tbos_history_t* hist, const char* line);

/**
 * @brief Get previous history entry (Up arrow)
 *
 * @param hist  History structure
 * @param line  Current line buffer (to save/restore)
 * @return Previous entry or NULL if at oldest
 */
const char* tbos_history_prev(tbos_history_t* hist, tbos_line_t* line);

/**
 * @brief Get next history entry (Down arrow)
 *
 * @param hist  History structure
 * @param line  Current line buffer (to save/restore)
 * @return Next entry or saved line if at newest
 */
const char* tbos_history_next(tbos_history_t* hist, tbos_line_t* line);

/**
 * @brief Reset history browsing position
 *
 * @param hist  History structure
 */
void tbos_history_reset_position(tbos_history_t* hist);

/**
 * @brief Search history for prefix match (Ctrl+R style)
 *
 * @param hist    History structure
 * @param prefix  Prefix to search for
 * @param start   Starting position (use 0 for first search)
 * @return Matching entry or NULL
 */
const char* tbos_history_search(tbos_history_t* hist, const char* prefix,
                                 size_t* start);

/**
 * @brief Get history count
 *
 * @param hist  History structure
 * @return Number of entries
 */
size_t tbos_history_count(const tbos_history_t* hist);

/**
 * @brief Get history entry by index (0 = oldest)
 *
 * @param hist   History structure
 * @param index  Entry index
 * @return Entry string or NULL if out of bounds
 */
const char* tbos_history_get(const tbos_history_t* hist, size_t index);

/* ═══════════════════════════════════════════════════════════════════════════
 * RENDERING HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Render line to terminal with cursor positioning
 *
 * Outputs: clear line, prompt, content, positions cursor
 *
 * @param line    Line buffer
 * @param prompt  Prompt string (e.g., "tbos> ")
 */
void tbos_line_render(const tbos_line_t* line, const char* prompt);

/**
 * @brief Refresh line display (redraw without clearing)
 *
 * @param line    Line buffer
 * @param prompt  Prompt string
 */
void tbos_line_refresh(const tbos_line_t* line, const char* prompt);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_LINE_EDITOR_H */
