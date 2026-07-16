/**
 * @file tbos_line_editor.c
 * @brief TBOS Line Editor Implementation
 *
 * Full readline-like functionality for the TBOS shell.
 */

#include "tbos_line_editor.h"
#include "tbos_terminal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * LINE BUFFER FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_line_init(tbos_line_t* line, size_t capacity) {
    if (!line || capacity == 0) return -1;

    line->buffer = (char*)malloc(capacity);
    if (!line->buffer) return -1;

    line->buffer[0] = '\0';
    line->len = 0;
    line->capacity = capacity;
    line->cursor = 0;
    line->insert_mode = true;
    line->yanked = NULL;
    line->yanked_len = 0;

    return 0;
}

void tbos_line_free(tbos_line_t* line) {
    if (!line) return;

    free(line->buffer);
    free(line->yanked);

    line->buffer = NULL;
    line->yanked = NULL;
    line->len = 0;
    line->capacity = 0;
    line->cursor = 0;
}

void tbos_line_clear(tbos_line_t* line) {
    if (!line || !line->buffer) return;

    line->buffer[0] = '\0';
    line->len = 0;
    line->cursor = 0;
}

int tbos_line_set(tbos_line_t* line, const char* text) {
    if (!line || !line->buffer) return -1;
    if (!text) {
        tbos_line_clear(line);
        return 0;
    }

    size_t len = strlen(text);

    /* Grow buffer if needed */
    if (len >= line->capacity) {
        size_t new_cap = len + 128;
        char* new_buf = (char*)realloc(line->buffer, new_cap);
        if (!new_buf) return -1;
        line->buffer = new_buf;
        line->capacity = new_cap;
    }

    strcpy(line->buffer, text);
    line->len = len;
    line->cursor = len;

    return 0;
}

const char* tbos_line_get(const tbos_line_t* line) {
    if (!line || !line->buffer) return "";
    return line->buffer;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CHARACTER OPERATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Ensure buffer has room for n more characters
 */
static int ensure_capacity(tbos_line_t* line, size_t additional) {
    size_t needed = line->len + additional + 1;  /* +1 for null */

    if (needed <= line->capacity) return 0;

    size_t new_cap = needed + 64;  /* Extra headroom */
    char* new_buf = (char*)realloc(line->buffer, new_cap);
    if (!new_buf) return -1;

    line->buffer = new_buf;
    line->capacity = new_cap;
    return 0;
}

int tbos_line_insert(tbos_line_t* line, char c) {
    if (!line || !line->buffer) return -1;

    if (ensure_capacity(line, 1) != 0) return -1;

    if (line->insert_mode || line->cursor >= line->len) {
        /* Insert: shift characters right */
        memmove(line->buffer + line->cursor + 1,
                line->buffer + line->cursor,
                line->len - line->cursor + 1);  /* +1 for null */
        line->buffer[line->cursor] = c;
        line->len++;
    } else {
        /* Overwrite */
        line->buffer[line->cursor] = c;
    }

    line->cursor++;
    return 0;
}

int tbos_line_insert_str(tbos_line_t* line, const char* str) {
    if (!line || !str) return -1;

    size_t str_len = strlen(str);
    if (str_len == 0) return 0;

    if (ensure_capacity(line, str_len) != 0) return -1;

    /* Shift existing characters right */
    memmove(line->buffer + line->cursor + str_len,
            line->buffer + line->cursor,
            line->len - line->cursor + 1);

    /* Insert new string */
    memcpy(line->buffer + line->cursor, str, str_len);
    line->len += str_len;
    line->cursor += str_len;

    return 0;
}

int tbos_line_delete(tbos_line_t* line) {
    if (!line || !line->buffer) return -1;
    if (line->cursor == 0) return -1;

    /* Shift characters left */
    memmove(line->buffer + line->cursor - 1,
            line->buffer + line->cursor,
            line->len - line->cursor + 1);

    line->len--;
    line->cursor--;
    return 0;
}

int tbos_line_delete_forward(tbos_line_t* line) {
    if (!line || !line->buffer) return -1;
    if (line->cursor >= line->len) return -1;

    /* Shift characters left (including null) */
    memmove(line->buffer + line->cursor,
            line->buffer + line->cursor + 1,
            line->len - line->cursor);

    line->len--;
    return 0;
}

int tbos_line_transpose(tbos_line_t* line) {
    if (!line || !line->buffer) return -1;
    if (line->len < 2) return -1;

    /* At end of line: swap last two chars */
    if (line->cursor >= line->len) {
        char tmp = line->buffer[line->len - 2];
        line->buffer[line->len - 2] = line->buffer[line->len - 1];
        line->buffer[line->len - 1] = tmp;
    }
    /* Not at end: swap char before and at cursor */
    else if (line->cursor > 0) {
        char tmp = line->buffer[line->cursor - 1];
        line->buffer[line->cursor - 1] = line->buffer[line->cursor];
        line->buffer[line->cursor] = tmp;
        line->cursor++;
    }
    else {
        return -1;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CURSOR MOVEMENT
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_line_cursor_left(tbos_line_t* line) {
    if (!line || line->cursor == 0) return -1;
    line->cursor--;
    return 0;
}

int tbos_line_cursor_right(tbos_line_t* line) {
    if (!line || line->cursor >= line->len) return -1;
    line->cursor++;
    return 0;
}

void tbos_line_cursor_home(tbos_line_t* line) {
    if (line) line->cursor = 0;
}

void tbos_line_cursor_end(tbos_line_t* line) {
    if (line) line->cursor = line->len;
}

/**
 * @brief Check if character is a word character
 */
static bool is_word_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

int tbos_line_cursor_word_left(tbos_line_t* line) {
    if (!line || line->cursor == 0) return -1;

    /* Skip whitespace */
    while (line->cursor > 0 && !is_word_char(line->buffer[line->cursor - 1])) {
        line->cursor--;
    }

    /* Skip word characters */
    while (line->cursor > 0 && is_word_char(line->buffer[line->cursor - 1])) {
        line->cursor--;
    }

    return 0;
}

int tbos_line_cursor_word_right(tbos_line_t* line) {
    if (!line || line->cursor >= line->len) return -1;

    /* Skip current word */
    while (line->cursor < line->len && is_word_char(line->buffer[line->cursor])) {
        line->cursor++;
    }

    /* Skip whitespace */
    while (line->cursor < line->len && !is_word_char(line->buffer[line->cursor])) {
        line->cursor++;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * KILL / YANK OPERATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Save text to yank buffer
 */
static void save_yank(tbos_line_t* line, const char* text, size_t len) {
    free(line->yanked);

    if (len == 0) {
        line->yanked = NULL;
        line->yanked_len = 0;
        return;
    }

    line->yanked = (char*)malloc(len + 1);
    if (line->yanked) {
        memcpy(line->yanked, text, len);
        line->yanked[len] = '\0';
        line->yanked_len = len;
    }
}

int tbos_line_kill_to_end(tbos_line_t* line) {
    if (!line || !line->buffer) return -1;

    /* Save killed text */
    save_yank(line, line->buffer + line->cursor, line->len - line->cursor);

    /* Truncate line at cursor */
    line->buffer[line->cursor] = '\0';
    line->len = line->cursor;

    return 0;
}

int tbos_line_kill_to_start(tbos_line_t* line) {
    if (!line || !line->buffer) return -1;
    if (line->cursor == 0) return 0;

    /* Save killed text */
    save_yank(line, line->buffer, line->cursor);

    /* Shift remaining text to start */
    memmove(line->buffer, line->buffer + line->cursor,
            line->len - line->cursor + 1);

    line->len -= line->cursor;
    line->cursor = 0;

    return 0;
}

int tbos_line_kill_word(tbos_line_t* line) {
    if (!line || !line->buffer) return -1;
    if (line->cursor == 0) return -1;

    size_t start = line->cursor;

    /* Move back over whitespace */
    while (start > 0 && !is_word_char(line->buffer[start - 1])) {
        start--;
    }

    /* Move back over word characters */
    while (start > 0 && is_word_char(line->buffer[start - 1])) {
        start--;
    }

    /* Save killed text */
    save_yank(line, line->buffer + start, line->cursor - start);

    /* Delete the word */
    memmove(line->buffer + start, line->buffer + line->cursor,
            line->len - line->cursor + 1);

    line->len -= (line->cursor - start);
    line->cursor = start;

    return 0;
}

int tbos_line_yank(tbos_line_t* line) {
    if (!line || !line->yanked || line->yanked_len == 0) return -1;
    return tbos_line_insert_str(line, line->yanked);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HISTORY FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_history_init(tbos_history_t* hist, size_t capacity) {
    if (!hist || capacity == 0) return -1;

    hist->entries = (char**)calloc(capacity, sizeof(char*));
    if (!hist->entries) return -1;

    hist->count = 0;
    hist->capacity = capacity;
    hist->position = 0;
    hist->saved_line = NULL;
    hist->browsing = false;

    return 0;
}

void tbos_history_free(tbos_history_t* hist) {
    if (!hist) return;

    for (size_t i = 0; i < hist->count; i++) {
        free(hist->entries[i]);
    }
    free(hist->entries);
    free(hist->saved_line);

    hist->entries = NULL;
    hist->saved_line = NULL;
    hist->count = 0;
    hist->capacity = 0;
}

void tbos_history_add(tbos_history_t* hist, const char* line) {
    if (!hist || !line) return;

    /* Skip empty lines and duplicates */
    size_t len = strlen(line);
    if (len == 0) return;

    /* Skip if same as last entry */
    if (hist->count > 0 &&
        strcmp(hist->entries[hist->count - 1], line) == 0) {
        return;
    }

    /* Make room if at capacity */
    if (hist->count >= hist->capacity) {
        /* Remove oldest entry */
        free(hist->entries[0]);
        memmove(hist->entries, hist->entries + 1,
                (hist->capacity - 1) * sizeof(char*));
        hist->count--;
    }

    /* Add new entry */
    hist->entries[hist->count] = strdup(line);
    if (hist->entries[hist->count]) {
        hist->count++;
    }

    /* Reset browsing state */
    hist->browsing = false;
    free(hist->saved_line);
    hist->saved_line = NULL;
}

const char* tbos_history_prev(tbos_history_t* hist, tbos_line_t* line) {
    if (!hist || hist->count == 0) return NULL;

    /* First time browsing: save current line and start from end */
    if (!hist->browsing) {
        hist->browsing = true;
        hist->position = hist->count;

        free(hist->saved_line);
        hist->saved_line = line ? strdup(line->buffer) : NULL;
    }

    /* Already at oldest */
    if (hist->position == 0) return NULL;

    hist->position--;
    return hist->entries[hist->position];
}

const char* tbos_history_next(tbos_history_t* hist, tbos_line_t* line) {
    (void)line;  /* Not needed for next */

    if (!hist || !hist->browsing) return NULL;

    hist->position++;

    /* Past newest: return saved line */
    if (hist->position >= hist->count) {
        hist->position = hist->count;
        hist->browsing = false;
        const char* saved = hist->saved_line;
        hist->saved_line = NULL;
        return saved;
    }

    return hist->entries[hist->position];
}

void tbos_history_reset_position(tbos_history_t* hist) {
    if (!hist) return;

    hist->browsing = false;
    free(hist->saved_line);
    hist->saved_line = NULL;
    hist->position = hist->count;
}

const char* tbos_history_search(tbos_history_t* hist, const char* prefix,
                                 size_t* start) {
    if (!hist || !prefix || !start) return NULL;

    size_t prefix_len = strlen(prefix);
    if (prefix_len == 0) return NULL;

    /* Search backwards from start position */
    for (size_t i = *start; i > 0; i--) {
        if (strncmp(hist->entries[i - 1], prefix, prefix_len) == 0) {
            *start = i - 1;
            return hist->entries[i - 1];
        }
    }

    return NULL;
}

size_t tbos_history_count(const tbos_history_t* hist) {
    return hist ? hist->count : 0;
}

const char* tbos_history_get(const tbos_history_t* hist, size_t index) {
    if (!hist || index >= hist->count) return NULL;
    return hist->entries[index];
}

/* ═══════════════════════════════════════════════════════════════════════════
 * RENDERING HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_line_render(const tbos_line_t* line, const char* prompt) {
    if (!line) return;

    /* Clear line and move to start */
    printf("\r\033[K");

    /* Print prompt */
    if (prompt) {
        printf("%s", prompt);
    }

    /* Print line content */
    printf("%s", line->buffer);

    /* Position cursor */
    size_t prompt_len = prompt ? strlen(prompt) : 0;
    size_t cursor_col = prompt_len + line->cursor + 1;

    printf("\r\033[%zuC", cursor_col - 1);
    fflush(stdout);
}

void tbos_line_refresh(const tbos_line_t* line, const char* prompt) {
    /* Same as render for now */
    tbos_line_render(line, prompt);
}
