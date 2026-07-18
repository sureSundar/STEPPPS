#include "tbos/hal.h"

#include <stddef.h>
#include <stdint.h>

#ifdef TBOS_HOSTED
/* Hosted builds use kernel/hal_hosted.c */
#else

static volatile uint16_t* const vga_buffer = (uint16_t*)0xB8000;
static const uint32_t vga_width = 80;
static const uint32_t vga_height = 25;

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void hal_baremetal_console_put_char(uint32_t x, uint32_t y, char c, uint8_t color) {
    if (x >= vga_width || y >= vga_height) {
        return;
    }
    vga_buffer[y * vga_width + x] = (uint16_t)((uint16_t)color << 8) | (uint8_t)c;
}

static void hal_baremetal_console_clear(uint8_t color) {
    uint16_t value = ((uint16_t)color << 8) | ' ';
    for (uint32_t i = 0; i < vga_width * vga_height; ++i) {
        vga_buffer[i] = value;
    }
}

static void hal_baremetal_serial_write(char c) {
    while ((inb(0x3FD) & 0x20u) == 0u) {
        /* wait */
    }
    outb(0x3F8, (uint8_t)c);
}

static void hal_baremetal_serial_init(void) {
    outb(0x3F9, 0x00); // Disable interrupts
    outb(0x3FB, 0x80); // Enable DLAB
    outb(0x3F8, 0x01); // Divisor low
    outb(0x3F9, 0x00); // Divisor high
    outb(0x3FB, 0x03); // 8 bits, no parity, one stop
    outb(0x3FA, 0xC7); // Enable FIFO
    outb(0x3FC, 0x0B); // IRQs off, RTS/DSR set
}

static void hal_baremetal_init(void) {
    hal_baremetal_serial_init();
}

static hal_capabilities_t hal_baremetal_capabilities(void) {
    hal_capabilities_t caps = {0};
    caps.has_console = 1;
    caps.has_timer = 0;
    caps.has_input = 0;
    caps.has_storage = 0;
    caps.has_network = 0;
    return caps;
}

static const hal_dispatch_table_t DISPATCH = {
    .init = hal_baremetal_init,
    .capabilities = hal_baremetal_capabilities,
    .console = {
        .put_char_xy = hal_baremetal_console_put_char,
        .clear = hal_baremetal_console_clear,
        .write_serial = hal_baremetal_serial_write,
    },
    .input = {
        .read_key = NULL,
        .set_callback = NULL,
    },
    .timer = {
        .sleep_ms = NULL,
        .busy_wait = NULL,
    },
    .storage = {
        .read_sector = NULL,
        .write_sector = NULL,
    },
};

const hal_dispatch_table_t* hal_get_dispatch(void) {
    return &DISPATCH;
}

#endif /* !TBOS_HOSTED */
