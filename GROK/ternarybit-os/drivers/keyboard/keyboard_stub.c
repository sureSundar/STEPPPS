#include "tbos/config.h"

#if !CONFIG_KEYBOARD

#include <stdint.h>

void keyboard_init(void) {}
void keyboard_interrupt_handler(void) {}

uint8_t keyboard_read_char(void) {
    return 0;
}

uint8_t keyboard_read_char_poll(void) {
    return 0;
}

#endif /* !CONFIG_KEYBOARD */
