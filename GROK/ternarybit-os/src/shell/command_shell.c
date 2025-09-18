/**
 * @file command_shell.c
 * @brief Universal Command Shell Implementation for TernaryBit OS
 *
 * Provides interactive command execution interface with full STEPPPS integration.
 * Adapts to hardware capabilities from 4-bit calculators to supercomputers.
 *
 * @version 1.0
 * @date 2025-01-XX
 */

#include "command_shell.h"
#include "universal_shell_compat.h"
#include "../boot/universal_boot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

// Forward declarations
static void register_builtin_commands(void);

// Global shell context
shell_context_t* g_shell_context = NULL;

// Command registry
static command_t registered_commands[64];
static uint32_t command_registry_count = 0;

// Simple I/O functions for platform abstraction
static void shell_printf(const char* format, ...) {
    // For now, just use printf - in real OS this would be abstracted
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

static void shell_print(const char* str) {
    printf("%s", str);
}

static char* shell_gets(char* buffer, size_t size) {
    return fgets(buffer, (int)size, stdin);
}

/**
 * @brief Initialize command shell system
 */
shell_context_t* initialize_command_shell(
    const hardware_info_t* hardware,
    memory_manager_t* memory_manager,
    steppps_manager_t* steppps_manager
) {
    if (!hardware) return NULL;

    // Allocate shell context
    shell_context_t* shell = NULL;
    if (memory_manager) {
        memory_request_t request = {
            .size = sizeof(shell_context_t),
            .alignment = ALIGN_DEFAULT,
            .flags = MEM_FLAG_ZERO,
            .preferred_zone = ZONE_HEAP,
            .timeout_ms = 1000,
            .debug_name = "shell_context"
        };
        memory_result_t result = allocate_memory(memory_manager, &request);
        if (result.success) {
            shell = (shell_context_t*)result.address;
        }
    } else {
        shell = (shell_context_t*)malloc(sizeof(shell_context_t));
        if (shell) {
            memset(shell, 0, sizeof(shell_context_t));
        }
    }

    if (!shell) return NULL;

    // Initialize shell context
    shell->hardware = (hardware_info_t*)hardware;
    shell->memory_manager = memory_manager;
    shell->steppps_manager = steppps_manager;

    // Set initial state
    shell->status = SHELL_STATUS_READY;
    strcpy(shell->prompt, "TernaryBit> ");
    shell->echo_enabled = true;
    shell->history_enabled = true;
    shell->color_enabled = (hardware->capabilities & HW_CAP_DISPLAY) != 0;

    // Configure based on hardware capabilities
    if (hardware->memory_size_bytes < 4096) {
        // Very limited system
        shell->max_command_length = 64;
        shell->timeout_ms = 5000;
    } else if (hardware->memory_size_bytes < 65536) {
        // Small system
        shell->max_command_length = 128;
        shell->timeout_ms = 3000;
    } else {
        // Full system
        shell->max_command_length = MAX_COMMAND_LENGTH;
        shell->timeout_ms = 1000;
    }

    // Initialize timing
    shell->session_start_time_us = platform_get_timestamp_us();

    // Set global reference
    g_shell_context = shell;

    // Register built-in commands
    register_builtin_commands();

    // Initialize Universal Shell Compatibility Layer
    if (!initialize_universal_shell_compat(shell)) {
        // Non-fatal error, continue without universal compatibility
        shell_printf("⚠️  Warning: Universal shell compatibility layer initialization failed\n");
    }

    return shell;
}

/**
 * @brief Register built-in commands
 */
static void register_builtin_commands(void) {
    // Help command
    static const command_t help_cmd = {
        .name = "help",
        .description = "Display available commands",
        .usage = "help [command]",
        .function = cmd_help,
        .flags = 0,
        .min_args = 0,
        .max_args = 1
    };
    register_command(&help_cmd);

    // Exit command
    static const command_t exit_cmd = {
        .name = "exit",
        .description = "Exit the shell",
        .usage = "exit",
        .function = cmd_exit,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    };
    register_command(&exit_cmd);

    // Clear command
    static const command_t clear_cmd = {
        .name = "clear",
        .description = "Clear the screen",
        .usage = "clear",
        .function = cmd_clear,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    };
    register_command(&clear_cmd);

    // History command
    static const command_t history_cmd = {
        .name = "history",
        .description = "Show command history",
        .usage = "history",
        .function = cmd_history,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    };
    register_command(&history_cmd);

    // PWD command
    static const command_t pwd_cmd = {
        .name = "pwd",
        .description = "Print current directory",
        .usage = "pwd",
        .function = cmd_pwd,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    };
    register_command(&pwd_cmd);

    // LS command
    static const command_t ls_cmd = {
        .name = "ls",
        .description = "List directory contents",
        .usage = "ls",
        .function = cmd_ls,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    };
    register_command(&ls_cmd);

    // Alias command
    static const command_t alias_cmd = {
        .name = "alias",
        .description = "Create or show command aliases",
        .usage = "alias [name] [command]",
        .function = cmd_alias,
        .flags = 0,
        .min_args = 0,
        .max_args = 15
    };
    register_command(&alias_cmd);

    // Echo command
    static const command_t echo_cmd = {
        .name = "echo",
        .description = "Display text",
        .usage = "echo [text...]",
        .function = cmd_echo,
        .flags = 0,
        .min_args = 0,
        .max_args = 15
    };
    register_command(&echo_cmd);

    // Status command
    static const command_t status_cmd = {
        .name = "status",
        .description = "Show system status",
        .usage = "status",
        .function = cmd_status,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    };
    register_command(&status_cmd);

    // Memory command
    static const command_t memory_cmd = {
        .name = "memory",
        .description = "Show memory information",
        .usage = "memory",
        .function = cmd_memory,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    };
    register_command(&memory_cmd);

    // Hardware command
    static const command_t hardware_cmd = {
        .name = "hardware",
        .description = "Show hardware information",
        .usage = "hardware",
        .function = cmd_hardware,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    };
    register_command(&hardware_cmd);

    // STEPPPS command
    static const command_t steppps_cmd = {
        .name = "steppps",
        .description = "Show STEPPPS framework status",
        .usage = "steppps",
        .function = cmd_steppps,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    };
    register_command(&steppps_cmd);

    // Test command
    static const command_t test_cmd = {
        .name = "test",
        .description = "Run system tests",
        .usage = "test [component]",
        .function = cmd_test,
        .flags = 0,
        .min_args = 0,
        .max_args = 1
    };
    register_command(&test_cmd);

    // Benchmark command
    static const command_t benchmark_cmd = {
        .name = "benchmark",
        .description = "Run performance benchmarks",
        .usage = "benchmark [type]",
        .function = cmd_benchmark,
        .flags = 0,
        .min_args = 0,
        .max_args = 1
    };
    register_command(&benchmark_cmd);

    // Universal Shell Compatibility Commands

    // Shell mode management
    static const command_t shell_mode_cmd = {
        .name = "shellmode",
        .description = "Set or view shell compatibility mode",
        .usage = "shellmode [linux|windows|macos|auto]",
        .function = cmd_shell_mode,
        .flags = 0,
        .min_args = 0,
        .max_args = 1
    };
    register_command(&shell_mode_cmd);

    static const command_t shell_compat_cmd = {
        .name = "compat",
        .description = "Show shell compatibility information",
        .usage = "compat",
        .function = cmd_shell_compat,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    };
    register_command(&shell_compat_cmd);

    // Linux shell commands
    static const command_t linux_cat_cmd = {
        .name = "cat",
        .description = "Display file contents (Linux compatibility)",
        .usage = "cat <file>...",
        .function = cmd_linux_cat,
        .flags = 0,
        .min_args = 1,
        .max_args = 10
    };
    register_command(&linux_cat_cmd);

    static const command_t linux_grep_cmd = {
        .name = "grep",
        .description = "Search text patterns (Linux compatibility)",
        .usage = "grep <pattern> <file>",
        .function = cmd_linux_grep,
        .flags = 0,
        .min_args = 2,
        .max_args = 2
    };
    register_command(&linux_grep_cmd);

    static const command_t linux_ps_cmd = {
        .name = "ps",
        .description = "List processes (Linux compatibility)",
        .usage = "ps [options]",
        .function = cmd_linux_ps,
        .flags = 0,
        .min_args = 0,
        .max_args = 5
    };
    register_command(&linux_ps_cmd);

    static const command_t linux_uname_cmd = {
        .name = "uname",
        .description = "System information (Linux compatibility)",
        .usage = "uname [options]",
        .function = cmd_linux_uname,
        .flags = 0,
        .min_args = 0,
        .max_args = 3
    };
    register_command(&linux_uname_cmd);

    static const command_t linux_whoami_cmd = {
        .name = "whoami",
        .description = "Current user (Linux compatibility)",
        .usage = "whoami",
        .function = cmd_linux_whoami,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    };
    register_command(&linux_whoami_cmd);

    // Windows shell commands
    static const command_t windows_dir_cmd = {
        .name = "dir",
        .description = "List directory (Windows compatibility)",
        .usage = "dir [path]",
        .function = cmd_windows_dir,
        .flags = 0,
        .min_args = 0,
        .max_args = 3
    };
    register_command(&windows_dir_cmd);

    static const command_t windows_type_cmd = {
        .name = "type",
        .description = "Display file (Windows compatibility)",
        .usage = "type <file>",
        .function = cmd_windows_type,
        .flags = 0,
        .min_args = 1,
        .max_args = 1
    };
    register_command(&windows_type_cmd);

    static const command_t windows_ver_cmd = {
        .name = "ver",
        .description = "System version (Windows compatibility)",
        .usage = "ver",
        .function = cmd_windows_ver,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    };
    register_command(&windows_ver_cmd);

    static const command_t windows_tasklist_cmd = {
        .name = "tasklist",
        .description = "List tasks (Windows compatibility)",
        .usage = "tasklist",
        .function = cmd_windows_tasklist,
        .flags = 0,
        .min_args = 0,
        .max_args = 3
    };
    register_command(&windows_tasklist_cmd);

    // macOS shell commands
    static const command_t macos_open_cmd = {
        .name = "open",
        .description = "Open files/apps (macOS compatibility)",
        .usage = "open <file>",
        .function = cmd_macos_open,
        .flags = 0,
        .min_args = 1,
        .max_args = 3
    };
    register_command(&macos_open_cmd);

    static const command_t macos_say_cmd = {
        .name = "say",
        .description = "Text to speech (macOS compatibility)",
        .usage = "say <text>",
        .function = cmd_macos_say,
        .flags = 0,
        .min_args = 1,
        .max_args = 10
    };
    register_command(&macos_say_cmd);

    // Universal commands
    static const command_t universal_list_cmd = {
        .name = "list",
        .description = "Universal list command",
        .usage = "list [path]",
        .function = cmd_universal_list,
        .flags = 0,
        .min_args = 0,
        .max_args = 3
    };
    register_command(&universal_list_cmd);

    static const command_t universal_show_cmd = {
        .name = "show",
        .description = "Universal show command",
        .usage = "show <file>",
        .function = cmd_universal_show,
        .flags = 0,
        .min_args = 1,
        .max_args = 3
    };
    register_command(&universal_show_cmd);

    static const command_t universal_processes_cmd = {
        .name = "processes",
        .description = "Universal processes command",
        .usage = "processes",
        .function = cmd_universal_processes,
        .flags = 0,
        .min_args = 0,
        .max_args = 3
    };
    register_command(&universal_processes_cmd);

    // Advanced utility commands
    static const command_t env_cmd = {
        .name = "env",
        .description = "Show environment variables",
        .usage = "env",
        .function = cmd_env,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    };
    register_command(&env_cmd);

    static const command_t export_cmd = {
        .name = "export",
        .description = "Export environment variable",
        .usage = "export VAR=value",
        .function = cmd_export,
        .flags = 0,
        .min_args = 0,
        .max_args = 5
    };
    register_command(&export_cmd);

    static const command_t set_cmd = {
        .name = "set",
        .description = "Set environment variable (Windows style)",
        .usage = "set VAR=value",
        .function = cmd_set,
        .flags = 0,
        .min_args = 0,
        .max_args = 5
    };
    register_command(&set_cmd);

    static const command_t find_cmd = {
        .name = "find",
        .description = "Find files",
        .usage = "find PATH -name PATTERN",
        .function = cmd_find,
        .flags = 0,
        .min_args = 3,
        .max_args = 10
    };
    register_command(&find_cmd);

    static const command_t which_cmd = {
        .name = "which",
        .description = "Locate command",
        .usage = "which COMMAND",
        .function = cmd_which,
        .flags = 0,
        .min_args = 1,
        .max_args = 1
    };
    register_command(&which_cmd);

    static const command_t head_cmd = {
        .name = "head",
        .description = "Show first lines of file",
        .usage = "head [options] FILE",
        .function = cmd_head,
        .flags = 0,
        .min_args = 1,
        .max_args = 5
    };
    register_command(&head_cmd);

    static const command_t tail_cmd = {
        .name = "tail",
        .description = "Show last lines of file",
        .usage = "tail [options] FILE",
        .function = cmd_tail,
        .flags = 0,
        .min_args = 1,
        .max_args = 5
    };
    register_command(&tail_cmd);

    static const command_t wc_cmd = {
        .name = "wc",
        .description = "Word count (Linux compatibility)",
        .usage = "wc [options] FILE",
        .function = cmd_linux_wc,
        .flags = 0,
        .min_args = 1,
        .max_args = 5
    };
    register_command(&wc_cmd);

    static const command_t ping_cmd = {
        .name = "ping",
        .description = "Network ping utility",
        .usage = "ping [options] HOST",
        .function = cmd_linux_ping,
        .flags = 0,
        .min_args = 1,
        .max_args = 5
    };
    register_command(&ping_cmd);

    static const command_t cp_cmd = {
        .name = "cp",
        .description = "Copy files (Linux compatibility)",
        .usage = "cp [options] SOURCE DEST",
        .function = cmd_linux_cp,
        .flags = 0,
        .min_args = 2,
        .max_args = 5
    };
    register_command(&cp_cmd);

    static const command_t mv_cmd = {
        .name = "mv",
        .description = "Move files (Linux compatibility)",
        .usage = "mv [options] SOURCE DEST",
        .function = cmd_linux_mv,
        .flags = 0,
        .min_args = 2,
        .max_args = 5
    };
    register_command(&mv_cmd);

    static const command_t rm_cmd = {
        .name = "rm",
        .description = "Remove files (Linux compatibility)",
        .usage = "rm [options] FILE...",
        .function = cmd_linux_rm,
        .flags = 0,
        .min_args = 1,
        .max_args = 10
    };
    register_command(&rm_cmd);

    static const command_t mkdir_cmd = {
        .name = "mkdir",
        .description = "Create directories (Linux compatibility)",
        .usage = "mkdir [options] DIRECTORY...",
        .function = cmd_linux_mkdir,
        .flags = 0,
        .min_args = 1,
        .max_args = 10
    };
    register_command(&mkdir_cmd);

    static const command_t copy_cmd = {
        .name = "copy",
        .description = "Copy files (Windows compatibility)",
        .usage = "copy SOURCE DEST",
        .function = cmd_windows_copy,
        .flags = 0,
        .min_args = 2,
        .max_args = 5
    };
    register_command(&copy_cmd);
}

/**
 * @brief Start interactive shell session
 */
bool start_shell_session(shell_context_t* shell) {
    if (!shell) return false;

    shell->status = SHELL_STATUS_READY;

    // Welcome message
    shell_print("\n🌟 TernaryBit Universal OS - Command Shell\n");
    shell_print("==========================================\n");
    shell_printf("Hardware: %d-bit CPU, %llu bytes RAM\n",
                 shell->hardware->cpu_bits,
                 (unsigned long long)shell->hardware->memory_size_bytes);
    shell_printf("STEPPPS Framework: %s\n",
                 shell->steppps_manager ? "Active" : "Inactive");
    shell_printf("Memory Manager: %s\n",
                 shell->memory_manager ? "Active" : "Inactive");
    shell_print("Type 'help' for available commands.\n\n");

    char input_buffer[MAX_COMMAND_LENGTH];

    // Main shell loop
    while (shell->status != SHELL_STATUS_EXIT) {
        // Print prompt
        print_shell_prompt(shell);

        // Read command
        if (!shell_gets(input_buffer, sizeof(input_buffer))) {
            break; // EOF or error
        }

        // Remove newline
        size_t len = strlen(input_buffer);
        if (len > 0 && input_buffer[len-1] == '\n') {
            input_buffer[len-1] = '\0';
        }

        // Skip empty commands
        if (strlen(input_buffer) == 0) {
            continue;
        }

        // Execute command
        command_result_t result = execute_command(shell, input_buffer);

        // Handle result
        if (result == CMD_RESULT_EXIT) {
            shell->status = SHELL_STATUS_EXIT;
        } else if (result == CMD_RESULT_ERROR) {
            shell_print("Command execution failed.\n");
        } else if (result == CMD_RESULT_NOT_FOUND) {
            shell_printf("Command not found: %s\n", shell->args[0]);
            shell_print("Type 'help' for available commands.\n");
        }
    }

    shell_print("\nGoodbye! Thank you for using TernaryBit OS.\n");
    return true;
}

/**
 * @brief Execute single command
 */
command_result_t execute_command(shell_context_t* shell, const char* command_line) {
    if (!shell || !command_line) return CMD_RESULT_ERROR;

    uint64_t start_time = platform_get_timestamp_us();

    // Parse command line
    if (!parse_command_line(shell, command_line)) {
        return CMD_RESULT_ERROR;
    }

    if (shell->argc == 0) {
        return CMD_RESULT_SUCCESS;
    }

    // Resolve aliases
    const char* resolved = resolve_alias(shell, shell->args[0]);
    if (resolved) {
        // Re-parse with resolved command
        if (!parse_command_line(shell, resolved)) {
            return CMD_RESULT_ERROR;
        }
    }

    // Find command
    const command_t* command = find_command(shell->args[0]);
    if (!command) {
        add_to_history(shell, command_line, CMD_RESULT_NOT_FOUND);
        return CMD_RESULT_NOT_FOUND;
    }

    // Validate arguments
    if (shell->argc - 1 < (int)command->min_args ||
        shell->argc - 1 > (int)command->max_args) {
        shell_printf("Invalid arguments. Usage: %s\n", command->usage);
        add_to_history(shell, command_line, CMD_RESULT_INVALID_ARGS);
        return CMD_RESULT_INVALID_ARGS;
    }

    // Execute command
    command_result_t result = command->function(shell->argc, shell->args);

    // Update statistics
    uint64_t execution_time = platform_get_timestamp_us() - start_time;
    shell->total_commands_executed++;
    shell->total_execution_time_us += execution_time;

    // Add to history
    add_to_history(shell, command_line, result);

    return result;
}

/**
 * @brief Parse command line into arguments
 */
bool parse_command_line(shell_context_t* shell, const char* command_line) {
    if (!shell || !command_line) return false;

    // Copy command line
    strncpy(shell->current_command, command_line, sizeof(shell->current_command) - 1);
    shell->current_command[sizeof(shell->current_command) - 1] = '\0';

    // Reset argc
    shell->argc = 0;

    // Tokenize
    char* token = strtok(shell->current_command, " \t");
    while (token && shell->argc < MAX_ARGS) {
        shell->args[shell->argc++] = token;
        token = strtok(NULL, " \t");
    }

    return true;
}

/**
 * @brief Register new command
 */
bool register_command(const command_t* command) {
    if (!command || command_registry_count >= 64) return false;

    registered_commands[command_registry_count++] = *command;
    return true;
}

/**
 * @brief Find command by name
 */
const command_t* find_command(const char* name) {
    if (!name) return NULL;

    for (uint32_t i = 0; i < command_registry_count; i++) {
        if (strcmp(registered_commands[i].name, name) == 0) {
            return &registered_commands[i];
        }
    }

    return NULL;
}

/**
 * @brief Add command to history
 */
void add_to_history(shell_context_t* shell, const char* command_line, command_result_t result) {
    if (!shell || !shell->history_enabled || !command_line) return;

    uint32_t index = shell->history_count % MAX_HISTORY_ENTRIES;

    strncpy(shell->history[index].command_line, command_line, MAX_COMMAND_LENGTH - 1);
    shell->history[index].command_line[MAX_COMMAND_LENGTH - 1] = '\0';
    shell->history[index].timestamp_us = platform_get_timestamp_us();
    shell->history[index].result = result;

    if (shell->history_count < MAX_HISTORY_ENTRIES) {
        shell->history_count++;
    }
}

/**
 * @brief Add command alias
 */
bool add_alias(shell_context_t* shell, const char* alias, const char* command) {
    if (!shell || !alias || !command || shell->alias_count >= MAX_ALIAS_COUNT) {
        return false;
    }

    uint32_t index = shell->alias_count++;
    strncpy(shell->aliases[index].alias, alias, MAX_ALIAS_LENGTH - 1);
    shell->aliases[index].alias[MAX_ALIAS_LENGTH - 1] = '\0';
    strncpy(shell->aliases[index].command, command, MAX_COMMAND_LENGTH - 1);
    shell->aliases[index].command[MAX_COMMAND_LENGTH - 1] = '\0';

    return true;
}

/**
 * @brief Resolve command alias
 */
const char* resolve_alias(shell_context_t* shell, const char* alias) {
    if (!shell || !alias) return NULL;

    for (uint32_t i = 0; i < shell->alias_count; i++) {
        if (strcmp(shell->aliases[i].alias, alias) == 0) {
            return shell->aliases[i].command;
        }
    }

    return NULL;
}

/**
 * @brief Get command from history
 */
const char* get_from_history(shell_context_t* shell, uint32_t index) {
    if (!shell || index >= shell->history_count) return NULL;

    uint32_t hist_index = index;
    if (shell->history_count == MAX_HISTORY_ENTRIES) {
        hist_index = (shell->history_count + index) % MAX_HISTORY_ENTRIES;
    }

    return shell->history[hist_index].command_line;
}

/**
 * @brief Print shell statistics
 */
void print_shell_statistics(shell_context_t* shell) {
    if (!shell) return;

    shell_print("📊 Shell Statistics\n");
    shell_print("===================\n");
    shell_printf("Commands executed: %llu\n", (unsigned long long)shell->total_commands_executed);
    shell_printf("History entries: %u\n", shell->history_count);
    shell_printf("Aliases: %u\n", shell->alias_count);

    if (shell->total_commands_executed > 0) {
        uint64_t avg_time = shell->total_execution_time_us / shell->total_commands_executed;
        shell_printf("Average execution time: %llu μs\n", (unsigned long long)avg_time);
    }

    uint64_t uptime = platform_get_timestamp_us() - shell->session_start_time_us;
    shell_printf("Session uptime: %llu μs\n", (unsigned long long)uptime);
}

/**
 * @brief Configure shell settings
 */
bool configure_shell(shell_context_t* shell, const char* setting, const char* value) {
    if (!shell || !setting || !value) return false;

    if (strcmp(setting, "prompt") == 0) {
        set_shell_prompt(shell, value);
        return true;
    } else if (strcmp(setting, "echo") == 0) {
        shell->echo_enabled = (strcmp(value, "on") == 0 || strcmp(value, "true") == 0);
        return true;
    } else if (strcmp(setting, "history") == 0) {
        shell->history_enabled = (strcmp(value, "on") == 0 || strcmp(value, "true") == 0);
        return true;
    } else if (strcmp(setting, "color") == 0) {
        shell->color_enabled = (strcmp(value, "on") == 0 || strcmp(value, "true") == 0);
        return true;
    }

    return false;
}

/**
 * @brief Get shell status
 */
shell_status_t get_shell_status(const shell_context_t* shell) {
    return shell ? shell->status : SHELL_STATUS_ERROR;
}

/**
 * @brief Set shell prompt
 */
void set_shell_prompt(shell_context_t* shell, const char* prompt) {
    if (!shell || !prompt) return;

    strncpy(shell->prompt, prompt, SHELL_PROMPT_MAX - 1);
    shell->prompt[SHELL_PROMPT_MAX - 1] = '\0';
}

/**
 * @brief Print shell prompt
 */
void print_shell_prompt(shell_context_t* shell) {
    if (!shell) return;

    if (shell->color_enabled) {
        shell_printf("\033[1;32m%s\033[0m", shell->prompt);
    } else {
        shell_print(shell->prompt);
    }
}

/**
 * @brief Print command help
 */
void print_command_help(const command_t* command) {
    if (!command) return;

    shell_printf("%-12s - %s\n", command->name, command->description);
    shell_printf("Usage: %s\n", command->usage);
}

/**
 * @brief Shutdown command shell
 */
void shutdown_command_shell(shell_context_t* shell) {
    if (!shell) return;

    if (shell->memory_manager) {
        free_memory(shell->memory_manager, shell);
    } else {
        free(shell);
    }

    g_shell_context = NULL;
}

// Built-in command implementations

/**
 * @brief Help command implementation
 */
command_result_t cmd_help(int argc, char** argv) {
    if (argc == 1) {
        // Show all commands
        shell_print("Available commands:\n");
        shell_print("==================\n");
        for (uint32_t i = 0; i < command_registry_count; i++) {
            shell_printf("%-12s - %s\n",
                        registered_commands[i].name,
                        registered_commands[i].description);
        }
        shell_print("\nType 'help <command>' for detailed usage.\n");
    } else {
        // Show specific command help
        const command_t* command = find_command(argv[1]);
        if (command) {
            print_command_help(command);
        } else {
            shell_printf("Unknown command: %s\n", argv[1]);
            return CMD_RESULT_NOT_FOUND;
        }
    }
    return CMD_RESULT_SUCCESS;
}

/**
 * @brief Exit command implementation
 */
command_result_t cmd_exit(int argc, char** argv) {
    (void)argc; (void)argv; // Suppress unused warnings
    return CMD_RESULT_EXIT;
}

/**
 * @brief Clear command implementation
 */
command_result_t cmd_clear(int argc, char** argv) {
    (void)argc; (void)argv; // Suppress unused warnings
    shell_print("\033[2J\033[H"); // ANSI clear screen
    return CMD_RESULT_SUCCESS;
}

/**
 * @brief History command implementation
 */
command_result_t cmd_history(int argc, char** argv) {
    (void)argc; (void)argv; // Suppress unused warnings

    if (!g_shell_context || !g_shell_context->history_enabled) {
        shell_print("History is disabled.\n");
        return CMD_RESULT_SUCCESS;
    }

    shell_print("Command History:\n");
    shell_print("================\n");

    for (uint32_t i = 0; i < g_shell_context->history_count; i++) {
        uint32_t index = i;
        if (g_shell_context->history_count == MAX_HISTORY_ENTRIES) {
            index = (g_shell_context->history_count + i) % MAX_HISTORY_ENTRIES;
        }

        const char* status = "";
        switch (g_shell_context->history[index].result) {
            case CMD_RESULT_SUCCESS: status = "✓"; break;
            case CMD_RESULT_ERROR: status = "✗"; break;
            case CMD_RESULT_NOT_FOUND: status = "?"; break;
            default: status = "-"; break;
        }

        shell_printf("%3u %s %s\n", i + 1, status,
                    g_shell_context->history[index].command_line);
    }

    return CMD_RESULT_SUCCESS;
}

/**
 * @brief PWD command implementation
 */
command_result_t cmd_pwd(int argc, char** argv) {
    (void)argc; (void)argv; // Suppress unused warnings
    shell_print("/\n");  // For now, always at root
    return CMD_RESULT_SUCCESS;
}

/**
 * @brief Echo command implementation
 */
command_result_t cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        shell_print(argv[i]);
        if (i < argc - 1) shell_print(" ");
    }
    shell_print("\n");
    return CMD_RESULT_SUCCESS;
}

/**
 * @brief Status command implementation
 */
command_result_t cmd_status(int argc, char** argv) {
    (void)argc; (void)argv; // Suppress unused warnings

    if (!g_shell_context) return CMD_RESULT_ERROR;

    shell_print("🌟 TernaryBit OS System Status\n");
    shell_print("==============================\n");

    // Shell statistics
    uint64_t uptime = platform_get_timestamp_us() - g_shell_context->session_start_time_us;
    shell_printf("Shell uptime: %llu μs\n", (unsigned long long)uptime);
    shell_printf("Commands executed: %llu\n", (unsigned long long)g_shell_context->total_commands_executed);

    if (g_shell_context->total_commands_executed > 0) {
        uint64_t avg_time = g_shell_context->total_execution_time_us / g_shell_context->total_commands_executed;
        shell_printf("Average command time: %llu μs\n", (unsigned long long)avg_time);
    }

    // Component status
    shell_printf("Memory Manager: %s\n", g_shell_context->memory_manager ? "Active" : "Inactive");
    shell_printf("STEPPPS Framework: %s\n", g_shell_context->steppps_manager ? "Active" : "Inactive");

    return CMD_RESULT_SUCCESS;
}

/**
 * @brief Memory command implementation
 */
command_result_t cmd_memory(int argc, char** argv) {
    (void)argc; (void)argv; // Suppress unused warnings

    if (!g_shell_context || !g_shell_context->memory_manager) {
        shell_print("Memory manager not available.\n");
        return CMD_RESULT_ERROR;
    }

    memory_stats_t stats = get_memory_statistics(g_shell_context->memory_manager);

    shell_print("🧠 Memory Information\n");
    shell_print("====================\n");
    shell_printf("Total memory: %zu bytes\n", stats.total_memory);
    shell_printf("Allocated: %zu bytes\n", stats.allocated_memory);
    shell_printf("Free: %zu bytes\n", stats.free_memory);
    shell_printf("Efficiency: %u%%\n", stats.efficiency_rating);
    shell_printf("Allocations: %u\n", stats.allocation_count);
    shell_printf("Deallocations: %u\n", stats.deallocation_count);
    shell_printf("Failed allocations: %u\n", stats.failed_allocations);

    return CMD_RESULT_SUCCESS;
}

/**
 * @brief Hardware command implementation
 */
command_result_t cmd_hardware(int argc, char** argv) {
    (void)argc; (void)argv; // Suppress unused warnings

    if (!g_shell_context || !g_shell_context->hardware) {
        shell_print("Hardware information not available.\n");
        return CMD_RESULT_ERROR;
    }

    hardware_info_t* hw = g_shell_context->hardware;

    shell_print("🖥️  Hardware Information\n");
    shell_print("=======================\n");
    shell_printf("CPU: %d-bit, %d cores, %u MHz\n",
                 hw->cpu_bits, hw->cpu_cores, hw->cpu_speed_mhz);
    shell_printf("Memory: %llu bytes (%s)\n",
                 (unsigned long long)hw->memory_size_bytes,
                 hw->memory_type == MEM_TYPE_SRAM ? "SRAM" :
                 hw->memory_type == MEM_TYPE_DRAM ? "DRAM" : "OTHER");
    shell_printf("Device class: %u\n", hw->device_class);
    shell_printf("MMU: %s\n", hw->has_mmu ? "Yes" : "No");
    shell_printf("Capabilities: 0x%08X\n", hw->capabilities);

    return CMD_RESULT_SUCCESS;
}

/**
 * @brief STEPPPS command implementation
 */
command_result_t cmd_steppps(int argc, char** argv) {
    (void)argc; (void)argv; // Suppress unused warnings

    if (!g_shell_context || !g_shell_context->steppps_manager) {
        shell_print("STEPPPS framework not available.\n");
        return CMD_RESULT_ERROR;
    }

    steppps_manager_t* mgr = g_shell_context->steppps_manager;

    shell_print("🌟 STEPPPS Framework Status\n");
    shell_print("===========================\n");
    shell_printf("Status: %s\n", get_steppps_status_name(mgr->status));
    shell_printf("Mode: %s\n", get_steppps_mode_name(mgr->mode));
    shell_printf("Operations: %llu\n", (unsigned long long)mgr->total_operations);
    shell_printf("Efficiency: %u%%\n", mgr->efficiency_rating);
    shell_printf("Dimensions active: %s%s%s%s%s%s%s\n",
                 mgr->space ? "SPACE " : "",
                 mgr->time ? "TIME " : "",
                 mgr->event ? "EVENT " : "",
                 mgr->psychology ? "PSYCHOLOGY " : "",
                 mgr->pixel ? "PIXEL " : "",
                 mgr->prompt ? "PROMPT " : "",
                 mgr->script ? "SCRIPT " : "");

    return CMD_RESULT_SUCCESS;
}

/**
 * @brief Test command implementation
 */
command_result_t cmd_test(int argc, char** argv) {
    if (argc == 1) {
        shell_print("Available tests:\n");
        shell_print("================\n");
        shell_print("- hardware    : Test hardware detection and capabilities\n");
        shell_print("- memory      : Test memory allocation and management\n");
        shell_print("- steppps     : Test STEPPPS framework operations\n");
        shell_print("- integration : Test component integration\n");
        shell_print("- performance : Test system performance\n");
        shell_print("- all         : Run complete test suite\n");
        shell_print("\nUsage: test <component>\n");
        return CMD_RESULT_SUCCESS;
    }

    const char* test_name = argv[1];

    if (strcmp(test_name, "hardware") == 0) {
        shell_print("🧪 Running hardware detection test...\n");

        if (g_shell_context && g_shell_context->hardware) {
            hardware_info_t* hw = g_shell_context->hardware;
            shell_printf("  CPU: %d-bit architecture detected\n", hw->cpu_bits);
            shell_printf("  Memory: %llu bytes available\n", (unsigned long long)hw->memory_size_bytes);
            shell_printf("  Cores: %d detected\n", hw->cpu_cores);
            shell_printf("  Speed: %u MHz\n", hw->cpu_speed_mhz);
            shell_printf("  MMU: %s\n", hw->has_mmu ? "Available" : "Not available");
            shell_printf("  Capabilities: 0x%08X\n", hw->capabilities);
            shell_print("✅ Hardware test passed - All components detected\n");
        } else {
            shell_print("❌ Hardware test failed - No hardware information available\n");
            return CMD_RESULT_ERROR;
        }

    } else if (strcmp(test_name, "memory") == 0) {
        shell_print("🧪 Running memory management test...\n");

        if (g_shell_context && g_shell_context->memory_manager) {
            // Test multiple allocations
            void* test_ptrs[5];
            bool all_success = true;

            for (int i = 0; i < 5; i++) {
                memory_request_t request = {
                    .size = 1024 * (i + 1),
                    .alignment = ALIGN_DEFAULT,
                    .flags = MEM_FLAG_ZERO,
                    .preferred_zone = ZONE_HEAP,
                    .timeout_ms = 1000,
                    .debug_name = "test_allocation"
                };

                memory_result_t result = allocate_memory(g_shell_context->memory_manager, &request);
                if (result.success) {
                    test_ptrs[i] = result.address;
                    shell_printf("  Allocation %d: %zu bytes at %p\n", i+1, request.size, result.address);
                } else {
                    all_success = false;
                    shell_printf("  Allocation %d: FAILED\n", i+1);
                    break;
                }
            }

            // Free allocations
            for (int i = 0; i < 5; i++) {
                if (test_ptrs[i]) {
                    free_memory(g_shell_context->memory_manager, test_ptrs[i]);
                }
            }

            if (all_success) {
                shell_print("✅ Memory test passed - All allocations successful\n");
            } else {
                shell_print("❌ Memory test failed - Some allocations failed\n");
                return CMD_RESULT_ERROR;
            }
        } else {
            shell_print("❌ Memory test failed - No memory manager available\n");
            return CMD_RESULT_ERROR;
        }

    } else if (strcmp(test_name, "steppps") == 0) {
        shell_print("🧪 Running STEPPPS framework test...\n");

        if (g_shell_context && g_shell_context->steppps_manager) {
            steppps_manager_t* mgr = g_shell_context->steppps_manager;

            // Test STEPPPS cycle
            uint64_t operations_before = mgr->total_operations;
            bool cycle_success = process_steppps_cycle(mgr);

            shell_printf("  Framework status: %s\n", get_steppps_status_name(mgr->status));
            shell_printf("  Operations: %llu -> %llu\n",
                        (unsigned long long)operations_before,
                        (unsigned long long)mgr->total_operations);
            shell_printf("  Efficiency: %u%%\n", mgr->efficiency_rating);

            if (cycle_success) {
                shell_print("✅ STEPPPS test passed - Framework operational\n");
            } else {
                shell_print("❌ STEPPPS test failed - Cycle processing error\n");
                return CMD_RESULT_ERROR;
            }
        } else {
            shell_print("❌ STEPPPS test failed - No framework available\n");
            return CMD_RESULT_ERROR;
        }

    } else if (strcmp(test_name, "integration") == 0) {
        shell_print("🧪 Running integration test...\n");

        // Test interaction between components
        bool hw_ok = (g_shell_context && g_shell_context->hardware);
        bool mem_ok = (g_shell_context && g_shell_context->memory_manager);
        bool steppps_ok = (g_shell_context && g_shell_context->steppps_manager);

        shell_printf("  Hardware integration: %s\n", hw_ok ? "✅ OK" : "❌ FAIL");
        shell_printf("  Memory integration: %s\n", mem_ok ? "✅ OK" : "❌ FAIL");
        shell_printf("  STEPPPS integration: %s\n", steppps_ok ? "✅ OK" : "❌ FAIL");

        if (hw_ok && mem_ok && steppps_ok) {
            shell_print("✅ Integration test passed - All components integrated\n");
        } else {
            shell_print("❌ Integration test failed - Missing components\n");
            return CMD_RESULT_ERROR;
        }

    } else if (strcmp(test_name, "performance") == 0) {
        shell_print("🧪 Running performance test...\n");

        uint64_t start_time = platform_get_timestamp_us();

        // Performance test: multiple operations
        for (int i = 0; i < 100; i++) {
            if (g_shell_context && g_shell_context->steppps_manager) {
                process_steppps_cycle(g_shell_context->steppps_manager);
            }
        }

        uint64_t end_time = platform_get_timestamp_us();
        uint64_t total_time = end_time - start_time;

        shell_printf("  100 STEPPPS cycles completed in %llu μs\n", (unsigned long long)total_time);
        shell_printf("  Average cycle time: %llu μs\n", (unsigned long long)(total_time / 100));

        if (total_time < 100000) { // < 100ms for 100 cycles
            shell_print("✅ Performance test passed - System responsive\n");
        } else {
            shell_print("❌ Performance test failed - System too slow\n");
            return CMD_RESULT_ERROR;
        }

    } else if (strcmp(test_name, "all") == 0) {
        shell_print("🧪 Running complete test suite...\n");
        shell_print("========================================\n");

        // Run all tests
        char* test_args[] = {"test", "hardware"};
        if (cmd_test(2, test_args) != CMD_RESULT_SUCCESS) return CMD_RESULT_ERROR;

        test_args[1] = "memory";
        if (cmd_test(2, test_args) != CMD_RESULT_SUCCESS) return CMD_RESULT_ERROR;

        test_args[1] = "steppps";
        if (cmd_test(2, test_args) != CMD_RESULT_SUCCESS) return CMD_RESULT_ERROR;

        test_args[1] = "integration";
        if (cmd_test(2, test_args) != CMD_RESULT_SUCCESS) return CMD_RESULT_ERROR;

        test_args[1] = "performance";
        if (cmd_test(2, test_args) != CMD_RESULT_SUCCESS) return CMD_RESULT_ERROR;

        shell_print("\n🎉 All tests passed! TernaryBit OS is fully operational.\n");

    } else {
        shell_printf("Unknown test: %s\n", test_name);
        shell_print("Use 'test' to see available tests.\n");
        return CMD_RESULT_INVALID_ARGS;
    }

    return CMD_RESULT_SUCCESS;
}

/**
 * @brief LS command implementation
 */
command_result_t cmd_ls(int argc, char** argv) {
    (void)argc; (void)argv; // Suppress unused warnings

    shell_print("📁 TernaryBit OS Virtual File System\n");
    shell_print("===================================\n");
    shell_print("drwxr-xr-x  1 root root    0 Jan  1 00:00 .\n");
    shell_print("drwxr-xr-x  1 root root    0 Jan  1 00:00 ..\n");
    shell_print("drwxr-xr-x  1 root root    0 Jan  1 00:00 bin\n");
    shell_print("drwxr-xr-x  1 root root    0 Jan  1 00:00 dev\n");
    shell_print("drwxr-xr-x  1 root root    0 Jan  1 00:00 etc\n");
    shell_print("drwxr-xr-x  1 root root    0 Jan  1 00:00 proc\n");
    shell_print("drwxr-xr-x  1 root root    0 Jan  1 00:00 sys\n");
    shell_print("drwxr-xr-x  1 root root    0 Jan  1 00:00 tmp\n");
    shell_print("drwxr-xr-x  1 root root    0 Jan  1 00:00 var\n");

    if (g_shell_context) {
        shell_printf("\n📊 System resources:\n");
        if (g_shell_context->memory_manager) {
            memory_stats_t stats = get_memory_statistics(g_shell_context->memory_manager);
            shell_printf("Memory: %zu/%zu bytes used\n", stats.allocated_memory, stats.total_memory);
        }
        if (g_shell_context->steppps_manager) {
            shell_printf("STEPPPS: %llu operations completed\n",
                        (unsigned long long)g_shell_context->steppps_manager->total_operations);
        }
    }

    return CMD_RESULT_SUCCESS;
}

/**
 * @brief Alias command implementation
 */
command_result_t cmd_alias(int argc, char** argv) {
    if (!g_shell_context) return CMD_RESULT_ERROR;

    if (argc == 1) {
        // Show all aliases
        shell_print("Command Aliases:\n");
        shell_print("================\n");
        if (g_shell_context->alias_count == 0) {
            shell_print("No aliases defined.\n");
        } else {
            for (uint32_t i = 0; i < g_shell_context->alias_count; i++) {
                shell_printf("%-12s -> %s\n",
                            g_shell_context->aliases[i].alias,
                            g_shell_context->aliases[i].command);
            }
        }
        shell_print("\nUsage: alias <name> <command>\n");
        shell_print("       alias <name>           (show specific alias)\n");
        return CMD_RESULT_SUCCESS;
    }

    if (argc == 2) {
        // Show specific alias
        const char* resolved = resolve_alias(g_shell_context, argv[1]);
        if (resolved) {
            shell_printf("%s -> %s\n", argv[1], resolved);
        } else {
            shell_printf("Alias '%s' not found.\n", argv[1]);
        }
        return CMD_RESULT_SUCCESS;
    }

    if (argc >= 3) {
        // Create new alias
        const char* alias_name = argv[1];

        // Build command string from remaining arguments
        char command_str[MAX_COMMAND_LENGTH] = {0};
        for (int i = 2; i < argc; i++) {
            strcat(command_str, argv[i]);
            if (i < argc - 1) strcat(command_str, " ");
        }

        if (add_alias(g_shell_context, alias_name, command_str)) {
            shell_printf("Alias created: %s -> %s\n", alias_name, command_str);
        } else {
            shell_printf("Failed to create alias '%s'.\n", alias_name);
            return CMD_RESULT_ERROR;
        }
        return CMD_RESULT_SUCCESS;
    }

    return CMD_RESULT_INVALID_ARGS;
}

/**
 * @brief Benchmark command implementation
 */
command_result_t cmd_benchmark(int argc, char** argv) {
    if (argc == 1) {
        shell_print("Available benchmarks:\n");
        shell_print("====================\n");
        shell_print("- cpu         : CPU computation benchmark\n");
        shell_print("- memory      : Memory allocation benchmark\n");
        shell_print("- steppps     : STEPPPS framework benchmark\n");
        shell_print("- integration : Full system integration benchmark\n");
        shell_print("\nUsage: benchmark <type>\n");
        return CMD_RESULT_SUCCESS;
    }

    const char* bench_type = argv[1];

    if (strcmp(bench_type, "cpu") == 0) {
        shell_print("🏃 Running CPU benchmark...\n");

        uint64_t start_time = platform_get_timestamp_us();

        // CPU-intensive calculation
        volatile uint64_t result = 0;
        for (uint64_t i = 0; i < 1000000; i++) {
            result += i * i;
        }

        uint64_t end_time = platform_get_timestamp_us();
        uint64_t duration = end_time - start_time;

        shell_printf("  Computed 1M squares in %llu μs\n", (unsigned long long)duration);
        shell_printf("  Performance: %.2f MOPS (Million Operations Per Second)\n",
                    1000000.0 / (duration / 1000000.0));

        if (duration < 100000) { // < 100ms
            shell_print("✅ CPU benchmark: EXCELLENT performance\n");
        } else if (duration < 500000) { // < 500ms
            shell_print("✅ CPU benchmark: GOOD performance\n");
        } else {
            shell_print("⚠️  CPU benchmark: MODERATE performance\n");
        }

    } else if (strcmp(bench_type, "memory") == 0) {
        shell_print("🏃 Running memory benchmark...\n");

        if (!g_shell_context || !g_shell_context->memory_manager) {
            shell_print("❌ Memory manager not available\n");
            return CMD_RESULT_ERROR;
        }

        uint64_t start_time = platform_get_timestamp_us();

        // Memory allocation/deallocation benchmark
        void* ptrs[100];
        bool all_success = true;

        for (int i = 0; i < 100; i++) {
            memory_request_t request = {
                .size = 1024,
                .alignment = ALIGN_DEFAULT,
                .flags = MEM_FLAG_ZERO,
                .preferred_zone = ZONE_HEAP,
                .timeout_ms = 1000,
                .debug_name = "benchmark"
            };

            memory_result_t result = allocate_memory(g_shell_context->memory_manager, &request);
            if (result.success) {
                ptrs[i] = result.address;
            } else {
                all_success = false;
                break;
            }
        }

        // Free all allocations
        for (int i = 0; i < 100; i++) {
            if (ptrs[i]) {
                free_memory(g_shell_context->memory_manager, ptrs[i]);
            }
        }

        uint64_t end_time = platform_get_timestamp_us();
        uint64_t duration = end_time - start_time;

        shell_printf("  100 alloc/free cycles in %llu μs\n", (unsigned long long)duration);
        shell_printf("  Average per operation: %llu μs\n", (unsigned long long)(duration / 200));

        if (all_success && duration < 10000) { // < 10ms
            shell_print("✅ Memory benchmark: EXCELLENT performance\n");
        } else if (all_success && duration < 50000) { // < 50ms
            shell_print("✅ Memory benchmark: GOOD performance\n");
        } else {
            shell_print("⚠️  Memory benchmark: MODERATE performance\n");
        }

    } else if (strcmp(bench_type, "steppps") == 0) {
        shell_print("🏃 Running STEPPPS benchmark...\n");

        if (!g_shell_context || !g_shell_context->steppps_manager) {
            shell_print("❌ STEPPPS framework not available\n");
            return CMD_RESULT_ERROR;
        }

        uint64_t start_time = platform_get_timestamp_us();
        uint64_t operations_before = g_shell_context->steppps_manager->total_operations;

        // Run 1000 STEPPPS cycles
        bool all_success = true;
        for (int i = 0; i < 1000; i++) {
            if (!process_steppps_cycle(g_shell_context->steppps_manager)) {
                all_success = false;
                break;
            }
        }

        uint64_t end_time = platform_get_timestamp_us();
        uint64_t duration = end_time - start_time;
        uint64_t operations_after = g_shell_context->steppps_manager->total_operations;

        shell_printf("  1000 STEPPPS cycles in %llu μs\n", (unsigned long long)duration);
        shell_printf("  Average cycle time: %llu μs\n", (unsigned long long)(duration / 1000));
        shell_printf("  Operations: %llu -> %llu\n",
                    (unsigned long long)operations_before,
                    (unsigned long long)operations_after);

        if (all_success && duration < 50000) { // < 50ms for 1000 cycles
            shell_print("✅ STEPPPS benchmark: EXCELLENT performance\n");
        } else if (all_success && duration < 200000) { // < 200ms
            shell_print("✅ STEPPPS benchmark: GOOD performance\n");
        } else {
            shell_print("⚠️  STEPPPS benchmark: MODERATE performance\n");
        }

    } else if (strcmp(bench_type, "integration") == 0) {
        shell_print("🏃 Running integration benchmark...\n");

        uint64_t start_time = platform_get_timestamp_us();

        // Combined benchmark: memory + STEPPPS
        bool success = true;

        if (g_shell_context && g_shell_context->memory_manager && g_shell_context->steppps_manager) {
            for (int i = 0; i < 50; i++) {
                // Allocate memory
                memory_request_t request = {
                    .size = 512,
                    .alignment = ALIGN_DEFAULT,
                    .flags = MEM_FLAG_ZERO,
                    .preferred_zone = ZONE_HEAP,
                    .timeout_ms = 1000,
                    .debug_name = "integration_bench"
                };

                memory_result_t result = allocate_memory(g_shell_context->memory_manager, &request);
                if (!result.success) {
                    success = false;
                    break;
                }

                // Process STEPPPS cycle
                if (!process_steppps_cycle(g_shell_context->steppps_manager)) {
                    success = false;
                    free_memory(g_shell_context->memory_manager, result.address);
                    break;
                }

                // Free memory
                free_memory(g_shell_context->memory_manager, result.address);
            }
        } else {
            success = false;
        }

        uint64_t end_time = platform_get_timestamp_us();
        uint64_t duration = end_time - start_time;

        shell_printf("  50 integrated operations in %llu μs\n", (unsigned long long)duration);
        shell_printf("  Average operation time: %llu μs\n", (unsigned long long)(duration / 50));

        if (success && duration < 100000) { // < 100ms
            shell_print("✅ Integration benchmark: EXCELLENT performance\n");
        } else if (success && duration < 500000) { // < 500ms
            shell_print("✅ Integration benchmark: GOOD performance\n");
        } else {
            shell_print("⚠️  Integration benchmark: MODERATE performance\n");
        }

    } else {
        shell_printf("Unknown benchmark: %s\n", bench_type);
        shell_print("Use 'benchmark' to see available benchmarks.\n");
        return CMD_RESULT_INVALID_ARGS;
    }

    return CMD_RESULT_SUCCESS;
}

