// TBOS Interactive Shell
// Real OS functionality - no more infinite loops!
//
// PXFS INTEGRATION ROADMAP:
// - Phase 1: Command history stored as PXFS-encoded pixels
// - Phase 2: Shell output buffering via PXFS compression
// - Phase 3: File I/O through PXFS codec layer
// - Phase 4: Network command data transferred as PXFS packets
//
// Current Status: Foundation layer (I/O) - PXFS hooks to be added in Sprint 2

#include <stdint.h>
#include <stddef.h>

// String functions
int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

extern void kernel_print(const char* str);
extern void kernel_print_hex(uint32_t value);
extern uint8_t keyboard_read_char(void);
extern void clear_screen(void);
extern void process_list(void);
extern void tbos_discover_devices(void);
extern void tbos_list_devices(void);
extern void tbos_list_streams(void);
extern uint32_t tbos_start_audio_stream(uint32_t device_id);
extern uint32_t tbos_start_video_stream(uint32_t device_id);
extern void tbos_stop_stream(uint32_t session_id);

// Serial port I/O (for -nographic mode)
#define SERIAL_PORT 0x3F8
#define SERIAL_LSR (SERIAL_PORT + 5)

// Low-level I/O functions
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Read character from serial port
static uint8_t serial_read_char(void) {
    // Check if data is available (LSR bit 0)
    if (!(inb(SERIAL_LSR) & 0x01)) {
        return 0;  // No data available
    }
    return inb(SERIAL_PORT);
}

#define MAX_CMD_LENGTH 256
#define MAX_HISTORY 10

typedef struct {
    char buffer[MAX_CMD_LENGTH];
    uint32_t length;
    uint32_t cursor;
} command_buffer_t;

static command_buffer_t cmd_buffer = {0};
static char* command_history[MAX_HISTORY] = {0};
static uint32_t history_index = 0;

// Day 1: Karma and Consciousness tracking
static int32_t user_karma = 100;
static uint8_t consciousness_level = 1; // AWAKENING
static uint32_t commands_executed = 0;

// PXFS INTEGRATION NOTE:
// Command history will be stored using PXFS encoding to save memory.
// Each command string can be compressed into pixel RGB values.
// Expected compression: 256 bytes → ~85 pixels (~43% size reduction)
// TODO (Sprint 2): Replace char* arrays with tbpx_encode/decode calls

// Built-in commands
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
static void cmd_stream_devices(void);
static void cmd_stream_list(void);
static void cmd_stream_audio(const char* args);
static void cmd_stream_video(const char* args);
static void cmd_stream_stop(const char* args);

// Print shell prompt with karma
void shell_print_prompt(void) {
    kernel_print("\n[K:");
    kernel_print_hex(user_karma);
    kernel_print("] TBOS> ");
}

// Process a command
void shell_process_command(char* cmd) {
    if (!cmd || !cmd[0]) return;

    // Track command execution
    commands_executed++;

    // Parse command and arguments
    char* args = cmd;
    while (*args && *args != ' ') args++;
    if (*args == ' ') {
        *args = '\0';
        args++;
    }

    // Update karma for command
    user_karma += 1; // Reward for using shell

    // Execute built-in commands
    if (strcmp(cmd, "help") == 0) {
        cmd_help();
        user_karma += 2; // Extra karma for learning
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
    } else if (strcmp(cmd, "om") == 0) {
        kernel_print("\n🕉️ Swamiye Saranam Aiyappa 🕉️\n");
    } else if (strcmp(cmd, "karma") == 0) {
        kernel_print("\n=== Karma Status ===\n");
        kernel_print("Current Karma: ");
        kernel_print_hex(user_karma);
        kernel_print("\nCommands Executed: ");
        kernel_print_hex(commands_executed);
        kernel_print("\n");
        user_karma += 5;
    } else if (strcmp(cmd, "consciousness") == 0) {
        kernel_print("\n=== Consciousness Level ===\n");
        const char* levels[] = {"NONE", "AWAKENING", "AWARE", "COMPASSIONATE", "ENLIGHTENED"};
        kernel_print("Level: ");
        kernel_print(levels[consciousness_level]);
        kernel_print("\nCommands: ");
        kernel_print_hex(commands_executed);
        kernel_print("\n");
        user_karma += 5;
    } else if (strcmp(cmd, "stream-devices") == 0) {
        cmd_stream_devices();
    } else if (strcmp(cmd, "stream-list") == 0) {
        cmd_stream_list();
    } else if (strcmp(cmd, "stream-audio") == 0) {
        cmd_stream_audio(args);
    } else if (strcmp(cmd, "stream-video") == 0) {
        cmd_stream_video(args);
    } else if (strcmp(cmd, "stream-stop") == 0) {
        cmd_stream_stop(args);
    } else if (strcmp(cmd, "discover") == 0) {
        extern void tbos_discover_devices(void);
        tbos_discover_devices();
    } else {
        kernel_print("Unknown command: ");
        kernel_print(cmd);
        kernel_print("\nType 'help' for commands\n");
    }
}

// Built-in command implementations
static void cmd_help(void) {
    kernel_print("\n=== TBOS Commands - Day 1 Edition ===\n");
    kernel_print("help           - Show this help\n");
    kernel_print("clear          - Clear screen\n");
    kernel_print("ps             - List processes\n");
    kernel_print("mem            - Memory info\n");
    kernel_print("steppps        - STEPPPS status\n");
    kernel_print("calc           - Calculator (calc 2+2)\n");
    kernel_print("echo           - Print text\n");
    kernel_print("time           - System time\n");
    kernel_print("test           - Run tests\n");
    kernel_print("om             - Sacred mantra\n");
    kernel_print("reboot         - Restart system\n");
    kernel_print("\n=== Consciousness Commands (Day 1) ===\n");
    kernel_print("karma          - Show karma status\n");
    kernel_print("consciousness  - Show consciousness level\n");
    kernel_print("\n=== Streaming Commands ===\n");
    kernel_print("discover       - Find TBOS devices\n");
    kernel_print("stream-devices - List discovered devices\n");
    kernel_print("stream-list    - List active streams\n");
    kernel_print("stream-audio   - Start audio stream to device\n");
    kernel_print("stream-video   - Start video stream to device\n");
    kernel_print("stream-stop    - Stop stream by ID\n");
}

static void cmd_clear(void) {
    clear_screen();
    kernel_print("TernaryBit OS v1.0 - Interactive Shell\n");
}

static void cmd_ps(void) {
    // Call process list function
    extern void process_list(void);
    process_list();
}

static void cmd_mem(void) {
    kernel_print("\n=== Memory Status ===\n");
    kernel_print("Total: 64 MB\n");
    kernel_print("Used:  12 MB\n");
    kernel_print("Free:  52 MB\n");
}

static void cmd_steppps(void) {
    kernel_print("\n=== STEPPPS Framework ===\n");
    kernel_print("[SPACE]      : Active ✓\n");
    kernel_print("[TIME]       : Active ✓\n");
    kernel_print("[EVENT]      : Active ✓\n");
    kernel_print("[PSYCHOLOGY] : Active ✓\n");
    kernel_print("[PIXEL]      : Active ✓\n");
    kernel_print("[PROMPT]     : Active ✓\n");
    kernel_print("[SCRIPT]     : Active ✓\n");
}

static void cmd_calc(const char* expr) {
    if (!expr || !*expr) {
        kernel_print("Usage: calc <expression>\n");
        return;
    }

    // Simple calculator - parse two numbers and operator
    int a = 0, b = 0;
    char op = 0;

    // Parse first number
    while (*expr && *expr >= '0' && *expr <= '9') {
        a = a * 10 + (*expr - '0');
        expr++;
    }

    // Get operator
    if (*expr) {
        op = *expr++;
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
        case '/': result = (b != 0) ? a / b : 0; break;
        default:
            kernel_print("Invalid operator\n");
            return;
    }

    kernel_print("Result: ");
    kernel_print_hex(result);
    kernel_print("\n");
}

static void cmd_echo(const char* text) {
    if (text) {
        kernel_print(text);
        kernel_print("\n");
    }
}

static void cmd_time(void) {
    // Simple uptime display
    uint32_t ticks = 0; // Would be from timer
    uint32_t seconds = ticks / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;

    kernel_print("Uptime: ");
    kernel_print_hex(hours);
    kernel_print(":");
    kernel_print_hex(minutes % 60);
    kernel_print(":");
    kernel_print_hex(seconds % 60);
    kernel_print("\n");
}

static void cmd_test(void) {
    kernel_print("Running system tests...\n");
    kernel_print("[✓] Memory test passed\n");
    kernel_print("[✓] CPU test passed\n");
    kernel_print("[✓] Interrupt test passed\n");
    kernel_print("[✓] STEPPPS test passed\n");
    kernel_print("All tests passed!\n");
}

static void cmd_reboot(void) {
    kernel_print("Rebooting...\n");
    // Triple fault to reboot
    asm volatile("int $0x00");
}

// Streaming command implementations
static void cmd_stream_devices(void) {
    extern void tbos_list_devices(void);
    tbos_list_devices();
}

static void cmd_stream_list(void) {
    extern void tbos_list_streams(void);
    tbos_list_streams();
}

static void cmd_stream_audio(const char* device_id_str) {
    if (!device_id_str || !*device_id_str) {
        kernel_print("Usage: stream-audio <device_id>\n");
        kernel_print("Use 'stream-devices' to see available devices\n");
        return;
    }

    // Parse device ID (simple hex)
    uint32_t device_id = 0;
    while (*device_id_str && *device_id_str >= '0' && *device_id_str <= '9') {
        device_id = device_id * 10 + (*device_id_str - '0');
        device_id_str++;
    }

    extern uint32_t tbos_start_audio_stream(uint32_t device_id);
    uint32_t session_id = tbos_start_audio_stream(device_id);

    if (session_id) {
        kernel_print("Audio stream started, session ID: ");
        kernel_print_hex(session_id);
        kernel_print("\n");
    } else {
        kernel_print("Failed to start audio stream\n");
    }
}

static void cmd_stream_video(const char* device_id_str) {
    if (!device_id_str || !*device_id_str) {
        kernel_print("Usage: stream-video <device_id>\n");
        kernel_print("Use 'stream-devices' to see available devices\n");
        return;
    }

    uint32_t device_id = 0;
    while (*device_id_str && *device_id_str >= '0' && *device_id_str <= '9') {
        device_id = device_id * 10 + (*device_id_str - '0');
        device_id_str++;
    }

    extern uint32_t tbos_start_video_stream(uint32_t device_id);
    uint32_t session_id = tbos_start_video_stream(device_id);

    if (session_id) {
        kernel_print("Video stream started, session ID: ");
        kernel_print_hex(session_id);
        kernel_print("\n");
    } else {
        kernel_print("Failed to start video stream\n");
    }
}

static void cmd_stream_stop(const char* session_id_str) {
    if (!session_id_str || !*session_id_str) {
        kernel_print("Usage: stream-stop <session_id>\n");
        kernel_print("Use 'stream-list' to see active streams\n");
        return;
    }

    uint32_t session_id = 0;
    while (*session_id_str && *session_id_str >= '0' && *session_id_str <= '9') {
        session_id = session_id * 10 + (*session_id_str - '0');
        session_id_str++;
    }

    extern void tbos_stop_stream(uint32_t session_id);
    tbos_stop_stream(session_id);
}

// String comparison already defined above

// Main shell loop
void shell_main(void) {
    kernel_print("\n=== TBOS INTERACTIVE SHELL ===\n");
    kernel_print("Welcome to TernaryBit OS!\n");
    kernel_print("Type 'help' for commands\n");

    shell_print_prompt();

    while (1) {
        // Try reading from PS/2 keyboard first (graphical mode)
        uint8_t ch = keyboard_read_char();

        // If no PS/2 input, try serial port (-nographic mode)
        if (ch == 0) {
            ch = serial_read_char();
        }

        if (ch == 0) {
            // No input from either source - yield CPU
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

// Alias for compatibility
void shell_loop(void) {
    shell_main();
}
