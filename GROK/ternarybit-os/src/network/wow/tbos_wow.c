/**
 * @file tbos_wow.c
 * @brief TernaryBit OS - WOW Universal Protocol Implementation
 *
 * Simple URL router delegating to HTTP/HTTPS/FTP/Sangha backends.
 */

#include "tbos/wow.h"
#include "tbos/net.h"
#include "../dns/tbos_dns.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * INTERNAL STATE
 * ═══════════════════════════════════════════════════════════════════════════ */

#define WOW_MAX_HANDLES 32

typedef struct {
    int in_use;
    wow_url_t url;
    int sock;
    int connected;
} wow_handle_t;

static int g_wow_initialized = 0;
static wow_handle_t g_handles[WOW_MAX_HANDLES];

/* ═══════════════════════════════════════════════════════════════════════════
 * URL PARSING
 * ═══════════════════════════════════════════════════════════════════════════ */

int wow_is_wow_url(const char* url) {
    return url && strncmp(url, "wow://", 6) == 0;
}

int wow_parse(const char* url, wow_url_t* out) {
    if (!url || !out) return WOW_ERROR;

    memset(out, 0, sizeof(wow_url_t));
    out->protocol = WOW_PROTO_UNKNOWN;

    const char* p = url;

    /* Skip wow:// prefix if present */
    if (strncmp(p, "wow://", 6) == 0) {
        p += 6;
    }

    /* Check for nested protocol */
    if (strncmp(p, "http://", 7) == 0) {
        out->protocol = WOW_PROTO_HTTP;
        out->port = 80;
        p += 7;
    } else if (strncmp(p, "https://", 8) == 0) {
        out->protocol = WOW_PROTO_HTTPS;
        out->port = 443;
        p += 8;
    } else if (strncmp(p, "ftp://", 6) == 0) {
        out->protocol = WOW_PROTO_FTP;
        out->port = 21;
        p += 6;
    } else {
        /* Treat as Sangha reference: sangha-name/path */
        out->protocol = WOW_PROTO_SANGHA;
        const char* slash = strchr(p, '/');
        if (slash) {
            size_t len = (size_t)(slash - p);
            if (len >= WOW_MAX_SANGHA) len = WOW_MAX_SANGHA - 1;
            strncpy(out->sangha, p, len);
            strncpy(out->path, slash, WOW_MAX_PATH - 1);
        } else {
            strncpy(out->sangha, p, WOW_MAX_SANGHA - 1);
            strcpy(out->path, "/");
        }
        return WOW_SUCCESS;
    }

    /* Parse user:pass@host:port/path for HTTP/HTTPS/FTP */
    const char* at = strchr(p, '@');
    const char* slash = strchr(p, '/');

    /* Extract credentials if present */
    if (at && (!slash || at < slash)) {
        const char* colon = strchr(p, ':');
        if (colon && colon < at) {
            size_t ulen = (size_t)(colon - p);
            size_t plen = (size_t)(at - colon - 1);
            if (ulen < 64) strncpy(out->user, p, ulen);
            if (plen < 64) strncpy(out->pass, colon + 1, plen);
        }
        p = at + 1;
    }

    /* Find end of host:port */
    const char* host_end = slash ? slash : p + strlen(p);
    const char* port_start = NULL;

    /* Look for port */
    for (const char* c = p; c < host_end; c++) {
        if (*c == ':') {
            port_start = c + 1;
            break;
        }
    }

    /* Extract host */
    size_t host_len;
    if (port_start) {
        host_len = (size_t)(port_start - p - 1);
        out->port = (uint16_t)atoi(port_start);
    } else {
        host_len = (size_t)(host_end - p);
    }

    if (host_len >= WOW_MAX_HOST) host_len = WOW_MAX_HOST - 1;
    strncpy(out->host, p, host_len);

    /* Extract path */
    if (slash) {
        strncpy(out->path, slash, WOW_MAX_PATH - 1);
    } else {
        strcpy(out->path, "/");
    }

    return WOW_SUCCESS;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * BACKEND DISPATCH
 * ═══════════════════════════════════════════════════════════════════════════ */

static int fetch_http(wow_url_t* url, wow_response_t* response) {
    char ip[64];
    if (tbos_dns_resolve_ipv4(url->host, ip, sizeof(ip)) != 0) {
        strncpy(ip, url->host, sizeof(ip) - 1);  /* Maybe it's already an IP */
    }

    int sock = tbos_net_socket(TBOS_SOCK_TCP);
    if (sock < 0) return WOW_ERROR;

    if (tbos_net_connect(sock, ip, url->port) != TBOS_NET_SUCCESS) {
        tbos_net_close(sock);
        return WOW_ERROR;
    }

    /* Build HTTP request */
    char request[2048];
    int len = snprintf(request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: TBOS-WOW/1.0\r\n"
        "Connection: close\r\n"
        "\r\n",
        url->path, url->host);

    if (tbos_net_send(sock, request, (size_t)len) <= 0) {
        tbos_net_close(sock);
        return WOW_ERROR;
    }

    /* Read response */
    char* buffer = malloc(65536);
    if (!buffer) {
        tbos_net_close(sock);
        return WOW_ERROR;
    }

    size_t total = 0;
    ssize_t n;
    while ((n = tbos_net_recv(sock, buffer + total, 65536 - total - 1)) > 0) {
        total += (size_t)n;
        if (total >= 65535) break;
    }
    buffer[total] = '\0';

    tbos_net_close(sock);

    /* Parse HTTP status */
    int status = 0;
    if (strncmp(buffer, "HTTP/", 5) == 0) {
        const char* space = strchr(buffer, ' ');
        if (space) status = atoi(space + 1);
    }

    /* Find body after \r\n\r\n */
    char* body = strstr(buffer, "\r\n\r\n");
    if (body) {
        body += 4;
        size_t body_len = total - (size_t)(body - buffer);

        response->data = malloc(body_len + 1);
        if (response->data) {
            memcpy(response->data, body, body_len);
            ((char*)response->data)[body_len] = '\0';
            response->data_len = body_len;
        }
    }

    free(buffer);

    response->status = status;
    response->karma = 5;  /* Karma for successful fetch */
    return WOW_SUCCESS;
}

static int fetch_ftp(wow_url_t* url, wow_response_t* response) {
    void* data = NULL;
    size_t len = 0;

    const char* user = url->user[0] ? url->user : "anonymous";
    const char* pass = url->pass[0] ? url->pass : "anonymous@tbos.local";

    /* Use tbos_ftp_fetch for one-shot retrieval */
    /* For now, simplified - would call actual FTP client */
    char ip[64];
    if (tbos_dns_resolve_ipv4(url->host, ip, sizeof(ip)) != 0) {
        strncpy(ip, url->host, sizeof(ip) - 1);
    }

    int sock = tbos_net_socket(TBOS_SOCK_TCP);
    if (sock < 0) {
        response->status = 500;
        return WOW_ERROR;
    }

    if (tbos_net_connect(sock, ip, url->port) != TBOS_NET_SUCCESS) {
        tbos_net_close(sock);
        response->status = 500;
        return WOW_ERROR;
    }

    /* Simple FTP dialog */
    char buf[4096];
    char cmd[512];

    /* Read welcome */
    tbos_net_recv(sock, buf, sizeof(buf));

    /* USER */
    snprintf(cmd, sizeof(cmd), "USER %s\r\n", user);
    tbos_net_send(sock, cmd, strlen(cmd));
    tbos_net_recv(sock, buf, sizeof(buf));

    /* PASS */
    snprintf(cmd, sizeof(cmd), "PASS %s\r\n", pass);
    tbos_net_send(sock, cmd, strlen(cmd));
    tbos_net_recv(sock, buf, sizeof(buf));

    /* QUIT */
    tbos_net_send(sock, "QUIT\r\n", 6);
    tbos_net_close(sock);

    response->status = 0;
    response->data = strdup("FTP connection established (full retrieval requires PASV mode)");
    response->data_len = strlen(response->data);
    response->karma = 5;

    return WOW_SUCCESS;
}

static int fetch_sangha(wow_url_t* url, wow_response_t* response) {
    /* Sangha lookup - would query peer network */
    (void)url;
    response->status = 0;
    response->data = strdup("Sangha resource lookup pending");
    response->data_len = strlen(response->data);
    response->karma = 10;  /* Extra karma for Sangha interaction */
    return WOW_SUCCESS;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PUBLIC API
 * ═══════════════════════════════════════════════════════════════════════════ */

int wow_init(void) {
    if (g_wow_initialized) return WOW_SUCCESS;

    tbos_net_init();
    tbos_dns_init();

    memset(g_handles, 0, sizeof(g_handles));
    g_wow_initialized = 1;

    return WOW_SUCCESS;
}

void wow_shutdown(void) {
    for (int i = 0; i < WOW_MAX_HANDLES; i++) {
        if (g_handles[i].in_use) {
            wow_close(i);
        }
    }
    g_wow_initialized = 0;
}

int wow_fetch(const char* url, wow_response_t* response) {
    if (!url || !response) return WOW_ERROR;
    if (!g_wow_initialized) wow_init();

    memset(response, 0, sizeof(wow_response_t));

    wow_url_t parsed;
    if (wow_parse(url, &parsed) != WOW_SUCCESS) {
        return WOW_ERROR;
    }

    switch (parsed.protocol) {
        case WOW_PROTO_HTTP:
        case WOW_PROTO_HTTPS:  /* HTTPS treated as HTTP for now */
            return fetch_http(&parsed, response);

        case WOW_PROTO_FTP:
            return fetch_ftp(&parsed, response);

        case WOW_PROTO_SANGHA:
            return fetch_sangha(&parsed, response);

        default:
            return WOW_ERROR;
    }
}

void wow_response_free(wow_response_t* response) {
    if (response && response->data) {
        free(response->data);
        response->data = NULL;
        response->data_len = 0;
    }
}

int wow_open(const char* url) {
    if (!g_wow_initialized) wow_init();

    int slot = -1;
    for (int i = 0; i < WOW_MAX_HANDLES; i++) {
        if (!g_handles[i].in_use) {
            slot = i;
            break;
        }
    }
    if (slot < 0) return WOW_ERROR;

    wow_url_t parsed;
    if (wow_parse(url, &parsed) != WOW_SUCCESS) {
        return WOW_ERROR;
    }

    g_handles[slot].in_use = 1;
    g_handles[slot].url = parsed;
    g_handles[slot].sock = -1;
    g_handles[slot].connected = 0;

    /* For HTTP/HTTPS/FTP, establish connection */
    if (parsed.protocol != WOW_PROTO_SANGHA) {
        char ip[64];
        if (tbos_dns_resolve_ipv4(parsed.host, ip, sizeof(ip)) != 0) {
            strncpy(ip, parsed.host, sizeof(ip) - 1);
        }

        int sock = tbos_net_socket(TBOS_SOCK_TCP);
        if (sock < 0) {
            g_handles[slot].in_use = 0;
            return WOW_ERROR;
        }

        if (tbos_net_connect(sock, ip, parsed.port) != TBOS_NET_SUCCESS) {
            tbos_net_close(sock);
            g_handles[slot].in_use = 0;
            return WOW_ERROR;
        }

        g_handles[slot].sock = sock;
        g_handles[slot].connected = 1;
    }

    return slot;
}

ssize_t wow_read(int handle, void* buf, size_t len) {
    if (handle < 0 || handle >= WOW_MAX_HANDLES) return -1;
    if (!g_handles[handle].in_use) return -1;

    if (g_handles[handle].sock >= 0) {
        return tbos_net_recv(g_handles[handle].sock, buf, len);
    }
    return -1;
}

ssize_t wow_write(int handle, const void* buf, size_t len) {
    if (handle < 0 || handle >= WOW_MAX_HANDLES) return -1;
    if (!g_handles[handle].in_use) return -1;

    if (g_handles[handle].sock >= 0) {
        return tbos_net_send(g_handles[handle].sock, buf, len);
    }
    return -1;
}

int wow_close(int handle) {
    if (handle < 0 || handle >= WOW_MAX_HANDLES) return WOW_ERROR;
    if (!g_handles[handle].in_use) return WOW_ERROR;

    if (g_handles[handle].sock >= 0) {
        tbos_net_close(g_handles[handle].sock);
    }

    g_handles[handle].in_use = 0;
    return WOW_SUCCESS;
}
