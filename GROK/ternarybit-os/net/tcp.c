// TernaryBit OS - TCP Implementation
// Basic TCP stack for network communication

#include <stdint.h>
#include <stddef.h>

// TCP States
typedef enum {
    TCP_CLOSED = 0,
    TCP_LISTEN,
    TCP_SYN_SENT,
    TCP_SYN_RECEIVED,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT_1,
    TCP_FIN_WAIT_2,
    TCP_CLOSE_WAIT,
    TCP_CLOSING,
    TCP_LAST_ACK,
    TCP_TIME_WAIT
} tcp_state_t;

// TCP Header
typedef struct {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t  data_offset;  // 4 bits offset, 4 bits reserved
    uint8_t  flags;        // URG, ACK, PSH, RST, SYN, FIN
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent_ptr;
} __attribute__((packed)) tcp_header_t;

// TCP Flags
#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20

// TCP Connection Control Block
typedef struct {
    uint32_t local_ip;
    uint32_t remote_ip;
    uint16_t local_port;
    uint16_t remote_port;
    tcp_state_t state;
    uint32_t send_seq;
    uint32_t recv_seq;
    uint16_t window_size;
    uint8_t  in_use;
} tcp_cb_t;

#define MAX_TCP_CONNECTIONS 16
static tcp_cb_t tcp_connections[MAX_TCP_CONNECTIONS];

// External functions
extern int ip_send(uint32_t dst, const void* payload, uint16_t len);
extern void kernel_print(const char* str);

// Initialize TCP stack
int tcp_init(void) {
    // Clear all connection control blocks
    for (int i = 0; i < MAX_TCP_CONNECTIONS; i++) {
        tcp_connections[i].in_use = 0;
        tcp_connections[i].state = TCP_CLOSED;
        tcp_connections[i].local_port = 0;
        tcp_connections[i].remote_port = 0;
        tcp_connections[i].send_seq = 0;
        tcp_connections[i].recv_seq = 0;
        tcp_connections[i].window_size = 8192;
    }

    kernel_print("[TCP] TCP stack initialized\n");
    return 0;
}

// Find free connection slot
static int tcp_find_free_slot(void) {
    for (int i = 0; i < MAX_TCP_CONNECTIONS; i++) {
        if (!tcp_connections[i].in_use) {
            return i;
        }
    }
    return -1;
}

// Calculate TCP checksum
static uint16_t tcp_checksum(const tcp_header_t* header, uint16_t len) {
    uint32_t sum = 0;
    const uint16_t* ptr = (const uint16_t*)header;

    // Sum all 16-bit words
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }

    // Add odd byte if present
    if (len > 0) {
        sum += *((uint8_t*)ptr);
    }

    // Fold 32-bit sum to 16 bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)~sum;
}

// Connect to remote host
int tcp_connect(uint32_t addr, uint16_t port) {
    // Find free slot
    int slot = tcp_find_free_slot();
    if (slot < 0) {
        kernel_print("[TCP] No free connections\n");
        return -1;
    }

    // Initialize connection
    tcp_cb_t* conn = &tcp_connections[slot];
    conn->in_use = 1;
    conn->remote_ip = addr;
    conn->remote_port = port;
    conn->local_port = 49152 + slot;  // Ephemeral port
    conn->state = TCP_SYN_SENT;
    conn->send_seq = 1000;  // Initial sequence number
    conn->recv_seq = 0;

    // Build SYN packet
    tcp_header_t header;
    header.src_port = conn->local_port;
    header.dst_port = port;
    header.seq_num = conn->send_seq;
    header.ack_num = 0;
    header.data_offset = (sizeof(tcp_header_t) / 4) << 4;
    header.flags = TCP_SYN;
    header.window = conn->window_size;
    header.urgent_ptr = 0;
    header.checksum = 0;
    header.checksum = tcp_checksum(&header, sizeof(header));

    // Send SYN packet via IP layer
    int result = ip_send(addr, &header, sizeof(header));
    if (result < 0) {
        conn->in_use = 0;
        conn->state = TCP_CLOSED;
        return -1;
    }

    kernel_print("[TCP] Connection initiated\n");
    return slot;
}

// Send data over TCP connection
int tcp_send(const void* data, uint16_t len) {
    if (!data || len == 0) {
        return -1;
    }

    // For now, send data on first established connection
    for (int i = 0; i < MAX_TCP_CONNECTIONS; i++) {
        if (tcp_connections[i].in_use && tcp_connections[i].state == TCP_ESTABLISHED) {
            tcp_cb_t* conn = &tcp_connections[i];

            // Build TCP header
            tcp_header_t header;
            header.src_port = conn->local_port;
            header.dst_port = conn->remote_port;
            header.seq_num = conn->send_seq;
            header.ack_num = conn->recv_seq;
            header.data_offset = (sizeof(tcp_header_t) / 4) << 4;
            header.flags = TCP_PSH | TCP_ACK;
            header.window = conn->window_size;
            header.urgent_ptr = 0;
            header.checksum = 0;
            header.checksum = tcp_checksum(&header, sizeof(header) + len);

            // TODO: Combine header and data into single packet
            // For now, just send header
            ip_send(conn->remote_ip, &header, sizeof(header));

            conn->send_seq += len;
            return len;
        }
    }

    kernel_print("[TCP] No established connection\n");
    return -1;
}

// Receive data from TCP connection
int tcp_recv(void* data, uint16_t maxlen) {
    if (!data || maxlen == 0) {
        return -1;
    }

    // TODO: Implement receive buffer and reassembly
    // For now, return 0 (no data available)
    return 0;
}

// Process incoming TCP packet
void tcp_process(const tcp_header_t* header, uint16_t len) {
    if (!header) return;

    // Find matching connection
    for (int i = 0; i < MAX_TCP_CONNECTIONS; i++) {
        tcp_cb_t* conn = &tcp_connections[i];

        if (conn->in_use &&
            conn->local_port == header->dst_port &&
            conn->remote_port == header->src_port) {

            // Update sequence numbers
            if (header->flags & TCP_ACK) {
                // Acknowledge received
            }

            if (header->flags & TCP_SYN) {
                // SYN received - send SYN-ACK
                conn->recv_seq = header->seq_num + 1;
                conn->state = TCP_SYN_RECEIVED;
            }

            if (header->flags & TCP_FIN) {
                // FIN received - connection closing
                conn->state = TCP_CLOSE_WAIT;
            }

            return;
        }
    }
}

// Close TCP connection
int tcp_close(int slot) {
    if (slot < 0 || slot >= MAX_TCP_CONNECTIONS) {
        return -1;
    }

    tcp_cb_t* conn = &tcp_connections[slot];
    if (!conn->in_use) {
        return -1;
    }

    // Send FIN packet
    tcp_header_t header;
    header.src_port = conn->local_port;
    header.dst_port = conn->remote_port;
    header.seq_num = conn->send_seq;
    header.ack_num = conn->recv_seq;
    header.data_offset = (sizeof(tcp_header_t) / 4) << 4;
    header.flags = TCP_FIN | TCP_ACK;
    header.window = conn->window_size;
    header.urgent_ptr = 0;
    header.checksum = 0;
    header.checksum = tcp_checksum(&header, sizeof(header));

    ip_send(conn->remote_ip, &header, sizeof(header));

    conn->state = TCP_FIN_WAIT_1;
    conn->in_use = 0;

    kernel_print("[TCP] Connection closed\n");
    return 0;
}
