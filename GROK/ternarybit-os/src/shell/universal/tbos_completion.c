/**
 * @file tbos_completion.c
 * @brief TBOS Tab Completion Engine Implementation
 *
 * Context-aware completion for commands, paths, and custom providers.
 */

#include "tbos_completion.h"
#include "tbos_universal_shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * CUSTOM PROVIDER REGISTRY
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_PROVIDERS 64

typedef struct {
    char* cmd;
    tbos_complete_provider_t provider;
} provider_entry_t;

static provider_entry_t g_providers[MAX_PROVIDERS];
static size_t g_provider_count = 0;
static bool g_initialized = false;

/* ═══════════════════════════════════════════════════════════════════════════
 * INITIALIZATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_complete_init(void) {
    if (g_initialized) return;

    memset(g_providers, 0, sizeof(g_providers));
    g_provider_count = 0;
    g_initialized = true;
}

void tbos_complete_shutdown(void) {
    for (size_t i = 0; i < g_provider_count; i++) {
        free(g_providers[i].cmd);
    }
    g_provider_count = 0;
    g_initialized = false;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMPLETION RESULT MANAGEMENT
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Allocate a completion result
 */
static tbos_completions_t* completions_alloc(void) {
    tbos_completions_t* c = (tbos_completions_t*)calloc(1, sizeof(tbos_completions_t));
    return c;
}

/**
 * @brief Add a match to completion result
 */
static int completions_add(tbos_completions_t* c, const char* match) {
    if (!c || !match) return -1;

    /* Grow array */
    char** new_matches = (char**)realloc(c->matches, (c->count + 1) * sizeof(char*));
    if (!new_matches) return -1;

    c->matches = new_matches;
    c->matches[c->count] = strdup(match);
    if (!c->matches[c->count]) return -1;

    c->count++;
    return 0;
}

/**
 * @brief Compute longest common prefix
 */
static void completions_compute_prefix(tbos_completions_t* c) {
    if (!c || c->count == 0) return;

    if (c->count == 1) {
        c->common_prefix = strdup(c->matches[0]);
        return;
    }

    /* Find common prefix length */
    size_t prefix_len = strlen(c->matches[0]);

    for (size_t i = 1; i < c->count; i++) {
        size_t j = 0;
        while (j < prefix_len &&
               c->matches[0][j] == c->matches[i][j] &&
               c->matches[0][j] != '\0') {
            j++;
        }
        prefix_len = j;
    }

    if (prefix_len > 0) {
        c->common_prefix = (char*)malloc(prefix_len + 1);
        if (c->common_prefix) {
            memcpy(c->common_prefix, c->matches[0], prefix_len);
            c->common_prefix[prefix_len] = '\0';
        }
    }
}

void tbos_completions_free(tbos_completions_t* c) {
    if (!c) return;

    for (size_t i = 0; i < c->count; i++) {
        free(c->matches[i]);
    }
    free(c->matches);
    free(c->common_prefix);
    free(c);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * NAVIGATION
 * ═══════════════════════════════════════════════════════════════════════════ */

const char* tbos_complete_next(tbos_completions_t* c) {
    if (!c || c->count == 0) return NULL;

    c->selected = (c->selected + 1) % c->count;
    return c->matches[c->selected];
}

const char* tbos_complete_prev(tbos_completions_t* c) {
    if (!c || c->count == 0) return NULL;

    if (c->selected == 0) {
        c->selected = c->count - 1;
    } else {
        c->selected--;
    }
    return c->matches[c->selected];
}

const char* tbos_complete_selected(const tbos_completions_t* c) {
    if (!c || c->count == 0) return NULL;
    return c->matches[c->selected];
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PROVIDER REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_complete_register(const char* cmd, tbos_complete_provider_t provider) {
    if (!cmd || !provider) return -1;
    if (g_provider_count >= MAX_PROVIDERS) return -1;

    g_providers[g_provider_count].cmd = strdup(cmd);
    g_providers[g_provider_count].provider = provider;
    g_provider_count++;

    return 0;
}

void tbos_complete_unregister(const char* cmd) {
    if (!cmd) return;

    for (size_t i = 0; i < g_provider_count; i++) {
        if (strcmp(g_providers[i].cmd, cmd) == 0) {
            free(g_providers[i].cmd);
            memmove(&g_providers[i], &g_providers[i + 1],
                    (g_provider_count - i - 1) * sizeof(provider_entry_t));
            g_provider_count--;
            return;
        }
    }
}

/**
 * @brief Find provider for command
 */
static tbos_complete_provider_t find_provider(const char* cmd) {
    for (size_t i = 0; i < g_provider_count; i++) {
        if (strcmp(g_providers[i].cmd, cmd) == 0) {
            return g_providers[i].provider;
        }
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * BUILT-IN COMPLETERS
 * ═══════════════════════════════════════════════════════════════════════════ */

tbos_completions_t* tbos_complete_commands(const char* partial) {
    tbos_completions_t* c = completions_alloc();
    if (!c) return NULL;

    c->type = COMPLETE_COMMAND;
    size_t partial_len = partial ? strlen(partial) : 0;

    /* Get commands from shell - iterate and check via universal_find_command */
    /* Use the shell's command iterator */
    const universal_command_t* commands[256];
    int count = universal_get_commands(commands, 256);

    for (int i = 0; i < count; i++) {
        if (partial_len == 0 ||
            strncmp(commands[i]->name, partial, partial_len) == 0) {
            completions_add(c, commands[i]->name);
        }
    }

    completions_compute_prefix(c);
    return c;
}

tbos_completions_t* tbos_complete_paths(const char* partial) {
    tbos_completions_t* c = completions_alloc();
    if (!c) return NULL;

    c->type = COMPLETE_PATH;

    /* Parse path into directory and prefix */
    char dir_path[512] = ".";
    const char* prefix = partial;
    size_t prefix_len = 0;

    if (partial) {
        const char* last_slash = strrchr(partial, '/');
        if (last_slash) {
            size_t dir_len = last_slash - partial;
            if (dir_len == 0) {
                strcpy(dir_path, "/");
            } else {
                strncpy(dir_path, partial, dir_len);
                dir_path[dir_len] = '\0';
            }
            prefix = last_slash + 1;
        }
        prefix_len = strlen(prefix);
    }

    /* Open directory */
    DIR* dir = opendir(dir_path);
    if (!dir) {
        /* Try VFS paths */
        /* For now, return empty */
        return c;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        /* Skip . and .. */
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        /* Check prefix match */
        if (prefix_len == 0 ||
            strncmp(entry->d_name, prefix, prefix_len) == 0) {

            /* Build full path for stat */
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

            struct stat st;
            if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                /* Append / for directories */
                char with_slash[512];
                snprintf(with_slash, sizeof(with_slash), "%s/", entry->d_name);
                completions_add(c, with_slash);
            } else {
                completions_add(c, entry->d_name);
            }
        }
    }

    closedir(dir);
    completions_compute_prefix(c);
    return c;
}

tbos_completions_t* tbos_complete_variables(const char* partial) {
    tbos_completions_t* c = completions_alloc();
    if (!c) return NULL;

    c->type = COMPLETE_VARIABLE;
    size_t partial_len = partial ? strlen(partial) : 0;

    /* Common environment variables */
    const char* common_vars[] = {
        "HOME", "PATH", "USER", "SHELL", "PWD", "TERM",
        "LANG", "EDITOR", "DISPLAY", NULL
    };

    for (const char** var = common_vars; *var; var++) {
        if (partial_len == 0 ||
            strncmp(*var, partial, partial_len) == 0) {
            completions_add(c, *var);
        }
    }

    completions_compute_prefix(c);
    return c;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN COMPLETION FUNCTION
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Parse line to find word at cursor
 */
static void find_word_at_cursor(const char* line, size_t cursor,
                                size_t* word_start, size_t* word_len,
                                int* word_num) {
    *word_start = 0;
    *word_len = 0;
    *word_num = 0;

    if (!line || cursor == 0) return;

    /* Find start of current word */
    size_t i = cursor;
    while (i > 0 && !isspace((unsigned char)line[i - 1])) {
        i--;
    }
    *word_start = i;
    *word_len = cursor - i;

    /* Count word number */
    int words = 0;
    bool in_word = false;
    for (size_t j = 0; j < cursor; j++) {
        if (isspace((unsigned char)line[j])) {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            words++;
        }
    }
    *word_num = words > 0 ? words - 1 : 0;
}

/**
 * @brief Extract first word (command name)
 */
static void extract_command(const char* line, char* cmd, size_t cmd_size) {
    cmd[0] = '\0';

    /* Skip leading whitespace */
    while (*line && isspace((unsigned char)*line)) line++;

    /* Copy command */
    size_t i = 0;
    while (*line && !isspace((unsigned char)*line) && i < cmd_size - 1) {
        cmd[i++] = *line++;
    }
    cmd[i] = '\0';
}

tbos_completions_t* tbos_complete(const char* line, size_t cursor) {
    if (!line) return NULL;

    size_t word_start, word_len;
    int word_num;
    find_word_at_cursor(line, cursor, &word_start, &word_len, &word_num);

    /* Extract partial word */
    char partial[256] = "";
    if (word_len > 0 && word_len < sizeof(partial)) {
        strncpy(partial, line + word_start, word_len);
        partial[word_len] = '\0';
    }

    tbos_completions_t* result = NULL;

    if (word_num == 0) {
        /* First word: complete command names */
        result = tbos_complete_commands(partial);
    }
    else {
        /* Not first word */

        /* Check for path completion (starts with /, ./, ../, or contains /) */
        if (partial[0] == '/' || partial[0] == '.' ||
            strchr(partial, '/') != NULL) {
            result = tbos_complete_paths(partial);
        }
        /* Check for variable completion ($) */
        else if (partial[0] == '$') {
            result = tbos_complete_variables(partial + 1);
        }
        else {
            /* Try custom provider */
            char cmd[256];
            extract_command(line, cmd, sizeof(cmd));

            tbos_complete_provider_t provider = find_provider(cmd);
            if (provider) {
                result = provider(cmd, partial, word_num - 1);
            }
            else {
                /* Default: path completion */
                result = tbos_complete_paths(partial);
            }
        }
    }

    if (result) {
        result->replace_start = word_start;
        result->replace_len = word_len;
    }

    return result;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DISPLAY HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_complete_display(const tbos_completions_t* c, int term_width) {
    if (!c || c->count == 0) return;

    /* Find max match length */
    size_t max_len = 0;
    for (size_t i = 0; i < c->count; i++) {
        size_t len = strlen(c->matches[i]);
        if (len > max_len) max_len = len;
    }

    /* Calculate columns */
    int col_width = (int)max_len + 2;  /* 2 spaces padding */
    int cols = term_width / col_width;
    if (cols < 1) cols = 1;

    /* Print matches */
    printf("\n");
    for (size_t i = 0; i < c->count; i++) {
        printf("%-*s", col_width, c->matches[i]);
        if ((i + 1) % cols == 0) {
            printf("\n");
        }
    }
    if (c->count % cols != 0) {
        printf("\n");
    }
}

bool tbos_complete_needs_display(const tbos_completions_t* c) {
    if (!c) return false;

    /* No matches or single match: no display needed */
    if (c->count <= 1) return false;

    /* If common prefix equals all matches, no display needed */
    if (c->common_prefix) {
        bool all_same = true;
        for (size_t i = 0; i < c->count; i++) {
            if (strcmp(c->matches[i], c->common_prefix) != 0) {
                all_same = false;
                break;
            }
        }
        if (all_same) return false;
    }

    return true;
}
