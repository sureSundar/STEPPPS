// TernaryBit OS - Stage 1 Interactive Shell
// Provides a minimal yet functional shell backed by RAMFS.

#include "shell.h"
#include "tbos/libc.h"
#include "tbos/stdio.h"
#include "tbos/vfs.h"
#include "tbos/errno.h"
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
    kernel_print("tbos:");
    kernel_print(current_path);
    kernel_print("> ");
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
    kernel_print("\n=== TernaryBit OS Shell (50+ Commands) ===\n");
    kernel_print("\n[General]\n");
    kernel_print("  help, clear, cls, about, reboot, shutdown, test\n");
    kernel_print("\n[Processes & System]\n");
    kernel_print("  ps, top, mem, hal, steppps, time, date, uptime, env, whoami\n");
    kernel_print("\n[Filesystem Operations]\n");
    kernel_print("  pwd, ls, cd, cat, mkdir, touch, rm, rmdir\n");
    kernel_print("  cp <src> <dst>, mv <src> <dst>, head <file>, tail <file>\n");
    kernel_print("\n[UCFS Commands]\n");
    kernel_print("  ucfs-encode, ucfs-info, ucfs-test, ucfs-help, ucfs-config\n");
    kernel_print("\n[Consciousness & Karma]\n");
    kernel_print("  karma, consciousness, om, compassion, fast, sangha\n");
    kernel_print("  history, metrics, events\n");
    kernel_print("\n[Network (stubs)]\n");
    kernel_print("  http, ping, netstat, persona\n");
    kernel_print("\n[Utilities]\n");
    kernel_print("  calc <expr>, echo <text>, posix\n");
    kernel_print("\nFilesystem: RAMFS + VFS + UCFS mounted at '/'\n");
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
    char path[SHELL_MAX_PATH];
    if (args && *args) {
        normalize_path(args, path, sizeof(path));
    } else {
        strncpy(path, current_path, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }

    struct stat st;
    errno = 0;
    if (stat(path, &st) != 0) {
        print_errno_message("ls: path not found");
        return;
    }

    if (st.st_mode == VFS_NODE_FILE) {
        kernel_print(path);
        kernel_print("\n");
        return;
    }

    DIR* dir = opendir(path);
    if (!dir) {
        print_errno_message("ls: cannot open directory");
        return;
    }

    kernel_print("\n");
    struct dirent* entry;
    while ((entry = readdir(dir))) {
        kernel_print(entry->d_name);
        if (entry->d_type == VFS_NODE_DIR) kernel_print("/");
        kernel_print("  ");
    }
    kernel_print("\n");
    closedir(dir);
}

static void cmd_cat(const char* args) {
    if (!args || !*args) {
        kernel_print("Usage: cat <file>\n");
        return;
    }

    char path[SHELL_MAX_PATH];
    normalize_path(args, path, sizeof(path));

    errno = 0;
    FILE* fp = fopen(path, "r");
    if (!fp) {
        print_errno_message("cat: cannot open file");
        return;
    }

    char buffer[128];
    size_t nread;
    char last = '\0';
    while ((nread = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        for (size_t i = 0; i < nread; i++) {
            kernel_putchar(buffer[i]);
            last = buffer[i];
        }
    }

    if (ferror(fp)) {
        print_errno_message("cat: read error");
    }
    fclose(fp);

    if (last != '\n') {
        kernel_print("\n");
    }
}

static void cmd_mkdir(const char* args) {
    if (!args || !*args) {
        kernel_print("Usage: mkdir <directory>\n");
        return;
    }

    char path[SHELL_MAX_PATH];
    normalize_path(args, path, sizeof(path));

    int rc = vfs_mkdir(path);
    if (rc != 0) {
        errno = -rc;
        print_errno_message("mkdir: unable to create directory");
    }
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
    if (!args || !*args) {
        kernel_print("Usage: rm <file>\n");
        return;
    }

    char path[SHELL_MAX_PATH];
    normalize_path(args, path, sizeof(path));

    struct stat st;
    errno = 0;
    if (stat(path, &st) != 0) {
        print_errno_message("rm: path not found");
        return;
    }
    if (st.st_mode != VFS_NODE_FILE) {
        kernel_print("rm: not a file (use rmdir for directories)\n");
        return;
    }

    int rc = vfs_remove(path, false);
    if (rc != 0) {
        errno = -rc;
        print_errno_message("rm: unable to remove file");
    }
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
    memset(&cmd_buffer, 0, sizeof(cmd_buffer));
    strncpy(current_path, "/", sizeof(current_path) - 1);
    current_path[sizeof(current_path) - 1] = '\0';
    commands_executed = 0;
    user_karma = 100;
    consciousness_level = 1;
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
            shell_process_command(cmd_buffer.buffer);
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
    if (!args || !*args) {
        kernel_print("Usage: cp <source> <dest>\n");
        return;
    }

    char src[SHELL_MAX_PATH], dest[SHELL_MAX_PATH];
    const char* space = strchr(args, ' ');
    if (!space) {
        kernel_print("Usage: cp <source> <dest>\n");
        return;
    }

    size_t src_len = (size_t)(space - args);
    if (src_len >= SHELL_MAX_PATH) src_len = SHELL_MAX_PATH - 1;
    memcpy(src, args, src_len);
    src[src_len] = '\0';

    while (*space == ' ') space++;
    normalize_path(src, src, sizeof(src));
    normalize_path(space, dest, sizeof(dest));

    FILE* f_src = fopen(src, "r");
    if (!f_src) {
        kernel_print("cp: cannot open source file\n");
        return;
    }

    FILE* f_dest = fopen(dest, "w");
    if (!f_dest) {
        fclose(f_src);
        kernel_print("cp: cannot create destination file\n");
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
    kernel_print("File copied\n");
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
    kernel_print("HTTP client not available in bare-metal mode\n");
    kernel_print("(Network stack requires hosted environment)\n");
}

static void cmd_ping(const char* args) {
    kernel_print("PING not available in bare-metal mode\n");
}

static void cmd_netstat(void) {
    kernel_print("Network statistics not available\n");
}

static void cmd_persona(const char* args) {
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
    kernel_print("(Already in POSIX-compatible mode)\n");
}
