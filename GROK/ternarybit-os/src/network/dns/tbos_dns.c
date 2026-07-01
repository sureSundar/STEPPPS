/**
 * @file tbos_dns.c
 * @brief TernaryBit OS - DNS Resolution Implementation
 */

#include "tbos_dns.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * INTERNAL STATE
 * ═══════════════════════════════════════════════════════════════════════════ */

static bool g_dns_initialized = false;
static tbos_dns_cache_entry_t g_cache[TBOS_DNS_MAX_CACHE];
static int g_cache_hits = 0;
static int g_cache_misses = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

static uint32_t get_time(void) {
    return (uint32_t)time(NULL);
}

static tbos_dns_cache_entry_t* cache_lookup(const char* hostname) {
    uint32_t now = get_time();

    for (int i = 0; i < TBOS_DNS_MAX_CACHE; i++) {
        if (g_cache[i].valid &&
            strcmp(g_cache[i].hostname, hostname) == 0) {
            if (now - g_cache[i].timestamp < g_cache[i].ttl) {
                return &g_cache[i];
            }
            /* Expired */
            g_cache[i].valid = false;
        }
    }
    return NULL;
}

static void cache_add(const char* hostname, const char* ip) {
    int slot = -1;
    uint32_t oldest = UINT32_MAX;

    /* Find empty or oldest slot */
    for (int i = 0; i < TBOS_DNS_MAX_CACHE; i++) {
        if (!g_cache[i].valid) {
            slot = i;
            break;
        }
        if (g_cache[i].timestamp < oldest) {
            oldest = g_cache[i].timestamp;
            slot = i;
        }
    }

    if (slot >= 0) {
        strncpy(g_cache[slot].hostname, hostname, TBOS_DNS_MAX_HOSTNAME - 1);
        strncpy(g_cache[slot].ip, ip, TBOS_DNS_MAX_IP - 1);
        g_cache[slot].timestamp = get_time();
        g_cache[slot].ttl = TBOS_DNS_CACHE_TTL;
        g_cache[slot].valid = true;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * API IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_dns_init(void) {
    if (g_dns_initialized) return 0;

    memset(g_cache, 0, sizeof(g_cache));
    g_cache_hits = 0;
    g_cache_misses = 0;
    g_dns_initialized = true;

    return 0;
}

int tbos_dns_resolve(const char* hostname, tbos_dns_result_t* result) {
    if (!hostname || !result) return -1;
    if (!g_dns_initialized) tbos_dns_init();

    memset(result, 0, sizeof(tbos_dns_result_t));

    /* Check cache first */
    tbos_dns_cache_entry_t* cached = cache_lookup(hostname);
    if (cached) {
        strncpy(result->ip, cached->ip, TBOS_DNS_MAX_IP - 1);
        result->family = AF_INET;
        g_cache_hits++;
        return 0;
    }

    g_cache_misses++;

    /* Use getaddrinfo for resolution */
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  /* IPv4 */
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(hostname, NULL, &hints, &res);
    if (err != 0) {
        return -1;
    }

    struct sockaddr_in* addr = (struct sockaddr_in*)res->ai_addr;
    inet_ntop(AF_INET, &addr->sin_addr, result->ip, TBOS_DNS_MAX_IP);
    result->family = AF_INET;

    /* Add to cache */
    cache_add(hostname, result->ip);

    freeaddrinfo(res);
    return 0;
}

int tbos_dns_resolve_ipv4(const char* hostname, char* ip_buf, size_t buf_len) {
    if (!hostname || !ip_buf || buf_len == 0) return -1;

    tbos_dns_result_t result;
    int err = tbos_dns_resolve(hostname, &result);
    if (err != 0) return -1;

    strncpy(ip_buf, result.ip, buf_len - 1);
    ip_buf[buf_len - 1] = '\0';
    return 0;
}

void tbos_dns_cache_clear(void) {
    for (int i = 0; i < TBOS_DNS_MAX_CACHE; i++) {
        g_cache[i].valid = false;
    }
}

void tbos_dns_cache_stats(int* entries, int* hits, int* misses) {
    int count = 0;
    for (int i = 0; i < TBOS_DNS_MAX_CACHE; i++) {
        if (g_cache[i].valid) count++;
    }

    if (entries) *entries = count;
    if (hits) *hits = g_cache_hits;
    if (misses) *misses = g_cache_misses;
}

void tbos_dns_shutdown(void) {
    tbos_dns_cache_clear();
    g_dns_initialized = false;
}
