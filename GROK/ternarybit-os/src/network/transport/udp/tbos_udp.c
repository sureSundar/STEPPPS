/**
 * @file tbos_udp.c
 * @brief TernaryBit OS - Conscious UDP Implementation
 */

#include "tbos_udp.h"
#include "tbos/net.h"
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * INTERNAL STATE
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_UDP_SOCKETS 64

static bool g_udp_initialized = false;
static tbos_udp_socket_t g_udp_sockets[MAX_UDP_SOCKETS];
static uint64_t g_total_datagrams_sent = 0;
static uint64_t g_total_datagrams_recv = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

static tbos_udp_socket_t* find_udp_socket(int sock) {
    if (sock < 0 || sock >= MAX_UDP_SOCKETS) return NULL;
    if (g_udp_sockets[sock].sock < 0) return NULL;
    return &g_udp_sockets[sock];
}

static int alloc_udp_slot(void) {
    for (int i = 0; i < MAX_UDP_SOCKETS; i++) {
        if (g_udp_sockets[i].sock < 0) return i;
    }
    return -1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * API IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_udp_init(void) {
    if (g_udp_initialized) return TBOS_NET_SUCCESS;

    tbos_net_init();

    for (int i = 0; i < MAX_UDP_SOCKETS; i++) {
        g_udp_sockets[i].sock = -1;
    }

    g_udp_initialized = true;
    return TBOS_NET_SUCCESS;
}

int tbos_udp_socket(void) {
    if (!g_udp_initialized) tbos_udp_init();

    int slot = alloc_udp_slot();
    if (slot < 0) return TBOS_NET_ERROR;

    int sock = tbos_net_socket(TBOS_SOCK_UDP);
    if (sock < 0) return TBOS_NET_ERROR;

    memset(&g_udp_sockets[slot], 0, sizeof(tbos_udp_socket_t));
    g_udp_sockets[slot].sock = sock;

    return slot;
}

int tbos_udp_bind(int sock, uint16_t port) {
    tbos_udp_socket_t* s = find_udp_socket(sock);
    if (!s) return TBOS_NET_ERROR;

    int result = tbos_net_bind(s->sock, port);
    if (result == TBOS_NET_SUCCESS) {
        s->local_port = port;
        s->bound = true;
    }
    return result;
}

ssize_t tbos_udp_sendto(int sock, const void* data, size_t len,
                        const char* host, uint16_t port) {
    tbos_udp_socket_t* s = find_udp_socket(sock);
    if (!s || !data || !host) return TBOS_NET_ERROR;

    ssize_t sent = tbos_net_sendto(s->sock, data, len, host, port);
    if (sent > 0) {
        s->datagrams_sent++;
        s->bytes_sent += (uint64_t)sent;
        s->karma += 1;
        g_total_datagrams_sent++;
    }
    return sent;
}

ssize_t tbos_udp_recvfrom(int sock, void* buf, size_t len,
                          char* from_addr, uint16_t* from_port) {
    tbos_udp_socket_t* s = find_udp_socket(sock);
    if (!s || !buf) return TBOS_NET_ERROR;

    ssize_t received = tbos_net_recvfrom(s->sock, buf, len, from_addr, from_port);
    if (received > 0) {
        s->datagrams_recv++;
        s->bytes_recv += (uint64_t)received;
        g_total_datagrams_recv++;
    }
    return received;
}

int tbos_udp_join_multicast(int sock, const char* group) {
    tbos_udp_socket_t* s = find_udp_socket(sock);
    if (!s || !group) return TBOS_NET_ERROR;

    /* Get underlying socket fd from tbos_net layer */
    const tbos_socket_t* net_sock = tbos_net_get_socket(s->sock);
    if (!net_sock) return TBOS_NET_ERROR;

    int fd = net_sock->fd;

    /* Set up multicast group membership */
    struct ip_mreq mreq;
    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(group);
    mreq.imr_interface.s_addr = INADDR_ANY;

    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        printf("[UDP] Failed to join multicast group %s\n", group);
        return TBOS_NET_ERROR;
    }

    /* Track the membership */
    strncpy(s->multicast_group, group, sizeof(s->multicast_group) - 1);
    s->multicast_group[sizeof(s->multicast_group) - 1] = '\0';
    s->multicast_enabled = true;
    s->karma += 5;  /* Good karma for joining community */

    printf("[UDP] Joined multicast group %s\n", group);
    return TBOS_NET_SUCCESS;
}

int tbos_udp_leave_multicast(int sock, const char* group) {
    tbos_udp_socket_t* s = find_udp_socket(sock);
    if (!s) return TBOS_NET_ERROR;

    /* Get underlying socket fd from tbos_net layer */
    const tbos_socket_t* net_sock = tbos_net_get_socket(s->sock);
    if (!net_sock) return TBOS_NET_ERROR;

    int fd = net_sock->fd;

    /* Use stored group if not specified */
    const char* leave_group = (group && group[0]) ? group : s->multicast_group;
    if (!leave_group[0]) return TBOS_NET_ERROR;

    /* Leave multicast group */
    struct ip_mreq mreq;
    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(leave_group);
    mreq.imr_interface.s_addr = INADDR_ANY;

    if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        printf("[UDP] Failed to leave multicast group %s\n", leave_group);
        return TBOS_NET_ERROR;
    }

    s->multicast_enabled = false;
    s->multicast_group[0] = '\0';

    printf("[UDP] Left multicast group %s\n", leave_group);
    return TBOS_NET_SUCCESS;
}

int tbos_udp_sangha_announce(int sock, const void* data, size_t len) {
    tbos_udp_socket_t* s = find_udp_socket(sock);
    if (!s || !data) return TBOS_NET_ERROR;

    ssize_t sent = tbos_udp_sendto(sock, data, len,
                                   TBOS_UDP_SANGHA_MCAST, TBOS_UDP_SANGHA_PORT);
    if (sent > 0) {
        s->karma += 10;  /* Extra karma for Sangha participation */
    }
    return (sent > 0) ? TBOS_NET_SUCCESS : TBOS_NET_ERROR;
}

int tbos_udp_close(int sock) {
    tbos_udp_socket_t* s = find_udp_socket(sock);
    if (!s) return TBOS_NET_ERROR;

    tbos_net_close(s->sock);
    s->sock = -1;

    return TBOS_NET_SUCCESS;
}

const tbos_udp_socket_t* tbos_udp_get_socket(int sock) {
    return find_udp_socket(sock);
}

void tbos_udp_print_status(void) {
    int active = 0;
    int64_t total_karma = 0;

    for (int i = 0; i < MAX_UDP_SOCKETS; i++) {
        if (g_udp_sockets[i].sock >= 0) {
            active++;
            total_karma += g_udp_sockets[i].karma;
        }
    }

    printf("\n");
    printf("UDP Status:\n");
    printf("  Active sockets:    %d\n", active);
    printf("  Datagrams sent:    %lu\n", g_total_datagrams_sent);
    printf("  Datagrams recv:    %lu\n", g_total_datagrams_recv);
    printf("  Collective karma:  %ld\n", total_karma);
    printf("\n");
}
