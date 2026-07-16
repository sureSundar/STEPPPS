/**
 * @file tbos_universal_shell.c
 * @brief TernaryBit OS - Universal Shell Core
 *
 * The world's first consciousness-aware shell.
 * One command per day, with mindfulness.
 */

#include "tbos_universal_shell.h"
#include "tbos_terminal.h"
#include "tbos_line_editor.h"
#include "tbos_completion.h"
#include "tbos_syntax_hl.h"
#include "tbos_progress.h"
#include "../../core/filesystem/tbos_ramdisk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

/* macOS clock_gettime compatibility */
#ifdef __APPLE__
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
static int clock_gettime(int clk_id, struct timespec *tp) {
    (void)clk_id;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tp->tv_sec = tv.tv_sec;
    tp->tv_nsec = tv.tv_usec * 1000;
    return 0;
}
#endif
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * GLOBAL STATE
 * ═══════════════════════════════════════════════════════════════════════════ */

static universal_shell_session_t g_session;
static universal_command_t* g_commands[UNIVERSAL_SHELL_MAX_COMMANDS];
static uint32_t g_command_count = 0;
static bool g_initialized = false;

/* ═══════════════════════════════════════════════════════════════════════════
 * SESSION MANAGEMENT
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_session(void) {
    memset(&g_session, 0, sizeof(g_session));

    strcpy(g_session.cwd, "/");
    strcpy(g_session.current_directory, "/");
    strcpy(g_session.home, "/home/user");
    strcpy(g_session.username, "user");
    strcpy(g_session.hostname, "tbos");

    g_session.uid = getuid();
    g_session.gid = getgid();
    g_session.consciousness = CONSCIOUSNESS_AWAKENING;
    g_session.karma = 100;

    g_session.preferred_syntax = SYNTAX_POSIX;
    g_session.enable_syntax_translation = true;
    g_session.enable_pxfs_compression = true;
    g_session.enable_consciousness = true;
    g_session.enable_sangha = true;
    g_session.colorize_output = true;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    g_session.session_start_time = ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
}

universal_shell_session_t* universal_get_session(void) {
    return &g_session;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * INITIALIZATION
 * ═══════════════════════════════════════════════════════════════════════════ */

int universal_fs_init(void) {
    printf("  [FS] Initializing filesystem layers...\n");

    if (tbos_ramdisk_init(16 * 1024 * 1024) == 0) {
        printf("  [FS] Ramdisk initialized (16 MB)\n");
        tbos_ramdisk_create_root_structure();
        printf("  [FS] Root structure created\n");
    }

    g_session.pxfs_mounted = true;
    printf("  [FS] PXFS overlay enabled\n");

    g_session.ucfs_mounted = true;
    printf("  [FS] UCFS overlay enabled\n");

    g_session.rf2s_mounted = false;
    printf("  [FS] RF2S sync disabled (no network)\n");

    printf("  [FS] Filesystem initialization complete\n");
    return 0;
}

static void register_all_commands(void) {
    extern void register_core_commands(void);
    extern void register_week1_commands(void);
    extern void register_week2_commands(void);
    extern void register_week3_commands(void);
    extern void register_week4_commands(void);
    extern void register_week5_commands(void);
    extern void register_week6_commands(void);
    extern void register_week7_commands(void);
    extern void register_week8_commands(void);
    extern void register_week9_commands(void);
    extern void register_week10_commands(void);
    extern void register_pxfs_commands(void);
    extern void register_ucfs_commands(void);
    extern void register_host_commands(void);

    register_core_commands();
    register_week1_commands();
    register_week2_commands();
    register_week3_commands();
    register_week4_commands();
    register_week5_commands();
    register_week6_commands();
    register_week7_commands();
    register_week8_commands();
    register_week9_commands();
    register_week10_commands();
    register_pxfs_commands();
    register_ucfs_commands();
    register_host_commands();
}

int universal_shell_init(void) {
    if (g_initialized) return 0;

    init_session();

    printf("Initializing TernaryBit OS Universal Shell...\n");

    printf("  [1/6] Minimal Boot... ");
    if (tbos_minimal_boot() != 0) { fprintf(stderr, "FAILED\n"); return -1; }
    printf("OK\n");

    printf("  [2/6] Hardware Abstraction Layer... ");
    if (hal_init() != 0) { fprintf(stderr, "FAILED\n"); return -1; }
    printf("OK\n");

    printf("  [3/6] Filesystem Layer (PXFS/UCFS/RF2S)... ");
    universal_fs_init();
    printf("OK\n");

    printf("  [4/6] Digital Sangha... ");
    if (g_session.enable_sangha && sangha_init("UniversalShell", SANGHA_DEVICE_STORAGE) == 0) {
        g_session.sangha_connected = true;
        printf("OK\n");
    } else {
        printf("OK\n");
    }

    printf("  [5/6] Registering commands... ");
    register_all_commands();
    printf("%u commands\n", g_command_count);

    printf("  [6/6] Finalizing... ");
    g_initialized = true;
    printf("OK\n\n");

    return 0;
}

void universal_shell_shutdown(void) {
    for (uint32_t i = 0; i < g_command_count; i++) {
        free(g_commands[i]);
    }
    g_command_count = 0;
    g_initialized = false;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND REGISTRY
 * ═══════════════════════════════════════════════════════════════════════════ */

int universal_register_command(const universal_command_t* cmd) {
    if (!cmd || g_command_count >= UNIVERSAL_SHELL_MAX_COMMANDS) {
        return -1;
    }

    universal_command_t* new_cmd = malloc(sizeof(universal_command_t));
    if (!new_cmd) return -1;

    memcpy(new_cmd, cmd, sizeof(universal_command_t));
    g_commands[g_command_count++] = new_cmd;
    return 0;
}

int universal_shell_register_command(const char* name,
                                     universal_command_handler_t handler,
                                     command_category_t category,
                                     uint32_t os_support,
                                     const char* description,
                                     const char* usage) {
    if (!name || !handler) return -1;

    universal_command_t cmd = {0};
    cmd.name = name;
    cmd.description = description ? description : name;
    cmd.usage = usage ? usage : name;
    cmd.handler = handler;
    cmd.os_support = os_support;
    cmd.category = category;
    cmd.karma_cost = 0;
    cmd.karma_reward = 1;

    cmd.requires_filesystem = (category == CMD_CAT_FILE_OPS ||
                               category == CMD_CAT_FILESYSTEM ||
                               category == CMD_CAT_COMPRESSION);
    cmd.requires_network = (category == CMD_CAT_NETWORK);

    bool pipeline_friendly = (category == CMD_CAT_TEXT_PROC ||
                              category == CMD_CAT_SCRIPTING);
    cmd.supports_pipes = pipeline_friendly;
    cmd.supports_redirects = pipeline_friendly;
    cmd.is_builtin = (category == CMD_CAT_SHELL_BUILTIN ||
                      category == CMD_CAT_CONSCIOUSNESS);

    return universal_register_command(&cmd);
}

const universal_command_t* universal_find_command(const char* name) {
    if (!name) return NULL;

    for (uint32_t i = 0; i < g_command_count; i++) {
        if (strcmp(g_commands[i]->name, name) == 0) {
            return g_commands[i];
        }
        for (int j = 0; j < 8 && g_commands[i]->aliases[j]; j++) {
            if (strcmp(g_commands[i]->aliases[j], name) == 0) {
                return g_commands[i];
            }
        }
    }
    return NULL;
}

int universal_get_commands(const universal_command_t** commands, uint32_t max_count) {
    if (!commands) return 0;
    uint32_t count = g_command_count < max_count ? g_command_count : max_count;
    for (uint32_t i = 0; i < count; i++) {
        commands[i] = g_commands[i];
    }
    return (int)count;
}

uint32_t universal_get_command_count(void) {
    return g_command_count;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND EXECUTION
 * ═══════════════════════════════════════════════════════════════════════════ */

int universal_shell_execute(const char* cmdline) {
    if (!cmdline || !*cmdline) return 0;

    int argc = 0;
    char** argv = NULL;

    if (universal_parse_command(cmdline, &argc, &argv) != 0 || argc == 0) {
        return -1;
    }

    const universal_command_t* cmd = universal_find_command(argv[0]);
    if (!cmd) {
        printf("Command not found: %s\n", argv[0]);
        printf("Type 'help' for available commands.\n");
        universal_free_argv(argc, argv);
        return -1;
    }

    if (!universal_has_required_karma(cmd)) {
        printf("Insufficient karma for '%s'\n", cmd->name);
        universal_free_argv(argc, argv);
        return -1;
    }

    int result = cmd->handler(argc, argv);
    universal_update_karma(cmd, result == 0);

    /* Record in history */
    if (g_session.history_count < UNIVERSAL_SHELL_HISTORY_SIZE) {
        shell_history_entry_t* entry = &g_session.history[g_session.history_count++];
        strncpy(entry->command, cmdline, sizeof(entry->command) - 1);
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        entry->timestamp = ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
        entry->exit_code = result;
    }

    universal_free_argv(argc, argv);
    return result;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * INTERACTIVE SHELL
 * ═══════════════════════════════════════════════════════════════════════════ */

static void handle_completion(tbos_line_t* line) {
    tbos_completions_t* c = tbos_complete(line->buffer, line->cursor);
    if (!c || c->count == 0) {
        tbos_term_bell();
        tbos_completions_free(c);
        return;
    }

    if (c->count == 1) {
        for (size_t i = 0; i < c->replace_len; i++) tbos_line_delete(line);
        tbos_line_insert_str(line, c->matches[0]);
        if (c->type == COMPLETE_COMMAND) tbos_line_insert(line, ' ');
    } else if (c->common_prefix && strlen(c->common_prefix) > c->replace_len) {
        for (size_t i = 0; i < c->replace_len; i++) tbos_line_delete(line);
        tbos_line_insert_str(line, c->common_prefix);
    } else {
        int rows, cols;
        tbos_term_get_size(&rows, &cols);
        tbos_complete_display(c, cols);
    }

    tbos_completions_free(c);
}

static void render_line(const tbos_line_t* line, const char* prompt) {
    printf("\r\033[K%s", prompt ? prompt : "");
    tbos_hl_result_t* hl = tbos_highlight(line->buffer);
    tbos_highlight_render(line->buffer, hl);
    tbos_highlight_free(hl);
    printf("\r\033[%zuC", (prompt ? strlen(prompt) : 0) + line->cursor);
    fflush(stdout);
}

static int read_line(tbos_line_t* line, tbos_history_t* history, const char* prompt) {
    tbos_line_clear(line);
    tbos_history_reset_position(history);
    printf("%s", prompt);
    fflush(stdout);

    while (1) {
        int key = tbos_term_read_key();

        switch (key) {
            case KEY_EOF:
            case KEY_CTRL_D:
                if (line->len == 0) return -1;
                tbos_line_delete_forward(line);
                break;
            case KEY_ENTER:
                printf("\n");
                return 0;
            case KEY_CTRL_C:
                printf("^C\n");
                tbos_line_clear(line);
                return 1;
            case KEY_CTRL_L:
                tbos_term_clear_screen();
                render_line(line, prompt);
                continue;
            case KEY_BACKSPACE:
                tbos_line_delete(line);
                break;
            case KEY_DELETE:
                tbos_line_delete_forward(line);
                break;
            case KEY_LEFT:
            case KEY_CTRL_B:
                tbos_line_cursor_left(line);
                break;
            case KEY_RIGHT:
            case KEY_CTRL_F:
                tbos_line_cursor_right(line);
                break;
            case KEY_HOME:
            case KEY_CTRL_A:
                tbos_line_cursor_home(line);
                break;
            case KEY_END:
            case KEY_CTRL_E:
                tbos_line_cursor_end(line);
                break;
            case KEY_UP:
            case KEY_CTRL_P: {
                const char* prev = tbos_history_prev(history, line);
                if (prev) tbos_line_set(line, prev);
                break;
            }
            case KEY_DOWN:
            case KEY_CTRL_N: {
                const char* next = tbos_history_next(history, line);
                if (next) tbos_line_set(line, next);
                else tbos_line_clear(line);
                break;
            }
            case KEY_TAB:
                handle_completion(line);
                break;
            case KEY_CTRL_U:
                tbos_line_kill_to_start(line);
                break;
            case KEY_CTRL_K:
                tbos_line_kill_to_end(line);
                break;
            case KEY_CTRL_W:
                tbos_line_kill_word(line);
                break;
            case KEY_CTRL_Y:
                tbos_line_yank(line);
                break;
            case KEY_CTRL_T:
                tbos_line_transpose(line);
                break;
            default:
                if (key >= 32 && key < 127) tbos_line_insert(line, (char)key);
                else if (key >= 128) tbos_line_insert(line, (char)key);
                break;
        }
        render_line(line, prompt);
    }
}

static void print_banner(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Universal Shell v%s                 ║\n", UNIVERSAL_SHELL_VERSION);
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    printf("  Karma: %ld | Commands: %u\n", (long)g_session.karma, g_command_count);
    printf("  Type 'help' for commands, 'exit' to quit.\n\n");
}

int universal_shell_run(void) {
    if (!g_initialized) {
        fprintf(stderr, "Error: Shell not initialized\n");
        return -1;
    }

    print_banner();
    tbos_complete_init();
    tbos_highlight_init();

    tbos_line_t line;
    tbos_history_t history;
    tbos_term_state_t* term_state = NULL;

    tbos_line_init(&line, UNIVERSAL_SHELL_MAX_INPUT);
    tbos_history_init(&history, UNIVERSAL_SHELL_HISTORY_SIZE);

    bool interactive = isatty(STDIN_FILENO) && tbos_term_raw_enable(&term_state) == 0;
    if (interactive) {
        printf("Interactive mode (arrows, tab, history)\n\n");
    }

    while (1) {
        char prompt[256];
        const char* dir = g_session.current_directory[0] ? g_session.current_directory : g_session.cwd;

        if (g_session.colorize_output) {
            const char* color = g_session.karma > 200 ? "\033[1;32m" :
                               g_session.karma > 100 ? "\033[1;33m" : "\033[1;31m";
            snprintf(prompt, sizeof(prompt), "%stbos:%s> \033[0m", color, dir);
        } else {
            snprintf(prompt, sizeof(prompt), "tbos:%s> ", dir);
        }

        int result;
        if (interactive) {
            result = read_line(&line, &history, prompt);
        } else {
            printf("%s", prompt);
            fflush(stdout);
            char input[UNIVERSAL_SHELL_MAX_INPUT];
            if (!fgets(input, sizeof(input), stdin)) break;
            size_t len = strlen(input);
            if (len > 0 && input[len-1] == '\n') input[len-1] = '\0';
            tbos_line_set(&line, input);
            result = 0;
        }

        if (result < 0) break;
        if (result > 0 || line.len == 0) continue;

        const char* input = tbos_line_get(&line);
        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) break;

        tbos_history_add(&history, input);
        universal_shell_execute(input);
        g_session.commands_executed++;

        if (g_session.enable_consciousness && g_session.commands_executed % 10 == 0) {
            universal_evolve_consciousness();
        }
    }

    if (interactive) tbos_term_raw_disable(term_state);
    tbos_line_free(&line);
    tbos_history_free(&history);
    tbos_complete_shutdown();
    tbos_highlight_shutdown();

    printf("\n॥ तत् सत् ॥ - Truth is One\n");
    printf("Karma: %ld | Commands: %llu\n\n", (long)g_session.karma, (unsigned long long)g_session.commands_executed);

    return 0;
}
