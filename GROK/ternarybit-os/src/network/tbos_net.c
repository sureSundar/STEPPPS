/**
 * @file tbos_net.c
 * @brief TernaryBit OS - Unified Network API Implementation
 *
 * Device-agnostic socket wrapper using HAL for platform operations.
 * Integrates with consciousness/karma system.
 */

#include "tbos/net.h"
#include "tbos/hal.h"
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * INTERNAL STATE
 * ═══════════════════════════════════════════════════════════════════════════ */

static int g_net_initialized = 0;
static tbos_socket_t g_sockets[TBOS_NET_MAX_SOCKETS];
static int g_socket_count = 0;
static uint64_t g_total_bytes_sent = 0;
static uint64_t g_total_bytes_recv = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * HELPER FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static tbos_socket_t* find_socket(int sock) {
    if (sock < 0 || sock >= TBOS_NET_MAX_SOCKETS) return NULL;
    if (g_sockets[sock].fd < 0) return NULL;
    return &g_sockets[sock];
}

static int alloc_socket_slot(void) {
    for (int i = 0; i < TBOS_NET_MAX_SOCKETS; i++) {
        if (g_sockets[i].fd < 0) {
            return i;
        }
    }
    return -1;
}

static const hal_dispatch_table_t* hal(void) {
    return hal_get_dispatch();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CORE API IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_net_init(void) {
    if (g_net_initialized) return TBOS_NET_SUCCESS;

    for (int i = 0; i < TBOS_NET_MAX_SOCKETS; i++) {
        g_sockets[i].fd = -1;
    }

    g_socket_count = 0;
    g_total_bytes_sent = 0;
    g_total_bytes_recv = 0;
    g_net_initialized = 1;

    return TBOS_NET_SUCCESS;
}

void tbos_net_shutdown(void) {
    for (int i = 0; i < TBOS_NET_MAX_SOCKETS; i++) {
        if (g_sockets[i].fd >= 0) {
            tbos_net_close(i);
        }
    }
    g_net_initialized = 0;
}

int tbos_net_socket(int type) {
    if (!g_net_initialized) tbos_net_init();

    int slot = alloc_socket_slot();
    if (slot < 0) return TBOS_NET_ERROR;

    int domain = AF_INET;
    int sock_type = (type == TBOS_SOCK_UDP) ? SOCK_DGRAM : SOCK_STREAM;
    int protocol = 0;

    int fd = hal()->network.socket(domain, sock_type, protocol);
    if (fd < 0) return TBOS_NET_ERROR;

    memset(&g_sockets[slot], 0, sizeof(tbos_socket_t));
    g_sockets[slot].fd = fd;
    g_sockets[slot].type = type;
    g_sockets[slot].karma = 0;
    g_socket_count++;

    return slot;
}

int tbos_net_connect(int sock, const char* host, uint16_t port) {
    tbos_socket_t* s = find_socket(sock);
    if (!s || !host) return TBOS_NET_ERROR;

    int result = hal()->network.connect(s->fd, host, port);
    if (result < 0) return TBOS_NET_ERROR;

    strncpy(s->remote_host, host, TBOS_NET_MAX_HOST - 1);
    s->remote_port = port;
    s->connected = 1;
    s->karma += 5;  /* Good karma for forming connection */

    return TBOS_NET_SUCCESS;
}

int tbos_net_bind(int sock, uint16_t port) {
    tbos_socket_t* s = find_socket(sock);
    if (!s) return TBOS_NET_ERROR;

    int result = hal()->network.bind(s->fd, port);
    if (result < 0) return TBOS_NET_ERROR;

    s->local_port = port;
    return TBOS_NET_SUCCESS;
}

int tbos_net_listen(int sock, int backlog) {
    tbos_socket_t* s = find_socket(sock);
    if (!s) return TBOS_NET_ERROR;

    int result = hal()->network.listen(s->fd, backlog);
    if (result < 0) return TBOS_NET_ERROR;

    s->listening = 1;
    s->karma += 10;  /* Good karma for opening to connections */
    return TBOS_NET_SUCCESS;
}

int tbos_net_accept(int sock) {
    tbos_socket_t* s = find_socket(sock);
    if (!s || !s->listening) return TBOS_NET_ERROR;

    int slot = alloc_socket_slot();
    if (slot < 0) return TBOS_NET_ERROR;

    char remote_addr[TBOS_NET_MAX_ADDR];
    uint16_t remote_port = 0;

    int fd = hal()->network.accept(s->fd, remote_addr, &remote_port);
    if (fd < 0) return TBOS_NET_ERROR;

    memset(&g_sockets[slot], 0, sizeof(tbos_socket_t));
    g_sockets[slot].fd = fd;
    g_sockets[slot].type = s->type;
    g_sockets[slot].connected = 1;
    g_sockets[slot].remote_port = remote_port;
    strncpy(g_sockets[slot].remote_host, remote_addr, TBOS_NET_MAX_HOST - 1);
    g_sockets[slot].karma = 5;  /* Inherit some karma */
    g_socket_count++;

    s->karma += 5;  /* Server gains karma for accepting */
    return slot;
}

ssize_t tbos_net_send(int sock, const void* data, size_t len) {
    tbos_socket_t* s = find_socket(sock);
    if (!s || !data) return TBOS_NET_ERROR;

    ssize_t sent = hal()->network.send(s->fd, data, len);
    if (sent > 0) {
        s->bytes_sent += (uint64_t)sent;
        s->packets_sent++;
        g_total_bytes_sent += (uint64_t)sent;
        s->karma += 1;  /* Small karma for communication */
    }
    return sent;
}

ssize_t tbos_net_recv(int sock, void* buf, size_t len) {
    tbos_socket_t* s = find_socket(sock);
    if (!s || !buf) return TBOS_NET_ERROR;

    ssize_t received = hal()->network.recv(s->fd, buf, len);
    if (received > 0) {
        s->bytes_recv += (uint64_t)received;
        s->packets_recv++;
        g_total_bytes_recv += (uint64_t)received;
    }
    return received;
}

ssize_t tbos_net_sendto(int sock, const void* data, size_t len,
                        const char* host, uint16_t port) {
    tbos_socket_t* s = find_socket(sock);
    if (!s || !data || !host) return TBOS_NET_ERROR;

    ssize_t sent = hal()->network.sendto(s->fd, data, len, host, port);
    if (sent > 0) {
        s->bytes_sent += (uint64_t)sent;
        s->packets_sent++;
        g_total_bytes_sent += (uint64_t)sent;
        s->karma += 1;
    }
    return sent;
}

ssize_t tbos_net_recvfrom(int sock, void* buf, size_t len,
                          char* from_host, uint16_t* from_port) {
    tbos_socket_t* s = find_socket(sock);
    if (!s || !buf) return TBOS_NET_ERROR;

    ssize_t received = hal()->network.recvfrom(s->fd, buf, len, from_host, from_port);
    if (received > 0) {
        s->bytes_recv += (uint64_t)received;
        s->packets_recv++;
        g_total_bytes_recv += (uint64_t)received;
    }
    return received;
}

int tbos_net_close(int sock) {
    tbos_socket_t* s = find_socket(sock);
    if (!s) return TBOS_NET_ERROR;

    hal()->network.close(s->fd);
    s->fd = -1;
    g_socket_count--;

    return TBOS_NET_SUCCESS;
}

int tbos_net_set_nonblocking(int sock, int enabled) {
    tbos_socket_t* s = find_socket(sock);
    if (!s) return TBOS_NET_ERROR;

    int result = hal()->network.set_nonblocking(s->fd, enabled);
    if (result == 0) {
        s->nonblocking = enabled;
    }
    return result;
}

int tbos_net_get_local_ip(char* buf, size_t len) {
    if (!buf || len == 0) return TBOS_NET_ERROR;
    return hal()->network.get_local_ip(buf, len);
}

const tbos_socket_t* tbos_net_get_socket(int sock) {
    return find_socket(sock);
}

void tbos_net_stats(uint64_t* total_sent, uint64_t* total_recv, int* active_sockets) {
    if (total_sent) *total_sent = g_total_bytes_sent;
    if (total_recv) *total_recv = g_total_bytes_recv;
    if (active_sockets) *active_sockets = g_socket_count;
}
