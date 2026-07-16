/**
 * @file test_network_e2e.c
 * @brief TernaryBit OS - Network Stack End-to-End Integration Test
 *
 * Tests TCP and UDP functionality using the tbos_net_* API.
 * Verifies that HAL + tbos_net.c work correctly end-to-end.
 *
 * Build: make test-network-e2e
 * Run: ./build/test_network_e2e
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include "tbos/net.h"

/* Test result tracking */
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_START(name) printf("\n[TEST] %s...\n", name)
#define TEST_PASS(name) do { printf("[PASS] %s\n", name); tests_passed++; } while(0)
#define TEST_FAIL(name, reason) do { printf("[FAIL] %s: %s\n", name, reason); tests_failed++; } while(0)

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: Network Initialization
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_net_init(void) {
    TEST_START("Network Initialization");

    int result = tbos_net_init();
    if (result == TBOS_NET_SUCCESS) {
        TEST_PASS("Network Initialization");
    } else {
        TEST_FAIL("Network Initialization", "tbos_net_init() failed");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: Socket Creation
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_socket_creation(void) {
    TEST_START("Socket Creation (TCP)");

    int tcp_sock = tbos_net_socket(TBOS_SOCK_TCP);
    if (tcp_sock >= 0) {
        TEST_PASS("Socket Creation (TCP)");
        tbos_net_close(tcp_sock);
    } else {
        TEST_FAIL("Socket Creation (TCP)", "Failed to create TCP socket");
    }

    TEST_START("Socket Creation (UDP)");

    int udp_sock = tbos_net_socket(TBOS_SOCK_UDP);
    if (udp_sock >= 0) {
        TEST_PASS("Socket Creation (UDP)");
        tbos_net_close(udp_sock);
    } else {
        TEST_FAIL("Socket Creation (UDP)", "Failed to create UDP socket");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: Local IP Retrieval
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_get_local_ip(void) {
    TEST_START("Get Local IP");

    char local_ip[64];
    int result = tbos_net_get_local_ip(local_ip, sizeof(local_ip));
    if (result == TBOS_NET_SUCCESS && strlen(local_ip) > 0) {
        printf("       Local IP: %s\n", local_ip);
        TEST_PASS("Get Local IP");
    } else {
        TEST_FAIL("Get Local IP", "Failed to get local IP address");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: Network Statistics
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_net_stats(void) {
    TEST_START("Network Statistics");

    uint64_t sent = 0, recv = 0;
    int active = -1;

    tbos_net_stats(&sent, &recv, &active);

    printf("       Bytes sent: %llu\n", (unsigned long long)sent);
    printf("       Bytes recv: %llu\n", (unsigned long long)recv);
    printf("       Active sockets: %d\n", active);

    if (active >= 0) {
        TEST_PASS("Network Statistics");
    } else {
        TEST_FAIL("Network Statistics", "Invalid stats returned");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: UDP Loopback (send/recv on localhost)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_udp_loopback(void) {
    TEST_START("UDP Loopback");

    int server_sock = tbos_net_socket(TBOS_SOCK_UDP);
    if (server_sock < 0) {
        TEST_FAIL("UDP Loopback", "Failed to create server socket");
        return;
    }

    /* Bind to a random high port */
    uint16_t port = 18888;
    if (tbos_net_bind(server_sock, port) != TBOS_NET_SUCCESS) {
        tbos_net_close(server_sock);
        TEST_FAIL("UDP Loopback", "Failed to bind server socket");
        return;
    }

    /* Set non-blocking for receive with timeout */
    tbos_net_set_nonblocking(server_sock, 1);

    int client_sock = tbos_net_socket(TBOS_SOCK_UDP);
    if (client_sock < 0) {
        tbos_net_close(server_sock);
        TEST_FAIL("UDP Loopback", "Failed to create client socket");
        return;
    }

    /* Send test message */
    const char* test_msg = "TBOS UDP Test Message";
    ssize_t sent = tbos_net_sendto(client_sock, test_msg, strlen(test_msg),
                                   "127.0.0.1", port);

    if (sent < 0) {
        tbos_net_close(server_sock);
        tbos_net_close(client_sock);
        TEST_FAIL("UDP Loopback", "Failed to send UDP message");
        return;
    }

    /* Small delay to let packet arrive */
    usleep(10000);

    /* Receive message */
    char buf[256];
    char from_addr[64];
    uint16_t from_port = 0;

    ssize_t received = tbos_net_recvfrom(server_sock, buf, sizeof(buf) - 1,
                                          from_addr, &from_port);

    tbos_net_close(server_sock);
    tbos_net_close(client_sock);

    if (received > 0) {
        buf[received] = '\0';
        if (strcmp(buf, test_msg) == 0) {
            printf("       Sent: %s\n", test_msg);
            printf("       Recv: %s\n", buf);
            printf("       From: %s:%u\n", from_addr, from_port);
            TEST_PASS("UDP Loopback");
        } else {
            TEST_FAIL("UDP Loopback", "Message content mismatch");
        }
    } else {
        TEST_FAIL("UDP Loopback", "No message received");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: TCP Loopback (server/client on localhost)
 * ═══════════════════════════════════════════════════════════════════════════ */

static volatile int g_server_ready = 0;

static void* tcp_test_server(void* arg) {
    uint16_t port = *(uint16_t*)arg;

    int server_sock = tbos_net_socket(TBOS_SOCK_TCP);
    if (server_sock < 0) return NULL;

    if (tbos_net_bind(server_sock, port) != TBOS_NET_SUCCESS) {
        tbos_net_close(server_sock);
        return NULL;
    }

    if (tbos_net_listen(server_sock, 1) != TBOS_NET_SUCCESS) {
        tbos_net_close(server_sock);
        return NULL;
    }

    g_server_ready = 1;

    int client = tbos_net_accept(server_sock);
    if (client >= 0) {
        /* Echo received data back */
        char buf[256];
        ssize_t n = tbos_net_recv(client, buf, sizeof(buf));
        if (n > 0) {
            tbos_net_send(client, buf, n);
        }
        tbos_net_close(client);
    }

    tbos_net_close(server_sock);
    return NULL;
}

static void test_tcp_loopback(void) {
    TEST_START("TCP Loopback");

    uint16_t port = 18889;
    g_server_ready = 0;

    /* Fork server process */
    pid_t pid = fork();
    if (pid < 0) {
        TEST_FAIL("TCP Loopback", "Failed to fork server process");
        return;
    }

    if (pid == 0) {
        /* Child: run server */
        tcp_test_server(&port);
        _exit(0);
    }

    /* Parent: run client after server is ready */
    usleep(100000);  /* 100ms for server to start */

    int client_sock = tbos_net_socket(TBOS_SOCK_TCP);
    if (client_sock < 0) {
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        TEST_FAIL("TCP Loopback", "Failed to create client socket");
        return;
    }

    if (tbos_net_connect(client_sock, "127.0.0.1", port) != TBOS_NET_SUCCESS) {
        tbos_net_close(client_sock);
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        TEST_FAIL("TCP Loopback", "Failed to connect to server");
        return;
    }

    /* Send test message */
    const char* test_msg = "TBOS TCP Echo Test";
    ssize_t sent = tbos_net_send(client_sock, test_msg, strlen(test_msg));
    if (sent < 0) {
        tbos_net_close(client_sock);
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        TEST_FAIL("TCP Loopback", "Failed to send TCP message");
        return;
    }

    /* Receive echo */
    char buf[256];
    ssize_t received = tbos_net_recv(client_sock, buf, sizeof(buf) - 1);

    tbos_net_close(client_sock);
    waitpid(pid, NULL, 0);

    if (received > 0) {
        buf[received] = '\0';
        if (strcmp(buf, test_msg) == 0) {
            printf("       Sent: %s\n", test_msg);
            printf("       Recv: %s\n", buf);
            TEST_PASS("TCP Loopback");
        } else {
            TEST_FAIL("TCP Loopback", "Echo content mismatch");
        }
    } else {
        TEST_FAIL("TCP Loopback", "No echo received");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: Socket Info
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_socket_info(void) {
    TEST_START("Socket Info");

    int sock = tbos_net_socket(TBOS_SOCK_TCP);
    if (sock < 0) {
        TEST_FAIL("Socket Info", "Failed to create socket");
        return;
    }

    const tbos_socket_t* info = tbos_net_get_socket(sock);
    if (info != NULL) {
        printf("       Socket fd: %d\n", info->fd);
        printf("       Socket type: %s\n", info->type == TBOS_SOCK_TCP ? "TCP" : "UDP");
        printf("       Socket karma: %ld\n", (long)info->karma);
        TEST_PASS("Socket Info");
    } else {
        TEST_FAIL("Socket Info", "Failed to get socket info");
    }

    tbos_net_close(sock);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST: Non-blocking Mode
 * ═══════════════════════════════════════════════════════════════════════════ */

static void test_nonblocking(void) {
    TEST_START("Non-blocking Mode");

    int sock = tbos_net_socket(TBOS_SOCK_TCP);
    if (sock < 0) {
        TEST_FAIL("Non-blocking Mode", "Failed to create socket");
        return;
    }

    int result = tbos_net_set_nonblocking(sock, 1);
    if (result == TBOS_NET_SUCCESS) {
        const tbos_socket_t* info = tbos_net_get_socket(sock);
        if (info && info->nonblocking) {
            TEST_PASS("Non-blocking Mode");
        } else {
            TEST_FAIL("Non-blocking Mode", "Flag not set");
        }
    } else {
        TEST_FAIL("Non-blocking Mode", "Failed to set non-blocking");
    }

    tbos_net_close(sock);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST RUNNER
 * ═══════════════════════════════════════════════════════════════════════════ */

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("       TernaryBit OS - Network Stack End-to-End Tests\n");
    printf("═══════════════════════════════════════════════════════════════\n");

    /* Run tests */
    test_net_init();
    test_socket_creation();
    test_get_local_ip();
    test_net_stats();
    test_socket_info();
    test_nonblocking();
    test_udp_loopback();
    test_tcp_loopback();

    /* Summary */
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("       Test Results: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");

    /* Cleanup */
    tbos_net_shutdown();

    return tests_failed > 0 ? 1 : 0;
}
