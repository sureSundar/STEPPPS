/**
 * @file tbos_shell.c
 * @brief TernaryBit OS - Conscious Command Shell Implementation
 */

#include "tbos_shell.h"
#include "../lifecycle/tbos_lifecycle.h"
#include "../karma/tbos_karma_ledger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "fs/ucfs_overlay.h"
#include "fs/pxfs_overlay.h"

/* ========================================================================= */
/* GLOBAL STATE                                                              */
/* ========================================================================= */

static tbos_shell_session_t g_session;
static tbos_shell_command_t* g_commands = NULL;
static uint32_t g_command_count = 0;
static bool g_shell_initialized = false;

static void tbos_shell_sanitize_input(char* input) {
    if (!input) return;
    size_t len = strlen(input);
    size_t start = 0;
    if (len >= 3 &&
        (unsigned char)input[0] == 0xEF &&
        (unsigned char)input[1] == 0xBB &&
        (unsigned char)input[2] == 0xBF) {
        start = 3;
    }
    while (start < len &&
           ((unsigned char)input[start] < 32) &&
           input[start] != '\0' &&
           input[start] != '\n') {
        start++;
    }
    if (start > 0) {
        memmove(input, input + start, len - start + 1);
    }
}

/* ========================================================================= */
/* HELPER FUNCTIONS                                                          */
/* ========================================================================= */

/**
 * @brief Validate command input follows Right Speech
 */
static bool validate_right_speech_input(const char* input) {
    if (!input || strlen(input) == 0) return false;

    /* Check for harmful commands */
    if (strstr(input, "rm -rf /") != NULL) {
        printf("❌ Right Speech violation: Potentially destructive command\n");
        return false;
    }

    /* Check for excessive length (denial of service) */
    if (strlen(input) > TBOS_SHELL_MAX_INPUT) {
        printf("❌ Right Speech violation: Input too long\n");
        return false;
    }

    return true;
}

const char* tbos_shell_format_path(const char* canonical, char* buffer, size_t buflen) {
    if (!canonical || !buffer || buflen == 0) {
        return canonical;
    }
    const char* formatted = canonical;
    char temp[512];
    if (strncmp(canonical, "/ucfs", 5) == 0) {
        formatted = ucfs_prompt_form(canonical, temp, sizeof(temp));
    } else if (strncmp(canonical, "/pxfs", 5) == 0) {
        formatted = pxfs_prompt_form(canonical, temp, sizeof(temp));
    }
    strncpy(buffer, formatted, buflen - 1);
    buffer[buflen - 1] = '\0';
    return buffer;
}

/**
 * @brief Update user karma based on action
 */
static void update_user_karma(karma_score_t delta, const char* reason) {
    g_session.user_karma += delta;

    if (delta > 0) {
        g_session.total_karma_earned += delta;
    } else if (delta < 0) {
        g_session.total_karma_lost += (-delta);
    }

    if (delta != 0) {
        tbos_karma_ledger_record(g_session.persona,
                                 delta,
                                 g_session.user_karma,
                                 reason);
    }
}

/**
 * @brief Get consciousness level name
 */
static const char* get_consciousness_name(consciousness_level_t level) {
    switch (level) {
        case CONSCIOUSNESS_NONE: return "NONE";
        case CONSCIOUSNESS_AWAKENING: return "AWAKENING";
        case CONSCIOUSNESS_AWARE: return "AWARE";
        case CONSCIOUSNESS_COMPASSIONATE: return "COMPASSIONATE";
        case CONSCIOUSNESS_ENLIGHTENED: return "ENLIGHTENED";
        default: return "UNKNOWN";
    }
}

/* ========================================================================= */
/* SHELL INITIALIZATION                                                      */
/* ========================================================================= */

int tbos_shell_init(void) {
    if (g_shell_initialized) {
        return TBOS_SHELL_SUCCESS;
    }

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Conscious Shell Initialization            ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    /* Initialize session */
    memset(&g_session, 0, sizeof(g_session));
    strcpy(g_session.cwd, "/");
    strcpy(g_session.prompt, "tbos> ");
    g_session.user_awareness = CONSCIOUSNESS_AWAKENING;
    g_session.user_karma = 100;
    g_session.show_consciousness_prompt = true;
    g_session.validate_right_speech = true;
    g_session.colorize_output = true;
    g_session.session_start_time = time(NULL);
    const char* persona = getenv("TBOS_PERSONA_NAME");
    if (!persona || !persona[0]) {
        persona = "desktop_512m";
    }
    strncpy(g_session.persona, persona, sizeof(g_session.persona) - 1);
    g_session.persona[sizeof(g_session.persona) - 1] = '\0';

    tbos_karma_ledger_init();
    karma_score_t ledger_total = 0;
    if (tbos_karma_ledger_get_total(g_session.persona, &ledger_total) == 0) {
        g_session.user_karma = ledger_total;
    }

    /* Allocate command registry */
    g_commands = malloc(sizeof(tbos_shell_command_t) * 50);
    if (!g_commands) {
        printf("❌ Failed to allocate command registry\n");
        return TBOS_SHELL_ERROR;
    }

    /* Register built-in commands */
    printf("Registering built-in commands...\n");

    /* Traditional commands */
    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "ls",
        .description = "List directory contents",
        .usage = "ls [path]",
        .handler = tbos_cmd_ls,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 1
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "cd",
        .description = "Change directory",
        .usage = "cd <path>",
        .handler = tbos_cmd_cd,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 1
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "pwd",
        .description = "Print working directory",
        .usage = "pwd",
        .handler = tbos_cmd_pwd,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 1
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "mkdir",
        .description = "Create directory",
        .usage = "mkdir <path>",
        .handler = tbos_cmd_mkdir,
        .requires_right_speech = true,
        .karma_cost = 5,
        .karma_reward = 10
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "rmdir",
        .description = "Remove directory",
        .usage = "rmdir <path>",
        .handler = tbos_cmd_rmdir,
        .requires_right_speech = true,
        .karma_cost = 5,
        .karma_reward = 2
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "rm",
        .description = "Remove file",
        .usage = "rm <path>",
        .handler = tbos_cmd_rm,
        .requires_right_speech = true,
        .karma_cost = 10,
        .karma_reward = 5
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "cat",
        .description = "Display file contents",
        .usage = "cat <path>",
        .handler = tbos_cmd_cat,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 2
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "echo",
        .description = "Display text",
        .usage = "echo <text>",
        .handler = tbos_cmd_echo,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 1
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "clear",
        .description = "Clear screen",
        .usage = "clear",
        .handler = tbos_cmd_clear,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 0
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "help",
        .description = "Show available commands",
        .usage = "help [command]",
        .handler = tbos_cmd_help,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 2
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "date",
        .description = "Show system date/time",
        .usage = "date",
        .handler = tbos_cmd_date,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 1
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "uptime",
        .description = "Show session uptime",
        .usage = "uptime",
        .handler = tbos_cmd_uptime,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 1
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "whoami",
        .description = "Show persona/user",
        .usage = "whoami",
        .handler = tbos_cmd_whoami,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 1
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "env",
        .description = "List environment variables",
        .usage = "env",
        .handler = tbos_cmd_env,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 1
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "touch",
        .description = "Create empty file",
        .usage = "touch <file> [file...]",
        .handler = tbos_cmd_touch,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 1
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "cp",
        .description = "Copy file",
        .usage = "cp <source> <dest>",
        .handler = tbos_cmd_cp,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 2
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "mv",
        .description = "Move/Rename file",
        .usage = "mv <source> <dest>",
        .handler = tbos_cmd_mv,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 2
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "head",
        .description = "Show beginning of file",
        .usage = "head [-n lines] <file>",
        .handler = tbos_cmd_head,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 1
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "tail",
        .description = "Show end of file",
        .usage = "tail [-n lines] <file>",
        .handler = tbos_cmd_tail,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 1
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "sh",
        .description = "Launch BusyBox shell",
        .usage = "sh",
        .handler = tbos_cmd_posix_shell,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 5
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "exit",
        .description = "Exit shell",
        .usage = "exit",
        .handler = tbos_cmd_exit,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 0
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "persona",
        .description = "Supershell persona controls",
        .usage = "persona list | persona morph <persona_id>",
        .handler = tbos_cmd_persona,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 0
    });

    /* Consciousness-aware commands */
    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "karma",
        .description = "Show karma status",
        .usage = "karma",
        .handler = tbos_cmd_karma,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 5
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "consciousness",
        .description = "Show consciousness level",
        .usage = "consciousness",
        .handler = tbos_cmd_consciousness,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 5
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "history",
        .description = "Show command history",
        .usage = "history",
        .handler = tbos_cmd_history,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 2
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "metrics",
        .description = "Show consciousness/watt metrics",
        .usage = "metrics [components|system|transform|all]",
        .handler = tbos_cmd_metrics,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 5
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "events",
        .description = "Show recent STEPPPS events",
        .usage = "events [count]",
        .handler = tbos_cmd_events,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 2
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "steppps",
        .description = "STEPPPS 7D computing framework",
        .usage = "steppps [space|time|event|psychology|pixel|prompt|script|evolve|help]",
        .handler = tbos_cmd_steppps,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 10
    });

    /* System commands */
    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "ps",
        .description = "Show process list",
        .usage = "ps",
        .handler = tbos_cmd_ps,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 3
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "top",
        .description = "Show scheduler status",
        .usage = "top",
        .handler = tbos_cmd_top,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 3
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "hal",
        .description = "Display HAL hardware profile",
        .usage = "hal",
        .handler = tbos_cmd_hal,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 2
    });

    /* Network commands */
    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "http",
        .description = "Mindful HTTP requests",
        .usage = "http <get|post> <url>",
        .handler = tbos_cmd_http,
        .requires_right_speech = true,
        .karma_cost = 5,
        .karma_reward = 10
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "ping",
        .description = "Ping host with consciousness",
        .usage = "ping <host>",
        .handler = tbos_cmd_ping,
        .requires_right_speech = false,
        .karma_cost = 2,
        .karma_reward = 5
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "netstat",
        .description = "Show network status",
        .usage = "netstat",
        .handler = tbos_cmd_netstat,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 3
    });

    /* UCFS Commands */
    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "ucfs-encode",
        .description = "Show canonical path for UCFS path",
        .usage = "ucfs-encode <ucfs-path>",
        .handler = tbos_cmd_ucfs_encode,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 2
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "ucfs-info",
        .description = "Display detailed UCFS path information",
        .usage = "ucfs-info <ucfs-path>",
        .handler = tbos_cmd_ucfs_info,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 2
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "ucfs-test",
        .description = "Run UCFS functionality tests",
        .usage = "ucfs-test",
        .handler = tbos_cmd_ucfs_test,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 5
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "ucfs-help",
        .description = "Show UCFS help and examples",
        .usage = "ucfs-help",
        .handler = tbos_cmd_ucfs_help,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 1
    });

    tbos_shell_register_command(&(tbos_shell_command_t){
        .name = "ucfs-config",
        .description = "Manage UCFS configuration",
        .usage = "ucfs-config <list|show|save>",
        .handler = tbos_cmd_ucfs_config,
        .requires_right_speech = false,
        .karma_cost = 0,
        .karma_reward = 2
    });

    printf("✅ Registered %u commands\n", g_command_count);
    printf("✅ Shell initialized with consciousness level: %s\n",
           get_consciousness_name(g_session.user_awareness));
    printf("✅ Starting karma: %ld\n\n", g_session.user_karma);

    g_shell_initialized = true;
    return TBOS_SHELL_SUCCESS;
}

/* ========================================================================= */
/* COMMAND PARSING                                                           */
/* ========================================================================= */

int tbos_shell_parse(const char* cmdline, int* argc, char** argv) {
    if (!cmdline || !argc || !argv) return TBOS_SHELL_ERROR;

    static char buffer[TBOS_SHELL_MAX_INPUT];
    strncpy(buffer, cmdline, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    *argc = 0;
    char* token = strtok(buffer, " \t\n");

    while (token && *argc < TBOS_SHELL_MAX_ARGS) {
        argv[(*argc)++] = token;
        token = strtok(NULL, " \t\n");
    }

    return TBOS_SHELL_SUCCESS;
}

/* ========================================================================= */
/* COMMAND EXECUTION                                                         */
/* ========================================================================= */

int tbos_shell_execute(const char* cmdline) {
    if (!cmdline || strlen(cmdline) == 0) {
        return TBOS_SHELL_CONTINUE;
    }

    /* Validate Right Speech if enabled */
    if (g_session.validate_right_speech) {
        if (!validate_right_speech_input(cmdline)) {
            update_user_karma(-10, "Right speech violation");
            return TBOS_SHELL_ERROR;
        }
    }

    /* Parse command */
    int argc = 0;
    char* argv[TBOS_SHELL_MAX_ARGS];
    if (tbos_shell_parse(cmdline, &argc, argv) != TBOS_SHELL_SUCCESS) {
        printf("❌ Failed to parse command\n");
        return TBOS_SHELL_ERROR;
    }

    if (argc == 0) return TBOS_SHELL_CONTINUE;

    /* Find command handler */
    tbos_shell_command_t* cmd = NULL;
    for (uint32_t i = 0; i < g_command_count; i++) {
        if (strcmp(g_commands[i].name, argv[0]) == 0) {
            cmd = &g_commands[i];
            break;
        }
    }

    if (!cmd) {
        printf("❌ Command not found: %s\n", argv[0]);
        printf("   Type 'help' for available commands\n");
        return TBOS_SHELL_ERROR;
    }

    /* Check karma cost */
    if (g_session.user_karma < cmd->karma_cost) {
        printf("❌ Insufficient karma: need %ld, have %ld\n",
               cmd->karma_cost, g_session.user_karma);
        return TBOS_SHELL_ERROR;
    }

    /* Execute command */
    karma_score_t karma_before = g_session.user_karma;
    if (cmd->karma_cost != 0) {
        char cost_reason[128];
        snprintf(cost_reason, sizeof(cost_reason),
                 "Command cost: %s", cmd->name);
        update_user_karma(-cmd->karma_cost, cost_reason);
    }

    int result = cmd->handler(argc, argv);

    if (result == TBOS_SHELL_SUCCESS) {
        if (cmd->karma_reward != 0) {
            char reward_reason[128];
            snprintf(reward_reason, sizeof(reward_reason),
                     "Command reward: %s", cmd->name);
            update_user_karma(cmd->karma_reward, reward_reason);
        }
    }

    karma_score_t karma_delta = g_session.user_karma - karma_before;
    g_session.commands_executed++;

    /* Add to history */
    tbos_shell_add_history(cmdline, result, karma_delta);

    return result;
}

/* ========================================================================= */
/* COMMAND REGISTRATION                                                      */
/* ========================================================================= */

int tbos_shell_register_command(const tbos_shell_command_t* cmd) {
    if (!cmd || !cmd->name || !cmd->handler) {
        return TBOS_SHELL_ERROR;
    }

    g_commands[g_command_count++] = *cmd;
    return TBOS_SHELL_SUCCESS;
}

/* ========================================================================= */
/* SESSION MANAGEMENT                                                        */
/* ========================================================================= */

const char* tbos_shell_get_cwd(void) {
    return g_session.cwd;
}

int tbos_shell_set_cwd(const char* path) {
    if (!path) return TBOS_SHELL_ERROR;

    strncpy(g_session.cwd, path, sizeof(g_session.cwd) - 1);
    g_session.cwd[sizeof(g_session.cwd) - 1] = '\0';

    return TBOS_SHELL_SUCCESS;
}

void tbos_shell_add_history(const char* cmdline, int exit_code, karma_score_t karma_impact) {
    if (g_session.history_count < TBOS_SHELL_HISTORY_SIZE) {
        tbos_shell_history_entry_t* entry = &g_session.history[g_session.history_count++];
        strncpy(entry->command, cmdline, sizeof(entry->command) - 1);
        entry->timestamp = time(NULL);
        entry->exit_code = exit_code;
        entry->karma_impact = karma_impact;
    }
}

tbos_shell_session_t* tbos_shell_get_session(void) {
    return &g_session;
}

/* ========================================================================= */
/* PROMPT                                                                    */
/* ========================================================================= */

void tbos_shell_print_prompt(void) {
    if (g_session.show_consciousness_prompt) {
        printf("[🕉️  Karma: %ld | %s] ",
               g_session.user_karma,
               get_consciousness_name(g_session.user_awareness));
    }
    char display[256];
    const char* friendly = tbos_shell_format_path(g_session.cwd, display, sizeof(display));
    printf("%s> ", friendly);
    fflush(stdout);
}

void tbos_shell_print_status(void) {
    uint64_t session_duration = time(NULL) - g_session.session_start_time;

    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      TernaryBit OS - Shell Session Status             ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    char display[256];
    tbos_shell_format_path(g_session.cwd, display, sizeof(display));
    printf("║ Current Directory:     %-30s  ║\n", display);
    printf("║ Consciousness:         %-30s  ║\n",
           get_consciousness_name(g_session.user_awareness));
    printf("║ Karma:                 %-10ld                    ║\n",
           g_session.user_karma);
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Commands Executed:     %-10lu                    ║\n",
           g_session.commands_executed);
    printf("║ Karma Earned:          %-10lu                    ║\n",
           g_session.total_karma_earned);
    printf("║ Karma Lost:            %-10lu                    ║\n",
           g_session.total_karma_lost);
    printf("║ Session Duration:      %-10lu seconds           ║\n",
           session_duration);
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

/* ========================================================================= */
/* INTERACTIVE SHELL                                                         */
/* ========================================================================= */

int tbos_shell_run(void) {
    if (!g_shell_initialized) {
        tbos_shell_init();
    }

    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   Welcome to TernaryBit OS - Conscious Shell                ║\n");
    printf("║                                                              ║\n");
    printf("║   Where Silicon Meets Spirituality                          ║\n");
    printf("║   Type 'help' for commands                                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    char input[TBOS_SHELL_MAX_INPUT];
    int result = TBOS_SHELL_CONTINUE;

    while (result != TBOS_SHELL_EXIT) {
        const char* shutdown_reason = NULL;
        if (tbos_lifecycle_should_shutdown(&shutdown_reason)) {
            if (shutdown_reason) {
                printf("\n[Lifecycle] %s\n\n", shutdown_reason);
            }
            break;
        }

        tbos_shell_print_prompt();

        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        /* Remove trailing newline */
        input[strcspn(input, "\n")] = '\0';
        tbos_shell_sanitize_input(input);

        result = tbos_shell_execute(input);
        tbos_lifecycle_record_command(input, result);

        if (result != TBOS_SHELL_EXIT &&
            tbos_lifecycle_should_shutdown(&shutdown_reason)) {
            if (shutdown_reason) {
                printf("\n[Lifecycle] %s\n\n", shutdown_reason);
            }
            break;
        }
    }

    tbos_lifecycle_session_end();

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   Shell Session Complete                                    ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Final Karma:           %-10ld                          ║\n",
           g_session.user_karma);
    printf("║ Commands Executed:     %-10lu                          ║\n",
           g_session.commands_executed);
    printf("║                                                              ║\n");
    printf("║   May your consciousness continue to grow                   ║\n");
    printf("║   ॥ तत् सत् ॥                                               ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    return TBOS_SHELL_SUCCESS;
}
