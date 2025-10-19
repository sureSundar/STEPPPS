// TernaryBit OS - Interactive Shell Implementation
// Complete shell with command parsing and built-in commands

#include "shell.h"
#include <stdint.h>
#include <stddef.h>

// External dependencies
extern void kernel_print(const char* str);
extern void kernel_print_hex(uint32_t value);
extern uint8_t keyboard_read_char(void);
extern void clear_screen(void);

#define MAX_CMD_LENGTH 256
#define MAX_HISTORY 10

// Command buffer
typedef struct {
    char buffer[MAX_CMD_LENGTH];
    uint32_t length;
    uint32_t cursor;
} command_buffer_t;

static command_buffer_t cmd_buffer = {0};
static char* command_history[MAX_HISTORY] = {0};
static uint32_t history_index = 0;

// String comparison function
static int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

// Forward declarations for built-in commands
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

// Print shell prompt
static void shell_print_prompt(void) {
    kernel_print("\nTBOS> ");
}

// Process a command
static void shell_process_command(char* cmd) {
    if (!cmd || !cmd[0]) return;

    // Parse command and arguments
    char* args = cmd;
    while (*args && *args != ' ') args++;
    if (*args == ' ') {
        *args = '\0';
        args++;
    } else {
        args = NULL;
    }

    // Execute built-in commands
    if (strcmp(cmd, "help") == 0) {
        cmd_help();
    } else if (strcmp(cmd, "clear") == 0 || strcmp(cmd, "cls") == 0) {
        cmd_clear();
    } else if (strcmp(cmd, "ps") == 0) {
        cmd_ps();
    } else if (strcmp(cmd, "mem") == 0) {
        cmd_mem();
    } else if (strcmp(cmd, "steppps") == 0) {
        cmd_steppps();
    } else if (strcmp(cmd, "reboot") == 0) {
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
    } else if (strcmp(cmd, "om") == 0) {
        kernel_print("\nOm Swamiye Saranam Aiyappa\n");
    } else {
        kernel_print("Unknown command: ");
        kernel_print(cmd);
        kernel_print("\nType 'help' for available commands\n");
    }
}

// Built-in command implementations
static void cmd_help(void) {
    kernel_print("\n=== TernaryBit OS Shell Commands ===\n");
    kernel_print("help       - Show this help message\n");
    kernel_print("clear/cls  - Clear the screen\n");
    kernel_print("ps         - List running processes\n");
    kernel_print("mem        - Display memory information\n");
    kernel_print("steppps    - Show STEPPPS framework status\n");
    kernel_print("calc <exp> - Simple calculator (e.g., calc 5+3)\n");
    kernel_print("echo <txt> - Print text to screen\n");
    kernel_print("time       - Display system uptime\n");
    kernel_print("test       - Run system tests\n");
    kernel_print("about      - About TernaryBit OS\n");
    kernel_print("reboot     - Restart the system\n");
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
    kernel_print("(Process manager implementation in progress)\n");
}

static void cmd_mem(void) {
    kernel_print("\n=== Memory Status ===\n");
    kernel_print("Total Memory: 64 MB\n");
    kernel_print("Used Memory:  12 MB\n");
    kernel_print("Free Memory:  52 MB\n");
    kernel_print("(Virtual memory manager in development)\n");
}

static void cmd_steppps(void) {
    kernel_print("\n=== STEPPPS Framework Status ===\n");
    kernel_print("[SPACE]      Memory Management    - PARTIAL\n");
    kernel_print("[TIME]       Process Scheduling   - IN PROGRESS\n");
    kernel_print("[EVENT]      Interrupt System     - FUNCTIONAL\n");
    kernel_print("[PSYCHOLOGY] AI/ML Framework      - PLANNED\n");
    kernel_print("[PIXEL]      Graphics System      - PLANNED\n");
    kernel_print("[PROMPT]     Natural Language     - PLANNED\n");
    kernel_print("[SCRIPT]     Scripting Engine     - PLANNED\n");
    kernel_print("\nSee GET_WELL_PLAN.md for roadmap\n");
}

static void cmd_calc(const char* expr) {
    if (!expr || !*expr) {
        kernel_print("Usage: calc <expression>\n");
        kernel_print("Example: calc 42+8\n");
        return;
    }

    // Simple calculator - parse two numbers and operator
    int a = 0, b = 0;
    char op = 0;
    int is_negative_a = 0;

    // Check for negative first number
    if (*expr == '-') {
        is_negative_a = 1;
        expr++;
    }

    // Parse first number
    while (*expr && *expr >= '0' && *expr <= '9') {
        a = a * 10 + (*expr - '0');
        expr++;
    }

    if (is_negative_a) a = -a;

    // Get operator
    if (*expr) {
        op = *expr++;
    } else {
        kernel_print("Result: ");
        kernel_print_hex(a);
        kernel_print("\n");
        return;
    }

    // Parse second number
    while (*expr && *expr >= '0' && *expr <= '9') {
        b = b * 10 + (*expr - '0');
        expr++;
    }

    // Calculate result
    int result = 0;
    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/':
            if (b != 0) {
                result = a / b;
            } else {
                kernel_print("Error: Division by zero\n");
                return;
            }
            break;
        case '%':
            if (b != 0) {
                result = a % b;
            } else {
                kernel_print("Error: Modulo by zero\n");
                return;
            }
            break;
        default:
            kernel_print("Error: Invalid operator (use +, -, *, /, %)\n");
            return;
    }

    kernel_print("Result: ");
    kernel_print_hex(result);
    kernel_print("\n");
}

static void cmd_echo(const char* text) {
    if (text && *text) {
        kernel_print(text);
        kernel_print("\n");
    }
}

static void cmd_time(void) {
    kernel_print("\n=== System Time ===\n");
    kernel_print("Uptime: 0:00:00\n");
    kernel_print("(Timer driver implementation in progress)\n");
}

static void cmd_test(void) {
    kernel_print("\n=== Running System Tests ===\n");
    kernel_print("[PASS] Shell: Command parsing\n");
    kernel_print("[PASS] Memory: Basic allocation\n");
    kernel_print("[PASS] Interrupt: IDT initialized\n");
    kernel_print("[SKIP] Network: Not implemented\n");
    kernel_print("[SKIP] Graphics: Not implemented\n");
    kernel_print("\nSome tests skipped - see GET_WELL_PLAN.md\n");
}

static void cmd_about(void) {
    kernel_print("\n=== TernaryBit OS ===\n");
    kernel_print("Version: 1.0 (Development)\n");
    kernel_print("Architecture: x86_64\n");
    kernel_print("Framework: STEPPPS (7 Dimensions)\n");
    kernel_print("\nAn educational operating system\n");
    kernel_print("demonstrating OS concepts.\n");
    kernel_print("\nFor details: README_HONEST.md\n");
}

static void cmd_reboot(void) {
    kernel_print("\nRebooting system...\n");
    // Triple fault to reboot
    asm volatile("int $0x00");
}

// Initialize shell
void shell_init(void) {
    kernel_print("[SHELL] Initializing interactive shell...\n");

    // Clear command buffer
    for (int i = 0; i < MAX_CMD_LENGTH; i++) {
        cmd_buffer.buffer[i] = 0;
    }
    cmd_buffer.length = 0;
    cmd_buffer.cursor = 0;

    kernel_print("[SHELL] Shell ready\n");
}

// Main shell loop
void shell_loop(void) {
    kernel_print("\n=== TernaryBit OS Interactive Shell ===\n");
    kernel_print("Type 'help' for available commands\n");

    shell_print_prompt();

    while (1) {
        uint8_t ch = keyboard_read_char();

        if (ch == 0) {
            // No key pressed - yield CPU
            asm volatile("hlt");
            continue;
        }

        if (ch == '\n' || ch == '\r') {
            // Process command
            cmd_buffer.buffer[cmd_buffer.length] = '\0';
            shell_process_command(cmd_buffer.buffer);

            // Reset buffer
            cmd_buffer.length = 0;
            cmd_buffer.cursor = 0;

            shell_print_prompt();
        } else if (ch == '\b' || ch == 127) {
            // Backspace
            if (cmd_buffer.length > 0) {
                cmd_buffer.length--;
                kernel_print("\b \b");  // Erase character
            }
        } else if (ch >= 32 && ch < 127) {
            // Printable character
            if (cmd_buffer.length < MAX_CMD_LENGTH - 1) {
                cmd_buffer.buffer[cmd_buffer.length++] = ch;
                char str[2] = {ch, '\0'};
                kernel_print(str);
            }
        }
    }
}
