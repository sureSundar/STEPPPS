/**
 * @file commands_week6.c
 * @brief Week 6 Text Transformation Commands (Days 36-42)
 *
 * Commands:
 *  - cut   : Extract columns from each line
 *  - paste : Merge lines of files
 *  - join  : Join lines on a common field
 *  - sort  : Sort lines alphabetically
 *  - uniq  : Report or omit repeated lines
 *  - tr    : Translate characters
 *  - fmt   : Reformat text paragraphs
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

#define DEFAULT_DELIMITER '\t'
#define DEFAULT_WRAP_WIDTH 75

typedef struct {
    FILE* file;
    bool need_close;
} input_handle_t;

static FILE* open_input(const char* path, input_handle_t* handle) {
    handle->need_close = false;
    if (!path || strcmp(path, "-") == 0) {
        handle->file = stdin;
        return handle->file;
    }

    handle->file = fopen(path, "rb");
    if (!handle->file) {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        return NULL;
    }
    handle->need_close = true;
    return handle->file;
}

static void close_input(input_handle_t* handle) {
    if (handle->need_close && handle->file) {
        fclose(handle->file);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * cut
 * ═══════════════════════════════════════════════════════════════════════════ */

static char* nth_field(char* line, char delimiter, int field) {
    int current = 1;
    char* token = line;
    while (*token) {
        if (current == field) {
            char* end = token;
            while (*end && *end != delimiter && *end != '\n') end++;
            size_t len = (size_t)(end - token);
            char* result = malloc(len + 1);
            if (!result) return NULL;
            memcpy(result, token, len);
            result[len] = '\0';
            return result;
        }
        while (*token && *token != delimiter && *token != '\n') token++;
        if (*token == delimiter) {
            token++;
            current++;
        } else {
            break;
        }
    }
    return strdup("");
}

static int cmd_cut(int argc, char** argv) {
    char delimiter = DEFAULT_DELIMITER;
    int field = -1;
    const char* path = NULL;

    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-d") == 0 || strncmp(argv[i], "--delimiter=", 12) == 0)) {
            const char* value = NULL;
            if (strcmp(argv[i], "-d") == 0) {
                if (i + 1 < argc) value = argv[++i];
            } else {
                value = argv[i] + 12;
            }
            if (value && value[0]) delimiter = value[0];
        } else if (strncmp(argv[i], "-f", 2) == 0 || strncmp(argv[i], "--fields=", 9) == 0) {
            const char* value = NULL;
            if (strcmp(argv[i], "-f") == 0) {
                if (i + 1 < argc) value = argv[++i];
            } else if (strncmp(argv[i], "--fields=", 9) == 0) {
                value = argv[i] + 9;
            } else {
                value = argv[i] + 2;
            }
            if (value) field = atoi(value);
        } else {
            path = argv[i];
        }
    }

    if (field <= 0) {
        fprintf(stderr, "cut: missing or invalid field list\n");
        return 1;
    }

    input_handle_t handle;
    FILE* in = open_input(path, &handle);
    if (!in) return 1;

    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    while ((nread = getline(&line, &len, in)) != -1) {
        char* field_str = nth_field(line, delimiter, field);
        if (!field_str) {
            free(line);
            close_input(&handle);
            fprintf(stderr, "cut: out of memory\n");
            return 1;
        }
        printf("%s\n", field_str);
        free(field_str);
    }

    free(line);
    close_input(&handle);
    if (ferror(in)) {
        fprintf(stderr, "cut: read error: %s\n", strerror(errno));
        return 1;
    }

    universal_add_karma(1, "Focused on essential fields (cut)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * paste
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_paste(int argc, char** argv) {
    if (argc < 2) {
        char* line = NULL;
        size_t len = 0;
        ssize_t nread;
        while ((nread = getline(&line, &len, stdin)) != -1) {
            fputs(line, stdout);
        }
        free(line);
        if (ferror(stdin)) {
            fprintf(stderr, "paste: read error: %s\n", strerror(errno));
            return 1;
        }
        universal_add_karma(1, "Flowed lines directly (paste)");
        return 0;
    }

    int files = argc - 1;
    input_handle_t* handles = calloc(files, sizeof(input_handle_t));
    FILE** streams = calloc(files, sizeof(FILE*));
    if (!handles || !streams) {
        free(handles);
        free(streams);
        fprintf(stderr, "paste: out of memory\n");
        return 1;
    }

    for (int i = 0; i < files; i++) {
        streams[i] = open_input(argv[i + 1], &handles[i]);
        if (!streams[i]) {
            for (int j = 0; j < i; j++) close_input(&handles[j]);
            free(handles);
            free(streams);
            return 1;
        }
    }

    bool eof_reached = false;
    char** lines = calloc(files, sizeof(char*));
    size_t* lengths = calloc(files, sizeof(size_t));
    if (!lines || !lengths) {
        fprintf(stderr, "paste: out of memory\n");
        goto cleanup;
    }

    while (!eof_reached) {
        eof_reached = true;
        for (int i = 0; i < files; i++) {
            if (!lines[i]) {
                lines[i] = NULL;
                lengths[i] = 0;
            }
            ssize_t nread = getline(&lines[i], &lengths[i], streams[i]);
            if (nread != -1) {
                eof_reached = false;
                if (nread > 0 && lines[i][nread - 1] == '\n') {
                    lines[i][nread - 1] = '\0';
                }
            } else {
                if (ferror(streams[i])) {
                    fprintf(stderr, "paste: read error: %s\n", strerror(errno));
                    goto cleanup;
                }
                if (!lines[i]) {
                    lines[i] = strdup("");
                    lengths[i] = 1;
                }
            }
        }
        if (!eof_reached) {
            for (int i = 0; i < files; i++) {
                printf("%s", lines[i] ? lines[i] : "");
                if (i != files - 1) putchar('\t');
            }
            putchar('\n');
        }
    }

    universal_add_karma(1, "Merged perspectives (paste)");

cleanup:
    if (lines) {
        for (int i = 0; i < files; i++) free(lines[i]);
    }
    free(lines);
    free(lengths);
    for (int i = 0; i < files; i++) close_input(&handles[i]);
    free(handles);
    free(streams);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * join
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct join_entry {
    char* key;
    char* rest;
} join_entry_t;

static int compare_keys(const void* a, const void* b) {
    const join_entry_t* ea = (const join_entry_t*)a;
    const join_entry_t* eb = (const join_entry_t*)b;
    return strcmp(ea->key, eb->key);
}

static bool parse_join_line(const char* line, int field, join_entry_t* entry) {
    char* copy = strdup(line);
    if (!copy) return false;

    char* token = strtok(copy, " \t\n");
    int current = 1;
    while (token && current < field) {
        token = strtok(NULL, " \t\n");
        current++;
    }
    if (!token) {
        free(copy);
        entry->key = strdup("");
        entry->rest = strdup("");
        return entry->key && entry->rest;
    }

    entry->key = strdup(token);
    char* remainder = strtok(NULL, "\n");
    entry->rest = remainder ? strdup(remainder) : strdup("");
    free(copy);
    return entry->key && entry->rest;
}

static int cmd_join(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: join file1 file2\n");
        return 1;
    }

    const char* file1 = argv[1];
    const char* file2 = argv[2];
    int field = 1;

    input_handle_t h1, h2;
    FILE* f1 = open_input(file1, &h1);
    if (!f1) return 1;
    FILE* f2 = open_input(file2, &h2);
    if (!f2) {
        close_input(&h1);
        return 1;
    }

    join_entry_t* entries = NULL;
    size_t entry_count = 0;
    size_t entry_cap = 0;

    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    while ((nread = getline(&line, &len, f1)) != -1) {
        join_entry_t entry;
        if (!parse_join_line(line, field, &entry)) {
            fprintf(stderr, "join: out of memory\n");
            free(line);
            close_input(&h1);
            close_input(&h2);
            return 1;
        }
        if (entry_count == entry_cap) {
            size_t new_cap = entry_cap ? entry_cap * 2 : 64;
            join_entry_t* tmp = realloc(entries, new_cap * sizeof(join_entry_t));
            if (!tmp) {
                fprintf(stderr, "join: out of memory\n");
                free(entry.key);
                free(entry.rest);
                free(line);
                close_input(&h1);
                close_input(&h2);
                return 1;
            }
            entries = tmp;
            entry_cap = new_cap;
        }
        entries[entry_count++] = entry;
    }
    free(line);

    qsort(entries, entry_count, sizeof(join_entry_t), compare_keys);

    line = NULL;
    len = 0;
    while ((nread = getline(&line, &len, f2)) != -1) {
        join_entry_t query;
        if (!parse_join_line(line, field, &query)) {
            fprintf(stderr, "join: out of memory\n");
            break;
        }
        join_entry_t* found = bsearch(&query, entries, entry_count,
                                      sizeof(join_entry_t), compare_keys);
        if (found) {
            printf("%s", query.key);
            if (found->rest && found->rest[0]) {
                printf(" %s", found->rest);
            }
            if (query.rest && query.rest[0]) {
                printf(" %s", query.rest);
            }
            putchar('\n');
        }
        free(query.key);
        free(query.rest);
    }
    free(line);

    for (size_t i = 0; i < entry_count; i++) {
        free(entries[i].key);
        free(entries[i].rest);
    }
    free(entries);
    close_input(&h1);
    close_input(&h2);

    universal_add_karma(2, "Unified datasets (join)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * sort
 * ═══════════════════════════════════════════════════════════════════════════ */

static int compare_lines(const void* a, const void* b) {
    const char* const* sa = (const char* const*)a;
    const char* const* sb = (const char* const*)b;
    return strcmp(*sa, *sb);
}

static int cmd_sort(int argc, char** argv) {
    bool reverse = false;
    const char* path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0) {
            reverse = true;
        } else {
            path = argv[i];
        }
    }

    input_handle_t handle;
    FILE* in = open_input(path, &handle);
    if (!in) return 1;

    char** lines = NULL;
    size_t count = 0;
    size_t capacity = 0;
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    while ((nread = getline(&line, &len, in)) != -1) {
        if (count == capacity) {
            size_t new_cap = capacity ? capacity * 2 : 128;
            char** tmp = realloc(lines, new_cap * sizeof(char*));
            if (!tmp) {
                fprintf(stderr, "sort: out of memory\n");
                free(line);
                close_input(&handle);
                for (size_t j = 0; j < count; j++) free(lines[j]);
                free(lines);
                return 1;
            }
            lines = tmp;
            capacity = new_cap;
        }
        lines[count++] = strdup(line);
    }
    free(line);

    qsort(lines, count, sizeof(char*), compare_lines);
    if (reverse) {
        for (size_t i = 0; i < count / 2; i++) {
            char* tmp = lines[i];
            lines[i] = lines[count - 1 - i];
            lines[count - 1 - i] = tmp;
        }
    }

    for (size_t i = 0; i < count; i++) {
        fputs(lines[i], stdout);
        free(lines[i]);
    }
    free(lines);
    close_input(&handle);

    universal_add_karma(1, "Ordered wisdom (sort)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * uniq
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_uniq(int argc, char** argv) {
    bool count_flag = false;
    const char* path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            count_flag = true;
        } else {
            path = argv[i];
        }
    }

    input_handle_t handle;
    FILE* in = open_input(path, &handle);
    if (!in) return 1;

    char* previous = NULL;
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    int occurrences = 0;
    while ((nread = getline(&line, &len, in)) != -1) {
        if (!previous || strcmp(previous, line) != 0) {
            if (previous) {
                if (count_flag) {
                    printf("%7d %s", occurrences, previous);
                } else {
                    fputs(previous, stdout);
                }
                free(previous);
            }
            previous = strdup(line);
            occurrences = 1;
        } else {
            occurrences++;
        }
    }
    if (previous) {
        if (count_flag) {
            printf("%7d %s", occurrences, previous);
        } else {
            fputs(previous, stdout);
        }
        free(previous);
    }
    free(line);
    close_input(&handle);

    universal_add_karma(1, "Recognized uniqueness (uniq)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * tr
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_tr(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: tr SET1 SET2\n");
        return 1;
    }

    const char* set1 = argv[1];
    const char* set2 = argv[2];
    size_t len1 = strlen(set1);
    size_t len2 = strlen(set2);

    if (len1 == 0 || len2 == 0) {
        fprintf(stderr, "tr: empty translation set\n");
        return 1;
    }

    unsigned char map[256];
    for (int i = 0; i < 256; i++) map[i] = (unsigned char)i;

    for (size_t i = 0; i < len1; i++) {
        unsigned char from = (unsigned char)set1[i];
        unsigned char to = (unsigned char)set2[i < len2 ? i : len2 - 1];
        map[from] = to;
    }

    int ch;
    while ((ch = getchar()) != EOF) {
        unsigned char translated = map[(unsigned char)ch];
        if (putchar(translated) == EOF) {
            fprintf(stderr, "tr: write error: %s\n", strerror(errno));
            return 1;
        }
    }

    if (ferror(stdin)) {
        fprintf(stderr, "tr: read error: %s\n", strerror(errno));
        return 1;
    }

    universal_add_karma(1, "Translated expression (tr)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * fmt
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_fmt(int argc, char** argv) {
    int width = DEFAULT_WRAP_WIDTH;
    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-w") == 0 || strncmp(argv[i], "--width=", 8) == 0)) {
            const char* value = NULL;
            if (strcmp(argv[i], "-w") == 0) {
                if (i + 1 < argc) value = argv[++i];
            } else {
                value = argv[i] + 8;
            }
            if (value) width = atoi(value);
        }
    }
    if (width <= 10) width = DEFAULT_WRAP_WIDTH;

    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    char buffer[4096];
    int buf_len = 0;

    while ((nread = getline(&line, &len, stdin)) != -1) {
        if (nread == 1 && line[0] == '\n') {
            if (buf_len > 0) {
                buffer[buf_len] = '\0';
                printf("%s\n", buffer);
                buf_len = 0;
            }
            putchar('\n');
            continue;
        }

        char* token = strtok(line, " \t\n");
        while (token) {
            int token_len = (int)strlen(token);
            if (buf_len > 0 && buf_len + token_len + 1 > width) {
                buffer[buf_len] = '\0';
                printf("%s\n", buffer);
                buf_len = 0;
            }
            if (buf_len > 0) {
                buffer[buf_len++] = ' ';
            }
            memcpy(&buffer[buf_len], token, token_len);
            buf_len += token_len;
            token = strtok(NULL, " \t\n");
        }
    }
    free(line);

    if (buf_len > 0) {
        buffer[buf_len] = '\0';
        printf("%s\n", buffer);
    }

    if (ferror(stdin)) {
        fprintf(stderr, "fmt: read error: %s\n", strerror(errno));
        return 1;
    }

    universal_add_karma(1, "Presented ideas gracefully (fmt)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WEEK 6 REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void register_week6_commands(void) {
    universal_shell_register_command(
        "cut", cmd_cut,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "remove sections from each line",
        "cut -f FIELD [-d DELIM] [FILE]\n"
        "Output selected fields from each line.");

    universal_shell_register_command(
        "paste", cmd_paste,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "merge lines of files",
        "paste FILE1 FILE2 ...\n"
        "Merge lines from multiple files (tab-separated).");

    universal_shell_register_command(
        "join", cmd_join,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "join lines on a common field",
        "join FILE1 FILE2\n"
        "Join lines sharing the same first field.");

    universal_shell_register_command(
        "sort", cmd_sort,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "sort lines of text files",
        "sort [-r] [FILE]\n"
        "Sort lines in lexical order.");

    universal_shell_register_command(
        "uniq", cmd_uniq,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "report or omit repeated lines",
        "uniq [-c] [FILE]\n"
        "Filter out repeated adjacent lines.");

    universal_shell_register_command(
        "tr", cmd_tr,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "translate characters",
        "tr SET1 SET2\n"
        "Translate characters from SET1 to SET2.");

    universal_shell_register_command(
        "fmt", cmd_fmt,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "simple text formatter",
        "fmt [-w WIDTH]\n"
        "Reformat text paragraphs to a given width.");
}
