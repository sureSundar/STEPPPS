/**
 * @file tbos_syntax_hl.c
 * @brief TBOS Syntax Highlighting Implementation
 *
 * Real-time syntax highlighting for shell input.
 */

#include "tbos_syntax_hl.h"
#include "tbos_terminal.h"
#include "tbos_universal_shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * COLOR DEFINITIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char* hl_colors[] = {
    [HL_DEFAULT]  = TERM_RESET,
    [HL_COMMAND]  = TERM_FG_GREEN,
    [HL_INVALID]  = TERM_FG_RED,
    [HL_ARGUMENT] = TERM_RESET,
    [HL_PATH]     = TERM_FG_BLUE,
    [HL_STRING]   = TERM_FG_YELLOW,
    [HL_OPTION]   = TERM_FG_MAGENTA,
    [HL_VARIABLE] = TERM_FG_BRIGHT_CYAN,
    [HL_OPERATOR] = TERM_BOLD TERM_FG_WHITE,
    [HL_COMMENT]  = TERM_FG_BRIGHT_BLACK,
    [HL_NUMBER]   = TERM_FG_CYAN,
    [HL_ERROR]    = TERM_BG_RED TERM_FG_WHITE
};

/* ═══════════════════════════════════════════════════════════════════════════
 * INITIALIZATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_highlight_init(void) {
    /* Nothing to initialize currently */
}

void tbos_highlight_shutdown(void) {
    /* Nothing to cleanup */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * RESULT MANAGEMENT
 * ═══════════════════════════════════════════════════════════════════════════ */

static tbos_hl_result_t* result_alloc(void) {
    tbos_hl_result_t* hl = (tbos_hl_result_t*)calloc(1, sizeof(tbos_hl_result_t));
    if (hl) {
        hl->capacity = 16;
        hl->spans = (tbos_hl_span_t*)calloc(hl->capacity, sizeof(tbos_hl_span_t));
        if (!hl->spans) {
            free(hl);
            return NULL;
        }
    }
    return hl;
}

static int result_add_span(tbos_hl_result_t* hl, size_t start, size_t end,
                           tbos_hl_type_t type) {
    if (!hl) return -1;
    if (start >= end) return 0;  /* Empty span */

    /* Grow if needed */
    if (hl->count >= hl->capacity) {
        size_t new_cap = hl->capacity * 2;
        tbos_hl_span_t* new_spans = (tbos_hl_span_t*)realloc(
            hl->spans, new_cap * sizeof(tbos_hl_span_t));
        if (!new_spans) return -1;
        hl->spans = new_spans;
        hl->capacity = new_cap;
    }

    hl->spans[hl->count].start = start;
    hl->spans[hl->count].end = end;
    hl->spans[hl->count].type = type;
    hl->count++;

    return 0;
}

void tbos_highlight_free(tbos_hl_result_t* hl) {
    if (!hl) return;
    free(hl->spans);
    free(hl);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * VALIDATION HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

bool tbos_highlight_is_valid_command(const char* cmd) {
    if (!cmd || !*cmd) return false;

    /* Check against registered commands using shell's find function */
    const universal_command_t* found = universal_find_command(cmd);
    return found != NULL;
}

bool tbos_highlight_is_path(const char* str) {
    if (!str || !*str) return false;

    /* Starts with / or ./ or ../ */
    if (str[0] == '/') return true;
    if (str[0] == '.' && str[1] == '/') return true;
    if (str[0] == '.' && str[1] == '.' && str[2] == '/') return true;

    /* Contains / */
    if (strchr(str, '/') != NULL) return true;

    /* Looks like a filename with extension */
    const char* dot = strrchr(str, '.');
    if (dot && dot != str && strlen(dot) > 1 && strlen(dot) <= 5) {
        return true;
    }

    return false;
}

bool tbos_highlight_is_option(const char* str) {
    if (!str || !*str) return false;

    if (str[0] == '-') {
        if (str[1] == '-' && isalpha((unsigned char)str[2])) {
            /* --long-option */
            return true;
        }
        if (isalpha((unsigned char)str[1])) {
            /* -f or -xyz */
            return true;
        }
    }

    return false;
}

/**
 * @brief Check if string is an operator
 */
static bool is_operator(char c) {
    return c == '|' || c == '>' || c == '<' || c == '&' || c == ';';
}

/**
 * @brief Check if string starts with $
 */
static bool is_variable(const char* str) {
    return str && str[0] == '$' && (isalpha((unsigned char)str[1]) || str[1] == '_');
}

/**
 * @brief Check if string is numeric
 */
static bool is_number(const char* str, size_t len) {
    if (!str || len == 0) return false;

    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char)str[i]) && str[i] != '.') {
            return false;
        }
    }
    return true;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN HIGHLIGHTING FUNCTION
 * ═══════════════════════════════════════════════════════════════════════════ */

tbos_hl_result_t* tbos_highlight(const char* line) {
    if (!line) return NULL;

    tbos_hl_result_t* hl = result_alloc();
    if (!hl) return NULL;

    size_t len = strlen(line);
    size_t pos = 0;
    int word_num = 0;
    (void)0;  /* in_quote handling reserved for future */

    while (pos < len) {
        /* Skip whitespace */
        while (pos < len && isspace((unsigned char)line[pos])) {
            pos++;
        }
        if (pos >= len) break;

        /* Check for comment */
        if (line[pos] == '#') {
            result_add_span(hl, pos, len, HL_COMMENT);
            break;
        }

        /* Check for quote start */
        if (line[pos] == '"' || line[pos] == '\'') {
            char quote = line[pos];
            size_t start = pos;
            pos++;

            /* Find closing quote */
            while (pos < len && line[pos] != quote) {
                if (line[pos] == '\\' && pos + 1 < len) pos++;  /* Skip escaped */
                pos++;
            }
            if (pos < len) pos++;  /* Skip closing quote */

            result_add_span(hl, start, pos, HL_STRING);
            word_num++;
            continue;
        }

        /* Check for operator */
        if (is_operator(line[pos])) {
            size_t start = pos;
            /* Handle >> or << */
            if ((line[pos] == '>' || line[pos] == '<') &&
                pos + 1 < len && line[pos + 1] == line[pos]) {
                pos++;
            }
            pos++;
            result_add_span(hl, start, pos, HL_OPERATOR);
            /* Reset word count after operator (next word is command) */
            word_num = 0;
            continue;
        }

        /* Find word end */
        size_t word_start = pos;
        while (pos < len && !isspace((unsigned char)line[pos]) &&
               line[pos] != '#' && !is_operator(line[pos]) &&
               line[pos] != '"' && line[pos] != '\'') {
            pos++;
        }
        size_t word_end = pos;
        size_t word_len = word_end - word_start;

        if (word_len == 0) continue;

        /* Extract word */
        char word[256];
        if (word_len < sizeof(word)) {
            strncpy(word, line + word_start, word_len);
            word[word_len] = '\0';
        } else {
            word[0] = '\0';
        }

        /* Determine type */
        tbos_hl_type_t type = HL_DEFAULT;

        if (word_num == 0) {
            /* First word: command */
            if (tbos_highlight_is_valid_command(word)) {
                type = HL_COMMAND;
            } else {
                type = HL_INVALID;
            }
        }
        else if (is_variable(word)) {
            type = HL_VARIABLE;
        }
        else if (tbos_highlight_is_option(word)) {
            type = HL_OPTION;
        }
        else if (tbos_highlight_is_path(word)) {
            type = HL_PATH;
        }
        else if (is_number(word, word_len)) {
            type = HL_NUMBER;
        }
        else {
            type = HL_ARGUMENT;
        }

        result_add_span(hl, word_start, word_end, type);
        word_num++;
    }

    return hl;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * RENDERING
 * ═══════════════════════════════════════════════════════════════════════════ */

const char* tbos_highlight_color(tbos_hl_type_t type) {
    if (type < 0 || type > HL_ERROR) return TERM_RESET;
    return hl_colors[type];
}

void tbos_highlight_render(const char* line, const tbos_hl_result_t* hl) {
    if (!line) return;

    size_t len = strlen(line);

    if (!hl || hl->count == 0) {
        /* No highlighting, just print */
        printf("%s", line);
        return;
    }

    /* Sort spans by start position (should already be sorted) */
    size_t pos = 0;
    size_t span_idx = 0;

    while (pos < len) {
        /* Find span covering this position */
        tbos_hl_type_t type = HL_DEFAULT;

        if (span_idx < hl->count && pos >= hl->spans[span_idx].start) {
            if (pos < hl->spans[span_idx].end) {
                type = hl->spans[span_idx].type;
            } else {
                span_idx++;
                /* Check next span */
                if (span_idx < hl->count && pos >= hl->spans[span_idx].start &&
                    pos < hl->spans[span_idx].end) {
                    type = hl->spans[span_idx].type;
                }
            }
        }

        /* Output character with color */
        printf("%s%c", tbos_highlight_color(type), line[pos]);
        pos++;
    }

    /* Reset color at end */
    printf("%s", TERM_RESET);
}
