#ifdef TBOS_HOSTED

#include "tbos/hal.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

static struct termios saved_termios;
static int termios_configured = 0;

static void hal_hosted_restore_terminal(void) {
    if (termios_configured) {
        tcsetattr(STDIN_FILENO, TCSANOW, &saved_termios);
        termios_configured = 0;
    }
    printf("\033[0m\033[?25h");
    fflush(stdout);
}

static void hal_hosted_console_put_char(uint32_t x, uint32_t y, char c, uint8_t color) {
    (void)color;
    printf("\033[%u;%uH%c", (unsigned)(y + 1), (unsigned)(x + 1), c);
    fflush(stdout);
}

static void hal_hosted_console_clear(uint8_t color) {
    (void)color;
    printf("\033[2J\033[H");
    fflush(stdout);
}

static void hal_hosted_write_serial(char c) {
    fputc(c, stderr);
    fflush(stderr);
}

static int hal_hosted_read_key(void) {
    unsigned char ch = 0;
    fd_set readfds;
    struct timeval tv = {0};
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    int ready = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
    if (ready > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
        ssize_t n = read(STDIN_FILENO, &ch, 1);
        if (n == 1) {
            return (int)ch;
        }
    }
    return -1;
}

static void hal_hosted_set_callback(void (*handler)(int)) {
    (void)handler;
    /* TODO: integrate with event loop */
}

static void hal_hosted_sleep_ms(uint32_t ms) {
    usleep(ms * 1000u);
}

static void hal_hosted_busy_wait(uint32_t cycles) {
    /* Approximate: treat cycles as microseconds */
    usleep(cycles);
}

static void hal_hosted_init(void) {
    if (!termios_configured && tcgetattr(STDIN_FILENO, &saved_termios) == 0) {
        struct termios raw = saved_termios;
        raw.c_lflag &= ~(ECHO | ICANON);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0) {
            termios_configured = 1;
            atexit(hal_hosted_restore_terminal);
            printf("\033[?25l");
        }
    }
    hal_hosted_console_clear(0);
}

static hal_capabilities_t hal_hosted_capabilities(void) {
    hal_capabilities_t caps = {0};
    caps.has_console = 1;
    caps.has_input = 1;
    caps.has_timer = 1;
    return caps;
}

static const hal_dispatch_table_t DISPATCH = {
    .init = hal_hosted_init,
    .capabilities = hal_hosted_capabilities,
    .console = {
        .put_char_xy = hal_hosted_console_put_char,
        .clear = hal_hosted_console_clear,
        .write_serial = hal_hosted_write_serial,
    },
    .input = {
        .read_key = hal_hosted_read_key,
        .set_callback = hal_hosted_set_callback,
    },
    .timer = {
        .sleep_ms = hal_hosted_sleep_ms,
        .busy_wait = hal_hosted_busy_wait,
    },
    .storage = {
        .read_sector = NULL,
        .write_sector = NULL,
    },
};

const hal_dispatch_table_t* hal_get_dispatch(void) {
    return &DISPATCH;
}

#endif /* TBOS_HOSTED */
