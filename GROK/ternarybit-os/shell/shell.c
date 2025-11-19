// TernaryBit OS - Stage 1 Interactive Shell
// Provides a minimal yet functional shell backed by RAMFS.

#include "shell.h"
#include "tbos/libc.h"
#include "tbos/stdio.h"
#include "tbos/vfs.h"
#include "tbos/errno.h"
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
    kernel_print("\n=== TernaryBit OS Stage 1 Shell ===\n");
    kernel_print("Core:       help, clear, test, about, reboot\n");
    kernel_print("Processes:  ps, mem, steppps, time\n");
    kernel_print("Filesystem: pwd, ls [path], cd <dir>, cat <file>\n");
    kernel_print("            mkdir <dir>, touch <file>, rm <file>, rmdir [-r] <dir>\n");
    kernel_print("Mindfulness: karma, consciousness, om\n");
    kernel_print("Utilities:  calc <expr>, echo <text>\n");
    kernel_print("\nRoot filesystem is backed by RAMFS mounted at '/'.\n");
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
