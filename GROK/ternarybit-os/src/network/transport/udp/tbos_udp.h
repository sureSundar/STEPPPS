/**
 * @file tbos_udp.h
 * @brief TernaryBit OS - Conscious UDP Implementation
 *
 * UDP with mindful datagram handling. Supports multicast for Sangha discovery.
 */

#ifndef TBOS_UDP_H
#define TBOS_UDP_H

#include "tbos/net.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSTANTS
 * ═══════════════════════════════════════════════════════════════════════════ */

#define TBOS_UDP_MAX_DATAGRAM     65535
#define TBOS_UDP_DEFAULT_BUFSIZE  8192
#define TBOS_UDP_SANGHA_PORT      7805   /* Sangha discovery port */
#define TBOS_UDP_SANGHA_MCAST     "239.7.8.5"  /* Sangha multicast group */

/* ═══════════════════════════════════════════════════════════════════════════
 * STRUCTURES
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    int sock;
    uint16_t local_port;
    bool bound;
    bool multicast_enabled;
    char multicast_group[64];

    /* Stats */
    uint64_t datagrams_sent;
    uint64_t datagrams_recv;
    uint64_t bytes_sent;
    uint64_t bytes_recv;
    int64_t karma;
} tbos_udp_socket_t;

typedef struct {
    char from_addr[64];
    uint16_t from_port;
    size_t length;
    uint8_t data[];  /* Flexible array */
} tbos_udp_datagram_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize UDP subsystem
 */
int tbos_udp_init(void);

/**
 * @brief Create UDP socket
 * @return Socket handle or error
 */
int tbos_udp_socket(void);

/**
 * @brief Bind to local port
 */
int tbos_udp_bind(int sock, uint16_t port);

/**
 * @brief Send datagram
 */
ssize_t tbos_udp_sendto(int sock, const void* data, size_t len,
                        const char* host, uint16_t port);

/**
 * @brief Receive datagram
 */
ssize_t tbos_udp_recvfrom(int sock, void* buf, size_t len,
                          char* from_addr, uint16_t* from_port);

/**
 * @brief Join multicast group
 */
int tbos_udp_join_multicast(int sock, const char* group);

/**
 * @brief Leave multicast group
 */
int tbos_udp_leave_multicast(int sock, const char* group);

/**
 * @brief Send to Sangha multicast group
 */
int tbos_udp_sangha_announce(int sock, const void* data, size_t len);

/**
 * @brief Close UDP socket
 */
int tbos_udp_close(int sock);

/**
 * @brief Get socket info
 */
const tbos_udp_socket_t* tbos_udp_get_socket(int sock);

/**
 * @brief Print UDP status
 */
void tbos_udp_print_status(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_UDP_H */
