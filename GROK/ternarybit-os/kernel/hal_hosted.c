#ifdef TBOS_HOSTED

/* System headers first for hosted mode */
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>
#include <ifaddrs.h>

/* TBOS headers after system headers */
#include "tbos/hal.h"

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
    caps.has_network = 1;
    caps.has_dynamic_memory = 1;  /* Real heap available (host libc). */
    return caps;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MEMORY OPERATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* hal_hosted_mem_alloc(size_t size) {
    return malloc(size);
}

static void* hal_hosted_mem_realloc(void* ptr, size_t new_size) {
    return realloc(ptr, new_size);
}

static void hal_hosted_mem_free(void* ptr) {
    free(ptr);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * NETWORK OPERATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int hal_hosted_net_socket(int domain, int type, int protocol) {
    return socket(domain, type, protocol);
}

static int hal_hosted_net_connect(int fd, const char* host, uint16_t port) {
    struct sockaddr_in addr;
    struct hostent* he;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    he = gethostbyname(host);
    if (he) {
        memcpy(&addr.sin_addr, he->h_addr_list[0], (size_t)he->h_length);
    } else if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
        return -1;
    }

    return connect(fd, (struct sockaddr*)&addr, sizeof(addr));
}

static int hal_hosted_net_bind(int fd, uint16_t port) {
    struct sockaddr_in addr;
    int opt = 1;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    return bind(fd, (struct sockaddr*)&addr, sizeof(addr));
}

static int hal_hosted_net_listen(int fd, int backlog) {
    return listen(fd, backlog);
}

static int hal_hosted_net_accept(int fd, char* remote_addr, uint16_t* remote_port) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    int client_fd = accept(fd, (struct sockaddr*)&addr, &addrlen);
    if (client_fd >= 0) {
        if (remote_addr) {
            inet_ntop(AF_INET, &addr.sin_addr, remote_addr, INET_ADDRSTRLEN);
        }
        if (remote_port) {
            *remote_port = ntohs(addr.sin_port);
        }
    }
    return client_fd;
}

static ssize_t hal_hosted_net_send(int fd, const void* buf, size_t len) {
    return send(fd, buf, len, 0);
}

static ssize_t hal_hosted_net_recv(int fd, void* buf, size_t len) {
    return recv(fd, buf, len, 0);
}

static int hal_hosted_net_close(int fd) {
    return close(fd);
}

static int hal_hosted_net_set_nonblocking(int fd, int enabled) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;

    if (enabled) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    return fcntl(fd, F_SETFL, flags);
}

static int hal_hosted_net_get_local_ip(char* buf, size_t len) {
    struct ifaddrs* ifaddr;
    struct ifaddrs* ifa;

    if (getifaddrs(&ifaddr) == -1) return -1;

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;
        if (strcmp(ifa->ifa_name, "lo") == 0) continue;

        struct sockaddr_in* addr = (struct sockaddr_in*)ifa->ifa_addr;
        inet_ntop(AF_INET, &addr->sin_addr, buf, len);
        freeifaddrs(ifaddr);
        return 0;
    }

    freeifaddrs(ifaddr);
    strncpy(buf, "127.0.0.1", len);
    return 0;
}

static ssize_t hal_hosted_net_sendto(int fd, const void* buf, size_t len,
                                      const char* host, uint16_t port) {
    struct sockaddr_in addr;
    struct hostent* he;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    he = gethostbyname(host);
    if (he) {
        memcpy(&addr.sin_addr, he->h_addr_list[0], (size_t)he->h_length);
    } else if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
        return -1;
    }

    return sendto(fd, buf, len, 0, (struct sockaddr*)&addr, sizeof(addr));
}

static ssize_t hal_hosted_net_recvfrom(int fd, void* buf, size_t len,
                                        char* from_addr, uint16_t* from_port) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    ssize_t n = recvfrom(fd, buf, len, 0, (struct sockaddr*)&addr, &addrlen);
    if (n >= 0) {
        if (from_addr) {
            inet_ntop(AF_INET, &addr.sin_addr, from_addr, INET_ADDRSTRLEN);
        }
        if (from_port) {
            *from_port = ntohs(addr.sin_port);
        }
    }
    return n;
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
    .network = {
        .socket = hal_hosted_net_socket,
        .connect = hal_hosted_net_connect,
        .bind = hal_hosted_net_bind,
        .listen = hal_hosted_net_listen,
        .accept = hal_hosted_net_accept,
        .send = hal_hosted_net_send,
        .recv = hal_hosted_net_recv,
        .close = hal_hosted_net_close,
        .set_nonblocking = hal_hosted_net_set_nonblocking,
        .get_local_ip = hal_hosted_net_get_local_ip,
        .sendto = hal_hosted_net_sendto,
        .recvfrom = hal_hosted_net_recvfrom,
    },
    .memory = {
        .alloc = hal_hosted_mem_alloc,
        .realloc = hal_hosted_mem_realloc,
        .free = hal_hosted_mem_free,
    },
};

const hal_dispatch_table_t* hal_get_dispatch(void) {
    return &DISPATCH;
}

#endif /* TBOS_HOSTED */
