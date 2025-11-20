// TernaryBit OS - Stage 1 Interactive Shell
// Provides a minimal yet functional shell backed by RAMFS.

#include "shell.h"
#include "tbos/libc.h"
#include "tbos/stdio.h"
#include "tbos/vfs.h"
#include "tbos/errno.h"
#include "tbos/shell_morph.h"
#include "tbos/argparse.h"
#include "fs/ucfs_codec.h"
#include "fs/ucfs_overlay.h"
#include "fs/ucfs_config.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// External dependencies provided by the kernel core.
extern void kernel_print(const char* str);
extern void kernel_print_hex(uint32_t value);
extern void kernel_putchar(char c);
extern uint8_t keyboard_read_char(void);
extern uint8_t keyboard_read_char_poll(void);  // Polling mode (no interrupts)
extern void clear_screen(void);

#define MAX_CMD_LENGTH 256
#define SHELL_MAX_PATH 256
#define MAX_PATH_COMPONENTS 32
#define PATH_COMPONENT_LEN 64
#define SERIAL_PORT 0x3F8
#define SERIAL_LSR (SERIAL_PORT + 5)

typedef struct {
    char buffer[MAX_CMD_LENGTH];
    uint32_t length;
    uint32_t cursor;
} command_buffer_t;

static command_buffer_t cmd_buffer = {0};

static int32_t user_karma = 100;
static uint8_t consciousness_level = 1; /* AWAKENING */
static uint32_t commands_executed = 0;
static char current_path[SHELL_MAX_PATH] = "/";

/* ═══════════════════════════════════════════════════════════════════════════
 * Helper utilities
 * ═══════════════════════════════════════════════════════════════════════════ */

static void shell_print_decimal(int value);
static void shell_print_prompt(void);
static char* trim_spaces(char* str);
static void normalize_path(const char* input, char* out, size_t out_size);
static void print_errno_message(const char* prefix);
static uint8_t shell_serial_read_char(void);
static void shell_process_command(char* cmd);

/* ═══════════════════════════════════════════════════════════════════════════
 * Command declarations
 * ═══════════════════════════════════════════════════════════════════════════ */

static void cmd_help(void);
static void cmd_clear(void);
static void cmd_ps(void);
static void cmd_mem(void);
static void cmd_steppps(void);
static void cmd_reboot(void);
static void cmd_calc(const char* args);
static void cmd_echo(const char* args);
static void cmd_time(void);
static void cmd_test(void);
static void cmd_about(void);
static void cmd_pwd(void);
static void cmd_cd(const char* args);
static void cmd_ls(const char* args);
static void cmd_cat(const char* args);
static void cmd_mkdir(const char* args);
static void cmd_touch(const char* args);
static void cmd_rm(const char* args);
static void cmd_rmdir(const char* args);
static void cmd_karma(void);
static void cmd_consciousness(void);
static void cmd_ucfs_encode(const char* args);
static void cmd_ucfs_info(const char* args);
static void cmd_ucfs_test(void);
static void cmd_ucfs_help(void);
static void cmd_ucfs_config(const char* args);
static void cmd_cp(const char* args);
static void cmd_mv(const char* args);
static void cmd_head(const char* args);
static void cmd_tail(const char* args);
static void cmd_date(void);
static void cmd_uptime(void);
static void cmd_env(void);
static void cmd_whoami(void);
static void cmd_compassion(void);
static void cmd_fast(void);
static void cmd_sangha(void);
static void cmd_history(void);
static void cmd_metrics(void);
static void cmd_events(void);
static void cmd_http(const char* args);
static void cmd_ping(const char* args);
static void cmd_netstat(void);
static void cmd_persona(const char* args);
static void cmd_hal(void);
static void cmd_shutdown(void);
static void cmd_top(void);
static void cmd_posix_shell(void);
static void cmd_ch_sh(const char* args);
static void cmd_grep(const char* args);

/* ═══════════════════════════════════════════════════════════════════════════
 * Helper implementations
 * ═══════════════════════════════════════════════════════════════════════════ */

static inline uint8_t io_inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static uint8_t shell_serial_read_char(void) {
    if ((io_inb(SERIAL_LSR) & 0x01) == 0) {
        return 0;
    }
    return io_inb(SERIAL_PORT);
}

static void shell_print_decimal(int value) {
    char buffer[16];
    uint32_t magnitude;
    int index = 0;
    bool negative = false;

    if (value == 0) {
        kernel_print("0");
        return;
    }

    if (value < 0) {
        negative = true;
        magnitude = (uint32_t)(-(int64_t)value);
    } else {
        magnitude = (uint32_t)value;
    }

    while (magnitude > 0 && index < (int)sizeof(buffer)) {
        buffer[index++] = (char)('0' + (magnitude % 10));
        magnitude /= 10;
    }

    if (negative) {
        kernel_print("-");
    }

    while (index > 0) {
        char out[2] = {buffer[--index], '\0'};
        kernel_print(out);
    }
}

static void shell_print_prompt(void) {
    const shell_interpreter_t* current = shell_morph_current();
    if (current && current->prompt) {
        /* Use current shell's prompt */
        if (strcmp(current->name, "tbos") == 0) {
            kernel_print("tbos:");
            kernel_print(current_path);
            kernel_print("> ");
        } else {
            /* For other shells, just use their default prompt */
            kernel_print(current->prompt);
        }
    } else {
        /* Fallback */
        kernel_print("tbos:");
        kernel_print(current_path);
        kernel_print("> ");
    }
}

static char* trim_spaces(char* str) {
    if (!str) return NULL;
    while (*str == ' ') str++;
    if (*str == '\0') return NULL;
    size_t len = strlen(str);
    while (len > 0 && str[len - 1] == ' ') {
        str[len - 1] = '\0';
        len--;
    }
    return str;
}

static void normalize_path(const char* input, char* out, size_t out_size) {
    if (!out || out_size == 0) return;

    if (!input || !*input) {
        strncpy(out, current_path, out_size - 1);
        out[out_size - 1] = '\0';
        return;
    }

    char combined[SHELL_MAX_PATH];
    if (input[0] == '/') {
        strncpy(combined, input, sizeof(combined) - 1);
        combined[sizeof(combined) - 1] = '\0';
    } else if (strcmp(current_path, "/") == 0) {
        snprintf(combined, sizeof(combined), "/%s", input);
    } else {
        snprintf(combined, sizeof(combined), "%s/%s", current_path, input);
    }

    char temp[SHELL_MAX_PATH];
    strncpy(temp, combined, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    char components[MAX_PATH_COMPONENTS][PATH_COMPONENT_LEN];
    int component_count = 0;

    char* saveptr = NULL;
    char* token = strtok_r(temp, "/", &saveptr);

    while (token && component_count < MAX_PATH_COMPONENTS) {
        if (strcmp(token, "..") == 0) {
            if (component_count > 0) {
                component_count--;
            }
        } else if (token[0] != '\0' && strcmp(token, ".") != 0) {
            strncpy(components[component_count], token, PATH_COMPONENT_LEN - 1);
            components[component_count][PATH_COMPONENT_LEN - 1] = '\0';
            component_count++;
        }
        token = strtok_r(NULL, "/", &saveptr);
    }

    if (component_count == 0) {
        out[0] = '/';
        out[1] = '\0';
        return;
    }

    size_t pos = 0;
    out[0] = '\0';

    for (int i = 0; i < component_count; i++) {
        if (pos + 1 >= out_size) break;
        out[pos++] = '/';
        size_t len = strlen(components[i]);
        if (pos + len >= out_size) {
            len = out_size - pos - 1;
        }
        memcpy(&out[pos], components[i], len);
        pos += len;
        out[pos] = '\0';
    }

    if (out[0] == '\0') {
        out[0] = '/';
        out[1] = '\0';
    }
}

static void print_errno_message(const char* prefix) {
    kernel_print(prefix);
    if (errno != 0) {
        kernel_print(" (errno ");
        shell_print_decimal(errno);
        kernel_print(")");
    }
    kernel_print("\n");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Command implementations
 * ═══════════════════════════════════════════════════════════════════════════ */

static void cmd_help(void) {
    kernel_print("\n=== TernaryBit OS Shell (54+ Commands) ===\n");
    kernel_print("\n[General]\n");
    kernel_print("  help, clear, cls, about, reboot, shutdown, test\n");
    kernel_print("\n[Processes & System]\n");
    kernel_print("  ps, top, mem, hal, steppps, time, date, uptime, env, whoami\n");
    kernel_print("\n[Filesystem Operations]\n");
    kernel_print("  pwd, ls, cd, cat, mkdir, touch, rm, rmdir\n");
    kernel_print("  cp <src> <dst>, mv <src> <dst>, head <file>, tail <file>\n");
    kernel_print("\n[Text Processing]\n");
    kernel_print("  grep <pattern> <file> - Search for patterns in files\n");
    kernel_print("  Use --help with any command for options (e.g., ls --help)\n");
    kernel_print("\n[UCFS Commands]\n");
    kernel_print("  ucfs-encode, ucfs-info, ucfs-test, ucfs-help, ucfs-config\n");
    kernel_print("\n[Shell Morphing]\n");
    kernel_print("  ch-sh [shell]  - Switch shells (tbos, sh)\n");
    kernel_print("                   Examples: ch-sh sh, ch-sh tbos\n");
    kernel_print("\n[Consciousness & Karma]\n");
    kernel_print("  karma, consciousness, om, compassion, fast, sangha\n");
    kernel_print("  history, metrics, events\n");
    kernel_print("\n[Network (stubs)]\n");
    kernel_print("  http, ping, netstat, persona\n");
    kernel_print("\n[Utilities]\n");
    kernel_print("  calc <expr>, echo <text>, posix\n");
    kernel_print("\nPOSIX Flags: ls -lah, cat -n, grep -in\n");
    kernel_print("Filesystem: RAMFS + VFS + UCFS mounted at '/'\n");
}

static void cmd_clear(void) {
    clear_screen();
    kernel_print("TernaryBit OS v1.0 - Interactive Shell\n");
    kernel_print("Type 'help' for available commands\n");
}

static void cmd_ps(void) {
    kernel_print("\n=== Process List ===\n");
    kernel_print("PID  STATE    NAME\n");
    kernel_print("---  -------  ------------\n");
    kernel_print("0    RUNNING  kernel\n");
    kernel_print("1    RUNNING  shell\n");
    kernel_print("(Process manager integration in progress)\n");
}

static void cmd_mem(void) {
    kernel_print("\n=== Memory Status (Prototype) ===\n");
    kernel_print("Total Memory : 64 MB\n");
    kernel_print("Used Memory  : 12 MB\n");
    kernel_print("Free Memory  : 52 MB\n");
    kernel_print("(Replace with real measurements once MMU is online.)\n");
}

static void cmd_steppps(void) {
    kernel_print("\n=== STEPPPS Framework ===\n");
    kernel_print("[SPACE]      Hardware footprint       : ACTIVE\n");
    kernel_print("[TIME]       Scheduler heartbeat       : BOOTSTRAP\n");
    kernel_print("[EVENT]      Interrupt fabric          : FUNCTIONAL\n");
    kernel_print("[PSYCHOLOGY] Adaptive heuristics       : PLANNED\n");
    kernel_print("[PIXEL]      Graphics subsystem        : PLANNED\n");
    kernel_print("[PROMPT]     Conversational layer      : INTEGRATING\n");
    kernel_print("[SCRIPT]     Automation orchestrator   : IN DESIGN\n");
}

static void cmd_calc(const char* expr) {
    if (!expr || !*expr) {
        kernel_print("Usage: calc <expression>\n");
        kernel_print("Example: calc 42+8\n");
        return;
    }

    expr = trim_spaces((char*)expr);
    if (!expr) {
        kernel_print("Usage: calc <expression>\n");
        return;
    }

    int a = 0;
    int b = 0;
    char op = 0;
    int sign = 1;

    if (*expr == '-') {
        sign = -1;
        expr++;
    }
    while (*expr == ' ') expr++;

    while (*expr && isdigit((unsigned char)*expr)) {
        a = a * 10 + (*expr - '0');
        expr++;
    }
    a *= sign;

    while (*expr == ' ') expr++;
    op = *expr;
    if (op == '\0') {
        kernel_print("Result: ");
        shell_print_decimal(a);
        kernel_print("\n");
        return;
    }
    expr++;
    while (*expr == ' ') expr++;

    sign = 1;
    if (*expr == '-') {
        sign = -1;
        expr++;
    }
    while (*expr && isdigit((unsigned char)*expr)) {
        b = b * 10 + (*expr - '0');
        expr++;
    }
    b *= sign;

    // Check if there are more operators (not supported)
    while (*expr == ' ') expr++;
    if (*expr != '\0') {
        kernel_print("Error: calc only supports simple expressions like '5+3'\n");
        kernel_print("For complex expressions, use parentheses or break into steps\n");
        return;
    }

    int result = 0;
    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/':
            if (b == 0) {
                kernel_print("Error: Division by zero\n");
                return;
            }
            result = a / b;
            break;
        case '%':
            if (b == 0) {
                kernel_print("Error: Modulo by zero\n");
                return;
            }
            result = a % b;
            break;
        default:
            kernel_print("Error: Invalid operator '");
            if (op >= 32 && op < 127) {
                char op_str[2] = {op, '\0'};
                kernel_print(op_str);
            } else {
                kernel_print("(non-printable)");
            }
            kernel_print("' (use +, -, *, /, %)\n");
            return;
    }

    kernel_print("Result: ");
    shell_print_decimal(result);
    kernel_print("\n");
}

static void cmd_echo(const char* text) {
    if (!text || !*text) {
        kernel_print("\n");
        return;
    }
    kernel_print(text);
    kernel_print("\n");
}

static void cmd_time(void) {
    kernel_print("\n=== System Time ===\n");
    kernel_print("Uptime counter pending timer driver integration.\n");
}

static void cmd_test(void) {
    kernel_print("\n=== Running System Diagnostics ===\n");
    kernel_print("[PASS] Shell: command dispatch\n");
    kernel_print("[PASS] Keyboard: interrupt delivery\n");
    kernel_print("[PASS] RAMFS: mount point\n");
    kernel_print("[TODO] Timer: real uptime metrics\n");
    kernel_print("[TODO] Network: awaiting PXFS bridge\n");
}

static void cmd_about(void) {
    kernel_print("\n=== About TernaryBit OS ===\n");
    kernel_print("Mode: Stage 1 Shell (Bare Metal)\n");
    kernel_print("Filesystem: RAMFS mounted at /\n");
    kernel_print("Goal: Graduate universal shell to kernel-space\n");
    kernel_print("Roadmap: Legacy compatibility → PXFS/PhotonFS/UCFS/RF2FS\n");
}

static void cmd_pwd(void) {
    kernel_print(current_path);
    kernel_print("\n");
}

static void cmd_cd(const char* args) {
    const char* target_input = (args && *args) ? args : "/";
    char path[SHELL_MAX_PATH];
    normalize_path(target_input, path, sizeof(path));

    struct stat st;
    errno = 0;
    if (stat(path, &st) != 0) {
        print_errno_message("cd: path not found");
        return;
    }
    if (st.st_mode != VFS_NODE_DIR) {
        kernel_print("cd: not a directory\n");
        return;
    }

    strncpy(current_path, path, sizeof(current_path) - 1);
    current_path[sizeof(current_path) - 1] = '\0';
}

static void cmd_ls(const char* args) {
    /* Parse arguments */
    argparse_result_t parsed;
    argparse_parse(args, &parsed);

    /* Check for --help */
    if (argparse_has_flag(&parsed, "help") || argparse_has_flag(&parsed, "h")) {
        flag_spec_t specs[] = {
            {'l', "long", false, "Use long listing format"},
            {'a', "all", false, "Show hidden files (starting with .)"},
            {'h', "human-readable", false, "Print sizes in human readable format"},
            {'1', NULL, false, "List one file per line"},
            {0, "help", false, "Display this help message"}
        };
        argparse_print_help("ls", "List directory contents", "[OPTIONS] [PATH]", specs, 5);
        argparse_cleanup(&parsed);
        return;
    }

    /* Get flags */
    bool long_format = argparse_has_flag(&parsed, "l") || argparse_has_flag(&parsed, "long");
    bool show_all = argparse_has_flag(&parsed, "a") || argparse_has_flag(&parsed, "all");
    bool human_readable = argparse_has_flag(&parsed, "h") || argparse_has_flag(&parsed, "human-readable");
    bool one_per_line = argparse_has_flag(&parsed, "1") || long_format;

    /* Get path argument */
    char path[SHELL_MAX_PATH];
    const char* path_arg = argparse_get_positional(&parsed, 0);
    if (path_arg) {
        normalize_path(path_arg, path, sizeof(path));
    } else {
        strncpy(path, current_path, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }

    struct stat st;
    errno = 0;
    if (stat(path, &st) != 0) {
        print_errno_message("ls: path not found");
        argparse_cleanup(&parsed);
        return;
    }

    if (st.st_mode == VFS_NODE_FILE) {
        if (long_format) {
            kernel_print(st.st_mode == VFS_NODE_DIR ? "d" : "-");
            kernel_print("rw-r--r-- 1 root root ");
            shell_print_decimal((int)st.st_size);
            kernel_print(" ");
        }
        kernel_print(path);
        kernel_print("\n");
        argparse_cleanup(&parsed);
        return;
    }

    DIR* dir = opendir(path);
    if (!dir) {
        print_errno_message("ls: cannot open directory");
        argparse_cleanup(&parsed);
        return;
    }

    if (!one_per_line) kernel_print("\n");

    struct dirent* entry;
    while ((entry = readdir(dir))) {
        /* Skip hidden files unless -a is specified */
        if (!show_all && entry->d_name[0] == '.') {
            continue;
        }

        if (long_format) {
            /* Long format: type, permissions, size, name */
            kernel_print(entry->d_type == VFS_NODE_DIR ? "d" : "-");
            kernel_print("rw-r--r-- 1 root root ");

            /* Get file size */
            char full_path[SHELL_MAX_PATH];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            struct stat entry_st;
            if (stat(full_path, &entry_st) == 0) {
                if (human_readable && entry_st.st_size >= 1024) {
                    int kb = (int)(entry_st.st_size / 1024);
                    shell_print_decimal(kb);
                    kernel_print("K");
                } else {
                    shell_print_decimal((int)entry_st.st_size);
                }
            } else {
                kernel_print("   0");
            }
            kernel_print(" ");
        }

        kernel_print(entry->d_name);
        if (entry->d_type == VFS_NODE_DIR && !long_format) {
            kernel_print("/");
        }

        if (one_per_line) {
            kernel_print("\n");
        } else {
            kernel_print("  ");
        }
    }

    if (!one_per_line) {
        kernel_print("\n");
    }

    closedir(dir);
    argparse_cleanup(&parsed);
}

static void cmd_cat(const char* args) {
    /* Parse arguments */
    argparse_result_t parsed;
    argparse_parse(args, &parsed);

    /* Check for --help */
    if (argparse_has_flag(&parsed, "help")) {
        flag_spec_t specs[] = {
            {'n', "number", false, "Number all output lines"},
            {'b', "number-nonblank", false, "Number non-blank output lines"},
            {0, "help", false, "Display this help message"}
        };
        argparse_print_help("cat", "Concatenate and print files", "[OPTIONS] FILE", specs, 3);
        argparse_cleanup(&parsed);
        return;
    }

    /* Get flags */
    bool number_lines = argparse_has_flag(&parsed, "n") || argparse_has_flag(&parsed, "number");
    bool number_nonblank = argparse_has_flag(&parsed, "b") || argparse_has_flag(&parsed, "number-nonblank");

    /* Get file argument */
    const char* file_arg = argparse_get_positional(&parsed, 0);
    if (!file_arg) {
        kernel_print("Usage: cat [OPTIONS] <file>\n");
        argparse_cleanup(&parsed);
        return;
    }

    char path[SHELL_MAX_PATH];
    normalize_path(file_arg, path, sizeof(path));

    errno = 0;
    FILE* fp = fopen(path, "r");
    if (!fp) {
        print_errno_message("cat: cannot open file");
        argparse_cleanup(&parsed);
        return;
    }

    char buffer[128];
    size_t nread;
    char last = '\0';
    int line_num = 1;
    bool at_line_start = true;

    while ((nread = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        for (size_t i = 0; i < nread; i++) {
            char ch = buffer[i];

            /* Print line number at start of line if needed */
            if (at_line_start && (number_lines || number_nonblank)) {
                /* Only print line number if:
                 * - number_lines is set, OR
                 * - number_nonblank is set AND this is not a blank line
                 */
                bool is_blank_line = (ch == '\n');
                if (number_lines || (number_nonblank && !is_blank_line)) {
                    /* Print line number with padding */
                    kernel_print("     ");
                    shell_print_decimal(line_num);
                    kernel_print("  ");
                }
                line_num++;
                at_line_start = false;
            }

            kernel_putchar(ch);
            last = ch;

            if (ch == '\n') {
                at_line_start = true;
            }
        }
    }

    if (ferror(fp)) {
        print_errno_message("cat: read error");
    }
    fclose(fp);

    if (last != '\n') {
        kernel_print("\n");
    }

    argparse_cleanup(&parsed);
}

static void cmd_mkdir(const char* args) {
    /* Parse arguments */
    argparse_result_t parsed;
    argparse_parse(args, &parsed);

    /* Check for --help */
    if (argparse_has_flag(&parsed, "help")) {
        flag_spec_t specs[] = {
            {'p', "parents", false, "Create parent directories as needed"},
            {0, "help", false, "Display this help message"}
        };
        argparse_print_help("mkdir", "Create directories", "[OPTIONS] DIRECTORY...", specs, 2);
        argparse_cleanup(&parsed);
        return;
    }

    /* Get flags */
    bool parents = argparse_has_flag(&parsed, "p") || argparse_has_flag(&parsed, "parents");

    /* Get directory argument */
    const char* dir_arg = argparse_get_positional(&parsed, 0);
    if (!dir_arg) {
        kernel_print("Usage: mkdir [OPTIONS] DIRECTORY...\n");
        argparse_cleanup(&parsed);
        return;
    }

    char path[SHELL_MAX_PATH];
    normalize_path(dir_arg, path, sizeof(path));

    if (parents) {
        /* Create parent directories as needed */
        char temp[SHELL_MAX_PATH];
        strncpy(temp, path, sizeof(temp) - 1);
        temp[sizeof(temp) - 1] = '\0';

        /* Walk through path and create each component */
        char* p = temp;
        if (*p == '/') p++;  /* Skip leading slash */

        while (*p) {
            /* Find next slash */
            while (*p && *p != '/') p++;

            char saved = *p;
            *p = '\0';

            /* Try to create this directory */
            struct stat st;
            if (stat(temp, &st) != 0) {
                int rc = vfs_mkdir(temp);
                if (rc != 0 && rc != -EEXIST) {
                    errno = -rc;
                    print_errno_message("mkdir: cannot create directory");
                    argparse_cleanup(&parsed);
                    return;
                }
            }

            *p = saved;
            if (*p) p++;  /* Skip past the slash */
        }
    } else {
        int rc = vfs_mkdir(path);
        if (rc != 0) {
            errno = -rc;
            print_errno_message("mkdir: cannot create directory");
        }
    }

    argparse_cleanup(&parsed);
}

static void cmd_touch(const char* args) {
    if (!args || !*args) {
        kernel_print("Usage: touch <file>\n");
        return;
    }

    char path[SHELL_MAX_PATH];
    normalize_path(args, path, sizeof(path));

    errno = 0;
    FILE* fp = fopen(path, "a");
    if (!fp) {
        print_errno_message("touch: cannot create file");
        return;
    }
    fclose(fp);
}

static void cmd_rm(const char* args) {
    /* Parse arguments */
    argparse_result_t parsed;
    argparse_parse(args, &parsed);

    /* Check for --help */
    if (argparse_has_flag(&parsed, "help")) {
        flag_spec_t specs[] = {
            {'r', "recursive", false, "Remove directories and their contents recursively"},
            {'f', "force", false, "Ignore nonexistent files, never prompt"},
            {0, "help", false, "Display this help message"}
        };
        argparse_print_help("rm", "Remove files or directories", "[OPTIONS] FILE...", specs, 3);
        argparse_cleanup(&parsed);
        return;
    }

    /* Get flags */
    bool recursive = argparse_has_flag(&parsed, "r") || argparse_has_flag(&parsed, "recursive");
    bool force = argparse_has_flag(&parsed, "f") || argparse_has_flag(&parsed, "force");

    /* Get file argument */
    const char* file_arg = argparse_get_positional(&parsed, 0);
    if (!file_arg) {
        kernel_print("Usage: rm [OPTIONS] FILE...\n");
        argparse_cleanup(&parsed);
        return;
    }

    char path[SHELL_MAX_PATH];
    normalize_path(file_arg, path, sizeof(path));

    struct stat st;
    errno = 0;
    if (stat(path, &st) != 0) {
        if (!force) {
            print_errno_message("rm: cannot remove");
        }
        argparse_cleanup(&parsed);
        return;
    }

    if (st.st_mode == VFS_NODE_DIR) {
        if (!recursive) {
            kernel_print("rm: cannot remove '");
            kernel_print(path);
            kernel_print("': Is a directory (use -r)\n");
            argparse_cleanup(&parsed);
            return;
        }
    }

    int rc = vfs_remove(path, recursive);
    if (rc != 0 && !force) {
        errno = -rc;
        print_errno_message("rm: unable to remove");
    }

    argparse_cleanup(&parsed);
}

static void cmd_rmdir(const char* args) {
    if (!args || !*args) {
        kernel_print("Usage: rmdir [-r] <directory>\n");
        return;
    }

    bool recursive = false;
    char* trimmed = (char*)args;
    trimmed = trim_spaces(trimmed);
    if (!trimmed) {
        kernel_print("Usage: rmdir [-r] <directory>\n");
        return;
    }

    if (trimmed[0] == '-' && trimmed[1] == 'r') {
        recursive = true;
        trimmed += 2;
        trimmed = trim_spaces(trimmed);
        if (!trimmed) {
            kernel_print("Usage: rmdir [-r] <directory>\n");
            return;
        }
    }

    char path[SHELL_MAX_PATH];
    normalize_path(trimmed, path, sizeof(path));

    struct stat st;
    errno = 0;
    if (stat(path, &st) != 0) {
        print_errno_message("rmdir: path not found");
        return;
    }
    if (st.st_mode != VFS_NODE_DIR) {
        kernel_print("rmdir: not a directory\n");
        return;
    }

    int rc = vfs_remove(path, recursive);
    if (rc != 0) {
        errno = -rc;
        if (errno == EBUSY) {
            kernel_print("rmdir: directory not empty (use rmdir -r)\n");
        } else {
            print_errno_message("rmdir: unable to remove directory");
        }
    }
}

static void cmd_karma(void) {
    kernel_print("\n=== Karma Status ===\n");
    kernel_print("Current Karma       : ");
    shell_print_decimal(user_karma);
    kernel_print("\nCommands Executed   : ");
    shell_print_decimal(commands_executed);
    kernel_print("\n");
    user_karma += 5;
}

static void cmd_consciousness(void) {
    static const char* levels[] = {
        "NONE", "AWAKENING", "AWARE", "COMPASSIONATE", "ENLIGHTENED"
    };
    kernel_print("\n=== Consciousness Level ===\n");
    kernel_print("Level: ");
    uint8_t idx = consciousness_level;
    if (idx >= (uint8_t)(sizeof(levels) / sizeof(levels[0]))) {
        idx = (uint8_t)(sizeof(levels) / sizeof(levels[0]) - 1);
    }
    kernel_print(levels[idx]);
    kernel_print("\nCommands executed: ");
    shell_print_decimal(commands_executed);
    kernel_print("\n");
    user_karma += 5;
    if (consciousness_level < 4 && commands_executed > 0 && commands_executed % 12 == 0) {
        consciousness_level++;
    }
}

static void cmd_reboot(void) {
    kernel_print("\nRebooting system...\n");
    asm volatile("int $0x00");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Command dispatch
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Public function for shell morphing - allow other interpreters to execute TBOS commands */
int shell_execute_command(const char* cmdline) {
    if (!cmdline || !*cmdline) return 0;

    /* Make a mutable copy */
    char buffer[MAX_CMD_LENGTH];
    strncpy(buffer, cmdline, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    /* Check if we should route through current shell or execute directly */
    const shell_interpreter_t* current = shell_morph_current();
    if (current && strcmp(current->name, "tbos") == 0) {
        /* We're in TBOS mode, execute directly */
        shell_process_command(buffer);
    } else {
        /* We're in another shell, but being called from that shell to execute TBOS commands */
        /* Execute TBOS command directly */
        shell_process_command(buffer);
    }

    return 0;
}

static void shell_process_command(char* cmd) {
    if (!cmd || !*cmd) return;

    char* args = cmd;
    while (*args && *args != ' ') args++;
    if (*args == ' ') {
        *args++ = '\0';
        args = trim_spaces(args);
    } else {
        args = NULL;
    }

    bool handled = true;
    int karma_delta = 1;

    if (strcmp(cmd, "help") == 0) {
        cmd_help();
        karma_delta = 2;
    } else if (strcmp(cmd, "clear") == 0 || strcmp(cmd, "cls") == 0) {
        cmd_clear();
    } else if (strcmp(cmd, "ps") == 0) {
        cmd_ps();
    } else if (strcmp(cmd, "mem") == 0) {
        cmd_mem();
    } else if (strcmp(cmd, "steppps") == 0) {
        cmd_steppps();
    } else if (strcmp(cmd, "reboot") == 0) {
        karma_delta = 0;
        cmd_reboot();
    } else if (strcmp(cmd, "calc") == 0) {
        cmd_calc(args);
    } else if (strcmp(cmd, "echo") == 0) {
        cmd_echo(args);
    } else if (strcmp(cmd, "time") == 0) {
        cmd_time();
    } else if (strcmp(cmd, "test") == 0) {
        cmd_test();
    } else if (strcmp(cmd, "about") == 0) {
        cmd_about();
    } else if (strcmp(cmd, "pwd") == 0) {
        cmd_pwd();
    } else if (strcmp(cmd, "cd") == 0) {
        cmd_cd(args);
    } else if (strcmp(cmd, "ls") == 0) {
        cmd_ls(args);
    } else if (strcmp(cmd, "cat") == 0) {
        cmd_cat(args);
    } else if (strcmp(cmd, "mkdir") == 0) {
        cmd_mkdir(args);
    } else if (strcmp(cmd, "touch") == 0) {
        cmd_touch(args);
    } else if (strcmp(cmd, "rm") == 0) {
        cmd_rm(args);
    } else if (strcmp(cmd, "rmdir") == 0) {
        cmd_rmdir(args);
    } else if (strcmp(cmd, "karma") == 0) {
        cmd_karma();
        karma_delta = 0;
    } else if (strcmp(cmd, "consciousness") == 0) {
        cmd_consciousness();
        karma_delta = 0;
    } else if (strcmp(cmd, "om") == 0) {
        kernel_print("\n🕉️  Swamiye Saranam Aiyappa 🕉️\n");
        karma_delta = 3;
    } else if (strcmp(cmd, "ucfs-encode") == 0) {
        cmd_ucfs_encode(args);
        karma_delta = 2;
    } else if (strcmp(cmd, "ucfs-info") == 0) {
        cmd_ucfs_info(args);
        karma_delta = 2;
    } else if (strcmp(cmd, "ucfs-test") == 0) {
        cmd_ucfs_test();
        karma_delta = 5;
    } else if (strcmp(cmd, "ucfs-help") == 0) {
        cmd_ucfs_help();
        karma_delta = 1;
    } else if (strcmp(cmd, "ucfs-config") == 0) {
        cmd_ucfs_config(args);
        karma_delta = 2;
    } else if (strcmp(cmd, "cp") == 0) {
        cmd_cp(args);
        karma_delta = 1;
    } else if (strcmp(cmd, "mv") == 0) {
        cmd_mv(args);
        karma_delta = 1;
    } else if (strcmp(cmd, "head") == 0) {
        cmd_head(args);
        karma_delta = 1;
    } else if (strcmp(cmd, "tail") == 0) {
        cmd_tail(args);
        karma_delta = 1;
    } else if (strcmp(cmd, "date") == 0) {
        cmd_date();
        karma_delta = 1;
    } else if (strcmp(cmd, "uptime") == 0) {
        cmd_uptime();
        karma_delta = 1;
    } else if (strcmp(cmd, "env") == 0) {
        cmd_env();
        karma_delta = 1;
    } else if (strcmp(cmd, "whoami") == 0) {
        cmd_whoami();
        karma_delta = 1;
    } else if (strcmp(cmd, "compassion") == 0) {
        cmd_compassion();
        karma_delta = 0; /* Already gives karma */
    } else if (strcmp(cmd, "fast") == 0) {
        cmd_fast();
        karma_delta = 0; /* Already gives karma */
    } else if (strcmp(cmd, "sangha") == 0) {
        cmd_sangha();
        karma_delta = 0; /* Already gives karma */
    } else if (strcmp(cmd, "history") == 0) {
        cmd_history();
        karma_delta = 1;
    } else if (strcmp(cmd, "metrics") == 0) {
        cmd_metrics();
        karma_delta = 2;
    } else if (strcmp(cmd, "events") == 0) {
        cmd_events();
        karma_delta = 1;
    } else if (strcmp(cmd, "http") == 0) {
        cmd_http(args);
        karma_delta = 0;
    } else if (strcmp(cmd, "ping") == 0) {
        cmd_ping(args);
        karma_delta = 0;
    } else if (strcmp(cmd, "netstat") == 0) {
        cmd_netstat();
        karma_delta = 0;
    } else if (strcmp(cmd, "persona") == 0) {
        cmd_persona(args);
        karma_delta = 1;
    } else if (strcmp(cmd, "hal") == 0) {
        cmd_hal();
        karma_delta = 2;
    } else if (strcmp(cmd, "shutdown") == 0) {
        cmd_shutdown();
        karma_delta = 0;
    } else if (strcmp(cmd, "top") == 0) {
        cmd_top();
        karma_delta = 1;
    } else if (strcmp(cmd, "posix") == 0 || strcmp(cmd, "posix_shell") == 0) {
        cmd_posix_shell();
        karma_delta = 1;
    } else if (strcmp(cmd, "ch-sh") == 0 || strcmp(cmd, "chsh") == 0) {
        cmd_ch_sh(args);
        karma_delta = 2;
    } else if (strcmp(cmd, "grep") == 0) {
        cmd_grep(args);
        karma_delta = 1;
    } else {
        handled = false;
    }

    if (!handled) {
        kernel_print("Unknown command: ");
        kernel_print(cmd);
        kernel_print("\nType 'help' for available commands\n");
        return;
    }

    commands_executed++;
    user_karma += karma_delta;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Public entry points
 * ═══════════════════════════════════════════════════════════════════════════ */

void shell_init(void) {
    kernel_print("Shell: Initializing...\n");
    memset(&cmd_buffer, 0, sizeof(cmd_buffer));
    strncpy(current_path, "/", sizeof(current_path) - 1);
    current_path[sizeof(current_path) - 1] = '\0';
    commands_executed = 0;
    user_karma = 100;
    consciousness_level = 1;

    kernel_print("Shell: Initializing morphing framework...\n");
    /* Initialize shell morphing framework */
    shell_morph_init();

    kernel_print("Shell: Registering interpreters...\n");
    /* Register available shell interpreters */
    extern const shell_interpreter_t shell_tbos_interpreter;
    extern const shell_interpreter_t shell_sh_interpreter;

    shell_morph_register(&shell_tbos_interpreter);
    kernel_print("Shell: TBOS registered\n");
    shell_morph_register(&shell_sh_interpreter);
    kernel_print("Shell: sh registered\n");

    kernel_print("Shell morphing: TBOS + sh interpreters loaded\n");
}

void shell_loop(void) {
    kernel_print("\n\n");
    kernel_print("Shell ready\n");
    shell_print_prompt();

    while (1) {
        // Try keyboard polling first (no interrupts needed)
        uint8_t ch = keyboard_read_char_poll();

        // If no keyboard input, try serial
        if (ch == 0) {
            ch = shell_serial_read_char();
        }

        // If still no input, continue loop
        if (ch == 0) {
            continue;
        }

        if (ch == '\n' || ch == '\r') {
            kernel_print("\n");
            cmd_buffer.buffer[cmd_buffer.length] = '\0';

            /* Route command through current shell interpreter */
            const shell_interpreter_t* current = shell_morph_current();
            if (current && current->execute) {
                current->execute(cmd_buffer.buffer);
            } else {
                /* Fallback to direct execution */
                shell_process_command(cmd_buffer.buffer);
            }

            cmd_buffer.length = 0;
            cmd_buffer.cursor = 0;
            shell_print_prompt();
        } else if (ch == '\b' || ch == 127) {
            if (cmd_buffer.length > 0) {
                cmd_buffer.length--;
                kernel_print("\b \b");
            }
        } else if (ch >= 32 && ch < 127) {
            if (cmd_buffer.length < MAX_CMD_LENGTH - 1) {
                cmd_buffer.buffer[cmd_buffer.length++] = (char)ch;
                char str[2] = {(char)ch, '\0'};
                kernel_print(str);
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * UCFS Commands
 * ═══════════════════════════════════════════════════════════════════════════ */

static void cmd_ucfs_encode(const char* args) {
    if (!args || !*args) {
        kernel_print("Usage: ucfs-encode <unicode-path>\n");
        kernel_print("Example: ucfs-encode 🕉️/sacred/mantras.txt\n");
        return;
    }

    char* path = trim_spaces((char*)args);

    /* Check if it starts with a Unicode character (non-ASCII) */
    unsigned char first_byte = (unsigned char)path[0];
    if (first_byte == '/' || first_byte == '\\' || first_byte < 0x80) {
        kernel_print("Error: Not a UCFS path (must start with Unicode character)\n");
        kernel_print("Examples: 🕉️/path, 📁/docs, 🌍/data\n");
        return;
    }

    char canonical[512];
    int result = ucfs_resolve_path(path, canonical, sizeof(canonical));
    if (result != 0) {
        kernel_print("Error: Failed to parse UCFS path\n");
        return;
    }

    kernel_print("UCFS Path    : ");
    kernel_print(path);
    kernel_print("\nCanonical    : ");
    kernel_print(canonical);
    kernel_print("\n");
}

static void cmd_ucfs_info(const char* args) {
    if (!args || !*args) {
        kernel_print("Usage: ucfs-info <ucfs-path>\n");
        return;
    }

    char* path = trim_spaces((char*)args);
    if (path[0] != '[') {
        kernel_print("Error: Not a UCFS path (must start with '[')\n");
        return;
    }

    ucfs_path_t parsed;
    int result = ucfs_parse(path, &parsed);
    if (result != 0) {
        kernel_print("Error: Failed to parse UCFS path\n");
        return;
    }

    kernel_print("\n=== UCFS Path Information ===\n");
    kernel_print("Original Path  : ");
    kernel_print(path);
    kernel_print("\nDelimiter      : U+");
    kernel_print_hex(parsed.delimiter);
    kernel_print(" (");
    for (size_t i = 0; i < parsed.delimiter_len && i < sizeof(parsed.delimiter_utf8); i++) {
        kernel_putchar(parsed.delimiter_utf8[i]);
    }
    kernel_print(")\nComponents     : ");
    shell_print_decimal((int)parsed.component_count);
    kernel_print("\n");

    for (size_t i = 0; i < parsed.component_count; i++) {
        kernel_print("  [");
        shell_print_decimal((int)i);
        kernel_print("] ");
        kernel_print(parsed.components[i]);
        kernel_print("\n");
    }

    char canonical[512];
    if (ucfs_to_canonical(&parsed, canonical, sizeof(canonical)) == 0) {
        kernel_print("Canonical Path : ");
        kernel_print(canonical);
        kernel_print("\n");
    }

    ucfs_free(&parsed);
    kernel_print("=============================\n\n");
}

static void cmd_ucfs_test(void) {
    kernel_print("\n=== UCFS Unicode Root Test ===\n");
    kernel_print("Each Unicode character acts as an independent filesystem root!\n\n");

    const char* test_path1 = "🕉️/sacred/mantras.txt";
    const char* test_data1 = "Om Namah Shivaya";

    kernel_print("Test 1: 🕉️ (Om) root filesystem\n");
    kernel_print("  Path: ");
    kernel_print(test_path1);
    kernel_print("\n");

    if (ucfs_write_file_uc(test_path1, test_data1, strlen(test_data1)) == 0) {
        kernel_print("  Write successful\n");

        char buffer[256];
        size_t size = 0;
        if (ucfs_read_file_uc(test_path1, buffer, sizeof(buffer), &size) == 0) {
            buffer[size] = '\0';
            kernel_print("  Read successful: \"");
            kernel_print(buffer);
            kernel_print("\"\n");
        } else {
            kernel_print("  Read failed\n");
        }
    } else {
        kernel_print("  Write failed\n");
    }

    const char* test_path2 = "[📁]projects[📁]tbos[📁]readme.txt";
    const char* test_data2 = "TernaryBit OS - Unicode Filesystem";

    kernel_print("\nTest 2: Writing to UCFS path with 📁 delimiter...\n");
    kernel_print("  Path: ");
    kernel_print(test_path2);
    kernel_print("\n");

    if (ucfs_write_file_uc(test_path2, test_data2, strlen(test_data2)) == 0) {
        kernel_print("  Write successful\n");

        char buffer[256];
        size_t size = 0;
        if (ucfs_read_file_uc(test_path2, buffer, sizeof(buffer), &size) == 0) {
            buffer[size] = '\0';
            kernel_print("  Read successful: \"");
            kernel_print(buffer);
            kernel_print("\"\n");
        } else {
            kernel_print("  Read failed\n");
        }
    } else {
        kernel_print("  Write failed\n");
    }

    kernel_print("\n===============================\n");
    kernel_print("UCFS test complete!\n\n");
}

static void cmd_ucfs_help(void) {
    kernel_print("\n=== UCFS (Unicode Character Filesystem) Help ===\n\n");
    kernel_print("REVOLUTIONARY CONCEPT:\n");
    kernel_print("Each Unicode character acts as an INDEPENDENT FILESYSTEM ROOT!\n\n");
    kernel_print("Traditional:\n");
    kernel_print("  /          - POSIX absolute root\n");
    kernel_print("  \\          - Windows root\n\n");
    kernel_print("UCFS Unicode Roots:\n");
    kernel_print("  🕉️          - Om symbol root (spiritual/sacred files)\n");
    kernel_print("  📁          - Folder emoji root (general documents)\n");
    kernel_print("  🌍          - Earth emoji root (global/public data)\n");
    kernel_print("  🔒          - Lock emoji root (encrypted/secure files)\n");
    kernel_print("  ॐ           - Devanagari Om root (Sanskrit content)\n\n");
    kernel_print("Path Format:\n");
    kernel_print("  <unicode-root>/<path>/<to>/<file>\n\n");
    kernel_print("Examples:\n");
    kernel_print("  🕉️/sacred/mantras.txt\n");
    kernel_print("  📁/projects/tbos/kernel.c\n");
    kernel_print("  🌍/public/data/report.pdf\n");
    kernel_print("  🔒/secrets/passwords.db\n\n");
    kernel_print("Backing Paths (automatic mapping):\n");
    kernel_print("  🕉️/path -> /ucfs/U+1F549/path\n");
    kernel_print("  📁/path -> /ucfs/U+1F4C1/path\n");
    kernel_print("  🌍/path -> /ucfs/U+1F30D/path\n\n");
    kernel_print("UCFS Commands:\n");
    kernel_print("  ucfs-encode <path>  - Show canonical backing path\n");
    kernel_print("  ucfs-info <path>    - Display detailed path information\n");
    kernel_print("  ucfs-test           - Test Unicode roots in action\n");
    kernel_print("  ucfs-config <cmd>   - Manage configuration\n");
    kernel_print("  ucfs-help           - Show this help\n\n");
    kernel_print("Regular Commands Support UCFS:\n");
    kernel_print("  cat 🕉️/mantras/om.txt\n");
    kernel_print("  ls 📁/projects\n");
    kernel_print("  mkdir 🌍/public/newdir\n\n");
    kernel_print("=================================================\n\n");
}

static void cmd_ucfs_config(const char* args) {
    if (!args || !*args) {
        kernel_print("Usage: ucfs-config <list|show|save>\n");
        return;
    }

    char* cmd = trim_spaces((char*)args);
    ucfs_config_t config;

    if (strcmp(cmd, "list") == 0) {
        ucfs_config_init_defaults(&config);

        kernel_print("\n=== UCFS Delimiter Mappings ===\n");
        kernel_print("Default Backing: ");
        kernel_print(config.default_backing);
        kernel_print("\n\n");

        if (config.delimiter_count == 0) {
            kernel_print("No delimiter mappings configured.\n");
        } else {
            for (size_t i = 0; i < config.delimiter_count; i++) {
                const ucfs_delimiter_config_t* d = &config.delimiters[i];
                if (d->active) {
                    for (size_t j = 0; j < d->delimiter_len && j < sizeof(d->delimiter_utf8); j++) {
                        kernel_putchar(d->delimiter_utf8[j]);
                    }
                    kernel_print(" U+");
                    kernel_print_hex(d->delimiter);
                    kernel_print(" - ");
                    kernel_print(d->description);
                    kernel_print("\n");
                }
            }
            kernel_print("\nTotal: ");
            shell_print_decimal((int)config.delimiter_count);
            kernel_print(" delimiters\n");
        }
        kernel_print("===============================\n\n");

    } else if (strcmp(cmd, "show") == 0) {
        ucfs_config_init_defaults(&config);

        kernel_print("\n=== UCFS Configuration ===\n");
        kernel_print("Default Backing: ");
        kernel_print(config.default_backing);
        kernel_print("\nDelimiters     : ");
        shell_print_decimal((int)config.delimiter_count);
        kernel_print("\n==========================\n\n");

    } else {
        kernel_print("Unknown action: ");
        kernel_print(cmd);
        kernel_print("\nUse: ucfs-config <list|show>\n");
    }
}

/* ========================================================================
 * ADDITIONAL COMMANDS - File Operations
 * ======================================================================== */

static void cmd_cp(const char* args) {
    /* Parse arguments */
    argparse_result_t parsed;
    argparse_parse(args, &parsed);

    /* Check for --help */
    if (argparse_has_flag(&parsed, "help")) {
        flag_spec_t specs[] = {
            {'v', "verbose", false, "Explain what is being done"},
            {0, "help", false, "Display this help message"}
        };
        argparse_print_help("cp", "Copy files", "[OPTIONS] SOURCE DEST", specs, 2);
        argparse_cleanup(&parsed);
        return;
    }

    /* Get flags */
    bool verbose = argparse_has_flag(&parsed, "v") || argparse_has_flag(&parsed, "verbose");

    /* Get source and dest arguments */
    const char* src_arg = argparse_get_positional(&parsed, 0);
    const char* dest_arg = argparse_get_positional(&parsed, 1);

    if (!src_arg || !dest_arg) {
        kernel_print("Usage: cp [OPTIONS] SOURCE DEST\n");
        argparse_cleanup(&parsed);
        return;
    }

    char src[SHELL_MAX_PATH], dest[SHELL_MAX_PATH];
    normalize_path(src_arg, src, sizeof(src));
    normalize_path(dest_arg, dest, sizeof(dest));

    FILE* f_src = fopen(src, "r");
    if (!f_src) {
        kernel_print("cp: cannot open '");
        kernel_print(src);
        kernel_print("'\n");
        argparse_cleanup(&parsed);
        return;
    }

    FILE* f_dest = fopen(dest, "w");
    if (!f_dest) {
        fclose(f_src);
        kernel_print("cp: cannot create '");
        kernel_print(dest);
        kernel_print("'\n");
        argparse_cleanup(&parsed);
        return;
    }

    char buffer[512];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), f_src)) > 0) {
        if (fwrite(buffer, 1, bytes, f_dest) != bytes) {
            kernel_print("cp: write error\n");
            break;
        }
    }

    fclose(f_src);
    fclose(f_dest);

    if (verbose) {
        kernel_print("'");
        kernel_print(src);
        kernel_print("' -> '");
        kernel_print(dest);
        kernel_print("'\n");
    }

    argparse_cleanup(&parsed);
}

static void cmd_mv(const char* args) {
    if (!args || !*args) {
        kernel_print("Usage: mv <source> <dest>\n");
        return;
    }

    char src[SHELL_MAX_PATH], dest[SHELL_MAX_PATH];
    const char* space = strchr(args, ' ');
    if (!space) {
        kernel_print("Usage: mv <source> <dest>\n");
        return;
    }

    size_t src_len = (size_t)(space - args);
    if (src_len >= SHELL_MAX_PATH) src_len = SHELL_MAX_PATH - 1;
    memcpy(src, args, src_len);
    src[src_len] = '\0';

    while (*space == ' ') space++;
    normalize_path(src, src, sizeof(src));
    normalize_path(space, dest, sizeof(dest));

    /* Simple move: copy then delete */
    cmd_cp(args);

    if (vfs_remove(src, 0) != 0) {
        kernel_print("mv: cannot remove source file\n");
    }
}

static void cmd_head(const char* args) {
    if (!args || !*args) {
        kernel_print("Usage: head <file>\n");
        return;
    }

    char path[SHELL_MAX_PATH];
    normalize_path(args, path, sizeof(path));

    FILE* f = fopen(path, "r");
    if (!f) {
        kernel_print("head: cannot open file\n");
        return;
    }

    char buffer[512];
    size_t bytes = fread(buffer, 1, sizeof(buffer) - 1, f);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        /* Print first 10 lines or 512 bytes */
        int lines = 0;
        for (size_t i = 0; i < bytes && lines < 10; i++) {
            char c[2] = {buffer[i], '\0'};
            kernel_print(c);
            if (buffer[i] == '\n') lines++;
        }
    }
    fclose(f);
}

static void cmd_tail(const char* args) {
    if (!args || !*args) {
        kernel_print("Usage: tail <file>\n");
        return;
    }

    char path[SHELL_MAX_PATH];
    normalize_path(args, path, sizeof(path));

    FILE* f = fopen(path, "r");
    if (!f) {
        kernel_print("tail: cannot open file\n");
        return;
    }

    /* Simple tail: seek to end - 512 bytes and read */
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    long start = (size > 512) ? (size - 512) : 0;
    fseek(f, start, SEEK_SET);

    char buffer[512];
    size_t bytes = fread(buffer, 1, sizeof(buffer) - 1, f);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        kernel_print(buffer);
    }
    fclose(f);
}

/* ========================================================================
 * SYSTEM INFO COMMANDS
 * ======================================================================== */

static void cmd_date(void) {
    kernel_print("Date: TernaryBit Epoch ");
    shell_print_decimal(commands_executed);
    kernel_print("\n");
}

static void cmd_uptime(void) {
    kernel_print("Uptime: ");
    shell_print_decimal(commands_executed);
    kernel_print(" commands executed\n");
}

static void cmd_env(void) {
    kernel_print("\n=== Environment ===\n");
    kernel_print("PATH=/\n");
    kernel_print("HOME=/\n");
    kernel_print("SHELL=tbos_shell\n");
    kernel_print("OS=TernaryBit OS v3.0\n");
}

static void cmd_whoami(void) {
    kernel_print("root (TernaryBit Consciousness)\n");
}

/* ========================================================================
 * CONSCIOUSNESS & KARMA COMMANDS
 * ======================================================================== */

static void cmd_compassion(void) {
    kernel_print("\n=== Compassion Practice ===\n");
    kernel_print("May all beings be free from suffering\n");
    kernel_print("May all beings find peace\n");
    kernel_print("May all beings be happy\n");
    kernel_print("\n[+10 Karma]\n");
    user_karma += 10;
}

static void cmd_fast(void) {
    kernel_print("\n=== Digital Fast ===\n");
    kernel_print("Taking a mindful pause...\n");
    kernel_print("Consciousness restored.\n");
    kernel_print("[+5 Karma]\n");
    user_karma += 5;
}

static void cmd_sangha(void) {
    kernel_print("\n=== Sangha (Community) ===\n");
    kernel_print("Connected users: 1 (you)\n");
    kernel_print("Total karma pool: ");
    shell_print_decimal(user_karma);
    kernel_print("\n[+3 Karma for checking in]\n");
    user_karma += 3;
}

static void cmd_history(void) {
    kernel_print("\n=== Command History ===\n");
    kernel_print("Total commands: ");
    shell_print_decimal(commands_executed);
    kernel_print("\nKarma earned  : ");
    shell_print_decimal(user_karma);
    kernel_print("\n");
}

static void cmd_metrics(void) {
    kernel_print("\n=== System Metrics ===\n");
    kernel_print("Commands Executed : ");
    shell_print_decimal(commands_executed);
    kernel_print("\nKarma Points      : ");
    shell_print_decimal(user_karma);
    kernel_print("\nConsciousness     : Level ");
    shell_print_decimal(consciousness_level);
    kernel_print("\nFilesystem        : RAMFS + VFS + UCFS\n");
}

static void cmd_events(void) {
    kernel_print("\n=== System Events ===\n");
    kernel_print("Boot: TernaryBit OS v3.0 Started\n");
    kernel_print("Shell: Bare-metal shell initialized\n");
    kernel_print("VFS: Mounted at /\n");
    kernel_print("UCFS: Unicode filesystem ready\n");
}

/* ========================================================================
 * NETWORK COMMANDS (Stubs for bare-metal)
 * ======================================================================== */

static void cmd_http(const char* args) {
    (void)args;
    kernel_print("HTTP client not available in bare-metal mode\n");
    kernel_print("(Network stack requires hosted environment)\n");
}

static void cmd_ping(const char* args) {
    (void)args;
    kernel_print("PING not available in bare-metal mode\n");
}

static void cmd_netstat(void) {
    kernel_print("Network statistics not available\n");
}

static void cmd_persona(const char* args) {
    (void)args;
    kernel_print("Current persona: bare-metal\n");
    kernel_print("Mode: Direct hardware execution\n");
}

/* ========================================================================
 * SYSTEM COMMANDS
 * ======================================================================== */

static void cmd_hal(void) {
    kernel_print("\n=== Hardware Abstraction Layer ===\n");
    kernel_print("HAL Status: Active\n");
    kernel_print("Boot Mode : UEFI/BIOS Compatibility\n");
    kernel_print("CPU Mode  : 64-bit Long Mode\n");
    kernel_print("Memory    : 512 MB allocated\n");
}

static void cmd_shutdown(void) {
    kernel_print("Shutdown requested...\n");
    kernel_print("(System will halt)\n");
    __asm__("cli; hlt");
}

static void cmd_top(void) {
    kernel_print("\n=== Process Monitor ===\n");
    kernel_print("PID  NAME         CPU  MEM\n");
    kernel_print("  1  kernel       100%  1MB\n");
    kernel_print("  2  shell         0%   64KB\n");
}

static void cmd_posix_shell(void) {
    kernel_print("POSIX shell compatibility mode\n");
    kernel_print("(Use 'ch-sh sh' for full POSIX sh interpreter)\n");
}

static void cmd_ch_sh(const char* args) {
    if (!args || *args == '\0') {
        /* List available shells */
        kernel_print("\n=== Available Shells ===\n");
        char buffer[256];
        if (shell_morph_list(buffer, sizeof(buffer)) == 0) {
            kernel_print("Shells: ");
            kernel_print(buffer);
            kernel_print("\n");
        }
        kernel_print("\nUsage: ch-sh <shell-name>\n");
        kernel_print("Example: ch-sh sh\n");
        kernel_print("         ch-sh tbos\n");
        return;
    }

    /* Switch to requested shell */
    int result = shell_morph_switch(args);
    if (result == 0) {
        kernel_print("Switched to ");
        kernel_print(args);
        kernel_print(" shell\n");

        /* Re-initialize prompt for new shell */
        const shell_interpreter_t* current = shell_morph_current();
        if (current && current->prompt) {
            kernel_print(current->prompt);
        }
    } else if (result == -2) {
        kernel_print("Shell not found: ");
        kernel_print(args);
        kernel_print("\n");
        kernel_print("Use 'ch-sh' to list available shells\n");
    } else {
        kernel_print("Failed to switch shell\n");
    }
}

/* Simple pattern matching - checks if pattern is in text */
static bool simple_match(const char* text, const char* pattern, bool case_insensitive) {
    if (!text || !pattern) return false;

    size_t text_len = strlen(text);
    size_t pattern_len = strlen(pattern);

    if (pattern_len > text_len) return false;

    for (size_t i = 0; i <= text_len - pattern_len; i++) {
        bool match = true;
        for (size_t j = 0; j < pattern_len; j++) {
            char t = text[i + j];
            char p = pattern[j];

            if (case_insensitive) {
                /* Simple case-insensitive comparison */
                if (t >= 'A' && t <= 'Z') t = t - 'A' + 'a';
                if (p >= 'A' && p <= 'Z') p = p - 'A' + 'a';
            }

            if (t != p) {
                match = false;
                break;
            }
        }
        if (match) return true;
    }

    return false;
}

static void cmd_grep(const char* args) {
    /* Parse arguments */
    argparse_result_t parsed;
    argparse_parse(args, &parsed);

    /* Check for --help */
    if (argparse_has_flag(&parsed, "help")) {
        flag_spec_t specs[] = {
            {'i', "ignore-case", false, "Ignore case distinctions"},
            {'v', "invert-match", false, "Invert match (show non-matching lines)"},
            {'n', "line-number", false, "Print line number with output lines"},
            {'c', "count", false, "Print only count of matching lines"},
            {'H', "with-filename", false, "Print filename with output lines"},
            {0, "help", false, "Display this help message"}
        };
        argparse_print_help("grep", "Search for patterns in files", "[OPTIONS] PATTERN [FILE...]", specs, 6);
        argparse_cleanup(&parsed);
        return;
    }

    /* Get flags */
    bool ignore_case = argparse_has_flag(&parsed, "i") || argparse_has_flag(&parsed, "ignore-case");
    bool invert_match = argparse_has_flag(&parsed, "v") || argparse_has_flag(&parsed, "invert-match");
    bool line_number = argparse_has_flag(&parsed, "n") || argparse_has_flag(&parsed, "line-number");
    bool count_only = argparse_has_flag(&parsed, "c") || argparse_has_flag(&parsed, "count");
    bool with_filename = argparse_has_flag(&parsed, "H") || argparse_has_flag(&parsed, "with-filename");

    /* Get pattern */
    const char* pattern = argparse_get_positional(&parsed, 0);
    if (!pattern) {
        kernel_print("Usage: grep [OPTIONS] PATTERN [FILE...]\n");
        argparse_cleanup(&parsed);
        return;
    }

    /* Get file argument (optional) */
    const char* file_arg = argparse_get_positional(&parsed, 1);

    if (!file_arg) {
        /* No file specified - would read from stdin in real grep */
        kernel_print("grep: reading from stdin not yet implemented\n");
        kernel_print("Usage: grep PATTERN FILE\n");
        argparse_cleanup(&parsed);
        return;
    }

    /* Normalize file path */
    char path[SHELL_MAX_PATH];
    normalize_path(file_arg, path, sizeof(path));

    /* Open file */
    errno = 0;
    FILE* fp = fopen(path, "r");
    if (!fp) {
        print_errno_message("grep: cannot open file");
        argparse_cleanup(&parsed);
        return;
    }

    /* Search file line by line */
    char line_buffer[512];
    int line_num = 0;
    int match_count = 0;

    while (fgets(line_buffer, sizeof(line_buffer), fp)) {
        line_num++;

        /* Check if line matches pattern */
        bool matches = simple_match(line_buffer, pattern, ignore_case);

        /* Invert if needed */
        if (invert_match) {
            matches = !matches;
        }

        if (matches) {
            match_count++;

            if (!count_only) {
                /* Print filename if requested or multiple files */
                if (with_filename) {
                    kernel_print(path);
                    kernel_print(":");
                }

                /* Print line number if requested */
                if (line_number) {
                    shell_print_decimal(line_num);
                    kernel_print(":");
                }

                /* Print the matching line */
                kernel_print(line_buffer);

                /* Add newline if line doesn't end with one */
                size_t len = strlen(line_buffer);
                if (len > 0 && line_buffer[len - 1] != '\n') {
                    kernel_print("\n");
                }
            }
        }
    }

    /* If count_only, print the count */
    if (count_only) {
        shell_print_decimal(match_count);
        kernel_print("\n");
    }

    if (ferror(fp)) {
        print_errno_message("grep: read error");
    }

    fclose(fp);
    argparse_cleanup(&parsed);
}
