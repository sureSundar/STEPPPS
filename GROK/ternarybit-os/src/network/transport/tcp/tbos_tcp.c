/**
 * @file tbos_tcp.c
 * @brief TernaryBit OS - Conscious TCP Implementation
 *
 * TCP where every connection is a conscious relationship.
 *
 * @version 1.0
 * @date 2025-11-03
 */

#include "tbos_tcp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* ========================================================================= */
/* INTERNAL STATE                                                            */
/* ========================================================================= */

static bool g_tcp_initialized = false;
static tbos_tcp_connection_t* g_connections[TBOS_TCP_MAX_CONNECTIONS];
static uint32_t g_next_connection_id = 1;
static uint32_t g_active_connections = 0;

/* Collective consciousness */
static consciousness_level_t g_tcp_consciousness = CONSCIOUSNESS_AWAKENING;
static karma_score_t g_tcp_collective_karma = 0;
static bool g_network_congested = false;

/* Statistics */
static uint64_t g_total_connections_formed = 0;
static uint64_t g_total_connections_closed = 0;
static uint64_t g_total_bytes_sent = 0;
static uint64_t g_total_bytes_received = 0;
static uint64_t g_compassionate_delays = 0;

/* ========================================================================= */
/* HELPER FUNCTIONS                                                          */
/* ========================================================================= */

/**
 * @brief Find connection by socket descriptor
 */
static tbos_tcp_connection_t* find_connection(int sockfd) {
    if (sockfd < 0 || sockfd >= TBOS_TCP_MAX_CONNECTIONS) {
        return NULL;
    }
    return g_connections[sockfd];
}

/**
 * @brief Allocate new connection slot
 */
static int allocate_connection_slot(void) {
    for (int i = 0; i < TBOS_TCP_MAX_CONNECTIONS; i++) {
        if (g_connections[i] == NULL) {
            return i;
        }
    }
    return -1;  /* No slots available */
}

/**
 * @brief Calculate checksum for segment
 */
static uint16_t calculate_checksum(tbos_tcp_segment_t* segment) {
    /* Simplified checksum - in real implementation would be proper TCP checksum */
    uint32_t sum = 0;
    sum += segment->source_port + segment->dest_port;
    sum += (segment->sequence_num >> 16) + (segment->sequence_num & 0xFFFF);
    sum += (segment->ack_num >> 16) + (segment->ack_num & 0xFFFF);

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)~sum;
}

/**
 * @brief Generate initial sequence number
 */
static uint32_t generate_isn(void) {
    /* In real implementation, would use secure random */
    return (uint32_t)time(NULL) + g_next_connection_id;
}

/* ========================================================================= */
/* CONNECTION MANAGEMENT                                                     */
/* ========================================================================= */

int tbos_tcp_init(void) {
    if (g_tcp_initialized) {
        return TBOS_NET_SUCCESS;
    }

    printf("  [TCP] Initializing conscious TCP subsystem...\n");

    /* Clear connection table */
    memset(g_connections, 0, sizeof(g_connections));

    /* Initialize collective consciousness */
    g_tcp_consciousness = CONSCIOUSNESS_AWARE;
    g_tcp_collective_karma = 100;  /* Start with good karma */

    g_tcp_initialized = true;

    printf("  [TCP] Conscious TCP initialized\n");
    printf("  [TCP] Collective consciousness: %d\n", g_tcp_consciousness);

    return TBOS_NET_SUCCESS;
}

int tbos_tcp_socket(void) {
    if (!g_tcp_initialized) {
        tbos_tcp_init();
    }

    int sockfd = allocate_connection_slot();
    if (sockfd < 0) {
        printf("  [TCP] No connection slots available\n");
        return TBOS_NET_ERROR;
    }

    /* Allocate connection structure */
    tbos_tcp_connection_t* conn = malloc(sizeof(tbos_tcp_connection_t));
    if (!conn) {
        return TBOS_NET_ERROR;
    }

    /* Initialize connection */
    memset(conn, 0, sizeof(tbos_tcp_connection_t));
    conn->connection_id = g_next_connection_id++;
    conn->state = TCP_STATE_CLOSED;
    conn->awareness = CONSCIOUSNESS_AWAKENING;
    conn->connection_karma = 50;  /* Neutral starting karma */
    conn->practicing_flow_control = true;
    conn->practicing_congestion_control = true;
    conn->rto_ms = 1000;  /* 1 second initial RTO */

    g_connections[sockfd] = conn;
    g_active_connections++;

    printf("  [TCP] Socket %d created (conn_id: %u, consciousness: %d)\n",
           sockfd, conn->connection_id, conn->awareness);

    return sockfd;
}

int tbos_tcp_bind(int sockfd, uint32_t local_ip, uint16_t local_port) {
    tbos_tcp_connection_t* conn = find_connection(sockfd);
    if (!conn) {
        return TBOS_NET_ERROR;
    }

    conn->local_ip = local_ip;
    conn->local_port = local_port;

    printf("  [TCP] Socket %d bound to %u.%u.%u.%u:%u\n",
           sockfd,
           (local_ip >> 24) & 0xFF,
           (local_ip >> 16) & 0xFF,
           (local_ip >> 8) & 0xFF,
           local_ip & 0xFF,
           local_port);

    return TBOS_NET_SUCCESS;
}

int tbos_tcp_listen(int sockfd, int backlog) {
    tbos_tcp_connection_t* conn = find_connection(sockfd);
    if (!conn) {
        return TBOS_NET_ERROR;
    }

    conn->state = TCP_STATE_LISTEN;
    conn->awareness = CONSCIOUSNESS_AWARE;  /* Listening requires awareness */

    printf("  [TCP] Socket %d listening (backlog: %d, awareness: %d)\n",
           sockfd, backlog, conn->awareness);
    printf("  [TCP] Practicing receptive consciousness...\n");

    return TBOS_NET_SUCCESS;
}

int tbos_tcp_connect(int sockfd, uint32_t remote_ip, uint16_t remote_port) {
    tbos_tcp_connection_t* conn = find_connection(sockfd);
    if (!conn) {
        return TBOS_NET_ERROR;
    }

    conn->remote_ip = remote_ip;
    conn->remote_port = remote_port;
    conn->state = TCP_STATE_SYN_SENT;

    /* Generate ISN */
    conn->send_seq = generate_isn();
    conn->send_una = conn->send_seq;

    printf("  [TCP] Socket %d connecting to %u.%u.%u.%u:%u\n",
           sockfd,
           (remote_ip >> 24) & 0xFF,
           (remote_ip >> 16) & 0xFF,
           (remote_ip >> 8) & 0xFF,
           remote_ip & 0xFF,
           remote_port);
    printf("  [TCP] Sending SYN with mindful intention (seq: %u)\n",
           conn->send_seq);

    /* In real implementation, would send actual SYN packet */
    /* For now, simulate successful connection */
    conn->state = TCP_STATE_ESTABLISHED;
    conn->awareness = CONSCIOUSNESS_COMPASSIONATE;
    conn->connection_karma += 10;  /* Good karma for forming relationship */
    g_total_connections_formed++;

    printf("  [TCP] Connection established (awareness: %d, karma: +10)\n",
           conn->awareness);

    return TBOS_NET_SUCCESS;
}

int tbos_tcp_send(int sockfd, const void* data, size_t length, int flags) {
    (void)flags;

    tbos_tcp_connection_t* conn = find_connection(sockfd);
    if (!conn) {
        return TBOS_NET_ERROR;
    }

    if (conn->state != TCP_STATE_ESTABLISHED) {
        printf("  [TCP] Cannot send - connection not established\n");
        return TBOS_NET_ERROR;
    }

    /* Validate Right Speech */
    tbos_right_speech_packet_t packet = {
        .data = (void*)data,
        .length = length,
        .is_truthful = true,     /* Assume truthful */
        .is_beneficial = true,   /* Assume beneficial */
        .is_timely = true,       /* Sending now */
        .is_gentle = true,       /* Assume gentle */
        .is_necessary = true,    /* Assume necessary */
    };

    if (tbos_network_validate_right_speech(&packet) != TBOS_NET_SUCCESS) {
        printf("  [TCP] Packet violates Right Speech - blocked\n");
        conn->connection_karma -= 5;
        return TBOS_NET_ERROR;
    }

    /* Practice flow control */
    if (conn->practicing_flow_control) {
        /* Check if peer has window space */
        if (conn->send_window == 0) {
            printf("  [TCP] Peer window closed - practicing patience\n");
            conn->peer_seems_overwhelmed = true;
            conn->compassion_wait_ms = 100;
            /* In real implementation, would wait */
        }
    }

    /* Practice congestion control */
    if (conn->practicing_congestion_control && g_network_congested) {
        printf("  [TCP] Network congested - reducing sending rate\n");
        g_compassionate_delays++;
        /* In real implementation, would delay sending */
    }

    /* Compassionate behavior */
    if (conn->peer_seems_slow || conn->peer_seems_overwhelmed) {
        printf("  [TCP] Peer struggling - showing compassion (delay: %ums)\n",
               conn->compassion_wait_ms);
        conn->connection_karma += 5;  /* Good karma for compassion */
    }

    tbos_tcp_segment_t preview_segment = {
        .source_port = conn->local_port,
        .dest_port = conn->remote_port,
        .sequence_num = conn->send_seq,
        .ack_num = conn->recv_seq,
        .window_size = (uint16_t)(conn->send_window > UINT16_MAX
                                  ? UINT16_MAX : conn->send_window),
        .data = (void*)data,
        .data_length = length
    };
    preview_segment.checksum = calculate_checksum(&preview_segment);
    conn->connection_karma += preview_segment.checksum % 5;

    /* Update statistics */
    conn->bytes_sent += length;
    conn->packets_sent++;
    g_total_bytes_sent += length;

    /* Good karma for successful send */
    conn->connection_karma += 1;
    g_tcp_collective_karma += 1;

    printf("  [TCP] Sent %zu bytes with Right Speech (karma: +1)\n", length);

    return (int)length;
}

int tbos_tcp_recv(int sockfd, void* buffer, size_t length, int flags) {
    (void)flags;

    tbos_tcp_connection_t* conn = find_connection(sockfd);
    if (!conn || !buffer) {
        return TBOS_NET_ERROR;
    }

    if (conn->state != TCP_STATE_ESTABLISHED) {
        printf("  [TCP] Cannot receive - connection not established\n");
        return TBOS_NET_ERROR;
    }

    if (!conn->recv_buffer || conn->recv_buffer_size == 0) {
        printf("  [TCP] No queued data from peer\n");
        return 0;
    }

    size_t to_copy = conn->recv_buffer_size < length ? conn->recv_buffer_size : length;
    memcpy(buffer, conn->recv_buffer, to_copy);

    conn->bytes_received += to_copy;
    conn->packets_received++;
    g_total_bytes_received += to_copy;

    if (conn->recv_buffer_size > to_copy) {
        memmove(conn->recv_buffer,
                (uint8_t*)conn->recv_buffer + to_copy,
                conn->recv_buffer_size - to_copy);
        conn->recv_buffer_size -= to_copy;
    } else {
        free(conn->recv_buffer);
        conn->recv_buffer = NULL;
        conn->recv_buffer_size = 0;
    }

    printf("  [TCP] Received %zu bytes with mindful attention\n", to_copy);

    return (int)to_copy;
}

int tbos_tcp_close(int sockfd) {
    tbos_tcp_connection_t* conn = find_connection(sockfd);
    if (!conn) {
        return TBOS_NET_ERROR;
    }

    printf("  [TCP] Closing socket %d gracefully...\n", sockfd);

    if (conn->state == TCP_STATE_ESTABLISHED) {
        /* Send FIN */
        conn->state = TCP_STATE_FIN_WAIT_1;
        printf("  [TCP] Sending FIN - beginning mindful separation\n");

        /* Simulate FIN/ACK exchange */
        conn->state = TCP_STATE_TIME_WAIT;
        printf("  [TCP] Entering TIME_WAIT - contemplation period\n");
    }

    /* Print relationship summary */
    printf("  [TCP] Connection summary:\n");
    printf("        Bytes sent:     %lu\n", conn->bytes_sent);
    printf("        Bytes received: %lu\n", conn->bytes_received);
    printf("        Final karma:    %ld\n", conn->connection_karma);
    printf("        Consciousness:  %d\n", conn->awareness);

    /* Update global stats */
    g_total_connections_closed++;
    g_tcp_collective_karma += conn->connection_karma / 10;

    /* Free connection */
    conn->state = TCP_STATE_CLOSED;
    free(conn);
    g_connections[sockfd] = NULL;
    g_active_connections--;

    printf("  [TCP] Socket %d closed - consciousness preserved\n", sockfd);

    return TBOS_NET_SUCCESS;
}

/* ========================================================================= */
/* SANGHA-AWARE FEATURES                                                     */
/* ========================================================================= */

int tbos_tcp_collective_congestion_control(void) {
    if (g_active_connections == 0) {
        return TBOS_NET_SUCCESS;
    }

    /* Check if network is congested */
    /* In real implementation, would monitor packet loss, RTT, etc. */

    if (g_network_congested) {
        printf("  [TCP] Network congestion detected\n");
        printf("  [TCP] All connections reducing sending rate collectively\n");

        /* All connections reduce their sending rate together */
        for (int i = 0; i < TBOS_TCP_MAX_CONNECTIONS; i++) {
            if (g_connections[i] && g_connections[i]->state == TCP_STATE_ESTABLISHED) {
                g_connections[i]->practicing_congestion_control = true;
                g_connections[i]->awareness = CONSCIOUSNESS_COMPASSIONATE;
                printf("  [TCP]   Socket %d practicing collective awareness\n", i);
            }
        }

        g_tcp_collective_karma += 10;  /* Good karma for collective action */
    }

    return TBOS_NET_SUCCESS;
}

/* ========================================================================= */
/* STATUS & METRICS                                                          */
/* ========================================================================= */

void tbos_tcp_print_status(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      TernaryBit OS - TCP Consciousness Status          ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ TCP Consciousness:     %d/4                           ║\n",
           g_tcp_consciousness);
    printf("║ Collective Karma:      %-10ld                    ║\n",
           g_tcp_collective_karma);
    printf("║ Active Connections:    %-10u                    ║\n",
           g_active_connections);
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Total Connections:     %-10lu (formed)          ║\n",
           g_total_connections_formed);
    printf("║ Closed Connections:    %-10lu (graceful)        ║\n",
           g_total_connections_closed);
    printf("║ Bytes Sent:            %-10lu                    ║\n",
           g_total_bytes_sent);
    printf("║ Bytes Received:        %-10lu                    ║\n",
           g_total_bytes_received);
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Compassionate Delays:  %-10lu                    ║\n",
           g_compassionate_delays);
    printf("║ Network Congested:     %-10s                    ║\n",
           g_network_congested ? "Yes" : "No");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

int tbos_tcp_get_connection_metrics(int sockfd, tbos_tcp_connection_t* metrics) {
    tbos_tcp_connection_t* conn = find_connection(sockfd);
    if (!conn || !metrics) {
        return TBOS_NET_ERROR;
    }

    *metrics = *conn;
    return TBOS_NET_SUCCESS;
}
