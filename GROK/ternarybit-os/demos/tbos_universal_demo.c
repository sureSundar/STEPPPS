/**
 * TBOS Universal Hardware Demo
 * Demonstrates TBOS running from 4-bit calculators to supercomputers
 *
 * Build for different tiers:
 *   # Calculator (8-bit simulation)
 *   gcc -DTBOS_HOSTED -DTBOS_TIER=0 -o tbos_calc demos/tbos_universal_demo.c -Iinclude
 *
 *   # Embedded (16-bit simulation)
 *   gcc -DTBOS_HOSTED -DTBOS_TIER=1 -o tbos_embed demos/tbos_universal_demo.c -Iinclude
 *
 *   # Desktop (64-bit native)
 *   gcc -DTBOS_HOSTED -DTBOS_TIER=3 -o tbos_desktop demos/tbos_universal_demo.c -Iinclude
 *
 *   # Supercomputer (simulation)
 *   gcc -DTBOS_HOSTED -DTBOS_TIER=5 -o tbos_super demos/tbos_universal_demo.c -Iinclude
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * HARDWARE TIER DEFINITIONS
 * From 4-bit calculators to quantum supercomputers
 * ═══════════════════════════════════════════════════════════════════════════ */

#ifndef TBOS_TIER
    #if defined(__AVR__)
        #define TBOS_TIER 0  /* Calculator: 8-bit AVR */
    #elif defined(__MSP430__)
        #define TBOS_TIER 1  /* Embedded: 16-bit MSP430 */
    #elif defined(__arm__) && !defined(__aarch64__)
        #define TBOS_TIER 2  /* Retro: 32-bit ARM */
    #elif defined(__x86_64__) || defined(__aarch64__)
        #define TBOS_TIER 3  /* Desktop: 64-bit */
    #else
        #define TBOS_TIER 3  /* Default to desktop */
    #endif
#endif

/* Tier names and descriptions */
static const char* TIER_NAMES[] = {
    "Calculator",    /* Tier 0: 4-8 bit, 256B-4KB RAM */
    "Embedded",      /* Tier 1: 16-bit, 4KB-64KB RAM */
    "Retro",         /* Tier 2: 32-bit, 64KB-16MB RAM */
    "Desktop",       /* Tier 3: 64-bit, 16MB-16GB RAM */
    "Server",        /* Tier 4: 64-bit, 16GB-1TB RAM */
    "Supercomputer", /* Tier 5: 64-bit+, 1TB+ RAM, distributed */
    "Quantum"        /* Tier 6: Future quantum computing */
};

static const char* TIER_BITS[] = {
    "4-8 bit",
    "16-bit",
    "32-bit",
    "64-bit",
    "64-bit SMP",
    "64-bit+ Distributed",
    "Quantum"
};

static const char* TIER_RAM[] = {
    "256B - 4KB",
    "4KB - 64KB",
    "64KB - 16MB",
    "16MB - 16GB",
    "16GB - 1TB",
    "1TB+ (distributed)",
    "Qubits"
};

/* ═══════════════════════════════════════════════════════════════════════════
 * TIER-ADAPTIVE TYPES
 * Native types scale with hardware capabilities
 * ═══════════════════════════════════════════════════════════════════════════ */

#if TBOS_TIER == 0
    /* Calculator: 8-bit native */
    typedef uint8_t  tbos_native_t;
    typedef int8_t   tbos_signed_t;
    typedef uint8_t  tbos_ptr_t;
    #define TBOS_MAX_MEMORY     256
    #define TBOS_MAX_ALLOC      64
    #define TBOS_WORD_SIZE      1
    #define TBOS_HAS_HEAP       0
    #define TBOS_HAS_DISPLAY    0
    #define TBOS_HAS_NETWORK    0
#elif TBOS_TIER == 1
    /* Embedded: 16-bit native */
    typedef uint16_t tbos_native_t;
    typedef int16_t  tbos_signed_t;
    typedef uint16_t tbos_ptr_t;
    #define TBOS_MAX_MEMORY     4096
    #define TBOS_MAX_ALLOC      1024
    #define TBOS_WORD_SIZE      2
    #define TBOS_HAS_HEAP       1
    #define TBOS_HAS_DISPLAY    0
    #define TBOS_HAS_NETWORK    0
#elif TBOS_TIER == 2
    /* Retro: 32-bit native */
    typedef uint32_t tbos_native_t;
    typedef int32_t  tbos_signed_t;
    typedef uint32_t tbos_ptr_t;
    #define TBOS_MAX_MEMORY     (64 * 1024)
    #define TBOS_MAX_ALLOC      (16 * 1024 * 1024)
    #define TBOS_WORD_SIZE      4
    #define TBOS_HAS_HEAP       1
    #define TBOS_HAS_DISPLAY    1
    #define TBOS_HAS_NETWORK    0
#elif TBOS_TIER == 3
    /* Desktop: 64-bit native */
    typedef uint64_t tbos_native_t;
    typedef int64_t  tbos_signed_t;
    typedef uint64_t tbos_ptr_t;
    #define TBOS_MAX_MEMORY     (1024 * 1024)
    #define TBOS_MAX_ALLOC      (256 * 1024 * 1024)
    #define TBOS_WORD_SIZE      8
    #define TBOS_HAS_HEAP       1
    #define TBOS_HAS_DISPLAY    1
    #define TBOS_HAS_NETWORK    1
#elif TBOS_TIER == 4
    /* Server: 64-bit with NUMA */
    typedef uint64_t tbos_native_t;
    typedef int64_t  tbos_signed_t;
    typedef uint64_t tbos_ptr_t;
    #define TBOS_MAX_MEMORY     (4 * 1024 * 1024)
    #define TBOS_MAX_ALLOC      (1024 * 1024 * 1024ULL)
    #define TBOS_WORD_SIZE      8
    #define TBOS_HAS_HEAP       1
    #define TBOS_HAS_DISPLAY    1
    #define TBOS_HAS_NETWORK    1
    #define TBOS_HAS_NUMA       1
#else
    /* Supercomputer: Distributed */
    typedef uint64_t tbos_native_t;
    typedef int64_t  tbos_signed_t;
    typedef uint64_t tbos_ptr_t;
    #define TBOS_MAX_MEMORY     (16 * 1024 * 1024)
    #define TBOS_MAX_ALLOC      (16ULL * 1024 * 1024 * 1024)
    #define TBOS_WORD_SIZE      8
    #define TBOS_HAS_HEAP       1
    #define TBOS_HAS_DISPLAY    1
    #define TBOS_HAS_NETWORK    1
    #define TBOS_HAS_NUMA       1
    #define TBOS_HAS_DISTRIBUTED 1
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * TIER-ADAPTIVE MEMORY POOL
 * Static allocation for constrained devices, dynamic for others
 * ═══════════════════════════════════════════════════════════════════════════ */

#if TBOS_TIER <= 1
/* Static memory pool for calculator/embedded */
static uint8_t g_memory_pool[TBOS_MAX_MEMORY];
static uint8_t g_pool_bitmap[(TBOS_MAX_MEMORY + 7) / 8];
static tbos_native_t g_pool_used = 0;

static void* tbos_alloc(tbos_native_t size) {
    if (size == 0 || size > TBOS_MAX_ALLOC) return NULL;

    /* Round up to word size */
    size = (size + TBOS_WORD_SIZE - 1) & ~(TBOS_WORD_SIZE - 1);

    /* Find free block */
    tbos_native_t start = 0;
    tbos_native_t count = 0;

    for (tbos_native_t i = 0; i < TBOS_MAX_MEMORY; i++) {
        if (!(g_pool_bitmap[i / 8] & (1 << (i % 8)))) {
            if (count == 0) start = i;
            count++;
            if (count >= size) {
                /* Mark as used */
                for (tbos_native_t j = start; j < start + size; j++) {
                    g_pool_bitmap[j / 8] |= (1 << (j % 8));
                }
                g_pool_used += size;
                return &g_memory_pool[start];
            }
        } else {
            count = 0;
        }
    }
    return NULL;
}

static void tbos_free(void* ptr, tbos_native_t size) {
    if (!ptr || size == 0) return;

    size = (size + TBOS_WORD_SIZE - 1) & ~(TBOS_WORD_SIZE - 1);
    tbos_native_t offset = (uint8_t*)ptr - g_memory_pool;

    for (tbos_native_t i = offset; i < offset + size && i < TBOS_MAX_MEMORY; i++) {
        g_pool_bitmap[i / 8] &= ~(1 << (i % 8));
    }
    g_pool_used -= size;
}

#else
/* Use standard heap for desktop+ */
static size_t g_pool_used = 0;
static size_t g_alloc_count = 0;

static void* tbos_alloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr) {
        g_pool_used += size;
        g_alloc_count++;
    }
    return ptr;
}

static void tbos_free(void* ptr, size_t size) {
    if (ptr) {
        free(ptr);
        g_pool_used -= size;
    }
}
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * STEPPPS FRAMEWORK - Scaled by Tier
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    const char* name;
    int enabled;
    const char* description;
} steppps_dimension_t;

static steppps_dimension_t g_steppps[] = {
    {"SPACE",      1, "Hardware Resources"},
    {"TIME",       1, "Task Scheduling"},
    {"EVENT",      TBOS_TIER >= 1, "I/O & Interrupts"},
    {"PSYCHOLOGY", TBOS_TIER >= 3, "AI Integration"},
    {"PIXEL",      TBOS_HAS_DISPLAY, "Display Output"},
    {"PROMPT",     TBOS_TIER >= 2, "User Interface"},
    {"SCRIPT",     TBOS_TIER >= 2, "Automation"}
};

#define STEPPPS_COUNT (sizeof(g_steppps) / sizeof(g_steppps[0]))

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSCIOUSNESS METRICS - Universal Karma
 * ═══════════════════════════════════════════════════════════════════════════ */

static tbos_native_t g_karma = 100;

static const char* get_consciousness_level(void) {
    if (g_karma > 150) return "Enlightened";
    if (g_karma > 100) return "Aware";
    if (g_karma > 50)  return "Awakening";
    return "Learning";
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DISPLAY FUNCTIONS - Tier Adaptive
 * ═══════════════════════════════════════════════════════════════════════════ */

#if TBOS_TIER == 0
/* Calculator: Minimal 7-segment style output */
static void display_banner(void) {
    printf("TBOS CALC\n");
    printf("========\n");
}

static void display_box(const char* title, const char* content) {
    printf("[%s]\n", title);
    printf("%s\n", content);
}

#elif TBOS_TIER == 1
/* Embedded: Simple text */
static void display_banner(void) {
    printf("\n");
    printf("+------------------+\n");
    printf("|  TBOS Embedded   |\n");
    printf("+------------------+\n");
}

static void display_box(const char* title, const char* content) {
    printf("+-- %s --+\n", title);
    printf("| %s\n", content);
    printf("+----------+\n");
}

#else
/* Desktop+: Full Unicode box drawing */
static void display_banner(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                               ║\n");
    printf("║   ████████╗██████╗  ██████╗ ███████╗                          ║\n");
    printf("║   ╚══██╔══╝██╔══██╗██╔═══██╗██╔════╝                          ║\n");
    printf("║      ██║   ██████╔╝██║   ██║███████╗                          ║\n");
    printf("║      ██║   ██╔══██╗██║   ██║╚════██║                          ║\n");
    printf("║      ██║   ██████╔╝╚██████╔╝███████║                          ║\n");
    printf("║      ╚═╝   ╚═════╝  ╚═════╝ ╚══════╝                          ║\n");
    printf("║                                                               ║\n");
    printf("║        TernaryBit Operating System - Universal Edition        ║\n");
    printf("║           From Calculators to Supercomputers                  ║\n");
    printf("║                                                               ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

static void display_box(const char* title, const char* content) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  %-56s  ║\n", title);
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  %-56s  ║\n", content);
    printf("╚════════════════════════════════════════════════════════════╝\n");
}
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN DEMO
 * ═══════════════════════════════════════════════════════════════════════════ */

static void show_hardware_info(void) {
    char buf[128];

    printf("\n");
#if TBOS_TIER <= 1
    printf("== HARDWARE ==\n");
    printf("Tier: %d (%s)\n", TBOS_TIER, TIER_NAMES[TBOS_TIER]);
    printf("Bits: %s\n", TIER_BITS[TBOS_TIER]);
    printf("RAM:  %s\n", TIER_RAM[TBOS_TIER]);
    printf("Word: %d bytes\n", TBOS_WORD_SIZE);
#else
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Hardware Configuration                                    ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    snprintf(buf, sizeof(buf), "Tier %d: %s", TBOS_TIER, TIER_NAMES[TBOS_TIER]);
    printf("║  %-56s  ║\n", buf);
    snprintf(buf, sizeof(buf), "Architecture: %s", TIER_BITS[TBOS_TIER]);
    printf("║  %-56s  ║\n", buf);
    snprintf(buf, sizeof(buf), "RAM Range: %s", TIER_RAM[TBOS_TIER]);
    printf("║  %-56s  ║\n", buf);
    snprintf(buf, sizeof(buf), "Word Size: %d bytes", TBOS_WORD_SIZE);
    printf("║  %-56s  ║\n", buf);
    snprintf(buf, sizeof(buf), "Max Pool: %lu bytes", (unsigned long)TBOS_MAX_MEMORY);
    printf("║  %-56s  ║\n", buf);

    /* Show capabilities */
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Capabilities:                                             ║\n");
    snprintf(buf, sizeof(buf), "  Heap: %s  Display: %s  Network: %s",
             TBOS_HAS_HEAP ? "Yes" : "No",
             TBOS_HAS_DISPLAY ? "Yes" : "No",
             TBOS_HAS_NETWORK ? "Yes" : "No");
    printf("║  %-56s  ║\n", buf);
#ifdef TBOS_HAS_NUMA
    printf("║    NUMA: Yes                                               ║\n");
#endif
#ifdef TBOS_HAS_DISTRIBUTED
    printf("║    Distributed Memory: Yes                                 ║\n");
#endif
    printf("╚════════════════════════════════════════════════════════════╝\n");
#endif
}

static void show_steppps_status(void) {
    printf("\n");
#if TBOS_TIER <= 1
    printf("== STEPPPS ==\n");
    for (int i = 0; i < (int)STEPPPS_COUNT; i++) {
        printf("%c %s\n",
               g_steppps[i].enabled ? '+' : '-',
               g_steppps[i].name);
    }
#else
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  STEPPPS Framework Status                                  ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    for (int i = 0; i < (int)STEPPPS_COUNT; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s %s: %s",
                 g_steppps[i].enabled ? "✓" : "✗",
                 g_steppps[i].name,
                 g_steppps[i].description);
        printf("║  %-56s  ║\n", buf);
    }
    printf("╚════════════════════════════════════════════════════════════╝\n");
#endif
}

static void show_memory_test(void) {
    printf("\n");
#if TBOS_TIER <= 1
    printf("== MEMORY ==\n");

    /* Allocate some blocks */
    void* p1 = tbos_alloc(16);
    void* p2 = tbos_alloc(32);

    printf("Alloc 16: %s\n", p1 ? "OK" : "FAIL");
    printf("Alloc 32: %s\n", p2 ? "OK" : "FAIL");
    printf("Used: %u/%u bytes\n", (unsigned)g_pool_used, TBOS_MAX_MEMORY);

    if (p1) tbos_free(p1, 16);
    if (p2) tbos_free(p2, 32);

    printf("After free: %u bytes\n", (unsigned)g_pool_used);
#else
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Memory Allocation Test                                    ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");

    /* Test allocations at different sizes */
    size_t sizes[] = {64, 256, 1024, 4096};
    void* ptrs[4];

    for (int i = 0; i < 4; i++) {
        ptrs[i] = tbos_alloc(sizes[i]);
        char buf[64];
        snprintf(buf, sizeof(buf), "Allocate %zu bytes: %s",
                 sizes[i], ptrs[i] ? "SUCCESS" : "FAILED");
        printf("║  %-56s  ║\n", buf);
    }

    char buf[64];
    snprintf(buf, sizeof(buf), "Total allocated: %zu bytes", g_pool_used);
    printf("║  %-56s  ║\n", buf);

    /* Free all */
    for (int i = 0; i < 4; i++) {
        if (ptrs[i]) tbos_free(ptrs[i], sizes[i]);
    }

    snprintf(buf, sizeof(buf), "After free: %zu bytes", g_pool_used);
    printf("║  %-56s  ║\n", buf);
    printf("╚════════════════════════════════════════════════════════════╝\n");
#endif
}

static void show_karma_status(void) {
    printf("\n");
#if TBOS_TIER <= 1
    printf("== KARMA ==\n");
    printf("Balance: %u\n", (unsigned)g_karma);
    printf("Level: %s\n", get_consciousness_level());
#else
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Consciousness Status                                      ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    char buf[64];
    snprintf(buf, sizeof(buf), "Karma Balance: %lu", (unsigned long)g_karma);
    printf("║  %-56s  ║\n", buf);
    snprintf(buf, sizeof(buf), "Consciousness Level: %s", get_consciousness_level());
    printf("║  %-56s  ║\n", buf);
    printf("╚════════════════════════════════════════════════════════════╝\n");
#endif

    /* Add karma for running the demo */
    g_karma += 10;
}

static void show_device_examples(void) {
#if TBOS_TIER >= 2
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Example Devices by Tier                                   ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Tier 0 (Calculator):                                      ║\n");
    printf("║    • TI-84 Calculator (Z80, 128KB)                         ║\n");
    printf("║    • Casio fx-9860 (SH4, 64KB)                             ║\n");
    printf("║    • Arduino Uno (AVR, 2KB)                                ║\n");
    printf("║                                                            ║\n");
    printf("║  Tier 1 (Embedded):                                        ║\n");
    printf("║    • ESP32 (Xtensa, 520KB)                                 ║\n");
    printf("║    • STM32F4 (ARM Cortex-M4, 192KB)                        ║\n");
    printf("║    • MSP430 (16-bit, 16KB)                                 ║\n");
    printf("║                                                            ║\n");
    printf("║  Tier 2 (Retro):                                           ║\n");
    printf("║    • Raspberry Pi Pico (ARM, 264KB)                        ║\n");
    printf("║    • BeagleBone Black (ARM, 512MB)                         ║\n");
    printf("║    • Original Raspberry Pi (ARM, 256MB)                    ║\n");
    printf("║                                                            ║\n");
    printf("║  Tier 3 (Desktop):                                         ║\n");
    printf("║    • Modern PCs (x86-64, 8-64GB)                           ║\n");
    printf("║    • Apple Silicon Macs (ARM64, 8-128GB)                   ║\n");
    printf("║    • Raspberry Pi 4/5 (ARM64, 4-8GB)                       ║\n");
    printf("║                                                            ║\n");
    printf("║  Tier 4 (Server):                                          ║\n");
    printf("║    • Dell PowerEdge (Xeon, 256GB-4TB)                      ║\n");
    printf("║    • AWS EC2 instances (variable)                          ║\n");
    printf("║    • IBM POWER servers (POWER10, 8TB)                      ║\n");
    printf("║                                                            ║\n");
    printf("║  Tier 5 (Supercomputer):                                   ║\n");
    printf("║    • Frontier (AMD, 9.2 PB)                                ║\n");
    printf("║    • Fugaku (ARM, 4.85 PB)                                 ║\n");
    printf("║    • Aurora (Intel, 10.9 PB)                               ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
#endif
}

int main(void) {
    /* Initialize memory pool for constrained tiers */
#if TBOS_TIER <= 1
    memset(g_memory_pool, 0, sizeof(g_memory_pool));
    memset(g_pool_bitmap, 0, sizeof(g_pool_bitmap));
#endif

    /* Display banner */
    display_banner();

    /* Show hardware configuration */
    show_hardware_info();

    /* Show STEPPPS status */
    show_steppps_status();

    /* Memory test */
    show_memory_test();

    /* Karma status */
    show_karma_status();

    /* Device examples (desktop+ only) */
    show_device_examples();

    /* Final message */
    printf("\n");
#if TBOS_TIER <= 1
    printf("TBOS: 4-bit to Supercomputer\n");
    printf("Tier %d active\n", TBOS_TIER);
#else
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  TBOS: From 4-bit Calculators to Supercomputers            ║\n");
    printf("║                                                            ║\n");
    printf("║  \"One OS for all devices, scaling with consciousness\"     ║\n");
    printf("║                                                            ║\n");
    printf("║  Current Tier: %d (%s)                              ║\n",
           TBOS_TIER, TIER_NAMES[TBOS_TIER]);
    printf("╚════════════════════════════════════════════════════════════╝\n");
#endif

    printf("\n");
    return 0;
}
