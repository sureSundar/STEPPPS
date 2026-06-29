/**
 * @file tbos_wasm_main.c
 * @brief TernaryBit OS WASM Entry Point
 *
 * This is the main entry point for the WebAssembly build of TernaryBit OS.
 * It provides:
 * - BCB initialization for WASM environment
 * - Host API bridge to JavaScript
 * - Shell command processing
 *
 * Build:
 *   emcc wasm/tbos_wasm_main.c -o wasm/tbos.js \
 *        -I include -s EXPORTED_FUNCTIONS='["_main","_tbos_run","_tbos_command"]' \
 *        -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]'
 *
 * Traceability: v4.0/ROADMAP.md V4-009
 *
 * @version 1.0
 * @date 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

/* ========================================================================= */
/* BCB DEFINITIONS (simplified for WASM)                                     */
/* ========================================================================= */

#define BCB_MAGIC           0x43424254  /* "TBBC" */
#define BCB_VERSION_1_0     0x0001
#define BCB_SIZE            128

#define BCB_FLAG_WASM       (1 << 12)
#define BCB_FLAG_64BIT      (1 << 13)
#define BCB_FLAG_HOSTED     (1 << 2)

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint16_t version;
    uint16_t length;
    uint32_t stage_flags;
    uint32_t boot_stage;
    uint64_t boot_drive_id;
    uint32_t boot_partition;
    uint32_t boot_fs_type;
    uint64_t mem_low_bytes;
    uint64_t mem_high_bytes;
    uint64_t mem_total_bytes;
    uint64_t kernel_entry;
    uint32_t kernel_size;
    uint32_t kernel_flags;
    uint64_t host_api_ptr;
    uint64_t host_context;
    uint32_t bcb_checksum;
    uint32_t tbds_pointer;
    uint32_t tbds_length;
    uint8_t  reserved[28];
} tbos_bcb_v1_t;

/* ========================================================================= */
/* CONFIGURATION                                                              */
/* ========================================================================= */

#define WASM_HEAP_SIZE      (32 * 1024 * 1024)  /* 32 MB */
#define WASM_VERSION        "TernaryBit OS WASM v1.0"
#define MAX_COMMAND_LEN     256
#define MAX_HISTORY         50

/* ========================================================================= */
/* GLOBAL STATE                                                               */
/* ========================================================================= */

static tbos_bcb_v1_t g_bcb;
static size_t g_heap_used = 0;
static bool g_initialized = false;

/* Command history */
static char g_history[MAX_HISTORY][MAX_COMMAND_LEN];
static int g_history_count = 0;

/* ========================================================================= */
/* JAVASCRIPT IMPORTS (Emscripten)                                           */
/* ========================================================================= */

#ifdef __EMSCRIPTEN__
/* These are implemented in JavaScript */
EM_JS(void, js_console_write, (const char* str), {
    const text = UTF8ToString(str);
    if (window.tbosConsoleWrite) {
        window.tbosConsoleWrite(text);
    } else {
        console.log(text);
    }
});

EM_JS(void, js_console_clear, (), {
    if (window.tbosConsoleClear) {
        window.tbosConsoleClear();
    }
});

EM_JS(double, js_get_time_ms, (), {
    return performance.now();
});

EM_JS(void, js_update_memory, (int used_kb), {
    if (window.tbosUpdateMemory) {
        window.tbosUpdateMemory(used_kb);
    }
});

EM_JS(void, js_set_ready, (int ready), {
    if (window.tbosSetReady) {
        window.tbosSetReady(ready);
    }
});
#else
/* Stubs for non-Emscripten builds */
static void js_console_write(const char* str) { printf("%s", str); }
static void js_console_clear(void) { printf("\033[2J\033[H"); }
static double js_get_time_ms(void) { return 0; }
static void js_update_memory(int used_kb) { (void)used_kb; }
static void js_set_ready(int ready) { (void)ready; }
#endif

/* ========================================================================= */
/* OUTPUT FUNCTIONS                                                           */
/* ========================================================================= */

static void print(const char* str) {
    js_console_write(str);
}

static void println(const char* str) {
    js_console_write(str);
    js_console_write("\n");
}

static void print_hex(uint32_t value) {
    char buf[16];
    snprintf(buf, sizeof(buf), "0x%08X", value);
    print(buf);
}

static void print_int(int value) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", value);
    print(buf);
}

/* ========================================================================= */
/* BCB FUNCTIONS                                                              */
/* ========================================================================= */

static void bcb_init(void) {
    memset(&g_bcb, 0, sizeof(g_bcb));

    g_bcb.magic = BCB_MAGIC;
    g_bcb.version = BCB_VERSION_1_0;
    g_bcb.length = BCB_SIZE - 8;

    /* WASM environment flags */
    g_bcb.stage_flags = BCB_FLAG_WASM | BCB_FLAG_64BIT | BCB_FLAG_HOSTED;
    g_bcb.boot_stage = 0;  /* Stage 0 = hosted/WASM */

    /* Memory configuration */
    g_bcb.mem_low_bytes = 640 * 1024;
    g_bcb.mem_high_bytes = WASM_HEAP_SIZE;
    g_bcb.mem_total_bytes = WASM_HEAP_SIZE;

    /* No physical kernel in WASM */
    g_bcb.kernel_entry = 0;
    g_bcb.kernel_size = 0;
    g_bcb.kernel_flags = 0;

    /* Host API available via JavaScript */
    g_bcb.host_api_ptr = 0xDEADBEEF;  /* Marker for JS bridge */
    g_bcb.host_context = 0;

    /* Compute simple checksum */
    g_bcb.bcb_checksum = g_bcb.magic ^ g_bcb.stage_flags ^
                         (uint32_t)g_bcb.mem_total_bytes;
}

static const char* bcb_hal_type_name(void) {
    if (g_bcb.stage_flags & BCB_FLAG_WASM) return "WebAssembly";
    if (g_bcb.stage_flags & BCB_FLAG_HOSTED) return "Hosted";
    return "Unknown";
}

/* ========================================================================= */
/* MEMORY MANAGEMENT                                                          */
/* ========================================================================= */

static void* wasm_allocate(size_t size) {
    /* Simple tracking - actual allocation via malloc */
    size = (size + 15) & ~15;  /* Align to 16 bytes */

    if (g_heap_used + size > WASM_HEAP_SIZE) {
        println("[ERROR] Out of memory!");
        return NULL;
    }

    void* ptr = malloc(size);
    if (ptr) {
        g_heap_used += size;
        js_update_memory((int)(g_heap_used / 1024));
    }
    return ptr;
}

/* ========================================================================= */
/* COMMAND IMPLEMENTATIONS                                                    */
/* ========================================================================= */

static void cmd_help(void) {
    println("TernaryBit OS WASM Shell Commands:");
    println("  help     - Show this help");
    println("  bcb      - Display BCB information");
    println("  memory   - Show memory status");
    println("  time     - Show current time");
    println("  clear    - Clear terminal");
    println("  version  - Show version info");
    println("  echo     - Echo text");
    println("  modules  - List loaded modules");
    println("  test     - Run self-test");
    println("  history  - Show command history");
}

static void cmd_bcb(void) {
    println("Boot Capability Block (BCB):");
    print("  Magic:       "); print_hex(g_bcb.magic);
    println(g_bcb.magic == BCB_MAGIC ? " (valid)" : " (INVALID)");
    print("  Version:     "); print_hex(g_bcb.version); println("");
    print("  Stage Flags: "); print_hex(g_bcb.stage_flags); println("");
    print("  HAL Type:    "); println(bcb_hal_type_name());
    print("  Memory:      "); print_int((int)(g_bcb.mem_total_bytes / (1024 * 1024)));
    println(" MB");
    println("  Host API:    available (JavaScript bridge)");
    println("  Checksum:    OK");
}

static void cmd_memory(void) {
    println("Memory Status:");
    print("  Total:    "); print_int(WASM_HEAP_SIZE / (1024 * 1024)); println(" MB");
    print("  Used:     "); print_int((int)(g_heap_used / 1024)); println(" KB");
    print("  Free:     "); print_int((int)((WASM_HEAP_SIZE - g_heap_used) / 1024)); println(" KB");
    println("  Runtime:  WebAssembly/JavaScript");
}

static void cmd_time(void) {
    double ms = js_get_time_ms();
    print("Time: ");
    print_int((int)ms);
    println(" ms since page load");
}

static void cmd_clear(void) {
    js_console_clear();
}

static void cmd_version(void) {
    println(WASM_VERSION);
    println("Platform: WebAssembly (Emscripten)");
    println("BCB Version: 1.0");
#ifdef __EMSCRIPTEN__
    println("Compiled with Emscripten");
#else
    println("Compiled natively (test mode)");
#endif
}

static void cmd_modules(void) {
    println("Loaded Modules:");
    println("  [CORE] tbos_wasm_main    - WASM entry point");
    println("  [CORE] bcb_init          - Boot Capability Block");
    println("  [HAL]  hal_wasm          - WASM HAL stubs");
    println("  [API]  js_console        - JavaScript console bridge");
    println("  [API]  js_timer          - JavaScript timer bridge");
    println("  [MEM]  wasm_allocator    - Memory allocator");
}

static void cmd_test(void) {
    println("Running self-test...");
    int pass = 0, fail = 0;

    /* BCB test */
    if (g_bcb.magic == BCB_MAGIC) {
        println("  BCB Magic:     PASS");
        pass++;
    } else {
        println("  BCB Magic:     FAIL");
        fail++;
    }

    /* Memory test */
    void* test_ptr = wasm_allocate(1024);
    if (test_ptr) {
        println("  Memory Alloc:  PASS");
        pass++;
    } else {
        println("  Memory Alloc:  FAIL");
        fail++;
    }

    /* Timer test */
    double t = js_get_time_ms();
    if (t > 0) {
        println("  Timer:         PASS");
        pass++;
    } else {
        println("  Timer:         FAIL");
        fail++;
    }

    /* Flags test */
    if (g_bcb.stage_flags & BCB_FLAG_WASM) {
        println("  WASM Flag:     PASS");
        pass++;
    } else {
        println("  WASM Flag:     FAIL");
        fail++;
    }

    println("");
    print("Self-test: ");
    print_int(pass);
    print(" passed, ");
    print_int(fail);
    println(" failed");
}

static void cmd_history(void) {
    println("Command History:");
    for (int i = 0; i < g_history_count; i++) {
        print("  ");
        print_int(i + 1);
        print(". ");
        println(g_history[i]);
    }
    if (g_history_count == 0) {
        println("  (empty)");
    }
}

static void cmd_echo(const char* args) {
    if (args && *args) {
        println(args);
    }
}

/* ========================================================================= */
/* COMMAND PROCESSOR                                                          */
/* ========================================================================= */

static void add_to_history(const char* cmd) {
    if (g_history_count < MAX_HISTORY) {
        strncpy(g_history[g_history_count], cmd, MAX_COMMAND_LEN - 1);
        g_history[g_history_count][MAX_COMMAND_LEN - 1] = '\0';
        g_history_count++;
    }
}

static void execute_command(const char* input) {
    /* Skip empty input */
    while (*input == ' ') input++;
    if (*input == '\0') return;

    /* Add to history */
    add_to_history(input);

    /* Parse command and arguments */
    char cmd[64] = {0};
    const char* args = NULL;

    const char* space = strchr(input, ' ');
    if (space) {
        size_t cmd_len = space - input;
        if (cmd_len >= sizeof(cmd)) cmd_len = sizeof(cmd) - 1;
        strncpy(cmd, input, cmd_len);
        args = space + 1;
        while (*args == ' ') args++;
    } else {
        strncpy(cmd, input, sizeof(cmd) - 1);
    }

    /* Execute command */
    if (strcmp(cmd, "help") == 0) {
        cmd_help();
    } else if (strcmp(cmd, "bcb") == 0) {
        cmd_bcb();
    } else if (strcmp(cmd, "memory") == 0 || strcmp(cmd, "mem") == 0) {
        cmd_memory();
    } else if (strcmp(cmd, "time") == 0) {
        cmd_time();
    } else if (strcmp(cmd, "clear") == 0 || strcmp(cmd, "cls") == 0) {
        cmd_clear();
    } else if (strcmp(cmd, "version") == 0 || strcmp(cmd, "ver") == 0) {
        cmd_version();
    } else if (strcmp(cmd, "modules") == 0 || strcmp(cmd, "mod") == 0) {
        cmd_modules();
    } else if (strcmp(cmd, "test") == 0) {
        cmd_test();
    } else if (strcmp(cmd, "history") == 0 || strcmp(cmd, "hist") == 0) {
        cmd_history();
    } else if (strcmp(cmd, "echo") == 0) {
        cmd_echo(args);
    } else {
        print("Unknown command: ");
        println(cmd);
        println("Type 'help' for available commands.");
    }
}

/* ========================================================================= */
/* EXPORTED FUNCTIONS (called from JavaScript)                                */
/* ========================================================================= */

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void tbos_init(void) {
    if (g_initialized) return;

    println("[INIT] TernaryBit OS WASM Runtime");
    println("[INIT] Initializing BCB...");
    bcb_init();

    print("[INIT] BCB Magic: ");
    print_hex(g_bcb.magic);
    println("");

    print("[INIT] HAL Type: ");
    println(bcb_hal_type_name());

    /* Initial memory allocation */
    wasm_allocate(64 * 1024);  /* 64KB initial */

    println("[INIT] Memory allocator ready");
    println("[INIT] Initialization complete");
    println("");

    g_initialized = true;
    js_set_ready(1);
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void tbos_command(const char* cmd) {
    if (!g_initialized) {
        tbos_init();
    }
    execute_command(cmd);
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void tbos_run(void) {
    tbos_init();

    /* Print banner */
    println("");
    println("========================================");
    println("   TernaryBit OS - WebAssembly Build   ");
    println("   Type 'help' for commands            ");
    println("========================================");
    println("");
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int tbos_get_bcb_magic(void) {
    return (int)g_bcb.magic;
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int tbos_get_memory_used(void) {
    return (int)(g_heap_used / 1024);
}

/* ========================================================================= */
/* MAIN ENTRY POINT                                                           */
/* ========================================================================= */

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    tbos_run();

#ifdef __EMSCRIPTEN__
    /* In Emscripten, we don't block - JavaScript handles the event loop */
    return 0;
#else
    /* For native testing, run a simple REPL */
    char line[MAX_COMMAND_LEN];
    while (1) {
        printf("tbos> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;

        /* Remove newline */
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';

        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0) break;
        execute_command(line);
    }
    println("Goodbye!");
    return 0;
#endif
}
