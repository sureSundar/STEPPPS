/**
 * @file net.h
 * @brief TernaryBit OS - Unified Network API
 *
 * Device-agnostic socket wrapper integrating with consciousness/karma system.
 * Uses HAL dispatch for platform-specific implementations.
 *
 * @version 1.0
 */

#ifndef TBOS_NET_H
#define TBOS_NET_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSTANTS
 * ═══════════════════════════════════════════════════════════════════════════ */

#define TBOS_NET_SUCCESS       0
#define TBOS_NET_ERROR        -1
#define TBOS_NET_TIMEOUT      -2
#define TBOS_NET_REFUSED      -3
#define TBOS_NET_UNREACHABLE  -4

#define TBOS_SOCK_TCP          1
#define TBOS_SOCK_UDP          2

#define TBOS_NET_MAX_SOCKETS   256
#define TBOS_NET_MAX_HOST      256
#define TBOS_NET_MAX_ADDR      64

/* ═══════════════════════════════════════════════════════════════════════════
 * SOCKET STRUCTURE
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    int fd;                     /* Underlying file descriptor */
    int type;                   /* TBOS_SOCK_TCP or TBOS_SOCK_UDP */
    uint16_t local_port;
    uint16_t remote_port;
    char remote_host[TBOS_NET_MAX_HOST];

    /* State */
    int connected;
    int listening;
    int nonblocking;

    /* Consciousness metrics */
    int64_t karma;
    uint64_t bytes_sent;
    uint64_t bytes_recv;
    uint64_t packets_sent;
    uint64_t packets_recv;
} tbos_socket_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * CORE API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize network subsystem
 * @return TBOS_NET_SUCCESS or error code
 */
int tbos_net_init(void);

/**
 * @brief Shutdown network subsystem
 */
void tbos_net_shutdown(void);

/**
 * @brief Create a socket
 * @param type TBOS_SOCK_TCP or TBOS_SOCK_UDP
 * @return Socket handle (>=0) or error code
 */
int tbos_net_socket(int type);

/**
 * @brief Connect to remote host
 * @param sock Socket handle
 * @param host Hostname or IP address
 * @param port Port number
 * @return TBOS_NET_SUCCESS or error code
 */
int tbos_net_connect(int sock, const char* host, uint16_t port);

/**
 * @brief Bind socket to local port
 * @param sock Socket handle
 * @param port Port number (0 for any)
 * @return TBOS_NET_SUCCESS or error code
 */
int tbos_net_bind(int sock, uint16_t port);

/**
 * @brief Listen for connections
 * @param sock Socket handle
 * @param backlog Connection queue size
 * @return TBOS_NET_SUCCESS or error code
 */
int tbos_net_listen(int sock, int backlog);

/**
 * @brief Accept incoming connection
 * @param sock Listening socket handle
 * @return New socket handle or error code
 */
int tbos_net_accept(int sock);

/**
 * @brief Send data (TCP)
 * @param sock Socket handle
 * @param data Data buffer
 * @param len Data length
 * @return Bytes sent or error code
 */
ssize_t tbos_net_send(int sock, const void* data, size_t len);

/**
 * @brief Receive data (TCP)
 * @param sock Socket handle
 * @param buf Receive buffer
 * @param len Buffer size
 * @return Bytes received or error code
 */
ssize_t tbos_net_recv(int sock, void* buf, size_t len);

/**
 * @brief Send datagram (UDP)
 * @param sock Socket handle
 * @param data Data buffer
 * @param len Data length
 * @param host Destination host
 * @param port Destination port
 * @return Bytes sent or error code
 */
ssize_t tbos_net_sendto(int sock, const void* data, size_t len,
                        const char* host, uint16_t port);

/**
 * @brief Receive datagram (UDP)
 * @param sock Socket handle
 * @param buf Receive buffer
 * @param len Buffer size
 * @param from_host Source host (output, can be NULL)
 * @param from_port Source port (output, can be NULL)
 * @return Bytes received or error code
 */
ssize_t tbos_net_recvfrom(int sock, void* buf, size_t len,
                          char* from_host, uint16_t* from_port);

/**
 * @brief Close socket
 * @param sock Socket handle
 * @return TBOS_NET_SUCCESS or error code
 */
int tbos_net_close(int sock);

/**
 * @brief Set socket non-blocking mode
 * @param sock Socket handle
 * @param enabled 1 for non-blocking, 0 for blocking
 * @return TBOS_NET_SUCCESS or error code
 */
int tbos_net_set_nonblocking(int sock, int enabled);

/**
 * @brief Get local IP address
 * @param buf Output buffer
 * @param len Buffer size
 * @return TBOS_NET_SUCCESS or error code
 */
int tbos_net_get_local_ip(char* buf, size_t len);

/**
 * @brief Get socket info
 * @param sock Socket handle
 * @return Pointer to socket structure or NULL
 */
const tbos_socket_t* tbos_net_get_socket(int sock);

/**
 * @brief Get network statistics
 * @param total_sent Output: total bytes sent
 * @param total_recv Output: total bytes received
 * @param active_sockets Output: number of active sockets
 */
void tbos_net_stats(uint64_t* total_sent, uint64_t* total_recv, int* active_sockets);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_NET_H */
