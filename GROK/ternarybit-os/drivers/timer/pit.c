// WINDSURF: Minimal PIT stub (no real IRQ wiring yet)
#include "pit.h"

static volatile uint64_t g_ticks = 0;

void pit_isr_tick(void) { // to be called from IRQ0 handler later
    g_ticks++;
}

void pit_init(uint32_t hz) {
    (void)hz; // TODO: program PIT 8254 via ports 0x40-0x43
    g_ticks = 0;
}

uint64_t pit_ticks(void) {
    return g_ticks;
}

void pit_sleep(uint32_t ms) {
    uint64_t target = g_ticks + (ms); // assuming 1kHz later; stub now
    while (g_ticks < target) {
        // busy wait stub
    }
}
