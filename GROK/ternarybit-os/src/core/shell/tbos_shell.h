/**
 * @file tbos_shell.h
 * @brief TernaryBit OS - Conscious Command Shell
 *
 * World-class CUI shell showcasing TBOS innovations:
 * - Traditional commands (ls, cd, pwd, mkdir, rm, cat)
 * - Consciousness-aware commands (karma, sangha, fast, consciousness)
 * - Right Speech validation on all inputs
 * - Karma tracking for user actions
 *
 * From WISDOM.md:
 * > "The shell is where consciousness meets computation"
 *
 * @version 1.0
 * @date 2025-11-04
 */

#ifndef TBOS_SHELL_H
#define TBOS_SHELL_H

#include "../tbos_base.h"
#include "../filesystem/tbos_ramdisk.h"
#include "../../network/core/tbos_network.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* CONSTANTS                                                                 */
/* ========================================================================= */

#define TBOS_SHELL_MAX_INPUT       4096
#define TBOS_SHELL_MAX_ARGS        64
#define TBOS_SHELL_MAX_PATH        1024
#define TBOS_SHELL_HISTORY_SIZE    100
#define TBOS_SHELL_MAX_COMMAND     256

/* Shell result codes */
#define TBOS_SHELL_SUCCESS         0
#define TBOS_SHELL_ERROR           -1
#define TBOS_SHELL_EXIT            1
#define TBOS_SHELL_CONTINUE        2

/* ========================================================================= */
/* SHELL STRUCTURES                                                          */
/* ========================================================================= */

/**
 * @brief Command handler function type
 */
typedef int (*tbos_command_handler_t)(int argc, char** argv);

/**
 * @brief Command registration structure
 */
typedef struct {
    const char* name;
    const char* description;
    const char* usage;
    tbos_command_handler_t handler;
    bool requires_right_speech;  /* Validate input */
    karma_score_t karma_cost;     /* Karma required to run */
    karma_score_t karma_reward;   /* Karma earned on success */
} tbos_shell_command_t;

/**
 * @brief Shell history entry
 */
typedef struct {
    char command[TBOS_SHELL_MAX_INPUT];
    uint64_t timestamp;
    int exit_code;
    karma_score_t karma_impact;
} tbos_shell_history_entry_t;

/**
 * @brief Shell session state
 */
typedef struct {
    /* Current state */
    char cwd[TBOS_SHELL_MAX_PATH];
    char prompt[256];
    char persona[64];

    /* User consciousness */
    consciousness_level_t user_awareness;
    karma_score_t user_karma;
    uint64_t commands_executed;
    uint64_t compassionate_actions;

    /* History */
    tbos_shell_history_entry_t history[TBOS_SHELL_HISTORY_SIZE];
    uint32_t history_count;
    uint32_t history_index;

    /* Configuration */
    bool show_consciousness_prompt;
    bool validate_right_speech;
    bool colorize_output;

    /* Statistics */
    uint64_t session_start_time;
    uint64_t total_karma_earned;
    uint64_t total_karma_lost;
} tbos_shell_session_t;

/* ========================================================================= */
/* SHELL API                                                                 */
/* ========================================================================= */

/**
 * @brief Initialize shell subsystem
 *
 * @return 0 on success, negative on error
 */
int tbos_shell_init(void);

/**
 * @brief Start interactive shell session
 *
 * @return Exit code
 */
int tbos_shell_run(void);

/**
 * @brief Execute a single command
 *
 * @param cmdline Command line string
 * @return Exit code
 */
int tbos_shell_execute(const char* cmdline);

/**
 * @brief Parse command line into argc/argv
 *
 * @param cmdline Command line string
 * @param argc Output: argument count
 * @param argv Output: argument array
 * @return 0 on success, negative on error
 */
int tbos_shell_parse(const char* cmdline, int* argc, char** argv);

/**
 * @brief Register a command handler
 *
 * @param cmd Command definition
 * @return 0 on success, negative on error
 */
int tbos_shell_register_command(const tbos_shell_command_t* cmd);

/**
 * @brief Get current working directory
 *
 * @return Current directory path
 */
const char* tbos_shell_get_cwd(void);

/**
 * @brief Set current working directory
 *
 * @param path New directory path
 * @return 0 on success, negative on error
 */
int tbos_shell_set_cwd(const char* path);

/**
 * @brief Format canonical path for display (handles UCFS/PXFS)
 *
 * @param canonical Canonical POSIX path
 * @param buffer Output buffer
 * @param buflen Buffer length
 * @return Pointer to display string (buffer or canonical)
 */
const char* tbos_shell_format_path(const char* canonical, char* buffer, size_t buflen);

/**
 * @brief Add command to history
 *
 * @param cmdline Command line
 * @param exit_code Exit code
 * @param karma_impact Karma change
 */
void tbos_shell_add_history(const char* cmdline, int exit_code, karma_score_t karma_impact);

/**
 * @brief Print shell prompt
 */
void tbos_shell_print_prompt(void);

/**
 * @brief Print shell status (consciousness, karma, etc.)
 */
void tbos_shell_print_status(void);

/**
 * @brief Get current session
 *
 * @return Pointer to session state
 */
tbos_shell_session_t* tbos_shell_get_session(void);

/* ========================================================================= */
/* BUILT-IN COMMANDS                                                         */
/* ========================================================================= */

/* Traditional commands */
int tbos_cmd_ls(int argc, char** argv);
int tbos_cmd_cd(int argc, char** argv);
int tbos_cmd_pwd(int argc, char** argv);
int tbos_cmd_mkdir(int argc, char** argv);
int tbos_cmd_rmdir(int argc, char** argv);
int tbos_cmd_rm(int argc, char** argv);
int tbos_cmd_cat(int argc, char** argv);
int tbos_cmd_echo(int argc, char** argv);
int tbos_cmd_clear(int argc, char** argv);
int tbos_cmd_exit(int argc, char** argv);
int tbos_cmd_help(int argc, char** argv);
int tbos_cmd_date(int argc, char** argv);
int tbos_cmd_uptime(int argc, char** argv);
int tbos_cmd_whoami(int argc, char** argv);
int tbos_cmd_env(int argc, char** argv);
int tbos_cmd_touch(int argc, char** argv);
int tbos_cmd_cp(int argc, char** argv);
int tbos_cmd_mv(int argc, char** argv);
int tbos_cmd_head(int argc, char** argv);
int tbos_cmd_tail(int argc, char** argv);
int tbos_cmd_posix_shell(int argc, char** argv);

/* Consciousness-aware commands */
int tbos_cmd_karma(int argc, char** argv);
int tbos_cmd_consciousness(int argc, char** argv);
int tbos_cmd_sangha(int argc, char** argv);
int tbos_cmd_fast(int argc, char** argv);
int tbos_cmd_compassion(int argc, char** argv);
int tbos_cmd_history(int argc, char** argv);
int tbos_cmd_metrics(int argc, char** argv);
int tbos_cmd_steppps(int argc, char** argv);
int tbos_cmd_events(int argc, char** argv);

/* Network commands */
int tbos_cmd_http(int argc, char** argv);
int tbos_cmd_ping(int argc, char** argv);
int tbos_cmd_netstat(int argc, char** argv);
int tbos_cmd_persona(int argc, char** argv);

/* System commands */
int tbos_cmd_ps(int argc, char** argv);
int tbos_cmd_top(int argc, char** argv);
int tbos_cmd_shutdown(int argc, char** argv);
int tbos_cmd_hal(int argc, char** argv);

/* UCFS commands */
int tbos_cmd_ucfs_encode(int argc, char** argv);
int tbos_cmd_ucfs_info(int argc, char** argv);
int tbos_cmd_ucfs_test(int argc, char** argv);
int tbos_cmd_ucfs_help(int argc, char** argv);
int tbos_cmd_ucfs_config(int argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_SHELL_H */
