/**
 * @file wow.h
 * @brief TernaryBit OS - WOW Universal Protocol
 *
 * Unified URL scheme for accessing resources across:
 *   - HTTP/HTTPS (internet)
 *   - FTP (file transfer)
 *   - Sangha (multiverse peer networks)
 *
 * URL Format:
 *   wow://http://host/path
 *   wow://https://host/path
 *   wow://ftp://host/path
 *   wow://sangha-name/path
 *
 * KISS: Simple URL parser + router delegating to existing backends.
 */

#ifndef TBOS_WOW_H
#define TBOS_WOW_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSTANTS
 * ═══════════════════════════════════════════════════════════════════════════ */

#define WOW_MAX_URL       2048
#define WOW_MAX_HOST      256
#define WOW_MAX_PATH      1024
#define WOW_MAX_SANGHA    64

#define WOW_SUCCESS       0
#define WOW_ERROR        -1
#define WOW_NOT_FOUND    -2
#define WOW_AUTH_FAILED  -3
#define WOW_TIMEOUT      -4

/* ═══════════════════════════════════════════════════════════════════════════
 * TYPES
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    WOW_PROTO_HTTP,
    WOW_PROTO_HTTPS,
    WOW_PROTO_FTP,
    WOW_PROTO_SANGHA,
    WOW_PROTO_UNKNOWN
} wow_protocol_t;

typedef struct {
    wow_protocol_t protocol;
    char host[WOW_MAX_HOST];
    uint16_t port;
    char path[WOW_MAX_PATH];
    char sangha[WOW_MAX_SANGHA];  /* Non-empty if Sangha resource */
    char user[64];                 /* For auth */
    char pass[64];                 /* For auth */
} wow_url_t;

typedef struct {
    int status;                    /* HTTP status or 0 for success */
    char content_type[128];
    size_t content_length;
    void* data;                    /* Allocated buffer - caller frees */
    size_t data_len;
    int64_t karma;                 /* Karma earned/lost */
} wow_response_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * URL PARSING
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Parse wow:// URL
 * @param url Full URL string (e.g., "wow://https://example.com/path")
 * @param out Parsed URL structure
 * @return WOW_SUCCESS or error code
 *
 * Examples:
 *   wow://http://example.com/page      -> HTTP, host=example.com, path=/page
 *   wow://https://api.example.com:8443 -> HTTPS, port=8443
 *   wow://ftp://files.host.com/f.zip   -> FTP
 *   wow://my-sangha/shared/doc.txt     -> SANGHA, sangha=my-sangha
 */
int wow_parse(const char* url, wow_url_t* out);

/**
 * @brief Check if URL is a wow:// URL
 */
int wow_is_wow_url(const char* url);

/* ═══════════════════════════════════════════════════════════════════════════
 * FETCH API - Simple one-shot retrieval
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Fetch resource from URL
 * @param url Full wow:// URL
 * @param response Output response (caller must call wow_response_free)
 * @return WOW_SUCCESS or error code
 */
int wow_fetch(const char* url, wow_response_t* response);

/**
 * @brief Free response data
 */
void wow_response_free(wow_response_t* response);

/* ═══════════════════════════════════════════════════════════════════════════
 * STREAM API - For larger transfers
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Open URL for streaming
 * @param url Full wow:// URL
 * @return Handle (>=0) or error code
 */
int wow_open(const char* url);

/**
 * @brief Read from open stream
 */
ssize_t wow_read(int handle, void* buf, size_t len);

/**
 * @brief Write to open stream (for POST/PUT)
 */
ssize_t wow_write(int handle, const void* buf, size_t len);

/**
 * @brief Close stream
 */
int wow_close(int handle);

/* ═══════════════════════════════════════════════════════════════════════════
 * INITIALIZATION
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize WOW subsystem
 */
int wow_init(void);

/**
 * @brief Shutdown WOW subsystem
 */
void wow_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_WOW_H */
