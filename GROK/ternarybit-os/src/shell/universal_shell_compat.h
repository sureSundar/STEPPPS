/**
 * @file universal_shell_compat.h
 * @brief Universal Shell Compatibility Layer for TernaryBit OS
 *
 * Implements ALL shell features from Linux, Windows, and macOS in a unified interface.
 * This revolutionary compatibility layer allows TernaryBit OS to run any shell command
 * from any major operating system, automatically translating and adapting as needed.
 *
 * Supported Shell Environments:
 * - Linux: bash, zsh, fish, dash, tcsh, csh
 * - Windows: cmd, PowerShell, WSL
 * - macOS: bash, zsh, Terminal.app features
 * - Universal: Cross-platform command translation
 *
 * @version 1.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#ifndef UNIVERSAL_SHELL_COMPAT_H
#define UNIVERSAL_SHELL_COMPAT_H

#include "command_shell.h"
#include <stdint.h>
#include <stdbool.h>

// Shell compatibility modes
typedef enum {
    SHELL_MODE_UNIVERSAL = 0,    // TernaryBit native mode
    SHELL_MODE_LINUX_BASH = 1,   // Linux bash compatibility
    SHELL_MODE_LINUX_ZSH = 2,    // Linux zsh compatibility
    SHELL_MODE_WINDOWS_CMD = 3,  // Windows cmd compatibility
    SHELL_MODE_WINDOWS_PS = 4,   // Windows PowerShell compatibility
    SHELL_MODE_MACOS_BASH = 5,   // macOS bash compatibility
    SHELL_MODE_MACOS_ZSH = 6,    // macOS zsh compatibility
    SHELL_MODE_AUTO = 7          // Auto-detect from command syntax
} shell_mode_t;

// Command categories for compatibility mapping
typedef enum {
    CMD_CAT_FILE_OPS = 0,        // File operations (ls, dir, copy, etc.)
    CMD_CAT_PROCESS = 1,         // Process management (ps, kill, etc.)
    CMD_CAT_NETWORK = 2,         // Network operations (ping, curl, etc.)
    CMD_CAT_SYSTEM = 3,          // System information (uname, ver, etc.)
    CMD_CAT_TEXT = 4,            // Text processing (grep, find, etc.)
    CMD_CAT_ARCHIVE = 5,         // Archive operations (tar, zip, etc.)
    CMD_CAT_PACKAGE = 6,         // Package management (apt, brew, etc.)
    CMD_CAT_DEVELOPMENT = 7,     // Development tools (git, make, etc.)
    CMD_CAT_ADMIN = 8,           // Administrative commands (sudo, etc.)
    CMD_CAT_SHELL = 9            // Shell built-ins (cd, export, etc.)
} command_category_t;

// Universal command mapping structure
typedef struct {
    const char* linux_cmd;       // Linux command
    const char* windows_cmd;     // Windows equivalent
    const char* macos_cmd;       // macOS equivalent
    const char* universal_cmd;   // TernaryBit universal command
    command_category_t category; // Command category
    const char* description;     // Command description
    command_function_t handler;  // Universal handler function
} universal_command_map_t;

// Shell environment structure
typedef struct {
    shell_mode_t current_mode;   // Current shell mode
    shell_mode_t preferred_mode; // User's preferred mode
    bool auto_translate;         // Auto-translate commands
    bool show_translations;      // Show command translations
    char prompt_style[64];       // Shell prompt style

    // Environment variables (cross-platform)
    char** env_vars;             // Environment variables
    uint32_t env_count;          // Number of environment variables

    // Command history per shell mode
    char shell_history[8][MAX_HISTORY_ENTRIES][MAX_COMMAND_LENGTH];
    uint32_t shell_history_count[8];

    // Shell-specific configurations
    bool case_sensitive;         // Case sensitivity
    char path_separator;         // Path separator (/ or \)
    bool glob_enabled;           // Glob pattern matching
    bool tab_completion;         // Tab completion enabled

} shell_environment_t;

// Enhanced command result for universal shell compatibility
typedef struct {
    bool success;                    // Execution success
    const char* message;            // Result message
    command_result_t result_code;   // Standard result code
} universal_command_result_t;

// Function prototypes

/**
 * @brief Initialize universal shell compatibility layer
 * @param shell Shell context
 * @return bool Success/failure
 */
bool initialize_universal_shell_compat(shell_context_t* shell);

/**
 * @brief Set shell compatibility mode
 * @param shell Shell context
 * @param mode Target shell mode
 * @return bool Success/failure
 */
bool set_shell_mode(shell_context_t* shell, shell_mode_t mode);

/**
 * @brief Translate command between shell environments
 * @param command Input command
 * @param from_mode Source shell mode
 * @param to_mode Target shell mode
 * @param output Output buffer for translated command
 * @param output_size Output buffer size
 * @return bool Translation success
 */
bool translate_command(const char* command, shell_mode_t from_mode,
                      shell_mode_t to_mode, char* output, size_t output_size);

/**
 * @brief Auto-detect shell mode from command syntax
 * @param command Command to analyze
 * @return shell_mode_t Detected shell mode
 */
shell_mode_t detect_shell_mode(const char* command);

/**
 * @brief Execute universal command with compatibility layer
 * @param shell Shell context
 * @param command Command to execute
 * @return command_result_t Execution result
 */
command_result_t execute_universal_command(shell_context_t* shell, const char* command);

// ============================================================================
// LINUX SHELL COMMANDS
// ============================================================================

// File operations
command_result_t cmd_linux_ls(int argc, char** argv);      // ls -la, -h, -R, etc.
command_result_t cmd_linux_cat(int argc, char** argv);     // cat, less, more
command_result_t cmd_linux_cp(int argc, char** argv);      // cp -r, -v, etc.
command_result_t cmd_linux_mv(int argc, char** argv);      // mv, rename
command_result_t cmd_linux_rm(int argc, char** argv);      // rm -rf, etc.
command_result_t cmd_linux_mkdir(int argc, char** argv);   // mkdir -p
command_result_t cmd_linux_rmdir(int argc, char** argv);   // rmdir, rm -d
command_result_t cmd_linux_chmod(int argc, char** argv);   // chmod +x, etc.
command_result_t cmd_linux_chown(int argc, char** argv);   // chown user:group
command_result_t cmd_linux_find(int argc, char** argv);    // find with all options
command_result_t cmd_linux_locate(int argc, char** argv);  // locate, updatedb
command_result_t cmd_linux_which(int argc, char** argv);   // which, whereis
command_result_t cmd_linux_ln(int argc, char** argv);      // ln -s (symlinks)
command_result_t cmd_linux_du(int argc, char** argv);      // du -h, -s
command_result_t cmd_linux_df(int argc, char** argv);      // df -h

// Text processing
command_result_t cmd_linux_grep(int argc, char** argv);    // grep -r, -i, -v
command_result_t cmd_linux_sed(int argc, char** argv);     // sed stream editor
command_result_t cmd_linux_awk(int argc, char** argv);     // awk programming
command_result_t cmd_linux_sort(int argc, char** argv);    // sort -n, -r
command_result_t cmd_linux_uniq(int argc, char** argv);    // uniq, uniq -c
command_result_t cmd_linux_cut(int argc, char** argv);     // cut fields
command_result_t cmd_linux_tr(int argc, char** argv);      // tr translate
command_result_t cmd_linux_wc(int argc, char** argv);      // wc word count
command_result_t cmd_linux_head(int argc, char** argv);    // head -n
command_result_t cmd_linux_tail(int argc, char** argv);    // tail -f

// Process management
command_result_t cmd_linux_ps(int argc, char** argv);      // ps aux, -ef
command_result_t cmd_linux_top(int argc, char** argv);     // top, htop
command_result_t cmd_linux_kill(int argc, char** argv);    // kill -9, killall
command_result_t cmd_linux_jobs(int argc, char** argv);    // jobs, bg, fg
command_result_t cmd_linux_nohup(int argc, char** argv);   // nohup background

// System information
command_result_t cmd_linux_uname(int argc, char** argv);   // uname -a
command_result_t cmd_linux_whoami(int argc, char** argv);  // whoami, id
command_result_t cmd_linux_uptime(int argc, char** argv);  // uptime, w
command_result_t cmd_linux_free(int argc, char** argv);    // free -h
command_result_t cmd_linux_lscpu(int argc, char** argv);   // lscpu, /proc/cpuinfo
command_result_t cmd_linux_lsblk(int argc, char** argv);   // lsblk block devices
command_result_t cmd_linux_lsusb(int argc, char** argv);   // lsusb USB devices
command_result_t cmd_linux_lspci(int argc, char** argv);   // lspci PCI devices

// Network commands
command_result_t cmd_linux_ping(int argc, char** argv);    // ping, ping6
command_result_t cmd_linux_wget(int argc, char** argv);    // wget downloads
command_result_t cmd_linux_curl(int argc, char** argv);    // curl HTTP client
command_result_t cmd_linux_ssh(int argc, char** argv);     // ssh, scp, sftp
command_result_t cmd_linux_netstat(int argc, char** argv); // netstat, ss
command_result_t cmd_linux_ifconfig(int argc, char** argv);// ifconfig, ip

// Archive operations
command_result_t cmd_linux_tar(int argc, char** argv);     // tar -xzf, etc.
command_result_t cmd_linux_gzip(int argc, char** argv);    // gzip, gunzip
command_result_t cmd_linux_zip(int argc, char** argv);     // zip, unzip

// Package management
command_result_t cmd_linux_apt(int argc, char** argv);     // apt, apt-get
command_result_t cmd_linux_yum(int argc, char** argv);     // yum, dnf
command_result_t cmd_linux_pacman(int argc, char** argv);  // pacman (Arch)

// ============================================================================
// WINDOWS SHELL COMMANDS
// ============================================================================

// File operations (CMD)
command_result_t cmd_windows_dir(int argc, char** argv);   // dir /s /a
command_result_t cmd_windows_copy(int argc, char** argv);  // copy, xcopy, robocopy
command_result_t cmd_windows_move(int argc, char** argv);  // move
command_result_t cmd_windows_del(int argc, char** argv);   // del, erase
command_result_t cmd_windows_md(int argc, char** argv);    // md, mkdir
command_result_t cmd_windows_rd(int argc, char** argv);    // rd, rmdir
command_result_t cmd_windows_attrib(int argc, char** argv);// attrib +r -h
command_result_t cmd_windows_type(int argc, char** argv);  // type (cat equivalent)
command_result_t cmd_windows_find(int argc, char** argv);  // find, findstr
command_result_t cmd_windows_where(int argc, char** argv); // where (which equivalent)

// System commands (CMD)
command_result_t cmd_windows_ver(int argc, char** argv);   // ver (version)
command_result_t cmd_windows_date(int argc, char** argv);  // date, time
command_result_t cmd_windows_systeminfo(int argc, char** argv); // systeminfo
command_result_t cmd_windows_tasklist(int argc, char** argv);   // tasklist
command_result_t cmd_windows_taskkill(int argc, char** argv);   // taskkill
command_result_t cmd_windows_ipconfig(int argc, char** argv);   // ipconfig

// PowerShell commands
command_result_t cmd_powershell_get_childitem(int argc, char** argv);  // Get-ChildItem (ls)
command_result_t cmd_powershell_get_content(int argc, char** argv);    // Get-Content (cat)
command_result_t cmd_powershell_copy_item(int argc, char** argv);      // Copy-Item
command_result_t cmd_powershell_remove_item(int argc, char** argv);    // Remove-Item
command_result_t cmd_powershell_get_process(int argc, char** argv);    // Get-Process
command_result_t cmd_powershell_stop_process(int argc, char** argv);   // Stop-Process
command_result_t cmd_powershell_get_service(int argc, char** argv);    // Get-Service
command_result_t cmd_powershell_invoke_webrequest(int argc, char** argv); // Invoke-WebRequest

// ============================================================================
// MACOS SHELL COMMANDS
// ============================================================================

// macOS-specific commands
command_result_t cmd_macos_open(int argc, char** argv);    // open (launch apps)
command_result_t cmd_macos_pbcopy(int argc, char** argv);  // pbcopy (clipboard)
command_result_t cmd_macos_pbpaste(int argc, char** argv); // pbpaste
command_result_t cmd_macos_say(int argc, char** argv);     // say (text-to-speech)
command_result_t cmd_macos_screencapture(int argc, char** argv); // screencapture
command_result_t cmd_macos_defaults(int argc, char** argv); // defaults (preferences)
command_result_t cmd_macos_launchctl(int argc, char** argv); // launchctl (services)
command_result_t cmd_macos_diskutil(int argc, char** argv); // diskutil
command_result_t cmd_macos_brew(int argc, char** argv);    // Homebrew package manager
command_result_t cmd_macos_softwareupdate(int argc, char** argv); // softwareupdate

// ============================================================================
// UNIVERSAL CROSS-PLATFORM COMMANDS
// ============================================================================

// Universal file operations
command_result_t cmd_universal_list(int argc, char** argv);     // Universal ls/dir
command_result_t cmd_universal_copy(int argc, char** argv);     // Universal cp/copy
command_result_t cmd_universal_move(int argc, char** argv);     // Universal mv/move
command_result_t cmd_universal_delete(int argc, char** argv);   // Universal rm/del
command_result_t cmd_universal_makedir(int argc, char** argv);  // Universal mkdir/md
command_result_t cmd_universal_show(int argc, char** argv);     // Universal cat/type

// Universal system commands
command_result_t cmd_universal_processes(int argc, char** argv); // Universal ps/tasklist
command_result_t cmd_universal_terminate(int argc, char** argv); // Universal kill/taskkill
command_result_t cmd_universal_sysinfo(int argc, char** argv);   // Universal system info
command_result_t cmd_universal_network(int argc, char** argv);   // Universal network info

// Shell environment commands
command_result_t cmd_shell_mode(int argc, char** argv);     // Switch shell modes
command_result_t cmd_shell_translate(int argc, char** argv); // Translate commands
command_result_t cmd_shell_compat(int argc, char** argv);   // Compatibility info

// Environment variable commands
command_result_t cmd_env(int argc, char** argv);            // Show environment variables
command_result_t cmd_export(int argc, char** argv);         // Export environment variable
command_result_t cmd_set(int argc, char** argv);            // Set environment variable (Windows style)

// Advanced file operations
command_result_t cmd_find(int argc, char** argv);           // Find files
command_result_t cmd_which(int argc, char** argv);          // Locate command
command_result_t cmd_head(int argc, char** argv);           // Show first lines of file
command_result_t cmd_tail(int argc, char** argv);           // Show last lines of file

// Global shell environment
extern shell_environment_t* g_shell_environment;

// Command mapping tables
extern const universal_command_map_t linux_command_map[];
extern const universal_command_map_t windows_command_map[];
extern const universal_command_map_t macos_command_map[];
extern const universal_command_map_t universal_command_map[];

#endif // UNIVERSAL_SHELL_COMPAT_H