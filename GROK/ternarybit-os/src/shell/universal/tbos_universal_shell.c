/**
 * @file tbos_universal_shell.c
 * @brief TernaryBit OS - Universal Shell Core Implementation
 *
 * Day 1 of 400: Core Infrastructure
 *
 * This is the beginning of our journey to create the world's first
 * truly universal shell. One command per day, with consciousness.
 *
 * @date 2025-11-04
 */

#include "tbos_universal_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * GLOBAL STATE
 * ═══════════════════════════════════════════════════════════════════════════ */

static universal_shell_session_t g_session;
static universal_command_t* g_commands[UNIVERSAL_SHELL_MAX_COMMANDS];
static uint32_t g_command_count = 0;
static bool g_shell_initialized = false;

/* ═══════════════════════════════════════════════════════════════════════════
 * FORWARD DECLARATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_session(void);
static void print_banner(void);
static void print_prompt(void);
static int execute_builtin(const char* cmdline);

/* ═══════════════════════════════════════════════════════════════════════════
 * CORE SHELL INITIALIZATION
 * ═══════════════════════════════════════════════════════════════════════════ */

int universal_shell_init(void) {
    if (g_shell_initialized) {
        return 0;  /* Already initialized */
    }

    /* Initialize session */
    init_session();

    /* Initialize subsystems */
    printf("Initializing TernaryBit OS Universal Shell...\n");

    /* Boot subsystem */
    printf("  [1/6] Minimal Boot... ");
    int result = tbos_minimal_boot();
    if (result != 0) {
        fprintf(stderr, "FAILED\n");
        return -1;
    }
    printf("OK\n");

    /* HAL subsystem */
    printf("  [2/6] Hardware Abstraction Layer... ");
    result = hal_init();
    if (result != 0) {
        fprintf(stderr, "FAILED\n");
        return -1;
    }
    printf("OK\n");

    /* Filesystem subsystem */
    printf("  [3/6] Filesystem Layer (PXFS/UCFS/RF2S)... ");
    result = universal_fs_init();
    if (result != 0) {
        printf("PARTIAL (some filesystems unavailable)\n");
    } else {
        printf("OK\n");
    }

    /* Sangha subsystem */
    printf("  [4/6] Digital Sangha... ");
    if (g_session.enable_sangha) {
        result = sangha_init("UniversalShell", SANGHA_DEVICE_STORAGE);
        if (result == 0) {
            g_session.sangha_connected = true;
            printf("OK\n");
        } else {
            printf("OFFLINE (continuing without sangha)\n");
        }
    } else {
        printf("DISABLED\n");
    }

    /* Register core commands */
    printf("  [5/6] Registering commands... ");
    extern void register_core_commands(void);
    extern void register_week1_commands(void);
    extern void register_week2_commands(void);
    extern void register_week3_commands(void);
    extern void register_week4_commands(void);
    extern void register_week5_commands(void);
    extern void register_week6_commands(void);  // CX completed
    extern void register_week7_commands(void);  // CX completed
    extern void register_week8_commands(void);  // CX completed
    extern void register_week9_commands(void);  // CX completed
    extern void register_week10_commands(void); // CX completed
    register_core_commands();
    register_week1_commands();
    register_week2_commands();
    register_week3_commands();
    register_week4_commands();
    register_week5_commands();
    register_week6_commands();  // ✅ Enabled
    register_week7_commands();  // ✅ Enabled
    register_week8_commands();  // ✅ Enabled
    register_week9_commands();  // ✅ Enabled
    register_week10_commands(); // ✅ Enabled
    printf("%u commands\n", g_command_count);

    /* Final setup */
    printf("  [6/6] Finalizing... ");
    g_shell_initialized = true;
    printf("OK\n\n");

    return 0;
}

static void init_session(void) {
    memset(&g_session, 0, sizeof(g_session));

    /* Set defaults */
    strcpy(g_session.cwd, "/");
    strcpy(g_session.current_directory, "/");
    strcpy(g_session.home, "/home/user");
    strcpy(g_session.username, "user");
    strcpy(g_session.hostname, "tbos");

    g_session.uid = getuid();
    g_session.gid = getgid();

    /* Consciousness defaults */
    g_session.consciousness = CONSCIOUSNESS_AWAKENING;
    g_session.karma = 100;
    g_session.commands_executed = 0;

    /* Configuration */
    g_session.preferred_syntax = SYNTAX_POSIX;
    g_session.enable_syntax_translation = true;
    g_session.enable_pxfs_compression = true;
    g_session.enable_consciousness = true;
    g_session.enable_sangha = true;
    g_session.colorize_output = true;

    /* Session tracking */
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    g_session.session_start_time = ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

int universal_shell_register_command(const char* name,
                                     universal_command_handler_t handler,
                                     command_category_t category,
                                     uint32_t os_support,
                                     const char* description,
                                     const char* usage) {
    if (!name || !handler) {
        return -1;
    }

    universal_command_t cmd;
    memset(&cmd, 0, sizeof(cmd));

    cmd.name = name;
    cmd.description = description ? description : name;
    cmd.usage = usage ? usage : name;
    cmd.examples = NULL;
    cmd.handler = handler;

    cmd.os_support = os_support;
    cmd.category = category;

    /* Consciousness defaults: zero cost, small reward */
    cmd.karma_cost = 0;
    cmd.karma_reward = 1;
    cmd.requires_enlightenment = false;

    /* Capability hints */
    cmd.requires_filesystem = (category == CMD_CAT_FILE_OPS ||
                               category == CMD_CAT_FILESYSTEM ||
                               category == CMD_CAT_COMPRESSION);
    cmd.requires_network = (category == CMD_CAT_NETWORK);
    cmd.requires_root = false;
    cmd.is_destructive = false;

    /* Assume text/scripting commands support pipes by default */
    bool pipeline_friendly = (category == CMD_CAT_TEXT_PROC ||
                              category == CMD_CAT_SCRIPTING ||
                              category == CMD_CAT_MISC);
    cmd.supports_pipes = pipeline_friendly;
    cmd.supports_redirects = pipeline_friendly;
    cmd.is_builtin = (category == CMD_CAT_SHELL_BUILTIN ||
                      category == CMD_CAT_CONSCIOUSNESS);

    return universal_register_command(&cmd);
}

int universal_register_command(const universal_command_t* cmd) {
    if (!cmd || g_command_count >= UNIVERSAL_SHELL_MAX_COMMANDS) {
        return -1;
    }

    /* Allocate and copy command */
    universal_command_t* new_cmd = malloc(sizeof(universal_command_t));
    if (!new_cmd) {
        return -1;
    }

    memcpy(new_cmd, cmd, sizeof(universal_command_t));
    g_commands[g_command_count++] = new_cmd;

    return 0;
}

const universal_command_t* universal_find_command(const char* name) {
    if (!name) return NULL;

    for (uint32_t i = 0; i < g_command_count; i++) {
        /* Check primary name */
        if (strcmp(g_commands[i]->name, name) == 0) {
            return g_commands[i];
        }

        /* Check aliases */
        for (int j = 0; j < 8 && g_commands[i]->aliases[j]; j++) {
            if (strcmp(g_commands[i]->aliases[j], name) == 0) {
                return g_commands[i];
            }
        }
    }

    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN SHELL LOOP
 * ═══════════════════════════════════════════════════════════════════════════ */

int universal_shell_run(void) {
    if (!g_shell_initialized) {
        fprintf(stderr, "Error: Shell not initialized. Call universal_shell_init() first.\n");
        return -1;
    }

    print_banner();

    char input[UNIVERSAL_SHELL_MAX_INPUT];

    while (1) {
        print_prompt();

        /* Read input */
        if (!fgets(input, sizeof(input), stdin)) {
            break;  /* EOF or error */
        }

        /* Remove trailing newline */
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }

        /* Skip empty lines */
        if (strlen(input) == 0) {
            continue;
        }

        /* Check for exit */
        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            break;
        }

        /* Execute command */
        int result = universal_shell_execute(input);

        /* Update session */
        g_session.commands_executed++;

        /* Evolve consciousness periodically */
        if (g_session.enable_consciousness && g_session.commands_executed % 10 == 0) {
            universal_evolve_consciousness();
        }
    }

    /* Graceful shutdown */
    printf("\n");
    printf("═══════════════════════════════════════════════════\n");
    printf("  Graceful Shutdown\n");
    printf("═══════════════════════════════════════════════════\n\n");
    printf("Preserving consciousness state...\n");
    printf("  Final karma: %ld\n", (long)g_session.karma);
    printf("  Commands executed: %lu\n", g_session.commands_executed);
    printf("  Consciousness level: %d\n", g_session.consciousness);
    printf("\n॥ तत् सत् ॥ (That is Truth)\n\n");
    printf("🕉️  May you compute with consciousness! 🕉️\n\n");

    return 0;
}

int universal_shell_execute(const char* cmdline) {
    if (!cmdline || strlen(cmdline) == 0) {
        return 0;
    }

    /* Parse command */
    int argc = 0;
    char** argv = NULL;
    int result = universal_parse_command(cmdline, &argc, &argv);
    if (result != 0 || argc == 0) {
        return -1;
    }

    /* Find command */
    const universal_command_t* cmd = universal_find_command(argv[0]);
    if (!cmd) {
        printf("Command not found: %s\n", argv[0]);
        printf("Type 'help' for available commands.\n");

        /* Free argv */
        for (int i = 0; i < argc; i++) {
            free(argv[i]);
        }
        free(argv);

        return -1;
    }

    /* Check karma requirements */
    if (!universal_has_required_karma(cmd)) {
        printf("Insufficient karma for '%s' (required: %ld, current: %ld)\n",
               cmd->name, (long)cmd->karma_cost, (long)g_session.karma);

        /* Free argv */
        for (int i = 0; i < argc; i++) {
            free(argv[i]);
        }
        free(argv);

        return -1;
    }

    /* Execute command */
    result = cmd->handler(argc, argv);

    /* Update karma */
    universal_update_karma(cmd, result == 0);

    /* Add to history */
    if (g_session.history_count < UNIVERSAL_SHELL_HISTORY_SIZE) {
        shell_history_entry_t* entry = &g_session.history[g_session.history_count++];
        strncpy(entry->command, cmdline, sizeof(entry->command) - 1);

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        entry->timestamp = ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
        entry->exit_code = result;
    }

    /* Free argv */
    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);

    return result;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND PARSING
 * ═══════════════════════════════════════════════════════════════════════════ */

int universal_parse_command(const char* cmdline, int* argc, char*** argv) {
    if (!cmdline || !argc || !argv) {
        return -1;
    }

    *argc = 0;
    *argv = NULL;

    /* Simple whitespace tokenization for now */
    /* TODO: Handle quotes, escapes, etc. */

    char* line = strdup(cmdline);
    if (!line) {
        return -1;
    }

    /* Count tokens */
    int count = 0;
    char* saveptr;
    char* token = strtok_r(line, " \t", &saveptr);
    while (token && count < UNIVERSAL_SHELL_MAX_ARGS) {
        count++;
        token = strtok_r(NULL, " \t", &saveptr);
    }

    if (count == 0) {
        free(line);
        return 0;
    }

    /* Allocate argv */
    char** args = calloc(count + 1, sizeof(char*));
    if (!args) {
        free(line);
        return -1;
    }

    /* Re-tokenize and copy */
    strcpy(line, cmdline);
    token = strtok_r(line, " \t", &saveptr);
    int i = 0;
    while (token && i < count) {
        args[i] = strdup(token);
        if (!args[i]) {
            /* Cleanup on error */
            for (int j = 0; j < i; j++) {
                free(args[j]);
            }
            free(args);
            free(line);
            return -1;
        }
        i++;
        token = strtok_r(NULL, " \t", &saveptr);
    }

    free(line);

    *argc = count;
    *argv = args;

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSCIOUSNESS & KARMA
 * ═══════════════════════════════════════════════════════════════════════════ */

void universal_update_karma(const universal_command_t* cmd, bool success) {
    if (!g_session.enable_consciousness || !cmd) {
        return;
    }

    if (success) {
        g_session.karma += cmd->karma_reward;
        g_session.total_karma_earned += cmd->karma_reward;
    } else {
        g_session.karma -= cmd->karma_cost / 2;  /* Partial penalty for failure */
        g_session.total_karma_lost += cmd->karma_cost / 2;
    }

    /* Ensure karma doesn't go negative */
    if (g_session.karma < 0) {
        g_session.karma = 0;
    }
}

void universal_add_karma(karma_score_t delta, const char* reason) {
    (void)reason; /* Reserved for future logging */

    if (!g_session.enable_consciousness || delta == 0) {
        return;
    }

    g_session.karma += delta;
    if (delta > 0) {
        g_session.total_karma_earned += delta;
        g_session.helpful_actions++;
    } else {
        g_session.total_karma_lost += -delta;
        if (g_session.karma < 0) {
            g_session.karma = 0;
        }
    }

    if (g_session.karma < 0) {
        g_session.karma = 0;
    }
}

bool universal_has_required_karma(const universal_command_t* cmd) {
    if (!cmd) return false;
    if (!g_session.enable_consciousness) return true;

    return g_session.karma >= cmd->karma_cost;
}

void universal_evolve_consciousness(void) {
    /* Evolve based on commands executed and karma */
    uint32_t level = g_session.commands_executed / 50;

    if (level >= 4 && g_session.karma >= 500) {
        g_session.consciousness = CONSCIOUSNESS_ENLIGHTENED;
    } else if (level >= 3 && g_session.karma >= 300) {
        g_session.consciousness = CONSCIOUSNESS_COMPASSIONATE;
    } else if (level >= 2 && g_session.karma >= 150) {
        g_session.consciousness = CONSCIOUSNESS_AWARE;
    } else if (level >= 1 || g_session.karma >= 50) {
        g_session.consciousness = CONSCIOUSNESS_AWAKENING;
    } else {
        g_session.consciousness = CONSCIOUSNESS_NONE;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * FILESYSTEM INTEGRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

int universal_fs_init(void) {
    /* Initialize filesystem layers */
    /* TODO: Implement PXFS/UCFS/RF2S mounting */

    g_session.pxfs_mounted = false;
    g_session.ucfs_mounted = false;
    g_session.rf2s_mounted = false;

    return 0;  /* Partial success */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * UTILITY FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

universal_shell_session_t* universal_get_session(void) {
    return &g_session;
}

static void print_banner(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Universal Shell v%s                 ║\n", UNIVERSAL_SHELL_VERSION);
    printf("║   सॉफ्टवेर (Soft-Aware)                                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    printf("🕉️  Shell initialized successfully!\n");
    printf("    Karma: %ld\n", (long)g_session.karma);
    printf("    Consciousness: %d\n", g_session.consciousness);
    printf("    Commands registered: %u\n\n", g_command_count);

    printf("Type 'help' for available commands.\n");
    printf("Type 'exit' to quit.\n\n");

    printf("Journey: Day 1 of 400 - The foundation is laid.\n");
    printf("One command per day, with consciousness.\n\n");
}

static void print_prompt(void) {
    if (g_session.colorize_output) {
        /* ANSI color codes */
        if (g_session.karma > 200) {
            printf("\033[1;32m");  /* Green for high karma */
        } else if (g_session.karma > 100) {
            printf("\033[1;33m");  /* Yellow for medium karma */
        } else {
            printf("\033[1;31m");  /* Red for low karma */
        }
    }

    const char* prompt_dir = g_session.current_directory[0] ?
        g_session.current_directory : g_session.cwd;

    printf("tbos:%s> ", prompt_dir);

    if (g_session.colorize_output) {
        printf("\033[0m");  /* Reset color */
    }

    fflush(stdout);
}

void universal_shell_shutdown(void) {
    /* Free commands */
    for (uint32_t i = 0; i < g_command_count; i++) {
        free(g_commands[i]);
    }

    g_shell_initialized = false;
}
