/**
 * @file tbos_http.h
 * @brief TernaryBit OS - Conscious HTTP Implementation
 *
 * HTTP with compassion - every request is mindful, every response is helpful.
 *
 * Philosophy:
 * - HTTP requests follow Right Speech (truthful, beneficial, timely, gentle, necessary)
 * - Servers practice compassion (serve all equally, helpful errors)
 * - Clients practice patience (respect rate limits, back off on errors)
 * - Cookies and sessions managed ethically
 *
 * @version 1.0
 * @date 2025-11-03
 */

#ifndef TBOS_HTTP_H
#define TBOS_HTTP_H

#include "../../core/tbos_network.h"
#include "../../transport/tcp/tbos_tcp.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* HTTP CONSTANTS                                                            */
/* ========================================================================= */

#define TBOS_HTTP_MAX_URL_LENGTH      2048
#define TBOS_HTTP_MAX_HEADER_LENGTH   8192
#define TBOS_HTTP_MAX_BODY_LENGTH     1048576  /* 1MB */
#define TBOS_HTTP_DEFAULT_PORT        80
#define TBOS_HTTP_DEFAULT_PORT_HTTPS  443

/* ========================================================================= */
/* HTTP METHODS                                                              */
/* ========================================================================= */

typedef enum {
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_PATCH,
} tbos_http_method_t;

/* ========================================================================= */
/* HTTP STATUS CODES                                                         */
/* ========================================================================= */

typedef enum {
    HTTP_STATUS_OK = 200,
    HTTP_STATUS_CREATED = 201,
    HTTP_STATUS_ACCEPTED = 202,
    HTTP_STATUS_NO_CONTENT = 204,

    HTTP_STATUS_MOVED_PERMANENTLY = 301,
    HTTP_STATUS_FOUND = 302,
    HTTP_STATUS_NOT_MODIFIED = 304,

    HTTP_STATUS_BAD_REQUEST = 400,
    HTTP_STATUS_UNAUTHORIZED = 401,
    HTTP_STATUS_FORBIDDEN = 403,
    HTTP_STATUS_NOT_FOUND = 404,
    HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
    HTTP_STATUS_TOO_MANY_REQUESTS = 429,

    HTTP_STATUS_INTERNAL_SERVER_ERROR = 500,
    HTTP_STATUS_NOT_IMPLEMENTED = 501,
    HTTP_STATUS_BAD_GATEWAY = 502,
    HTTP_STATUS_SERVICE_UNAVAILABLE = 503,
} tbos_http_status_t;

/* ========================================================================= */
/* HTTP STRUCTURES                                                           */
/* ========================================================================= */

/**
 * @brief HTTP header
 */
typedef struct tbos_http_header {
    char* name;
    char* value;
    struct tbos_http_header* next;
} tbos_http_header_t;

/**
 * @brief Conscious HTTP request
 */
typedef struct {
    /* Request line */
    tbos_http_method_t method;
    char url[TBOS_HTTP_MAX_URL_LENGTH];
    char version[16];  /* e.g., "HTTP/1.1" */

    /* Headers */
    tbos_http_header_t* headers;
    uint32_t header_count;

    /* Body */
    void* body;
    size_t body_length;

    /* Consciousness */
    bool follows_right_speech;
    bool is_mindful_request;
    karma_score_t request_karma;

    /* Rate limiting awareness */
    uint32_t requests_this_minute;
    bool respecting_rate_limit;

} tbos_http_request_t;

/**
 * @brief Compassionate HTTP response
 */
typedef struct {
    /* Status line */
    char version[16];  /* e.g., "HTTP/1.1" */
    tbos_http_status_t status_code;
    char reason_phrase[128];

    /* Headers */
    tbos_http_header_t* headers;
    uint32_t header_count;

    /* Body */
    void* body;
    size_t body_length;

    /* Consciousness */
    bool was_helpful;
    bool served_with_compassion;
    karma_score_t response_karma;

} tbos_http_response_t;

/**
 * @brief Conscious HTTP client
 */
typedef struct {
    /* Connection */
    int sockfd;
    char host[256];
    uint16_t port;
    bool connected;

    /* Consciousness */
    consciousness_level_t awareness;
    karma_score_t client_karma;

    /* Mindful behavior */
    uint32_t requests_per_minute_limit;
    uint32_t current_request_rate;
    bool server_seems_slow;
    bool backing_off;
    uint32_t backoff_time_ms;

    /* Statistics */
    uint64_t successful_requests;
    uint64_t failed_requests;
    uint64_t bytes_sent;
    uint64_t bytes_received;

} tbos_http_client_t;

/**
 * @brief Compassionate HTTP server
 */
typedef struct {
    /* Listen socket */
    int listen_sockfd;
    uint16_t port;
    bool running;

    /* Consciousness */
    consciousness_level_t awareness;
    karma_score_t server_karma;

    /* Compassion metrics */
    uint64_t clients_served;
    uint64_t slow_clients_helped;
    uint64_t errors_handled_gracefully;
    bool serves_all_equally;
    bool provides_helpful_errors;

    /* Statistics */
    uint64_t total_requests;
    uint64_t bytes_served;

} tbos_http_server_t;

/* ========================================================================= */
/* HTTP CLIENT API                                                           */
/* ========================================================================= */

/**
 * @brief Create conscious HTTP client
 */
tbos_http_client_t* tbos_http_client_create(const char* host, uint16_t port);

/**
 * @brief Make GET request with mindfulness
 */
tbos_http_response_t* tbos_http_get(tbos_http_client_t* client, const char* path);

/**
 * @brief Make POST request with Right Speech
 */
tbos_http_response_t* tbos_http_post(tbos_http_client_t* client,
                                      const char* path,
                                      const void* body,
                                      size_t body_length);

/**
 * @brief Make PUT request
 */
tbos_http_response_t* tbos_http_put(tbos_http_client_t* client,
                                     const char* path,
                                     const void* body,
                                     size_t body_length);

/**
 * @brief Make DELETE request
 */
tbos_http_response_t* tbos_http_delete(tbos_http_client_t* client, const char* path);

/**
 * @brief Close client connection
 */
void tbos_http_client_destroy(tbos_http_client_t* client);

/**
 * @brief Print client consciousness status
 */
void tbos_http_client_print_status(tbos_http_client_t* client);

/* ========================================================================= */
/* HTTP SERVER API                                                           */
/* ========================================================================= */

/**
 * @brief Create compassionate HTTP server
 */
tbos_http_server_t* tbos_http_server_create(uint16_t port);

/**
 * @brief Start server with compassion
 */
int tbos_http_server_start(tbos_http_server_t* server);

/**
 * @brief Stop server gracefully
 */
int tbos_http_server_stop(tbos_http_server_t* server);

/**
 * @brief Print server consciousness status
 */
void tbos_http_server_print_status(tbos_http_server_t* server);

/* ========================================================================= */
/* RESPONSE HELPERS                                                          */
/* ========================================================================= */

/**
 * @brief Free HTTP response
 */
void tbos_http_response_free(tbos_http_response_t* response);

/**
 * @brief Get header value
 */
const char* tbos_http_get_header(tbos_http_response_t* response, const char* name);

/**
 * @brief Print response details
 */
void tbos_http_response_print(tbos_http_response_t* response);

/* ========================================================================= */
/* INITIALIZATION                                                            */
/* ========================================================================= */

/**
 * @brief Initialize conscious HTTP subsystem
 */
int tbos_http_init(void);

/**
 * @brief Print HTTP consciousness status
 */
void tbos_http_print_status(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_HTTP_H */
