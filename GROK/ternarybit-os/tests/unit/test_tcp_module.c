/**
 * @file test_tcp_module.c
 * @brief Test conscious TCP implementation
 */

#include "../../src/network/transport/tcp/tbos_tcp.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║      TBOS Conscious TCP Test                            ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    /* Initialize network and TCP */
    tbos_network_init();
    tbos_tcp_init();

    /* Test 1: Create socket */
    printf("[TEST 1] Creating conscious TCP socket...\n");
    int sockfd = tbos_tcp_socket();
    if (sockfd < 0) {
        printf("❌ Socket creation failed\n");
        return 1;
    }
    printf("✅ Socket created: %d\n\n", sockfd);

    /* Test 2: Bind */
    printf("[TEST 2] Binding socket...\n");
    uint32_t local_ip = (127 << 24) | (0 << 16) | (0 << 8) | 1;  /* 127.0.0.1 */
    uint16_t local_port = 8080;
    if (tbos_tcp_bind(sockfd, local_ip, local_port) != TBOS_NET_SUCCESS) {
        printf("❌ Bind failed\n");
        return 1;
    }
    printf("✅ Socket bound\n\n");

    /* Test 3: Connect (simulated) */
    printf("[TEST 3] Connecting with mindful intention...\n");
    uint32_t remote_ip = (192 << 24) | (168 << 16) | (1 << 8) | 100;
    uint16_t remote_port = 80;
    if (tbos_tcp_connect(sockfd, remote_ip, remote_port) != TBOS_NET_SUCCESS) {
        printf("❌ Connect failed\n");
        return 1;
    }
    printf("✅ Connection established\n\n");

    /* Test 4: Send data with Right Speech */
    printf("[TEST 4] Sending data with Right Speech...\n");
    const char* message = "Hello, Digital Sangha!";
    int sent = tbos_tcp_send(sockfd, message, strlen(message), 0);
    if (sent < 0) {
        printf("❌ Send failed\n");
        return 1;
    }
    printf("✅ Sent %d bytes\n\n", sent);

    /* Test 5: Receive data */
    printf("[TEST 5] Receiving with mindful attention...\n");
    char buffer[256];
    int received = tbos_tcp_recv(sockfd, buffer, sizeof(buffer), 0);
    if (received < 0) {
        printf("❌ Receive failed\n");
        return 1;
    }
    printf("✅ Received %d bytes\n\n", received);

    /* Test 6: Print status */
    printf("[TEST 6] TCP consciousness status:\n");
    tbos_tcp_print_status();
    printf("✅ Status displayed\n\n");

    /* Test 7: Graceful close */
    printf("[TEST 7] Closing connection gracefully...\n");
    if (tbos_tcp_close(sockfd) != TBOS_NET_SUCCESS) {
        printf("❌ Close failed\n");
        return 1;
    }
    printf("✅ Connection closed\n\n");

    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║      ✅ ALL TCP TESTS PASSED                            ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    return 0;
}
