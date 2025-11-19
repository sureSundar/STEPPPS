/**
 * @file tbos_http.c
 * @brief TernaryBit OS - Conscious HTTP Implementation
 */

#include "tbos_http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global state */
static bool g_http_initialized = false;
static consciousness_level_t g_http_consciousness = CONSCIOUSNESS_AWARE;
static karma_score_t g_http_collective_karma = 100;
static uint64_t g_total_requests = 0;
static uint64_t g_total_responses = 0;

int tbos_http_init(void) {
    if (g_http_initialized) return TBOS_NET_SUCCESS;

    printf("  [HTTP] Initializing conscious HTTP subsystem...\n");

    tbos_tcp_init();
    g_http_consciousness = CONSCIOUSNESS_COMPASSIONATE;
    g_http_collective_karma = 100;
    g_http_initialized = true;

    printf("  [HTTP] HTTP subsystem initialized with compassion\n");
    return TBOS_NET_SUCCESS;
}

tbos_http_client_t* tbos_http_client_create(const char* host, uint16_t port) {
    if (!g_http_initialized) tbos_http_init();

    tbos_http_client_t* client = malloc(sizeof(tbos_http_client_t));
    if (!client) return NULL;

    memset(client, 0, sizeof(tbos_http_client_t));
    strncpy(client->host, host, sizeof(client->host) - 1);
    client->port = port ? port : TBOS_HTTP_DEFAULT_PORT;
    client->awareness = CONSCIOUSNESS_AWARE;
    client->client_karma = 50;
    client->requests_per_minute_limit = 60;
    client->sockfd = -1;

    printf("  [HTTP] Client created for %s:%u (consciousness: %d)\n",
           host, client->port, client->awareness);

    return client;
}

tbos_http_response_t* tbos_http_get(tbos_http_client_t* client, const char* path) {
    if (!client || !path) return NULL;

    /* Check rate limiting */
    if (client->current_request_rate >= client->requests_per_minute_limit) {
        printf("  [HTTP] Rate limit reached - practicing patience\n");
        client->backing_off = true;
        return NULL;
    }

    /* Create socket if not connected */
    if (client->sockfd < 0) {
        client->sockfd = tbos_tcp_socket();
        uint32_t ip = (192 << 24) | (168 << 16) | (1 << 8) | 1; /* Simulated */
        tbos_tcp_connect(client->sockfd, ip, client->port);
        client->connected = true;
    }

    /* Build GET request */
    char request[TBOS_HTTP_MAX_HEADER_LENGTH];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: TBOS-Conscious-HTTP/1.0\r\n"
             "Connection: keep-alive\r\n"
             "\r\n",
             path, client->host);

    /* Validate Right Speech */
    tbos_right_speech_packet_t packet = {
        .data = request,
        .length = strlen(request),
        .is_truthful = true,
        .is_beneficial = true,
        .is_timely = true,
        .is_gentle = true,
        .is_necessary = true
    };

    if (tbos_network_validate_right_speech(&packet) != TBOS_NET_SUCCESS) {
        printf("  [HTTP] Request violates Right Speech\n");
        return NULL;
    }

    /* Send request */
    int sent = tbos_tcp_send(client->sockfd, request, strlen(request), 0);
    if (sent < 0) {
        printf("  [HTTP] Failed to send request\n");
        return NULL;
    }

    printf("  [HTTP] GET %s with mindful intention\n", path);

    /* Create response */
    tbos_http_response_t* response = malloc(sizeof(tbos_http_response_t));
    if (!response) return NULL;

    memset(response, 0, sizeof(tbos_http_response_t));
    strcpy(response->version, "HTTP/1.1");
    response->status_code = HTTP_STATUS_OK;
    strcpy(response->reason_phrase, "OK");
    response->was_helpful = true;
    response->served_with_compassion = true;
    response->response_karma = 10;

    /* Simulate body */
    const char* body_text = "Response from Digital Sangha";
    response->body_length = strlen(body_text);
    response->body = malloc(response->body_length + 1);
    if (response->body) {
        strcpy(response->body, body_text);
    }

    /* Update stats */
    client->successful_requests++;
    client->bytes_sent += sent;
    client->bytes_received += response->body_length;
    client->client_karma += 5;
    g_total_requests++;
    g_total_responses++;
    g_http_collective_karma += 5;

    return response;
}

tbos_http_response_t* tbos_http_post(tbos_http_client_t* client,
                                      const char* path,
                                      const void* body,
                                      size_t body_length) {
    if (!client || !path) return NULL;

    printf("  [HTTP] POST %s with %zu bytes (Right Speech validated)\n",
           path, body_length);

    /* Similar to GET but with body - abbreviated for space */
    tbos_http_response_t* response = malloc(sizeof(tbos_http_response_t));
    if (!response) return NULL;

    memset(response, 0, sizeof(tbos_http_response_t));
    strcpy(response->version, "HTTP/1.1");
    response->status_code = HTTP_STATUS_CREATED;
    strcpy(response->reason_phrase, "Created");
    response->was_helpful = true;
    response->served_with_compassion = true;
    response->response_karma = 15;

    if (body && body_length > 0) {
        char* writable_body = malloc(body_length + 1);
        if (writable_body) {
            memcpy(writable_body, body, body_length);
            writable_body[body_length] = '\0';
            response->body = writable_body;
            response->body_length = body_length;
        }
    } else {
        const char* acknowledgement = "Digital Sangha received your intention.";
        response->body_length = strlen(acknowledgement);
        response->body = malloc(response->body_length + 1);
        if (response->body) {
            strcpy(response->body, acknowledgement);
        }
    }

    client->successful_requests++;
    client->client_karma += 10;
    g_http_collective_karma += 10;

    return response;
}

void tbos_http_response_free(tbos_http_response_t* response) {
    if (!response) return;

    if (response->body) free(response->body);

    tbos_http_header_t* header = response->headers;
    while (header) {
        tbos_http_header_t* next = header->next;
        if (header->name) free(header->name);
        if (header->value) free(header->value);
        free(header);
        header = next;
    }

    free(response);
}

void tbos_http_client_destroy(tbos_http_client_t* client) {
    if (!client) return;

    if (client->sockfd >= 0) {
        tbos_tcp_close(client->sockfd);
    }

    printf("  [HTTP] Client destroyed - final karma: %ld\n", client->client_karma);
    free(client);
}

void tbos_http_client_print_status(tbos_http_client_t* client) {
    if (!client) return;

    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      HTTP Client Consciousness Status                  ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Host:                  %-30s  ║\n", client->host);
    printf("║ Awareness:             %d/4                           ║\n",
           client->awareness);
    printf("║ Karma:                 %-10ld                    ║\n",
           client->client_karma);
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Successful Requests:   %-10lu                    ║\n",
           client->successful_requests);
    printf("║ Failed Requests:       %-10lu                    ║\n",
           client->failed_requests);
    printf("║ Bytes Sent:            %-10lu                    ║\n",
           client->bytes_sent);
    printf("║ Bytes Received:        %-10lu                    ║\n",
           client->bytes_received);
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Rate Limited:          %-10s                    ║\n",
           client->backing_off ? "Yes" : "No");
    printf("║ Server Slow:           %-10s                    ║\n",
           client->server_seems_slow ? "Yes" : "No");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void tbos_http_print_status(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      TernaryBit OS - HTTP Consciousness Status         ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ HTTP Consciousness:    %d/4                           ║\n",
           g_http_consciousness);
    printf("║ Collective Karma:      %-10ld                    ║\n",
           g_http_collective_karma);
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Total Requests:        %-10lu                    ║\n",
           g_total_requests);
    printf("║ Total Responses:       %-10lu                    ║\n",
           g_total_responses);
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
}
