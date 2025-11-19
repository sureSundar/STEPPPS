/**
 * @file commands_week7.c
 * @brief Week 7 Text Layout Utilities (Days 43-49)
 *
 * Commands:
 *  - expand   : Convert tabs to spaces
 *  - unexpand : Convert spaces to tabs
 *  - col      : Normalize line feeds
 *  - colrm    : Remove character columns
 *  - column   : Format input into aligned columns
 *  - rev      : Reverse characters in each line
 *  - tac      : Reverse line order
 */

#include "tbos_universal_shell.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_TAB_WIDTH 8
#define MAX_COLUMNS 128

typedef struct {
    char** fields;
    size_t count;
} column_line_t;

typedef struct {
    column_line_t* lines;
    size_t count;
    size_t capacity;
} column_table_t;

static FILE* open_stream(const char* path, const char* mode, bool* need_close) {
    if (!path || strcmp(path, "-") == 0) {
        *need_close = false;
        return (mode[0] == 'r') ? stdin : stdout;
    }

    FILE* fp = fopen(path, mode);
    if (!fp) {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        return NULL;
    }
    *need_close = true;
    return fp;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * expand / unexpand
 * ═══════════════════════════════════════════════════════════════════════════ */

static int process_expand(FILE* in, FILE* out, int tab_width) {
    int ch;
    int column = 0;

    while ((ch = fgetc(in)) != EOF) {
        if (ch == '\t') {
            int spaces = tab_width - (column % tab_width);
            for (int i = 0; i < spaces; i++) {
                if (fputc(' ', out) == EOF) return 1;
                column++;
            }
        } else {
            if (fputc(ch, out) == EOF) return 1;
            if (ch == '\n' || ch == '\r') {
                column = 0;
            } else {
                column++;
            }
        }
    }
    return ferror(in) ? 1 : 0;
}

static int process_unexpand(FILE* in, FILE* out, int tab_width) {
    int ch;
    int column = 0;
    int space_count = 0;

    while ((ch = fgetc(in)) != EOF) {
        if (ch == ' ') {
            space_count++;
            column++;
            if (column % tab_width == 0) {
                if (fputc('\t', out) == EOF) return 1;
                space_count = 0;
            }
        } else {
            while (space_count > 0) {
                if (fputc(' ', out) == EOF) return 1;
                space_count--;
            }
            if (fputc(ch, out) == EOF) return 1;
            if (ch == '\n' || ch == '\r') {
                column = 0;
                space_count = 0;
            } else {
                column++;
            }
        }
    }

    while (space_count-- > 0) {
        if (fputc(' ', out) == EOF) return 1;
    }

    return ferror(in) ? 1 : 0;
}

static int cmd_expand(int argc, char** argv) {
    int tab_width = DEFAULT_TAB_WIDTH;
    const char* input_path = NULL;
    const char* output_path = NULL;

    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-t") == 0 || strncmp(argv[i], "--tabs=", 7) == 0)) {
            const char* value = NULL;
            if (strcmp(argv[i], "-t") == 0) {
                if (i + 1 < argc) value = argv[++i];
            } else {
                value = argv[i] + 7;
            }
            if (value) tab_width = atoi(value);
        } else if (!input_path) {
            input_path = argv[i];
        } else {
            output_path = argv[i];
        }
    }

    if (tab_width <= 0) tab_width = DEFAULT_TAB_WIDTH;

    bool close_in = false;
    bool close_out = false;
    FILE* in = open_stream(input_path, "rb", &close_in);
    if (!in) return 1;
    FILE* out = open_stream(output_path, "wb", &close_out);
    if (!out) {
        if (close_in) fclose(in);
        return 1;
    }

    int status = process_expand(in, out, tab_width);
    if (close_in) fclose(in);
    if (close_out) fclose(out);
    if (status == 0) {
        universal_add_karma(1, "Smoothed structure with spaces (expand)");
    }
    return status;
}

static int cmd_unexpand(int argc, char** argv) {
    int tab_width = DEFAULT_TAB_WIDTH;
    const char* input_path = NULL;
    const char* output_path = NULL;

    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-t") == 0 || strncmp(argv[i], "--tabs=", 7) == 0)) {
            const char* value = NULL;
            if (strcmp(argv[i], "-t") == 0) {
                if (i + 1 < argc) value = argv[++i];
            } else {
                value = argv[i] + 7;
            }
            if (value) tab_width = atoi(value);
        } else if (!input_path) {
            input_path = argv[i];
        } else {
            output_path = argv[i];
        }
    }

    if (tab_width <= 0) tab_width = DEFAULT_TAB_WIDTH;

    bool close_in = false;
    bool close_out = false;
    FILE* in = open_stream(input_path, "rb", &close_in);
    if (!in) return 1;
    FILE* out = open_stream(output_path, "wb", &close_out);
    if (!out) {
        if (close_in) fclose(in);
        return 1;
    }

    int status = process_unexpand(in, out, tab_width);
    if (close_in) fclose(in);
    if (close_out) fclose(out);
    if (status == 0) {
        universal_add_karma(1, "Restored tabs mindfully (unexpand)");
    }
    return status;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * col
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_col(int argc, char** argv) {
    (void)argc;
    (void)argv;

    int ch;
    int last = 0;
    while ((ch = fgetc(stdin)) != EOF) {
        if (ch == '\r') {
            last = 0;
            continue;
        }
        if (ch == '\b') {
            if (last != '\n') {
                if (fputc('\b', stdout) == EOF) return 1;
            }
            continue;
        }
        if (fputc(ch, stdout) == EOF) return 1;
        last = ch;
    }

    if (ferror(stdin)) {
        fprintf(stderr, "col: read error: %s\n", strerror(errno));
        return 1;
    }

    universal_add_karma(1, "Streamlined control flows (col)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * colrm
 * ═══════════════════════════════════════════════════════════════════════════ */

static int remove_columns(FILE* in, FILE* out, int start_col, int end_col) {
    if (start_col <= 0) start_col = 1;
    if (end_col > 0 && end_col < start_col) end_col = start_col;

    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    while ((nread = getline(&line, &len, in)) != -1) {
        int col = 1;
        for (ssize_t i = 0; i < nread; i++) {
            char c = line[i];
            bool in_range = (col >= start_col) && (end_col <= 0 || col <= end_col);
            if (!in_range || c == '\n') {
                if (fputc(c, out) == EOF) {
                    free(line);
                    return 1;
                }
            }
            if (c == '\n') {
                col = 1;
            } else if (c == '\t') {
                col += DEFAULT_TAB_WIDTH;
            } else {
                col++;
            }
        }
    }

    free(line);
    return ferror(in) ? 1 : 0;
}

static int cmd_colrm(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: colrm start [end]\n");
        return 1;
    }

    int start = atoi(argv[1]);
    int end = (argc >= 3) ? atoi(argv[2]) : -1;
    int status = remove_columns(stdin, stdout, start, end);
    if (status == 0) {
        universal_add_karma(1, "Trimmed distractions (colrm)");
    }
    return status;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * column
 * ═══════════════════════════════════════════════════════════════════════════ */

static void column_table_init(column_table_t* table) {
    table->lines = NULL;
    table->count = 0;
    table->capacity = 0;
}

static void column_table_free(column_table_t* table) {
    for (size_t i = 0; i < table->count; i++) {
        for (size_t j = 0; j < table->lines[i].count; j++) {
            free(table->lines[i].fields[j]);
        }
        free(table->lines[i].fields);
    }
    free(table->lines);
}

static bool column_table_push(column_table_t* table, column_line_t line) {
    if (table->count == table->capacity) {
        size_t new_cap = table->capacity ? table->capacity * 2 : 32;
        column_line_t* tmp = realloc(table->lines, new_cap * sizeof(column_line_t));
        if (!tmp) return false;
        table->lines = tmp;
        table->capacity = new_cap;
    }
    table->lines[table->count++] = line;
    return true;
}

static bool parse_column_line(const char* line, column_line_t* out_line) {
    char* copy = strdup(line);
    if (!copy) return false;

    size_t fields_cap = 8;
    size_t fields_count = 0;
    char** fields = malloc(fields_cap * sizeof(char*));
    if (!fields) {
        free(copy);
        return false;
    }

    char* token;
    char* saveptr = NULL;
    token = strtok_r(copy, " \t\r\n", &saveptr);
    while (token) {
        if (fields_count == fields_cap) {
            size_t new_cap = fields_cap * 2;
            char** tmp = realloc(fields, new_cap * sizeof(char*));
            if (!tmp) {
                free(copy);
                for (size_t i = 0; i < fields_count; i++) free(fields[i]);
                free(fields);
                return false;
            }
            fields = tmp;
            fields_cap = new_cap;
        }
        fields[fields_count++] = strdup(token);
        token = strtok_r(NULL, " \t\r\n", &saveptr);
    }
    /* CX REVIEW: The whitespace-only split misses quoted columns (e.g. CSV with
     * spaces). Maybe we should optionally honour quotes or delimiter flags to
     * match POSIX column -t behaviour? */

    free(copy);
    out_line->fields = fields;
    out_line->count = fields_count;
    return true;
}

static int cmd_column(int argc, char** argv) {
    (void)argc;
    (void)argv;

    column_table_t table;
    column_table_init(&table);

    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    while ((nread = getline(&line, &len, stdin)) != -1) {
        column_line_t parsed;
        if (!parse_column_line(line, &parsed)) {
            free(line);
            column_table_free(&table);
            fprintf(stderr, "column: out of memory\n");
            return 1;
        }
        if (!column_table_push(&table, parsed)) {
            free(line);
            column_table_free(&table);
            fprintf(stderr, "column: out of memory\n");
            return 1;
        }
    }
    free(line);

    if (table.count == 0) {
        column_table_free(&table);
        return 0;
    }

    size_t max_columns = 0;
    for (size_t i = 0; i < table.count; i++) {
        if (table.lines[i].count > max_columns) {
            max_columns = table.lines[i].count;
        }
    }
    if (max_columns > MAX_COLUMNS) max_columns = MAX_COLUMNS;

    size_t* widths = calloc(max_columns, sizeof(size_t));
    if (!widths) {
        column_table_free(&table);
        fprintf(stderr, "column: out of memory\n");
        return 1;
    }

    for (size_t i = 0; i < table.count; i++) {
        for (size_t j = 0; j < table.lines[i].count && j < max_columns; j++) {
            size_t len_field = strlen(table.lines[i].fields[j]);
            if (len_field > widths[j]) widths[j] = len_field;
        }
    }

    for (size_t i = 0; i < table.count; i++) {
        for (size_t j = 0; j < table.lines[i].count && j < max_columns; j++) {
            printf("%-*s", (int)(widths[j] + 2), table.lines[i].fields[j]);
        }
        putchar('\n');
    }

    free(widths);
    column_table_free(&table);
    universal_add_karma(2, "Presented data with clarity (column)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * rev
 * ═══════════════════════════════════════════════════════════════════════════ */

static void reverse_buffer(char* buf, size_t len) {
    size_t i = 0;
    size_t j = len ? len - 1 : 0;
    while (i < j) {
        char tmp = buf[i];
        buf[i] = buf[j];
        buf[j] = tmp;
        i++;
        j--;
    }
}

static int cmd_rev(int argc, char** argv) {
    (void)argc;
    (void)argv;

    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    while ((nread = getline(&line, &len, stdin)) != -1) {
        size_t line_len = (size_t)nread;
        bool has_newline = (line_len > 0 && line[line_len - 1] == '\n');
        if (has_newline) {
            line[line_len - 1] = '\0';
            reverse_buffer(line, line_len - 1);
            printf("%s\n", line);
        } else {
            reverse_buffer(line, line_len);
            printf("%s", line);
        }
    }
    free(line);

    if (ferror(stdin)) {
        fprintf(stderr, "rev: read error: %s\n", strerror(errno));
        return 1;
    }

    universal_add_karma(1, "Reflected on each line (rev)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * tac
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_tac(int argc, char** argv) {
    (void)argc;
    (void)argv;

    char** lines = NULL;
    size_t count = 0;
    size_t capacity = 0;

    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    while ((nread = getline(&line, &len, stdin)) != -1) {
        char* stored = strdup(line);
        if (!stored) {
            free(line);
            for (size_t i = 0; i < count; i++) free(lines[i]);
            free(lines);
            fprintf(stderr, "tac: out of memory\n");
            return 1;
        }
        if (count == capacity) {
            size_t new_cap = capacity ? capacity * 2 : 64;
            char** tmp = realloc(lines, new_cap * sizeof(char*));
            if (!tmp) {
                free(line);
                free(stored);
                for (size_t i = 0; i < count; i++) free(lines[i]);
                free(lines);
                fprintf(stderr, "tac: out of memory\n");
                return 1;
            }
            lines = tmp;
            capacity = new_cap;
        }
        lines[count++] = stored;
    }
    free(line);

    if (ferror(stdin)) {
        fprintf(stderr, "tac: read error: %s\n", strerror(errno));
        for (size_t i = 0; i < count; i++) free(lines[i]);
        free(lines);
        return 1;
    }

    for (ssize_t i = (ssize_t)count - 1; i >= 0; i--) {
        fputs(lines[i], stdout);
        free(lines[i]);
    }
    free(lines);

    universal_add_karma(1, "Gained perspective by reversing order (tac)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WEEK 7 REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void register_week7_commands(void) {
    universal_shell_register_command(
        "expand", cmd_expand,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "convert tabs to spaces",
        "expand [-t NUM] [FILE [OUT]]\n"
        "Convert TAB characters to spaces (default tab stop 8).");

    universal_shell_register_command(
        "unexpand", cmd_unexpand,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "convert spaces to tabs",
        "unexpand [-t NUM] [FILE [OUT]]\n"
        "Convert spaces to TAB characters (default tab stop 8).");

    universal_shell_register_command(
        "col", cmd_col,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "filter reverse line feeds",
        "col\n"
        "Filter reverse line feeds and backspaces for display.");

    universal_shell_register_command(
        "colrm", cmd_colrm,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "remove columns from input",
        "colrm start [end]\n"
        "Remove character columns from input lines.");

    universal_shell_register_command(
        "column", cmd_column,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "format input into columns",
        "column\n"
        "Align whitespace-separated data into columns");

    universal_shell_register_command(
        "rev", cmd_rev,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "reverse characters in each line",
        "rev\n"
        "Reverse characters in every input line.");

    universal_shell_register_command(
        "tac", cmd_tac,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "concatenate and print files in reverse",
        "tac\n"
        "Print input lines in reverse order.");
}
