/**
 * @file command_shell.h
 * @brief Universal Command Shell Interface for TernaryBit OS
 *
 * Provides interactive command execution interface for testing and user interaction.
 * Adapts to hardware capabilities and integrates with STEPPPS framework.
 *
 * Traceability:
 * - PRD: PR-024 (User Interface)
 * - FRD: FR-USR-001 (Command Interface)
 * - NFRD: NFR-USR-002 (Usability)
 * - HLD: Command Shell Component
 * - LLD: shell_context_t structure
 *
 * @version 1.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#ifndef COMMAND_SHELL_H
#define COMMAND_SHELL_H

#include "../core/hardware_detector.h"
#include "../core/memory_manager.h"
#include "../core/steppps_framework.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Shell configuration constants
#define MAX_COMMAND_LENGTH      256
#define MAX_ARGS                16
#define MAX_HISTORY_ENTRIES     50
#define MAX_ALIAS_LENGTH        32
#define MAX_ALIAS_COUNT         20
#define SHELL_PROMPT_MAX        64

// Shell status
typedef enum {
    SHELL_STATUS_READY = 0,
    SHELL_STATUS_BUSY = 1,
    SHELL_STATUS_ERROR = 2,
    SHELL_STATUS_EXIT = 3
} shell_status_t;

// Command execution result
typedef enum {
    CMD_RESULT_SUCCESS = 0,
    CMD_RESULT_ERROR = 1,
    CMD_RESULT_NOT_FOUND = 2,
    CMD_RESULT_INVALID_ARGS = 3,
    CMD_RESULT_PERMISSION_DENIED = 4,
    CMD_RESULT_EXIT = 5
} command_result_t;

// Command function pointer type
typedef command_result_t (*command_function_t)(int argc, char** argv);

// Command definition
typedef struct {
    const char* name;           // Command name
    const char* description;    // Brief description
    const char* usage;          // Usage syntax
    command_function_t function; // Function pointer
    uint32_t flags;             // Command flags (admin, debug, etc.)
    uint32_t min_args;          // Minimum arguments required
    uint32_t max_args;          // Maximum arguments allowed
} command_t;

// Command history entry
typedef struct {
    char command_line[MAX_COMMAND_LENGTH];
    uint64_t timestamp_us;
    command_result_t result;
} history_entry_t;

// Command alias
typedef struct {
    char alias[MAX_ALIAS_LENGTH];
    char command[MAX_COMMAND_LENGTH];
} command_alias_t;

// Shell context
typedef struct {
    // Hardware and system integration
    hardware_info_t* hardware;
    memory_manager_t* memory_manager;
    steppps_manager_t* steppps_manager;

    // Shell state
    shell_status_t status;
    char prompt[SHELL_PROMPT_MAX];
    bool echo_enabled;
    bool history_enabled;
    bool color_enabled;

    // Command processing
    char current_command[MAX_COMMAND_LENGTH];
    char* args[MAX_ARGS];
    int argc;
    uint32_t command_count;

    // History management
    history_entry_t history[MAX_HISTORY_ENTRIES];
    uint32_t history_count;
    uint32_t history_index;

    // Aliases
    command_alias_t aliases[MAX_ALIAS_COUNT];
    uint32_t alias_count;

    // Performance tracking
    uint64_t session_start_time_us;
    uint64_t total_commands_executed;
    uint64_t total_execution_time_us;

    // Configuration
    bool debug_mode;
    bool admin_mode;
    uint32_t max_command_length;
    uint32_t timeout_ms;

} shell_context_t;

// Function prototypes

/**
 * @brief Initialize command shell system
 * @param hardware Hardware information
 * @param memory_manager Memory manager instance
 * @param steppps_manager STEPPPS manager instance
 * @return shell_context_t* Initialized shell context
 */
shell_context_t* initialize_command_shell(
    const hardware_info_t* hardware,
    memory_manager_t* memory_manager,
    steppps_manager_t* steppps_manager
);

/**
 * @brief Start interactive shell session
 * @param shell Shell context
 * @return bool Success/failure
 */
bool start_shell_session(shell_context_t* shell);

/**
 * @brief Execute single command
 * @param shell Shell context
 * @param command_line Command string to execute
 * @return command_result_t Execution result
 */
command_result_t execute_command(shell_context_t* shell, const char* command_line);

/**
 * @brief Parse command line into arguments
 * @param shell Shell context
 * @param command_line Input command line
 * @return bool Parse success
 */
bool parse_command_line(shell_context_t* shell, const char* command_line);

/**
 * @brief Register new command
 * @param command Command definition
 * @return bool Registration success
 */
bool register_command(const command_t* command);

/**
 * @brief Find command by name
 * @param name Command name
 * @return command_t* Command definition or NULL
 */
const command_t* find_command(const char* name);

/**
 * @brief Add command to history
 * @param shell Shell context
 * @param command_line Command line
 * @param result Execution result
 */
void add_to_history(shell_context_t* shell, const char* command_line, command_result_t result);

/**
 * @brief Get command from history
 * @param shell Shell context
 * @param index History index (0 = most recent)
 * @return const char* Command line or NULL
 */
const char* get_from_history(shell_context_t* shell, uint32_t index);

/**
 * @brief Add command alias
 * @param shell Shell context
 * @param alias Alias name
 * @param command Target command
 * @return bool Success/failure
 */
bool add_alias(shell_context_t* shell, const char* alias, const char* command);

/**
 * @brief Resolve command alias
 * @param shell Shell context
 * @param alias Alias name
 * @return const char* Resolved command or NULL
 */
const char* resolve_alias(shell_context_t* shell, const char* alias);

/**
 * @brief Set shell prompt
 * @param shell Shell context
 * @param prompt New prompt string
 */
void set_shell_prompt(shell_context_t* shell, const char* prompt);

/**
 * @brief Print shell prompt
 * @param shell Shell context
 */
void print_shell_prompt(shell_context_t* shell);

/**
 * @brief Print command help
 * @param command Command definition
 */
void print_command_help(const command_t* command);

/**
 * @brief Print shell statistics
 * @param shell Shell context
 */
void print_shell_statistics(shell_context_t* shell);

/**
 * @brief Configure shell settings
 * @param shell Shell context
 * @param setting Setting name
 * @param value Setting value
 * @return bool Configuration success
 */
bool configure_shell(shell_context_t* shell, const char* setting, const char* value);

/**
 * @brief Get shell status
 * @param shell Shell context
 * @return shell_status_t Current status
 */
shell_status_t get_shell_status(const shell_context_t* shell);

/**
 * @brief Shutdown command shell
 * @param shell Shell context
 */
void shutdown_command_shell(shell_context_t* shell);

// Built-in command function prototypes
command_result_t cmd_help(int argc, char** argv);
command_result_t cmd_exit(int argc, char** argv);
command_result_t cmd_clear(int argc, char** argv);
command_result_t cmd_history(int argc, char** argv);
command_result_t cmd_alias(int argc, char** argv);
command_result_t cmd_pwd(int argc, char** argv);
command_result_t cmd_ls(int argc, char** argv);
command_result_t cmd_echo(int argc, char** argv);
command_result_t cmd_status(int argc, char** argv);
command_result_t cmd_memory(int argc, char** argv);
command_result_t cmd_steppps(int argc, char** argv);
command_result_t cmd_hardware(int argc, char** argv);
command_result_t cmd_test(int argc, char** argv);
command_result_t cmd_benchmark(int argc, char** argv);

// Global shell context
extern shell_context_t* g_shell_context;

#endif // COMMAND_SHELL_H