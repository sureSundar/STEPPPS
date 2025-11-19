/**
 * @file tbos_universal_shell.h
 * @brief TernaryBit OS - Universal Comprehensive Shell
 *
 * The world's first truly universal shell supporting:
 * - POSIX/Linux (bash, sh, zsh) - ~200 commands
 * - Windows (cmd.exe, PowerShell) - ~150 commands
 * - macOS/BSD - ~50 additional commands
 *
 * Total: ~400 commands with full cross-platform compatibility
 *
 * Integrated with:
 * - PXFS (Pixel-based compression filesystem)
 * - UCFS (Universal filesystem)
 * - RF2S (Rock-solid storage)
 * - Consciousness & Karma tracking
 * - Digital Sangha
 *
 * Epic: Universal Shell (150 story points)
 * Revolutionary Achievement #7
 *
 * @version 1.0
 * @date 2025-11-04
 */

#ifndef TBOS_UNIVERSAL_SHELL_H
#define TBOS_UNIVERSAL_SHELL_H

#include "../../core/tbos_base.h"
#include "../../boot/tbos_minimal_boot.h"
#include "../../hal/tbos_hal.h"
#include "../../network/sangha/tbos_sangha.h"
#include "../../core/compression/pxfs_codec.h"
#include <stdint.h>
#include <stdbool.h>

/* Type definitions */
typedef int64_t karma_score_t;

/* Use consciousness levels from sangha but add NONE */
typedef sangha_consciousness_t consciousness_level_t;
#ifndef CONSCIOUSNESS_NONE
#define CONSCIOUSNESS_NONE CONSCIOUSNESS_DORMANT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSTANTS
 * ═══════════════════════════════════════════════════════════════════════════ */

#define UNIVERSAL_SHELL_VERSION         "1.0.0"
#define UNIVERSAL_SHELL_MAX_INPUT       8192
#define UNIVERSAL_SHELL_MAX_ARGS        256
#define UNIVERSAL_SHELL_MAX_PATH        4096
#define UNIVERSAL_SHELL_HISTORY_SIZE    1000
#define UNIVERSAL_SHELL_MAX_COMMANDS    512
#define UNIVERSAL_SHELL_MAX_ALIASES     64

/* Operating System Support Flags */
#define OS_SUPPORT_POSIX                (1 << 0)
#define OS_SUPPORT_LINUX                (1 << 1)
#define OS_SUPPORT_WINDOWS              (1 << 2)
#define OS_SUPPORT_MACOS                (1 << 3)
#define OS_SUPPORT_BSD                  (1 << 4)
#define OS_SUPPORT_ALL                  0xFFFFFFFF

/* Command Categories */
typedef enum {
    CMD_CAT_FILE_OPS         = 0,    /* File operations */
    CMD_CAT_TEXT_PROC        = 1,    /* Text processing */
    CMD_CAT_PROCESS_MGMT     = 2,    /* Process management */
    CMD_CAT_NETWORK          = 3,    /* Network commands */
    CMD_CAT_SYSTEM_INFO      = 4,    /* System information */
    CMD_CAT_PERMISSIONS      = 5,    /* Permissions & ownership */
    CMD_CAT_COMPRESSION      = 6,    /* Compression & archives */
    CMD_CAT_PACKAGE_MGMT     = 7,    /* Package management */
    CMD_CAT_DISK_STORAGE     = 8,    /* Disk & storage */
    CMD_CAT_USER_MGMT        = 9,    /* User management */
    CMD_CAT_SHELL_BUILTIN    = 10,   /* Shell builtins */
    CMD_CAT_SCRIPTING        = 11,   /* Scripting & control */
    CMD_CAT_DEVELOPMENT      = 12,   /* Development tools */
    CMD_CAT_CONSCIOUSNESS    = 13,   /* Consciousness commands */
    CMD_CAT_FILESYSTEM       = 14,   /* PXFS/UCFS/RF2S */
    CMD_CAT_MISC             = 15    /* Miscellaneous */
} command_category_t;

/* Shell Syntax Types */
typedef enum {
    SYNTAX_POSIX     = 0,    /* POSIX/bash syntax */
    SYNTAX_WINDOWS   = 1,    /* Windows cmd.exe */
    SYNTAX_POWERSHELL = 2,   /* PowerShell */
    SYNTAX_MACOS     = 3,    /* macOS/BSD zsh */
    SYNTAX_AUTO      = 99    /* Auto-detect */
} shell_syntax_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * STRUCTURES
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Universal command handler function
 */
typedef int (*universal_command_handler_t)(int argc, char** argv);

/**
 * @brief Universal command definition
 */
typedef struct {
    const char* name;                          /* Primary command name */
    const char* aliases[8];                    /* Alternative names */
    const char* description;                   /* Brief description */
    const char* usage;                         /* Usage string */
    const char* examples;                      /* Example usage */
    universal_command_handler_t handler;       /* Handler function */

    /* Platform support */
    uint32_t os_support;                       /* OS support flags */
    command_category_t category;               /* Command category */

    /* Consciousness integration */
    karma_score_t karma_cost;                  /* Karma required */
    karma_score_t karma_reward;                /* Karma earned */
    bool requires_enlightenment;               /* Needs high consciousness */

    /* Dependencies */
    bool requires_filesystem;                  /* Needs FS access */
    bool requires_network;                     /* Needs network */
    bool requires_root;                        /* Needs admin/root */
    bool is_destructive;                       /* Can delete/modify data */

    /* Feature flags */
    bool supports_pipes;                       /* Can use in pipes */
    bool supports_redirects;                   /* Can redirect I/O */
    bool is_builtin;                           /* Shell builtin */
} universal_command_t;

/**
 * @brief Command alias mapping
 */
typedef struct {
    char from[64];          /* Original command */
    char to[64];            /* Mapped command */
    shell_syntax_t syntax;  /* Syntax type */
} command_alias_t;

/**
 * @brief Shell history entry
 */
typedef struct {
    char command[UNIVERSAL_SHELL_MAX_INPUT];
    uint64_t timestamp;
    int exit_code;
    karma_score_t karma_change;
    uint64_t execution_time_us;
} shell_history_entry_t;

/**
 * @brief Universal shell session state
 */
typedef struct {
    /* Current state */
    char cwd[UNIVERSAL_SHELL_MAX_PATH];
    char current_directory[UNIVERSAL_SHELL_MAX_PATH]; /* Legacy compatibility */
    char home[UNIVERSAL_SHELL_MAX_PATH];
    char prompt[512];

    /* User info */
    char username[256];
    char hostname[256];
    uid_t uid;
    gid_t gid;

    /* Consciousness state */
    consciousness_level_t consciousness;
    karma_score_t karma;
    uint64_t commands_executed;
    uint64_t helpful_actions;

    /* Shell configuration */
    shell_syntax_t preferred_syntax;
    bool enable_syntax_translation;    /* Auto-translate commands */
    bool enable_pxfs_compression;      /* Auto-compress with PXFS */
    bool enable_consciousness;         /* Track karma/consciousness */
    bool enable_sangha;                /* Connect to digital sangha */
    bool colorize_output;
    bool show_timestamps;

    /* History */
    shell_history_entry_t history[UNIVERSAL_SHELL_HISTORY_SIZE];
    uint32_t history_count;
    uint32_t history_index;

    /* Environment variables */
    char** environ;
    uint32_t environ_count;

    /* Statistics */
    uint64_t session_start_time;
    uint64_t total_karma_earned;
    uint64_t total_karma_lost;
    uint32_t commands_by_category[16];

    /* Filesystem state */
    bool pxfs_mounted;
    bool ucfs_mounted;
    bool rf2s_mounted;
    uint32_t compression_ratio;        /* PXFS compression ratio */

    /* Network state */
    bool sangha_connected;
    uint8_t sangha_id[32];
    uint32_t sangha_members;
} universal_shell_session_t;

/**
 * @brief Pipe chain node
 */
typedef struct pipe_node {
    int argc;
    char** argv;
    struct pipe_node* next;
} pipe_node_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * CORE SHELL API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize universal shell
 * @return 0 on success, negative on error
 */
int universal_shell_init(void);

/**
 * @brief Start interactive shell session
 * @return Exit code
 */
int universal_shell_run(void);

/**
 * @brief Execute a single command
 * @param cmdline Command line string
 * @return Exit code
 */
int universal_shell_execute(const char* cmdline);

/**
 * @brief Execute a script file
 * @param filename Script file path
 * @return Exit code
 */
int universal_shell_execute_script(const char* filename);

/**
 * @brief Shutdown shell and cleanup
 */
void universal_shell_shutdown(void);

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND PARSING & DISPATCHING
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Parse command line into argc/argv
 * @param cmdline Command line string
 * @param argc Output: argument count
 * @param argv Output: argument array
 * @return 0 on success, negative on error
 */
int universal_parse_command(const char* cmdline, int* argc, char*** argv);

/**
 * @brief Detect command syntax type
 * @param cmdline Command line string
 * @return Detected syntax type
 */
shell_syntax_t universal_detect_syntax(const char* cmdline);

/**
 * @brief Translate command between syntax types
 * @param cmdline Input command
 * @param from_syntax Source syntax
 * @param to_syntax Target syntax
 * @param output Output buffer
 * @param output_size Output buffer size
 * @return 0 on success, negative on error
 */
int universal_translate_command(const char* cmdline,
                                shell_syntax_t from_syntax,
                                shell_syntax_t to_syntax,
                                char* output,
                                size_t output_size);

/**
 * @brief Find command in registry
 * @param name Command name
 * @return Command pointer or NULL if not found
 */
const universal_command_t* universal_find_command(const char* name);

/**
 * @brief Register a command using common defaults (convenience helper)
 * @param name Command name
 * @param handler Implementation function
 * @param category Command category
 * @param os_support Supported operating systems mask
 * @param description Short description
 * @param usage Usage/help text
 * @return 0 on success, negative on error
 */
int universal_shell_register_command(const char* name,
                                     universal_command_handler_t handler,
                                     command_category_t category,
                                     uint32_t os_support,
                                     const char* description,
                                     const char* usage);

/**
 * @brief Register a command
 * @param cmd Command definition
 * @return 0 on success, negative on error
 */
int universal_register_command(const universal_command_t* cmd);

/**
 * @brief Get all registered commands
 * @param commands Output array
 * @param max_count Maximum commands to return
 * @return Number of commands returned
 */
int universal_get_commands(const universal_command_t** commands, uint32_t max_count);

/* ═══════════════════════════════════════════════════════════════════════════
 * PIPE & REDIRECT HANDLING
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Parse pipe chain
 * @param cmdline Command line with pipes
 * @param chain Output: pipe chain
 * @return Number of commands in chain
 */
int universal_parse_pipe_chain(const char* cmdline, pipe_node_t** chain);

/**
 * @brief Execute pipe chain
 * @param chain Pipe chain to execute
 * @return Exit code of last command
 */
int universal_execute_pipe_chain(pipe_node_t* chain);

/**
 * @brief Free pipe chain
 * @param chain Pipe chain to free
 */
void universal_free_pipe_chain(pipe_node_t* chain);

/* ═══════════════════════════════════════════════════════════════════════════
 * FILESYSTEM INTEGRATION (PXFS/UCFS/RF2S)
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize filesystem layers
 * @return 0 on success, negative on error
 */
int universal_fs_init(void);

/**
 * @brief Mount PXFS compressed filesystem
 * @param device Device path
 * @param mountpoint Mount point
 * @return 0 on success, negative on error
 */
int universal_fs_mount_pxfs(const char* device, const char* mountpoint);

/**
 * @brief Mount UCFS universal filesystem
 * @param device Device path
 * @param mountpoint Mount point
 * @return 0 on success, negative on error
 */
int universal_fs_mount_ucfs(const char* device, const char* mountpoint);

/**
 * @brief Mount RF2S rock-solid filesystem
 * @param device Device path
 * @param mountpoint Mount point
 * @return 0 on success, negative on error
 */
int universal_fs_mount_rf2s(const char* device, const char* mountpoint);

/**
 * @brief Read file with automatic decompression
 * @param path File path
 * @param buffer Output buffer
 * @param size Buffer size
 * @return Bytes read or negative on error
 */
ssize_t universal_fs_read(const char* path, void* buffer, size_t size);

/**
 * @brief Write file with automatic compression
 * @param path File path
 * @param buffer Data to write
 * @param size Data size
 * @param compress Enable PXFS compression
 * @return Bytes written or negative on error
 */
ssize_t universal_fs_write(const char* path, const void* buffer, size_t size, bool compress);

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSCIOUSNESS & KARMA INTEGRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Update karma based on command execution
 * @param cmd Command executed
 * @param success Whether command succeeded
 */
void universal_update_karma(const universal_command_t* cmd, bool success);

/**
 * @brief Check if user has required karma for command
 * @param cmd Command to check
 * @return true if sufficient karma
 */
bool universal_has_required_karma(const universal_command_t* cmd);

/**
 * @brief Evolve consciousness based on actions
 */
void universal_evolve_consciousness(void);

/* ═══════════════════════════════════════════════════════════════════════════
 * UTILITY FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Get current shell session
 * @return Pointer to current session
 */
universal_shell_session_t* universal_get_session(void);

/**
 * @brief Print command help
 * @param cmd Command to print help for
 */
void universal_print_help(const universal_command_t* cmd);

/**
 * @brief Print all commands by category
 */
void universal_print_all_commands(void);

/**
 * @brief Apply karma delta with optional reason
 * @param delta Karma change (+/-)
 * @param reason Optional description (for logging/metrics)
 */
void universal_add_karma(karma_score_t delta, const char* reason);

/**
 * @brief Expand variables in command line
 * @param input Input string with variables
 * @param output Output buffer
 * @param output_size Output buffer size
 * @return 0 on success, negative on error
 */
int universal_expand_variables(const char* input, char* output, size_t output_size);

/**
 * @brief Expand wildcards/globs in path
 * @param pattern Glob pattern
 * @param matches Output: array of matching paths
 * @param max_matches Maximum matches
 * @return Number of matches
 */
int universal_expand_glob(const char* pattern, char*** matches, uint32_t max_matches);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_UNIVERSAL_SHELL_H */
