#ifndef TBOS_PIT_H
#define TBOS_PIT_H

#include <stdint.h>

// WINDSURF: Minimal PIT interface
void pit_init(uint32_t hz);
uint64_t pit_ticks(void);
void pit_sleep(uint32_t ms);

#endif // TBOS_PIT_H
