/**
 * @file tbos_hosted_linux.c
 * @brief TernaryBit OS Hosted Linux Stub
 *
 * This stub allows TBOS to run as a userspace application on Linux.
 * It provides:
 * - BCB population with hosted environment info
 * - Host API table for system call shims
 * - Memory allocation via mmap
 * - Console I/O via stdin/stdout
 *
 * Usage:
 *   gcc -o tbos-linux tbos_hosted_linux.c -I../include
 *   ./tbos-linux
 *
 * Traceability: v4.0/ROADMAP.md V4-004
 *
 * @version 1.0
 * @date 2025
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>

#include "tbos/bcb.h"

/* ========================================================================= */
/* CONFIGURATION                                                              */
/* ========================================================================= */

#define HOSTED_HEAP_SIZE    (64 * 1024 * 1024)  /* 64 MB heap */
#define HOSTED_VERSION      "TernaryBit OS Hosted Linux v1.0"

/* ========================================================================= */
/* GLOBAL STATE                                                               */
/* ========================================================================= */

static tbos_bcb_v1_t g_bcb;
static tbos_host_api_t g_host_api;
static void* g_heap_base = NULL;
static size_t g_heap_used = 0;
static struct termios g_orig_termios;
static bool g_termios_saved = false;

/* ========================================================================= */
/* HOST API IMPLEMENTATION                                                    */
/* ========================================================================= */

/**
 * @brief Write to console (stdout)
 */
static int hosted_write_console(const char* buf, uint32_t len) {
    if (!buf || len == 0) return 0;
    return (int)write(STDOUT_FILENO, buf, len);
}

/**
 * @brief Read from console (stdin)
 */
static int hosted_read_console(char* buf, uint32_t max_len) {
    if (!buf || max_len == 0) return 0;
    return (int)read(STDIN_FILENO, buf, max_len);
}

/**
 * @brief Simple bump allocator from heap
 */
static void* hosted_allocate(uint32_t size) {
    if (!g_heap_base) return NULL;

    /* Align to 16 bytes */
    size = (size + 15) & ~15;

    if (g_heap_used + size > HOSTED_HEAP_SIZE) {
        fprintf(stderr, "[HOST] Out of memory! Used: %zu, Requested: %u\n",
                g_heap_used, size);
        return NULL;
    }

    void* ptr = (uint8_t*)g_heap_base + g_heap_used;
    g_heap_used += size;
    return ptr;
}

/**
 * @brief Deallocate (no-op for bump allocator)
 */
static void hosted_deallocate(void* ptr) {
    (void)ptr;
    /* Bump allocator doesn't support individual frees */
}

/**
 * @brief Open file
 */
static int hosted_file_open(const char* path, int flags) {
    return open(path, flags, 0644);
}

/**
 * @brief Close file
 */
static int hosted_file_close(int fd) {
    return close(fd);
}

/**
 * @brief Read from file
 */
static int hosted_file_read(int fd, void* buf, uint32_t len) {
    return (int)read(fd, buf, len);
}

/**
 * @brief Write to file
 */
static int hosted_file_write(int fd, const void* buf, uint32_t len) {
    return (int)write(fd, buf, len);
}

/**
 * @brief Get current time in milliseconds
 */
static uint64_t hosted_get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
}

/**
 * @brief Sleep for milliseconds
 */
static void hosted_sleep_ms(uint32_t ms) {
    usleep(ms * 1000);
}

/**
 * @brief Exit hosted environment
 */
static void hosted_exit(int status) {
    /* Restore terminal */
    if (g_termios_saved) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);
    }
    exit(status);
}

/**
 * @brief Initialize host API table
 */
static void init_host_api(void) {
    memset(&g_host_api, 0, sizeof(g_host_api));

    g_host_api.version = 0x0001;
    g_host_api.flags = 0;

    g_host_api.write_console = hosted_write_console;
    g_host_api.read_console = hosted_read_console;
    g_host_api.allocate = hosted_allocate;
    g_host_api.deallocate = hosted_deallocate;
    g_host_api.file_open = hosted_file_open;
    g_host_api.file_close = hosted_file_close;
    g_host_api.file_read = hosted_file_read;
    g_host_api.file_write = hosted_file_write;
    g_host_api.get_time_ms = hosted_get_time_ms;
    g_host_api.sleep_ms = hosted_sleep_ms;
    g_host_api.exit = hosted_exit;
}

/* ========================================================================= */
/* BCB INITIALIZATION                                                         */
/* ========================================================================= */

/**
 * @brief Initialize BCB for hosted environment
 */
static void init_bcb(void) {
    bcb_init(&g_bcb);

    /* Set hosted environment flags */
    g_bcb.stage_flags = BCB_FLAG_HOSTED |
                        BCB_FLAG_64BIT |
                        BCB_FLAG_SERIAL_CONSOLE;

    g_bcb.boot_stage = 0;  /* 0 = hosted (no Stage 1/2) */

    /* Boot device is stdin/stdout */
    g_bcb.boot_drive_id = 0;
    g_bcb.boot_partition = 0;
    g_bcb.boot_fs_type = 0;

    /* Memory info */
    g_bcb.mem_low_bytes = 640 * 1024;  /* Simulated conventional memory */
    g_bcb.mem_high_bytes = HOSTED_HEAP_SIZE;
    g_bcb.mem_total_bytes = HOSTED_HEAP_SIZE;

    /* Kernel entry (not used in hosted mode) */
    g_bcb.kernel_entry = 0;
    g_bcb.kernel_size = 0;
    g_bcb.kernel_flags = 0;

    /* Host API pointer */
    g_bcb.host_api_ptr = (uint64_t)(uintptr_t)&g_host_api;
    g_bcb.host_context = 0;

    /* No TBDS in hosted mode */
    g_bcb.tbds_pointer = 0;
    g_bcb.tbds_length = 0;

    /* Compute checksum */
    bcb_compute_checksum(&g_bcb);
}

/* ========================================================================= */
/* TERMINAL SETUP                                                             */
/* ========================================================================= */

/**
 * @brief Configure terminal for raw mode
 */
static void setup_terminal(void) {
    if (!isatty(STDIN_FILENO)) return;

    if (tcgetattr(STDIN_FILENO, &g_orig_termios) == 0) {
        g_termios_saved = true;

        struct termios raw = g_orig_termios;
        raw.c_lflag &= ~(ECHO | ICANON);  /* Disable echo and canonical mode */
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;  /* 100ms timeout */

        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    }
}

/**
 * @brief Signal handler for clean exit
 */
static void signal_handler(int sig) {
    (void)sig;
    hosted_exit(128 + sig);
}

/* ========================================================================= */
/* HOSTED SHELL IMPLEMENTATION                                                */
/* ========================================================================= */

/**
 * @brief Print string to console
 */
static void print(const char* str) {
    if (str) {
        hosted_write_console(str, strlen(str));
    }
}

/**
 * @brief Print character
 */
static void putch(char c) {
    hosted_write_console(&c, 1);
}

/**
 * @brief Read line from console
 */
static int readline(char* buf, int max_len) {
    int pos = 0;
    char c;

    while (pos < max_len - 1) {
        int n = hosted_read_console(&c, 1);
        if (n <= 0) {
            usleep(10000);  /* 10ms delay */
            continue;
        }

        if (c == '\n' || c == '\r') {
            putch('\n');
            break;
        } else if (c == 127 || c == '\b') {
            if (pos > 0) {
                pos--;
                print("\b \b");
            }
        } else if (c >= 32 && c < 127) {
            buf[pos++] = c;
            putch(c);
        } else if (c == 3) {  /* Ctrl+C */
            print("^C\n");
            return -1;
        } else if (c == 4) {  /* Ctrl+D */
            print("^D\n");
            return -2;
        }
    }

    buf[pos] = '\0';
    return pos;
}

/**
 * @brief Simple command parser
 */
static void execute_command(const char* cmd) {
    if (strlen(cmd) == 0) return;

    if (strcmp(cmd, "help") == 0) {
        print("TernaryBit OS Hosted Shell Commands:\n");
        print("  help     - Show this help\n");
        print("  bcb      - Display BCB information\n");
        print("  memory   - Show memory status\n");
        print("  time     - Show current time\n");
        print("  clear    - Clear screen\n");
        print("  exit     - Exit hosted environment\n");
        print("  version  - Show version info\n");
    } else if (strcmp(cmd, "bcb") == 0) {
        print("Boot Capability Block (BCB):\n");
        printf("  Magic:       0x%08X (%s)\n", g_bcb.magic,
               g_bcb.magic == BCB_MAGIC ? "valid" : "INVALID");
        printf("  Version:     0x%04X\n", g_bcb.version);
        printf("  Stage Flags: 0x%08X\n", g_bcb.stage_flags);
        printf("  HAL Type:    %s\n", bcb_hal_type_name(bcb_get_hal_type(&g_bcb)));

        char mem_buf[16];
        bcb_format_memory(g_bcb.mem_total_bytes, mem_buf);
        printf("  Memory:      %s\n", mem_buf);

        printf("  Host API:    %s\n", g_bcb.host_api_ptr ? "available" : "none");
        printf("  Checksum:    %s\n", bcb_verify_checksum(&g_bcb) ? "OK" : "FAILED");
    } else if (strcmp(cmd, "memory") == 0) {
        printf("Memory Status:\n");
        printf("  Heap Base:   %p\n", g_heap_base);
        printf("  Heap Size:   %u MB\n", HOSTED_HEAP_SIZE / (1024 * 1024));
        printf("  Used:        %zu bytes (%.1f%%)\n", g_heap_used,
               (double)g_heap_used / HOSTED_HEAP_SIZE * 100.0);
        printf("  Free:        %zu bytes\n", HOSTED_HEAP_SIZE - g_heap_used);
    } else if (strcmp(cmd, "time") == 0) {
        uint64_t ms = hosted_get_time_ms();
        printf("Time: %lu ms since epoch\n", (unsigned long)ms);
    } else if (strcmp(cmd, "clear") == 0) {
        print("\033[2J\033[H");  /* ANSI clear screen */
    } else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
        print("Goodbye!\n");
        hosted_exit(0);
    } else if (strcmp(cmd, "version") == 0) {
        print(HOSTED_VERSION "\n");
        print("Built: " __DATE__ " " __TIME__ "\n");
    } else {
        printf("Unknown command: %s\n", cmd);
        print("Type 'help' for available commands.\n");
    }
}

/**
 * @brief Main hosted shell loop
 */
static void shell_loop(void) {
    char line[256];

    print("\n");
    print("╔══════════════════════════════════════════════════════════════╗\n");
    print("║   TernaryBit OS - Hosted Linux Environment                   ║\n");
    print("║   Type 'help' for commands, 'exit' to quit                   ║\n");
    print("╚══════════════════════════════════════════════════════════════╝\n");
    print("\n");

    /* Display BCB summary */
    printf("[BCB] HAL Type: %s, Memory: %u MB\n",
           bcb_hal_type_name(bcb_get_hal_type(&g_bcb)),
           (unsigned)(g_bcb.mem_total_bytes / (1024 * 1024)));
    print("[BCB] Host API initialized\n");
    print("\n");

    while (1) {
        print("tbos> ");
        fflush(stdout);

        int len = readline(line, sizeof(line));
        if (len < 0) {
            if (len == -2) {  /* Ctrl+D */
                print("Goodbye!\n");
                break;
            }
            continue;
        }

        execute_command(line);
    }
}

/* ========================================================================= */
/* MAIN ENTRY POINT                                                           */
/* ========================================================================= */

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    printf("\n%s\n", HOSTED_VERSION);
    printf("Initializing hosted environment...\n\n");

    /* Set up signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Allocate heap */
    g_heap_base = mmap(NULL, HOSTED_HEAP_SIZE,
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS,
                       -1, 0);
    if (g_heap_base == MAP_FAILED) {
        fprintf(stderr, "ERROR: Failed to allocate heap: %s\n", strerror(errno));
        return 1;
    }
    printf("[INIT] Allocated %u MB heap at %p\n",
           HOSTED_HEAP_SIZE / (1024 * 1024), g_heap_base);

    /* Initialize host API */
    init_host_api();
    printf("[INIT] Host API initialized\n");

    /* Initialize BCB */
    init_bcb();
    printf("[INIT] BCB initialized (checksum: %s)\n",
           bcb_verify_checksum(&g_bcb) ? "OK" : "FAILED");

    /* Setup terminal */
    setup_terminal();
    printf("[INIT] Terminal configured\n");

    /* Run shell */
    shell_loop();

    /* Cleanup */
    if (g_termios_saved) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);
    }
    munmap(g_heap_base, HOSTED_HEAP_SIZE);

    return 0;
}
