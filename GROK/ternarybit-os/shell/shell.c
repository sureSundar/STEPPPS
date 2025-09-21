// WINDSURF: Minimal shell loop stub
#include "shell.h"

extern void vga_write(const char* s);

void shell_init(void) {
    vga_write("TBOS Shell v0.0.1\n> ");
}

void shell_loop(void) {
    // TODO: integrate with keyboard driver and parser
    // For now, just idle
    for (;;) {
        // spin
    }
}
