/**
 * Quick network stack test - standalone
 *
 * Build & run:
 *   gcc -DTBOS_HOSTED -o test_quick tests/test_net_quick.c \
 *       src/network/tbos_net.c \
 *       src/network/dns/tbos_dns.c \
 *       src/network/wow/tbos_wow.c \
 *       kernel/hal_hosted.c \
 *       -Iinclude -Isrc/network && ./test_quick
 */

#include <stdio.h>
#include <string.h>
#include "tbos/net.h"
#include "tbos/wow.h"
#include "dns/tbos_dns.h"

int main(void) {
    printf("\n=== TBOS Network Stack Quick Test ===\n\n");

    /* Test 1: Initialize */
    printf("1. Initializing network... ");
    if (tbos_net_init() == TBOS_NET_SUCCESS) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        return 1;
    }

    /* Test 2: Get local IP */
    printf("2. Getting local IP... ");
    char ip[64];
    if (tbos_net_get_local_ip(ip, sizeof(ip)) == 0) {
        printf("%s\n", ip);
    } else {
        printf("FAILED\n");
    }

    /* Test 3: DNS resolution */
    printf("3. Resolving 'localhost'... ");
    char resolved_ip[64];
    if (tbos_dns_resolve_ipv4("localhost", resolved_ip, sizeof(resolved_ip)) == 0) {
        printf("%s\n", resolved_ip);
    } else {
        printf("FAILED\n");
    }

    printf("4. Resolving 'google.com'... ");
    if (tbos_dns_resolve_ipv4("google.com", resolved_ip, sizeof(resolved_ip)) == 0) {
        printf("%s\n", resolved_ip);
    } else {
        printf("FAILED (may need internet)\n");
    }

    /* Test 4: TCP socket */
    printf("5. Creating TCP socket... ");
    int sock = tbos_net_socket(TBOS_SOCK_TCP);
    if (sock >= 0) {
        printf("OK (handle=%d)\n", sock);
        tbos_net_close(sock);
    } else {
        printf("FAILED\n");
    }

    /* Test 5: UDP socket */
    printf("6. Creating UDP socket... ");
    sock = tbos_net_socket(TBOS_SOCK_UDP);
    if (sock >= 0) {
        printf("OK (handle=%d)\n", sock);
        tbos_net_close(sock);
    } else {
        printf("FAILED\n");
    }

    /* Test 6: WOW URL parsing */
    printf("7. Parsing WOW URLs...\n");
    wow_init();

    wow_url_t url;
    const char* test_urls[] = {
        "wow://http://example.com/path",
        "wow://https://api.example.com:8443/v1",
        "wow://ftp://files.example.com/file.zip",
        "wow://my-sangha/shared/doc.txt",
        NULL
    };

    for (int i = 0; test_urls[i]; i++) {
        if (wow_parse(test_urls[i], &url) == WOW_SUCCESS) {
            printf("   %s\n", test_urls[i]);
            printf("     -> proto=%d host=%s port=%d path=%s sangha=%s\n",
                   url.protocol, url.host, url.port, url.path, url.sangha);
        }
    }

    /* Test 7: HTTP fetch (if internet available) */
    printf("8. Fetching http://example.com... ");
    fflush(stdout);

    wow_response_t response;
    if (wow_fetch("wow://http://example.com/", &response) == WOW_SUCCESS) {
        printf("OK (status=%d, %zu bytes)\n", response.status, response.data_len);
        if (response.data && response.data_len > 0) {
            /* Show first 100 chars */
            char preview[101];
            size_t len = response.data_len < 100 ? response.data_len : 100;
            memcpy(preview, response.data, len);
            preview[len] = '\0';
            printf("   Preview: %.60s...\n", preview);
        }
        wow_response_free(&response);
    } else {
        printf("FAILED (need internet)\n");
    }

    /* Stats */
    printf("\n9. Network stats:\n");
    uint64_t sent, recv;
    int active;
    tbos_net_stats(&sent, &recv, &active);
    printf("   Bytes sent: %llu\n", (unsigned long long)sent);
    printf("   Bytes recv: %llu\n", (unsigned long long)recv);
    printf("   Active sockets: %d\n", active);

    printf("\n=== All tests completed ===\n\n");
    return 0;
}
