/**
 * @file test_network_hosted.c
 * @brief Unit tests for TBOS hosted network stack
 *
 * Build:
 *   gcc -DTBOS_HOSTED -DHOST_BUILD -o test_net \
 *       tests/unit/test_network_hosted.c \
 *       src/network/tbos_net.c \
 *       src/network/dns/tbos_dns.c \
 *       src/network/transport/udp/tbos_udp.c \
 *       src/network/wow/tbos_wow.c \
 *       kernel/hal_hosted.c \
 *       -Iinclude -Isrc/network
 *   ./test_net
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tbos/net.h"
#include "tbos/wow.h"
#include "dns/tbos_dns.h"
#include "transport/udp/tbos_udp.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST UTILITIES
 * ═══════════════════════════════════════════════════════════════════════════ */

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  %-40s ", #name); \
    fflush(stdout); \
    test_##name(); \
    tests_run++; \
    tests_passed++; \
    printf("[PASS]\n"); \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        exit(1); \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_STR_EQ(a, b) ASSERT(strcmp((a), (b)) == 0)

/* ═══════════════════════════════════════════════════════════════════════════
 * NET API TESTS
 * ═══════════════════════════════════════════════════════════════════════════ */

TEST(net_init) {
    int result = tbos_net_init();
    ASSERT_EQ(result, TBOS_NET_SUCCESS);
}

TEST(net_socket_tcp) {
    int sock = tbos_net_socket(TBOS_SOCK_TCP);
    ASSERT(sock >= 0);
    tbos_net_close(sock);
}

TEST(net_socket_udp) {
    int sock = tbos_net_socket(TBOS_SOCK_UDP);
    ASSERT(sock >= 0);
    tbos_net_close(sock);
}

TEST(net_get_local_ip) {
    char ip[64];
    int result = tbos_net_get_local_ip(ip, sizeof(ip));
    ASSERT_EQ(result, 0);
    ASSERT(strlen(ip) > 0);
}

TEST(net_stats) {
    uint64_t sent, recv;
    int active;
    tbos_net_stats(&sent, &recv, &active);
    /* Should not crash, values can be anything */
}

TEST(net_bind) {
    int sock = tbos_net_socket(TBOS_SOCK_TCP);
    ASSERT(sock >= 0);

    /* Bind to ephemeral port */
    int result = tbos_net_bind(sock, 0);
    ASSERT_EQ(result, TBOS_NET_SUCCESS);

    tbos_net_close(sock);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DNS TESTS
 * ═══════════════════════════════════════════════════════════════════════════ */

TEST(dns_init) {
    int result = tbos_dns_init();
    ASSERT_EQ(result, 0);
}

TEST(dns_resolve_localhost) {
    char ip[64];
    int result = tbos_dns_resolve_ipv4("localhost", ip, sizeof(ip));
    ASSERT_EQ(result, 0);
    ASSERT(strlen(ip) > 0);
}

TEST(dns_cache_stats) {
    int entries, hits, misses;
    tbos_dns_cache_stats(&entries, &hits, &misses);
    /* Should not crash */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * UDP TESTS
 * ═══════════════════════════════════════════════════════════════════════════ */

TEST(udp_init) {
    int result = tbos_udp_init();
    ASSERT_EQ(result, TBOS_NET_SUCCESS);
}

TEST(udp_socket) {
    int sock = tbos_udp_socket();
    ASSERT(sock >= 0);
    tbos_udp_close(sock);
}

TEST(udp_bind) {
    int sock = tbos_udp_socket();
    ASSERT(sock >= 0);

    int result = tbos_udp_bind(sock, 0);
    ASSERT_EQ(result, TBOS_NET_SUCCESS);

    tbos_udp_close(sock);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WOW PROTOCOL TESTS
 * ═══════════════════════════════════════════════════════════════════════════ */

TEST(wow_init) {
    int result = wow_init();
    ASSERT_EQ(result, WOW_SUCCESS);
}

TEST(wow_is_wow_url) {
    ASSERT_EQ(wow_is_wow_url("wow://http://example.com"), 1);
    ASSERT_EQ(wow_is_wow_url("http://example.com"), 0);
    ASSERT_EQ(wow_is_wow_url(NULL), 0);
}

TEST(wow_parse_http) {
    wow_url_t url;
    int result = wow_parse("wow://http://example.com/path", &url);
    ASSERT_EQ(result, WOW_SUCCESS);
    ASSERT_EQ(url.protocol, WOW_PROTO_HTTP);
    ASSERT_STR_EQ(url.host, "example.com");
    ASSERT_STR_EQ(url.path, "/path");
    ASSERT_EQ(url.port, 80);
}

TEST(wow_parse_https) {
    wow_url_t url;
    int result = wow_parse("wow://https://secure.example.com:8443/api", &url);
    ASSERT_EQ(result, WOW_SUCCESS);
    ASSERT_EQ(url.protocol, WOW_PROTO_HTTPS);
    ASSERT_STR_EQ(url.host, "secure.example.com");
    ASSERT_EQ(url.port, 8443);
}

TEST(wow_parse_ftp) {
    wow_url_t url;
    int result = wow_parse("wow://ftp://files.example.com/file.zip", &url);
    ASSERT_EQ(result, WOW_SUCCESS);
    ASSERT_EQ(url.protocol, WOW_PROTO_FTP);
    ASSERT_STR_EQ(url.host, "files.example.com");
}

TEST(wow_parse_sangha) {
    wow_url_t url;
    int result = wow_parse("wow://my-sangha/shared/doc.txt", &url);
    ASSERT_EQ(result, WOW_SUCCESS);
    ASSERT_EQ(url.protocol, WOW_PROTO_SANGHA);
    ASSERT_STR_EQ(url.sangha, "my-sangha");
    ASSERT_STR_EQ(url.path, "/shared/doc.txt");
}

TEST(wow_parse_with_auth) {
    wow_url_t url;
    int result = wow_parse("wow://http://user:pass@example.com/", &url);
    ASSERT_EQ(result, WOW_SUCCESS);
    ASSERT_STR_EQ(url.user, "user");
    ASSERT_STR_EQ(url.pass, "pass");
    ASSERT_STR_EQ(url.host, "example.com");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════════════════════════ */

int main(void) {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("  TBOS Network Stack - Hosted Mode Tests\n");
    printf("═══════════════════════════════════════════════════════════\n\n");

    printf("NET API Tests:\n");
    RUN_TEST(net_init);
    RUN_TEST(net_socket_tcp);
    RUN_TEST(net_socket_udp);
    RUN_TEST(net_get_local_ip);
    RUN_TEST(net_stats);
    RUN_TEST(net_bind);

    printf("\nDNS Tests:\n");
    RUN_TEST(dns_init);
    RUN_TEST(dns_resolve_localhost);
    RUN_TEST(dns_cache_stats);

    printf("\nUDP Tests:\n");
    RUN_TEST(udp_init);
    RUN_TEST(udp_socket);
    RUN_TEST(udp_bind);

    printf("\nWOW Protocol Tests:\n");
    RUN_TEST(wow_init);
    RUN_TEST(wow_is_wow_url);
    RUN_TEST(wow_parse_http);
    RUN_TEST(wow_parse_https);
    RUN_TEST(wow_parse_ftp);
    RUN_TEST(wow_parse_sangha);
    RUN_TEST(wow_parse_with_auth);

    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("  Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("═══════════════════════════════════════════════════════════\n\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
