#ifndef TBOS_PS2_H
#define TBOS_PS2_H

#include <stdint.h>
#include <stdbool.h>

// WINDSURF: Minimal PS/2 keyboard interface
void ps2_init(void);
bool ps2_has_key(void);
uint8_t ps2_read_scancode(void);
char ps2_translate(uint8_t sc);

#endif // TBOS_PS2_H
