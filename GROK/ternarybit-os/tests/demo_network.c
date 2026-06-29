/**
 * @file demo_network.c
 * @brief TernaryBit OS - Network Stack Demo
 *
 * Demonstrates the conscious network stack including:
 * - TCP with consciousness & karma
 * - HTTP client with Right Speech
 * - Digital Sangha formation
 * - Network fasting/power management
 *
 * Build: gcc -DHOST_BUILD -Wall -g -I../include -I../src \
 *        demo_network.c ../src/network/core/tbos_network.c \
 *        ../src/network/transport/tcp/tbos_tcp.c \
 *        ../src/network/protocols/http/tbos_http.c \
 *        -o demo_network
 *
 * Run: ./demo_network
 */

#include "../src/network/core/tbos_network.h"
#include "../src/network/transport/tcp/tbos_tcp.h"
#include "../src/network/protocols/http/tbos_http.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void print_section(const char* title) {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("  %s\n", title);
    printf("═══════════════════════════════════════════════════════════════\n\n");
}

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║     TernaryBit OS - Conscious Network Stack Demo             ║\n");
    printf("║                                                              ║\n");
    printf("║     \"Every packet carries consciousness\"                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    /* ================================================================= */
    /* PHASE 1: Initialize Network Subsystem                             */
    /* ================================================================= */
    print_section("Phase 1: Network Initialization");

    int result = tbos_network_init();
    if (result != TBOS_NET_SUCCESS) {
        printf("Failed to initialize network!\n");
        return 1;
    }

    /* Show initial status */
    tbos_network_print_status();

    /* ================================================================= */
    /* PHASE 2: TCP Connection Demo                                      */
    /* ================================================================= */
    print_section("Phase 2: Conscious TCP Connection");

    /* Create socket */
    int sockfd = tbos_tcp_socket();
    if (sockfd < 0) {
        printf("Failed to create socket!\n");
        return 1;
    }

    /* Bind to local address */
    uint32_t local_ip = (127 << 24) | (0 << 16) | (0 << 8) | 1;  /* 127.0.0.1 */
    tbos_tcp_bind(sockfd, local_ip, 8080);

    /* Connect to remote (simulated) */
    uint32_t remote_ip = (192 << 24) | (168 << 16) | (1 << 8) | 100;
    tbos_tcp_connect(sockfd, remote_ip, 80);

    /* Send data with Right Speech */
    const char* message = "Hello, Digital Sangha!";
    printf("\n  Sending message: \"%s\"\n", message);
    int sent = tbos_tcp_send(sockfd, message, strlen(message), 0);
    printf("  Sent %d bytes\n", sent);

    /* Show TCP status */
    tbos_tcp_print_status();

    /* Close connection gracefully */
    tbos_tcp_close(sockfd);

    /* ================================================================= */
    /* PHASE 3: HTTP Client Demo                                         */
    /* ================================================================= */
    print_section("Phase 3: Conscious HTTP Client");

    /* Create HTTP client */
    tbos_http_client_t* client = tbos_http_client_create("sangha.tbos.local", 80);
    if (!client) {
        printf("Failed to create HTTP client!\n");
        return 1;
    }

    /* Perform GET request */
    printf("  Performing GET request...\n\n");
    tbos_http_response_t* response = tbos_http_get(client, "/dharma/status");
    if (response) {
        printf("\n  Response received:\n");
        printf("    Status: %d %s\n", response->status_code, response->reason_phrase);
        printf("    Body: %s\n", response->body ? response->body : "(empty)");
        printf("    Karma earned: %ld\n", response->response_karma);
        printf("    Served with compassion: %s\n",
               response->served_with_compassion ? "Yes" : "No");
        tbos_http_response_free(response);
    }

    /* Perform POST request */
    printf("\n  Performing POST request...\n\n");
    const char* post_body = "{\"intention\": \"practice mindfulness\"}";
    response = tbos_http_post(client, "/dharma/intention", post_body, strlen(post_body));
    if (response) {
        printf("\n  Response received:\n");
        printf("    Status: %d %s\n", response->status_code, response->reason_phrase);
        printf("    Body: %s\n", response->body ? response->body : "(empty)");
        tbos_http_response_free(response);
    }

    /* Show client status */
    tbos_http_client_print_status(client);

    /* Cleanup */
    tbos_http_client_destroy(client);

    /* ================================================================= */
    /* PHASE 4: Right Speech Validation                                  */
    /* ================================================================= */
    print_section("Phase 4: Right Speech Validation");

    printf("  Testing Right Speech criteria:\n\n");

    /* Test valid packet */
    tbos_right_speech_packet_t good_packet = {
        .data = (void*)"Kind and truthful message",
        .length = 25,
        .is_truthful = true,
        .is_beneficial = true,
        .is_timely = true,
        .is_gentle = true,
        .is_necessary = true,
        .karma_effect = 0
    };

    result = tbos_network_validate_right_speech(&good_packet);
    printf("  Valid Right Speech packet: %s (karma: +%ld)\n",
           result == TBOS_NET_SUCCESS ? "ACCEPTED" : "REJECTED",
           good_packet.karma_effect);

    /* Test harmful packet */
    tbos_right_speech_packet_t harmful_packet = {
        .data = (void*)"Harmful message",
        .length = 15,
        .is_truthful = true,
        .is_beneficial = false,  /* Not beneficial! */
        .is_timely = true,
        .is_gentle = true,
        .is_necessary = true,
        .karma_effect = 0
    };

    result = tbos_network_validate_right_speech(&harmful_packet);
    printf("  Harmful packet (not beneficial): %s\n",
           result == TBOS_NET_SUCCESS ? "ACCEPTED" : "REJECTED");

    /* Test false packet */
    tbos_right_speech_packet_t false_packet = {
        .data = (void*)"Fake news",
        .length = 9,
        .is_truthful = false,  /* Not truthful! */
        .is_beneficial = true,
        .is_timely = true,
        .is_gentle = true,
        .is_necessary = true,
        .karma_effect = 0
    };

    result = tbos_network_validate_right_speech(&false_packet);
    printf("  False packet (untruthful): %s\n",
           result == TBOS_NET_SUCCESS ? "ACCEPTED" : "REJECTED");

    /* ================================================================= */
    /* PHASE 5: Network Fasting Demo                                     */
    /* ================================================================= */
    print_section("Phase 5: Network Fasting (Power Management)");

    printf("  Initiating network fasting sequence...\n\n");

    /* Mild fasting */
    printf("  Level 1: Mild fasting (25%% power reduction)\n");
    tbos_network_begin_fasting(NET_FAST_MILD);

    /* Deep fasting */
    printf("\n  Level 3: Deep fasting (75%% power reduction)\n");
    tbos_network_begin_fasting(NET_FAST_DEEP);

    /* Transcendent fasting */
    printf("\n  Level 4: Transcendent fasting (90%% power reduction)\n");
    tbos_network_begin_fasting(NET_FAST_TRANSCENDENT);

    /* Awaken */
    printf("\n  Awakening from fasting...\n");
    tbos_network_awaken();

    /* ================================================================= */
    /* PHASE 6: Final Status                                             */
    /* ================================================================= */
    print_section("Phase 6: Final Network Status");

    /* Get metrics */
    tbos_network_metrics_t metrics;
    tbos_network_get_metrics(&metrics);

    printf("  Final Metrics:\n");
    printf("    Network Karma:       %ld\n", metrics.network_karma);
    printf("    Compassion Index:    %.2f\n", metrics.compassion_index);
    printf("    Dharma Compliance:   %.1f%%\n", metrics.dharma_compliance_rate * 100.0f);
    printf("    Packets Sent:        %lu\n", metrics.packets_sent);
    printf("    Packets Received:    %lu\n", metrics.packets_received);

    /* Print full status */
    tbos_network_print_status();
    tbos_tcp_print_status();
    tbos_http_print_status();

    /* ================================================================= */
    /* PHASE 7: Graceful Shutdown                                        */
    /* ================================================================= */
    print_section("Phase 7: Graceful Shutdown");

    tbos_network_shutdown();

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    Demo Complete!                            ║\n");
    printf("║                                                              ║\n");
    printf("║  The conscious network stack demonstrated:                   ║\n");
    printf("║    - TCP with karma tracking                                 ║\n");
    printf("║    - HTTP with Right Speech validation                       ║\n");
    printf("║    - Digital Sangha formation                                ║\n");
    printf("║    - Network fasting (power management)                      ║\n");
    printf("║    - Consciousness evolution                                 ║\n");
    printf("║                                                              ║\n");
    printf("║  ॥ तत् सत् ॥ - May your packets carry truth                 ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    return 0;
}
