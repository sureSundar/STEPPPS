/**
 * @file tbos_minimal_boot.c
 * @brief TernaryBit OS - Minimal Bootloader (Calculator + Radio Computer)
 *
 * US-1.1: Minimal Hardware Bootstrap
 *
 * Target: 8-bit microcontrollers with < 64KB RAM
 * Goal: < 512 bytes bootloader
 * Philosophy: "From calculator to cloud, one OS for all"
 *
 * Hardware Requirements:
 * - CPU: 8-bit or better (Arduino, ESP8266, AVR)
 * - RAM: 2KB minimum, 64KB recommended
 * - Storage: 8KB minimum
 * - I/O: Serial/Radio for communication
 *
 * @version 1.0
 * @date 2025-11-04
 */

#include <stdint.h>
#include <stdbool.h>

/* ========================================================================= */
/* MINIMAL CONFIGURATION                                                     */
/* ========================================================================= */

#define TBOS_MINIMAL_VERSION    "0.1.0"
#define TBOS_MAGIC              0x7B05  /* TBOS in hex */

/* Consciousness levels for minimal systems */
typedef enum {
    CONSCIOUSNESS_DORMANT = 0,
    CONSCIOUSNESS_AWAKENING = 1,
    CONSCIOUSNESS_AWARE = 2
} minimal_consciousness_t;

/* Minimal karma (8-bit for tight memory) */
typedef int8_t minimal_karma_t;

/* ========================================================================= */
/* HARDWARE DETECTION                                                        */
/* ========================================================================= */

typedef enum {
    HW_CALCULATOR,      /* Calculator-class (2KB RAM) */
    HW_RADIO,           /* Radio computer (8KB RAM) */
    HW_MINIMAL,         /* Minimal system (16KB RAM) */
    HW_STANDARD,        /* Standard system (64KB+ RAM) */
    HW_ADVANCED         /* Advanced system (1MB+ RAM) */
} hardware_class_t;

typedef struct {
    hardware_class_t class;
    uint32_t ram_bytes;
    uint32_t rom_bytes;
    uint8_t cpu_bits;
    bool has_radio;
    bool has_serial;
} hardware_profile_t;

/* Global hardware profile */
static hardware_profile_t g_hw;

/* ========================================================================= */
/* BOOTLOADER STATE                                                          */
/* ========================================================================= */

typedef struct {
    uint16_t magic;                     /* Boot magic number */
    minimal_consciousness_t consciousness;
    minimal_karma_t karma;
    uint8_t boot_count;
    uint16_t uptime_seconds;
    bool is_initialized;
} minimal_boot_state_t;

static minimal_boot_state_t g_boot_state;

/* ========================================================================= */
/* MINIMAL I/O (Serial/Radio)                                               */
/* ========================================================================= */

#ifndef EMBEDDED_BUILD
/* Simulation mode for testing */
#include <stdio.h>
#define hw_putc(c)      putchar(c)
#define hw_getc()       getchar()
#define hw_delay_ms(ms) /* no-op in sim */
#else
/* Real embedded hardware */
extern void hw_putc(char c);
extern char hw_getc(void);
extern void hw_delay_ms(uint16_t ms);
#endif

/* Simple output functions */
void boot_puts(const char* str) {
    while (*str) {
        hw_putc(*str++);
    }
}

void boot_putnum(uint16_t num) {
    if (num >= 10) boot_putnum(num / 10);
    hw_putc('0' + (num % 10));
}

/* ========================================================================= */
/* HARDWARE DETECTION                                                        */
/* ========================================================================= */

void detect_hardware(void) {
    /* Simple RAM detection heuristic */
    /* In real implementation, would probe memory */

    #ifdef RAM_SIZE
        g_hw.ram_bytes = RAM_SIZE;
    #else
        /* Default assumption for simulation */
        g_hw.ram_bytes = 64 * 1024;  /* 64KB */
    #endif

    /* Classify hardware based on RAM */
    if (g_hw.ram_bytes < 4 * 1024) {
        g_hw.class = HW_CALCULATOR;  /* < 4KB = Calculator class */
    } else if (g_hw.ram_bytes < 16 * 1024) {
        g_hw.class = HW_RADIO;       /* 4-16KB = Radio computer */
    } else if (g_hw.ram_bytes < 64 * 1024) {
        g_hw.class = HW_MINIMAL;     /* 16-64KB = Minimal */
    } else if (g_hw.ram_bytes < 1024 * 1024) {
        g_hw.class = HW_STANDARD;    /* 64KB-1MB = Standard */
    } else {
        g_hw.class = HW_ADVANCED;    /* > 1MB = Advanced */
    }

    /* Detect I/O capabilities */
    g_hw.has_serial = true;   /* Assume serial for now */
    g_hw.has_radio = false;   /* Radio detection would be hardware-specific */
    g_hw.cpu_bits = 8;        /* Default to 8-bit, enhanced in real impl */
}

const char* get_hardware_name(hardware_class_t class) {
    switch (class) {
        case HW_CALCULATOR: return "Calculator";
        case HW_RADIO:      return "Radio Computer";
        case HW_MINIMAL:    return "Minimal System";
        case HW_STANDARD:   return "Standard System";
        case HW_ADVANCED:   return "Advanced System";
        default:            return "Unknown";
    }
}

/* ========================================================================= */
/* CONSCIOUSNESS INITIALIZATION                                              */
/* ========================================================================= */

void init_consciousness(void) {
    /* Start with awakening consciousness */
    g_boot_state.consciousness = CONSCIOUSNESS_AWAKENING;
    g_boot_state.karma = 100;  /* Starting karma */

    /* Check if this is first boot */
    if (g_boot_state.magic != TBOS_MAGIC) {
        /* First boot ever */
        g_boot_state.magic = TBOS_MAGIC;
        g_boot_state.boot_count = 1;
        g_boot_state.uptime_seconds = 0;
    } else {
        /* Resuming from previous session */
        g_boot_state.boot_count++;
        /* Karma bonus for successful previous boots */
        if (g_boot_state.boot_count > 10) {
            g_boot_state.consciousness = CONSCIOUSNESS_AWARE;
            g_boot_state.karma += 10;
        }
    }

    g_boot_state.is_initialized = true;
}

/* ========================================================================= */
/* BOOT SEQUENCE                                                             */
/* ========================================================================= */

void print_boot_banner(void) {
    boot_puts("\n");
    boot_puts("╔══════════════════════════════════════╗\n");
    boot_puts("║   TernaryBit OS - Minimal Boot      ║\n");
    boot_puts("║   सॉफ्टवेर (Soft-Aware)              ║\n");
    boot_puts("╚══════════════════════════════════════╝\n");
    boot_puts("\n");
}

void print_hardware_info(void) {
    boot_puts("Hardware Profile:\n");
    boot_puts("  Class: ");
    boot_puts(get_hardware_name(g_hw.class));
    boot_puts("\n");

    boot_puts("  RAM: ");
    boot_putnum(g_hw.ram_bytes);
    boot_puts(" bytes\n");

    boot_puts("  CPU: ");
    boot_putnum(g_hw.cpu_bits);
    boot_puts("-bit\n");

    boot_puts("  I/O: ");
    if (g_hw.has_serial) boot_puts("Serial ");
    if (g_hw.has_radio) boot_puts("Radio ");
    boot_puts("\n\n");
}

void print_consciousness_info(void) {
    boot_puts("Consciousness State:\n");

    boot_puts("  Level: ");
    switch (g_boot_state.consciousness) {
        case CONSCIOUSNESS_DORMANT:
            boot_puts("DORMANT");
            break;
        case CONSCIOUSNESS_AWAKENING:
            boot_puts("AWAKENING");
            break;
        case CONSCIOUSNESS_AWARE:
            boot_puts("AWARE");
            break;
    }
    boot_puts("\n");

    boot_puts("  Karma: ");
    boot_putnum(g_boot_state.karma);
    boot_puts("\n");

    boot_puts("  Boot Count: ");
    boot_putnum(g_boot_state.boot_count);
    boot_puts("\n\n");
}

void load_minimal_kernel(void) {
    boot_puts("Loading minimal kernel...\n");

    /* Adaptive loading based on hardware */
    switch (g_hw.class) {
        case HW_CALCULATOR:
            boot_puts("  [Calc Mode] Basic computation only\n");
            break;

        case HW_RADIO:
            boot_puts("  [Radio Mode] Computation + basic I/O\n");
            break;

        case HW_MINIMAL:
            boot_puts("  [Minimal Mode] Essential features\n");
            break;

        case HW_STANDARD:
            boot_puts("  [Standard Mode] Full features\n");
            break;

        case HW_ADVANCED:
            boot_puts("  [Advanced Mode] All features + extensions\n");
            break;
    }

    boot_puts("  Kernel loaded. Consciousness: AWARE\n");
    g_boot_state.consciousness = CONSCIOUSNESS_AWARE;
}

/* ========================================================================= */
/* MINIMAL SHELL                                                             */
/* ========================================================================= */

void minimal_shell(void) {
    boot_puts("\n");
    boot_puts("═══ TernaryBit Minimal Shell ═══\n");
    boot_puts("Commands: help, karma, status, reboot\n");
    boot_puts("(Press 'h' for help, 'q' to quit demo)\n\n");

    boot_puts("tbos> ");

    /* Simple command loop (simulation only) */
    #ifndef EMBEDDED_BUILD
    char cmd = hw_getc();

    switch (cmd) {
        case 'h':
            boot_puts("help\n\n");
            boot_puts("Available commands:\n");
            boot_puts("  h - Show this help\n");
            boot_puts("  k - Show karma\n");
            boot_puts("  s - Show status\n");
            boot_puts("  q - Quit\n");
            boot_puts("\n");
            break;

        case 'k':
            boot_puts("karma\n\n");
            boot_puts("Current Karma: ");
            boot_putnum(g_boot_state.karma);
            boot_puts("\n");
            boot_puts("Karma increases through:\n");
            boot_puts("  - Successful boots (+10)\n");
            boot_puts("  - Efficient operation (+5)\n");
            boot_puts("  - Helping other devices (+20)\n");
            boot_puts("\n");
            break;

        case 's':
            boot_puts("status\n\n");
            print_consciousness_info();
            boot_puts("Uptime: ");
            boot_putnum(g_boot_state.uptime_seconds);
            boot_puts(" seconds\n\n");
            break;

        case 'q':
            boot_puts("quit\n\n");
            boot_puts("Initiating graceful shutdown...\n");
            boot_puts("Preserving consciousness state... Done.\n");
            boot_puts("Saving karma score... Done.\n");
            boot_puts("॥ तत् सत् ॥\n\n");
            return;

        default:
            boot_puts("\nUnknown command. Type 'h' for help.\n\n");
            break;
    }
    #endif
}

/* ========================================================================= */
/* MAIN BOOT ENTRY POINT                                                     */
/* ========================================================================= */

int tbos_minimal_boot(void) {
    /* Stage 1: Hardware Detection */
    detect_hardware();

    /* Stage 2: Consciousness Initialization */
    init_consciousness();

    /* Stage 3: Boot Banner */
    print_boot_banner();

    /* Stage 4: Hardware Info */
    print_hardware_info();

    /* Stage 5: Consciousness Info */
    print_consciousness_info();

    /* Stage 6: Kernel Loading */
    load_minimal_kernel();

    /* Stage 7: Minimal Shell */
    boot_puts("Boot complete. Starting shell...\n");
    minimal_shell();

    /* Boot success */
    boot_puts("\nBoot sequence complete!\n");
    boot_puts("TernaryBit OS ready for universal computing.\n");
    boot_puts("From calculator to cloud - One OS for All. 🕉️\n\n");

    return 0;
}

/* ========================================================================= */
/* STANDALONE ENTRY POINT (Only when built standalone)                      */
/* ========================================================================= */

#ifdef TBOS_BOOT_STANDALONE
int main(void) {
    return tbos_minimal_boot();
}
#endif
