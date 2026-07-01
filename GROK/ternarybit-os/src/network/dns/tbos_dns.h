/**
 * @file tbos_dns.h
 * @brief TernaryBit OS - DNS Resolution
 *
 * Simple DNS client with caching.
 * Uses getaddrinfo() for hosted mode.
 */

#ifndef TBOS_DNS_H
#define TBOS_DNS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSTANTS
 * ═══════════════════════════════════════════════════════════════════════════ */

#define TBOS_DNS_MAX_CACHE      64
#define TBOS_DNS_CACHE_TTL      300   /* 5 minutes */
#define TBOS_DNS_MAX_HOSTNAME   256
#define TBOS_DNS_MAX_IP         64

/* ═══════════════════════════════════════════════════════════════════════════
 * STRUCTURES
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    char hostname[TBOS_DNS_MAX_HOSTNAME];
    char ip[TBOS_DNS_MAX_IP];
    uint32_t timestamp;
    uint32_t ttl;
    bool valid;
} tbos_dns_cache_entry_t;

typedef struct {
    char ip[TBOS_DNS_MAX_IP];
    int family;  /* AF_INET or AF_INET6 */
} tbos_dns_result_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize DNS subsystem
 */
int tbos_dns_init(void);

/**
 * @brief Resolve hostname to IP address
 * @param hostname Hostname to resolve
 * @param result Output: resolved IP
 * @return 0 on success, -1 on error
 */
int tbos_dns_resolve(const char* hostname, tbos_dns_result_t* result);

/**
 * @brief Resolve hostname to IPv4 string
 * @param hostname Hostname to resolve
 * @param ip_buf Output buffer for IP string
 * @param buf_len Buffer length
 * @return 0 on success, -1 on error
 */
int tbos_dns_resolve_ipv4(const char* hostname, char* ip_buf, size_t buf_len);

/**
 * @brief Clear DNS cache
 */
void tbos_dns_cache_clear(void);

/**
 * @brief Get cache statistics
 */
void tbos_dns_cache_stats(int* entries, int* hits, int* misses);

/**
 * @brief Shutdown DNS subsystem
 */
void tbos_dns_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_DNS_H */
