/**
 * @file commands_week2.c
 * @brief Week 2 Text Processing Commands (Days 10-17)
 *
 * AITO MODE: All In That Order - FULL SPEED!
 * Week 2: Text Processing & File Search
 *
 * Commands 10-17:
 * - Day 10: cat    (concatenate and display files)
 * - Day 11: head   (display first lines)
 * - Day 12: tail   (display last lines)
 * - Day 13: more   (page through text)
 * - Day 14: less   (better pager)
 * - Day 15: wc     (word/line/byte count)
 * - Day 16: grep   (search text patterns)
 * - Day 17: find   (search for files)
 *
 * @date 2025-11-05
 * @author TernaryBit OS Team (AITO SPEED MODE)
 */

#include "tbos_universal_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <fnmatch.h>

static char* tbos_strcasestr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return NULL;
    if (*needle == '\0') return (char*)haystack;

    size_t needle_len = strlen(needle);
    for (const char* p = haystack; *p; ++p) {
        size_t i = 0;
        while (i < needle_len &&
               tolower((unsigned char)p[i]) == tolower((unsigned char)needle[i])) {
            i++;
        }
        if (i == needle_len) {
            return (char*)p;
        }
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 10: cat - Concatenate and Display Files
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_cat(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "cat: missing file operand\n");
        return 1;
    }

    bool number_lines = false;
    int first_file = 1;

    /* Parse options */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            number_lines = true;
            first_file++;
        }
    }

    /* Process each file */
    for (int i = first_file; i < argc; i++) {
        FILE* fp = fopen(argv[i], "r");
        if (!fp) {
            fprintf(stderr, "cat: %s: %s\n", argv[i], strerror(errno));
            continue;
        }

        char line[4096];
        int line_num = 1;
        while (fgets(line, sizeof(line), fp)) {
            if (number_lines) {
                printf("%6d  %s", line_num++, line);
            } else {
                printf("%s", line);
            }
        }

        fclose(fp);
    }

    universal_add_karma(1, "Read and shared knowledge");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 11: head - Display First Lines
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_head(int argc, char** argv) {
    int num_lines = 10;
    int first_file = 1;

    /* Parse -n option */
    if (argc > 2 && strcmp(argv[1], "-n") == 0) {
        num_lines = atoi(argv[2]);
        first_file = 3;
    }

    if (first_file >= argc) {
        fprintf(stderr, "head: missing file operand\n");
        return 1;
    }

    for (int i = first_file; i < argc; i++) {
        FILE* fp = fopen(argv[i], "r");
        if (!fp) {
            fprintf(stderr, "head: %s: %s\n", argv[i], strerror(errno));
            continue;
        }

        if (argc - first_file > 1) {
            printf("==> %s <==\n", argv[i]);
        }

        char line[4096];
        int count = 0;
        while (count < num_lines && fgets(line, sizeof(line), fp)) {
            printf("%s", line);
            count++;
        }

        fclose(fp);

        if (i < argc - 1) {
            printf("\n");
        }
    }

    universal_add_karma(1, "Focused on beginnings");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 12: tail - Display Last Lines
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_tail(int argc, char** argv) {
    int num_lines = 10;
    int first_file = 1;

    /* Parse -n option */
    if (argc > 2 && strcmp(argv[1], "-n") == 0) {
        num_lines = atoi(argv[2]);
        first_file = 3;
    }

    if (first_file >= argc) {
        fprintf(stderr, "tail: missing file operand\n");
        return 1;
    }

    for (int i = first_file; i < argc; i++) {
        FILE* fp = fopen(argv[i], "r");
        if (!fp) {
            fprintf(stderr, "tail: %s: %s\n", argv[i], strerror(errno));
            continue;
        }

        /* Read all lines into circular buffer */
        char** lines = calloc(num_lines, sizeof(char*));
        int total_lines = 0;
        char buffer[4096];

        while (fgets(buffer, sizeof(buffer), fp)) {
            int idx = total_lines % num_lines;
            free(lines[idx]);
            lines[idx] = strdup(buffer);
            total_lines++;
        }

        /* Print last N lines */
        if (argc - first_file > 1) {
            printf("==> %s <==\n", argv[i]);
        }

        int start = (total_lines > num_lines) ? (total_lines % num_lines) : 0;
        int count = (total_lines < num_lines) ? total_lines : num_lines;

        for (int j = 0; j < count; j++) {
            int idx = (start + j) % num_lines;
            if (lines[idx]) {
                printf("%s", lines[idx]);
            }
        }

        /* Cleanup */
        for (int j = 0; j < num_lines; j++) {
            free(lines[j]);
        }
        free(lines);
        fclose(fp);
    }

    universal_add_karma(1, "Focused on endings");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 13: more - Page Through Text
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_more(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "more: missing file operand\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        FILE* fp = fopen(argv[i], "r");
        if (!fp) {
            fprintf(stderr, "more: %s: %s\n", argv[i], strerror(errno));
            continue;
        }

        char line[4096];
        int line_count = 0;
        int lines_per_page = 24;

        printf("==> %s <==\n", argv[i]);

        while (fgets(line, sizeof(line), fp)) {
            printf("%s", line);
            line_count++;

            if (line_count >= lines_per_page) {
                printf("--More-- (Press Enter)");
                fflush(stdout);
                getchar();
                line_count = 0;
            }
        }

        fclose(fp);
        printf("\n");
    }

    universal_add_karma(1, "Practiced patience (paging)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 14: less - Better Pager (simplified version)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_less(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "less: missing file operand\n");
        return 1;
    }

    /* For now, less acts like more with better messaging */
    printf("less: Using simplified pager mode\n");
    return cmd_more(argc, argv);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 15: wc - Word/Line/Byte Count
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_wc(int argc, char** argv) {
    bool count_lines = true;
    bool count_words = true;
    bool count_bytes = true;
    int first_file = 1;

    /* Parse options */
    for (int i = 1; i < argc && argv[i][0] == '-'; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            count_lines = true;
            count_words = count_bytes = false;
        } else if (strcmp(argv[i], "-w") == 0) {
            count_words = true;
            count_lines = count_bytes = false;
        } else if (strcmp(argv[i], "-c") == 0) {
            count_bytes = true;
            count_lines = count_words = false;
        }
        first_file++;
    }

    if (first_file >= argc) {
        fprintf(stderr, "wc: missing file operand\n");
        return 1;
    }

    long total_lines = 0, total_words = 0, total_bytes = 0;

    for (int i = first_file; i < argc; i++) {
        FILE* fp = fopen(argv[i], "r");
        if (!fp) {
            fprintf(stderr, "wc: %s: %s\n", argv[i], strerror(errno));
            continue;
        }

        long lines = 0, words = 0, bytes = 0;
        char buffer[4096];
        bool in_word = false;

        while (fgets(buffer, sizeof(buffer), fp)) {
            lines++;
            bytes += strlen(buffer);

            for (char* p = buffer; *p; p++) {
                if (isspace(*p)) {
                    in_word = false;
                } else if (!in_word) {
                    in_word = true;
                    words++;
                }
            }
        }

        if (count_lines) printf("%8ld ", lines);
        if (count_words) printf("%8ld ", words);
        if (count_bytes) printf("%8ld ", bytes);
        printf("%s\n", argv[i]);

        total_lines += lines;
        total_words += words;
        total_bytes += bytes;

        fclose(fp);
    }

    /* Print totals if multiple files */
    if (argc - first_file > 1) {
        if (count_lines) printf("%8ld ", total_lines);
        if (count_words) printf("%8ld ", total_words);
        if (count_bytes) printf("%8ld ", total_bytes);
        printf("total\n");
    }

    universal_add_karma(1, "Measured with precision");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 16: grep - Search Text Patterns
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_grep(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "grep: usage: grep PATTERN FILE...\n");
        return 1;
    }

    const char* pattern = argv[1];
    bool ignore_case = false;
    bool line_numbers = false;
    bool invert = false;

    /* Simple option parsing */
    int first_file = 2;
    if (argv[1][0] == '-') {
        for (const char* p = &argv[1][1]; *p; p++) {
            if (*p == 'i') ignore_case = true;
            if (*p == 'n') line_numbers = true;
            if (*p == 'v') invert = true;
        }
        pattern = argv[2];
        first_file = 3;
    }

    if (first_file >= argc) {
        fprintf(stderr, "grep: missing file operand\n");
        return 1;
    }

    for (int i = first_file; i < argc; i++) {
        FILE* fp = fopen(argv[i], "r");
        if (!fp) {
            fprintf(stderr, "grep: %s: %s\n", argv[i], strerror(errno));
            continue;
        }

        char line[4096];
        int line_num = 1;
        bool show_filename = (argc - first_file > 1);

        while (fgets(line, sizeof(line), fp)) {
            /* Simple case-insensitive search */
            bool match = (ignore_case ? tbos_strcasestr(line, pattern) : strstr(line, pattern)) != NULL;
            if (invert) match = !match;

            if (match) {
                if (show_filename) printf("%s:", argv[i]);
                if (line_numbers) printf("%d:", line_num);
                printf("%s", line);
            }
            line_num++;
        }

        fclose(fp);
    }

    universal_add_karma(2, "Searched for truth (patterns)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 17: find - Search for Files
 * ═══════════════════════════════════════════════════════════════════════════ */

static void find_recursive(const char* path, const char* pattern, bool type_file, bool type_dir) {
    DIR* dir = opendir(path);
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat st;
        if (lstat(full_path, &st) != 0) continue;

        bool is_dir = S_ISDIR(st.st_mode);
        bool is_file = S_ISREG(st.st_mode);

        /* Check type filter */
        if ((type_dir && !is_dir) || (type_file && !is_file)) {
            goto recurse;
        }

        /* Check name pattern */
        if (pattern && fnmatch(pattern, entry->d_name, 0) != 0) {
            goto recurse;
        }

        /* Print match */
        printf("%s\n", full_path);

recurse:
        /* Recurse into directories */
        if (is_dir) {
            find_recursive(full_path, pattern, type_file, type_dir);
        }
    }

    closedir(dir);
}

static int cmd_find(int argc, char** argv) {
    const char* start_path = ".";
    const char* pattern = NULL;
    bool type_file = false;
    bool type_dir = false;

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-name") == 0 && i + 1 < argc) {
            pattern = argv[++i];
        } else if (strcmp(argv[i], "-type") == 0 && i + 1 < argc) {
            char type = argv[++i][0];
            if (type == 'f') type_file = true;
            if (type == 'd') type_dir = true;
        } else if (argv[i][0] != '-') {
            start_path = argv[i];
        }
    }

    find_recursive(start_path, pattern, type_file, type_dir);

    universal_add_karma(2, "Searched mindfully (files)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WEEK 2 REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void register_week2_commands(void) {
    // REVIEW(CX): Same issue as Week 1 – universal_shell_register_command() is undefined,
    // so none of these handlers are currently linkable until that API is implemented.
    universal_shell_register_command("cat", cmd_cat,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "concatenate and print files",
        "cat [OPTION]... FILE...\n\n"
        "Concatenate FILE(s) to standard output.\n\n"
        "Options:\n"
        "  -n    number all output lines");

    universal_shell_register_command("head", cmd_head,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "output the first part of files",
        "head [OPTION]... FILE...\n\n"
        "Print first 10 lines of each FILE.\n\n"
        "Options:\n"
        "  -n NUM    print first NUM lines");

    universal_shell_register_command("tail", cmd_tail,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "output the last part of files",
        "tail [OPTION]... FILE...\n\n"
        "Print last 10 lines of each FILE.\n\n"
        "Options:\n"
        "  -n NUM    print last NUM lines");

    universal_shell_register_command("more", cmd_more,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "file perusal filter for paging",
        "more FILE...\n\n"
        "Page through FILE(s) one screen at a time.");

    universal_shell_register_command("less", cmd_less,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "opposite of more",
        "less FILE...\n\n"
        "Enhanced file pager (simplified version).");

    universal_shell_register_command("wc", cmd_wc,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "print line, word, and byte counts",
        "wc [OPTION]... FILE...\n\n"
        "Print newline, word, and byte counts.\n\n"
        "Options:\n"
        "  -l    print line counts\n"
        "  -w    print word counts\n"
        "  -c    print byte counts");

    universal_shell_register_command("grep", cmd_grep,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "print lines matching a pattern",
        "grep [OPTION]... PATTERN FILE...\n\n"
        "Search for PATTERN in each FILE.\n\n"
        "Options:\n"
        "  -i    ignore case\n"
        "  -n    show line numbers\n"
        "  -v    invert match");

    universal_shell_register_command("find", cmd_find,
        CMD_CAT_FILE_OPS, OS_SUPPORT_ALL,
        "search for files in a directory hierarchy",
        "find [PATH] [OPTION]...\n\n"
        "Search for files in directory hierarchy.\n\n"
        "Options:\n"
        "  -name PATTERN    match file name\n"
        "  -type f|d        match file type");

    printf("🚀 [Week 2] Registered 8 text processing commands\n");
    printf("   Commands 10-17: cat, head, tail, more, less, wc, grep, find\n");
}
