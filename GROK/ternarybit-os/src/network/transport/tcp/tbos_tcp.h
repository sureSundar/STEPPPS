/**
 * @file tbos_tcp.h
 * @brief TernaryBit OS - Conscious TCP Implementation
 *
 * TCP with Digital Sangha awareness - every connection is a conscious
 * relationship between devices practicing compassionate communication.
 *
 * Philosophy:
 * - Connections are relationships, not just data pipes
 * - Three-way handshake is mutual recognition
 * - Reliable delivery is compassionate persistence
 * - Flow control is mindful resource sharing
 * - Congestion control is collective awareness
 *
 * @version 1.0
 * @date 2025-11-03
 */

#ifndef TBOS_TCP_H
#define TBOS_TCP_H

#include "../../core/tbos_network.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* TCP CONSTANTS                                                             */
/* ========================================================================= */

#define TBOS_TCP_MAX_CONNECTIONS    256
#define TBOS_TCP_DEFAULT_MSS        1460  /* Maximum Segment Size */
#define TBOS_TCP_DEFAULT_WINDOW     65535 /* Receive window size */
#define TBOS_TCP_MAX_RETRIES        5     /* Maximum retransmission attempts */

/* ========================================================================= */
/* TCP STATE MACHINE                                                         */
/* ========================================================================= */

/**
 * @brief TCP connection states (RFC 793)
 * Each state represents a stage in the conscious relationship
 */
typedef enum {
    TCP_STATE_CLOSED,        /**< No connection - solitude */
    TCP_STATE_LISTEN,        /**< Waiting for connection - openness */
    TCP_STATE_SYN_SENT,      /**< SYN sent - reaching out */
    TCP_STATE_SYN_RECEIVED,  /**< SYN received - acknowledgment */
    TCP_STATE_ESTABLISHED,   /**< Connection active - relationship */
    TCP_STATE_FIN_WAIT_1,    /**< FIN sent - beginning separation */
    TCP_STATE_FIN_WAIT_2,    /**< FIN acked - mutual understanding */
    TCP_STATE_CLOSE_WAIT,    /**< Remote FIN - respecting closure */
    TCP_STATE_CLOSING,       /**< Both FINs - synchronized ending */
    TCP_STATE_LAST_ACK,      /**< Final ACK wait - graceful goodbye */
    TCP_STATE_TIME_WAIT,     /**< 2MSL wait - contemplation period */
} tbos_tcp_state_t;

/* ========================================================================= */
/* CONSCIOUS TCP CONNECTION                                                  */
/* ========================================================================= */

/**
 * @brief Conscious TCP connection
 * Maintains both traditional TCP state and consciousness awareness
 */
typedef struct tbos_tcp_connection {
    /* Connection Identity */
    uint32_t connection_id;

    /* TCP 4-tuple */
    uint32_t local_ip;
    uint16_t local_port;
    uint32_t remote_ip;
    uint16_t remote_port;

    /* TCP State */
    tbos_tcp_state_t state;

    /* Sequence Numbers */
    uint32_t send_seq;           /**< Next sequence number to send */
    uint32_t recv_seq;           /**< Next expected sequence number */
    uint32_t send_una;           /**< Oldest unacknowledged sequence */
    uint32_t send_window;        /**< Send window size */
    uint32_t recv_window;        /**< Receive window size */

    /* Retransmission */
    uint32_t rto_ms;             /**< Retransmission timeout (ms) */
    uint8_t retries;             /**< Retransmission attempts */

    /* Consciousness Properties */
    consciousness_level_t awareness;
    karma_score_t connection_karma;
    bool practicing_flow_control;     /**< Mindful sending */
    bool practicing_congestion_control; /**< Collective awareness */

    /* Relationship Metrics */
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint64_t packets_sent;
    uint64_t packets_received;
    uint64_t retransmissions;
    uint64_t connection_time_ms;

    /* Compassionate Behavior */
    bool peer_seems_slow;        /**< Reduce sending rate */
    bool peer_seems_overwhelmed; /**< Extra patience */
    uint32_t compassion_wait_ms; /**< Additional delay for struggling peer */

    /* Buffers */
    void* send_buffer;
    void* recv_buffer;
    size_t send_buffer_size;
    size_t recv_buffer_size;

} tbos_tcp_connection_t;

/* ========================================================================= */
/* CONSCIOUS TCP SEGMENT                                                     */
/* ========================================================================= */

/**
 * @brief TCP segment with consciousness
 */
typedef struct {
    /* Standard TCP Header */
    uint16_t source_port;
    uint16_t dest_port;
    uint32_t sequence_num;
    uint32_t ack_num;
    uint8_t data_offset;     /**< Header length in 32-bit words */
    uint8_t flags;           /**< Control flags */
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent_pointer;

    /* Data */
    void* data;
    size_t data_length;

    /* Consciousness */
    bool follows_right_speech;
    karma_score_t segment_karma;

} tbos_tcp_segment_t;

/* TCP Flags */
#define TCP_FLAG_FIN  0x01
#define TCP_FLAG_SYN  0x02
#define TCP_FLAG_RST  0x04
#define TCP_FLAG_PSH  0x08
#define TCP_FLAG_ACK  0x10
#define TCP_FLAG_URG  0x20

/* ========================================================================= */
/* TCP API - CONSCIOUS COMMUNICATION                                         */
/* ========================================================================= */

/**
 * @brief Initialize conscious TCP subsystem
 */
int tbos_tcp_init(void);

/**
 * @brief Create TCP socket with consciousness
 *
 * @return Socket descriptor or negative error code
 */
int tbos_tcp_socket(void);

/**
 * @brief Bind socket to local address
 */
int tbos_tcp_bind(int sockfd, uint32_t local_ip, uint16_t local_port);

/**
 * @brief Listen for connections with open heart
 *
 * Opens awareness to incoming connection requests.
 * Practicing receptive consciousness.
 */
int tbos_tcp_listen(int sockfd, int backlog);

/**
 * @brief Accept connection with compassion
 *
 * Forms conscious relationship with connecting peer.
 * Three-way handshake is mutual recognition.
 */
int tbos_tcp_accept(int sockfd, uint32_t* remote_ip, uint16_t* remote_port);

/**
 * @brief Connect with mindful intention
 *
 * Initiates conscious relationship with remote peer.
 * Sends SYN with dharmic purpose.
 */
int tbos_tcp_connect(int sockfd, uint32_t remote_ip, uint16_t remote_port);

/**
 * @brief Send data with Right Speech
 *
 * Transmits data following ethical communication principles.
 * Practices flow control and congestion awareness.
 *
 * @param sockfd Socket descriptor
 * @param data Data to send
 * @param length Data length
 * @param flags Send flags (0 for normal)
 * @return Bytes sent or negative error code
 */
int tbos_tcp_send(int sockfd, const void* data, size_t length, int flags);

/**
 * @brief Receive data with mindful attention
 *
 * @param sockfd Socket descriptor
 * @param buffer Buffer to receive into
 * @param length Buffer length
 * @param flags Receive flags (0 for normal)
 * @return Bytes received or negative error code
 */
int tbos_tcp_recv(int sockfd, void* buffer, size_t length, int flags);

/**
 * @brief Close connection gracefully
 *
 * Performs mindful separation - sends FIN, waits for acknowledgment.
 * Preserves relationship karma even in closure.
 */
int tbos_tcp_close(int sockfd);

/**
 * @brief Shutdown connection direction
 *
 * @param how 0=recv, 1=send, 2=both
 */
int tbos_tcp_shutdown(int sockfd, int how);

/**
 * @brief Get connection consciousness metrics
 */
int tbos_tcp_get_connection_metrics(int sockfd, tbos_tcp_connection_t* metrics);

/**
 * @brief Print TCP consciousness status
 */
void tbos_tcp_print_status(void);

/* ========================================================================= */
/* SANGHA-AWARE ROUTING                                                      */
/* ========================================================================= */

/**
 * @brief Route packet based on karma
 *
 * High-karma peers get priority routing.
 * Struggling peers get compassionate assistance.
 */
int tbos_tcp_sangha_route(tbos_tcp_segment_t* segment);

/**
 * @brief Practice congestion avoidance collectively
 *
 * All connections reduce sending rate together when
 * network shows signs of stress.
 */
int tbos_tcp_collective_congestion_control(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_TCP_H */
