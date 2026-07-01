/**
 * @file tbos_ftp.c
 * @brief TernaryBit OS - Simple FTP Client Implementation
 */

#include "tbos_ftp.h"
#include "tbos/net.h"
#include "dns/tbos_dns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int ftp_read_response(tbos_ftp_t* ftp) {
    char buf[TBOS_FTP_MAX_RESPONSE];
    ssize_t n = tbos_net_recv(ftp->ctrl_sock, buf, sizeof(buf) - 1);
    if (n <= 0) return TBOS_FTP_ERROR;

    buf[n] = '\0';
    strncpy(ftp->last_response, buf, TBOS_FTP_MAX_RESPONSE - 1);

    /* Parse response code (first 3 digits) */
    if (n >= 3) {
        ftp->last_code = (buf[0] - '0') * 100 +
                         (buf[1] - '0') * 10 +
                         (buf[2] - '0');
    }

    return ftp->last_code;
}

static int ftp_send_cmd(tbos_ftp_t* ftp, const char* cmd) {
    char buf[512];
    int len = snprintf(buf, sizeof(buf), "%s\r\n", cmd);
    if (tbos_net_send(ftp->ctrl_sock, buf, (size_t)len) <= 0) {
        return TBOS_FTP_ERROR;
    }
    return ftp_read_response(ftp);
}

static int ftp_enter_passive(tbos_ftp_t* ftp, char* ip, uint16_t* port) {
    int code = ftp_send_cmd(ftp, "PASV");
    if (code != 227) return TBOS_FTP_ERROR;

    /* Parse: 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2) */
    char* start = strchr(ftp->last_response, '(');
    if (!start) return TBOS_FTP_ERROR;

    int h1, h2, h3, h4, p1, p2;
    if (sscanf(start, "(%d,%d,%d,%d,%d,%d)",
               &h1, &h2, &h3, &h4, &p1, &p2) != 6) {
        return TBOS_FTP_ERROR;
    }

    snprintf(ip, 64, "%d.%d.%d.%d", h1, h2, h3, h4);
    *port = (uint16_t)(p1 * 256 + p2);

    return TBOS_FTP_SUCCESS;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * API IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_ftp_connect(tbos_ftp_t* ftp, const char* host, uint16_t port) {
    if (!ftp || !host) return TBOS_FTP_ERROR;

    memset(ftp, 0, sizeof(tbos_ftp_t));
    ftp->ctrl_sock = -1;
    ftp->data_sock = -1;

    strncpy(ftp->host, host, sizeof(ftp->host) - 1);
    ftp->port = port ? port : TBOS_FTP_PORT;

    /* Resolve hostname */
    char ip[64];
    if (tbos_dns_resolve_ipv4(host, ip, sizeof(ip)) != 0) {
        strncpy(ip, host, sizeof(ip) - 1);
    }

    /* Connect control socket */
    ftp->ctrl_sock = tbos_net_socket(TBOS_SOCK_TCP);
    if (ftp->ctrl_sock < 0) return TBOS_FTP_ERROR;

    if (tbos_net_connect(ftp->ctrl_sock, ip, ftp->port) != TBOS_NET_SUCCESS) {
        tbos_net_close(ftp->ctrl_sock);
        ftp->ctrl_sock = -1;
        return TBOS_FTP_ERROR;
    }

    /* Read welcome message */
    int code = ftp_read_response(ftp);
    if (code != 220) {
        tbos_ftp_close(ftp);
        return TBOS_FTP_ERROR;
    }

    return TBOS_FTP_SUCCESS;
}

int tbos_ftp_login(tbos_ftp_t* ftp, const char* user, const char* pass) {
    if (!ftp || ftp->ctrl_sock < 0) return TBOS_FTP_ERROR;

    char cmd[256];

    /* Send USER */
    snprintf(cmd, sizeof(cmd), "USER %s", user ? user : "anonymous");
    int code = ftp_send_cmd(ftp, cmd);
    if (code != 331 && code != 230) return TBOS_FTP_AUTH_FAIL;

    /* Send PASS if needed */
    if (code == 331) {
        snprintf(cmd, sizeof(cmd), "PASS %s", pass ? pass : "anonymous@");
        code = ftp_send_cmd(ftp, cmd);
        if (code != 230) return TBOS_FTP_AUTH_FAIL;
    }

    ftp->logged_in = 1;

    /* Set binary mode */
    ftp_send_cmd(ftp, "TYPE I");

    return TBOS_FTP_SUCCESS;
}

int tbos_ftp_get(tbos_ftp_t* ftp, const char* path, void** data, size_t* len) {
    if (!ftp || !path || !data || !len) return TBOS_FTP_ERROR;
    if (!ftp->logged_in) return TBOS_FTP_ERROR;

    *data = NULL;
    *len = 0;

    /* Enter passive mode */
    char pasv_ip[64];
    uint16_t pasv_port;
    if (ftp_enter_passive(ftp, pasv_ip, &pasv_port) != TBOS_FTP_SUCCESS) {
        return TBOS_FTP_ERROR;
    }

    /* Connect data socket */
    ftp->data_sock = tbos_net_socket(TBOS_SOCK_TCP);
    if (ftp->data_sock < 0) return TBOS_FTP_ERROR;

    if (tbos_net_connect(ftp->data_sock, pasv_ip, pasv_port) != TBOS_NET_SUCCESS) {
        tbos_net_close(ftp->data_sock);
        ftp->data_sock = -1;
        return TBOS_FTP_ERROR;
    }

    /* Send RETR command */
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "RETR %s", path);
    int code = ftp_send_cmd(ftp, cmd);
    if (code != 150 && code != 125) {
        tbos_net_close(ftp->data_sock);
        ftp->data_sock = -1;
        return TBOS_FTP_NOT_FOUND;
    }

    /* Read data */
    size_t capacity = 65536;
    char* buffer = malloc(capacity);
    if (!buffer) {
        tbos_net_close(ftp->data_sock);
        ftp->data_sock = -1;
        return TBOS_FTP_ERROR;
    }

    size_t total = 0;
    ssize_t n;
    while ((n = tbos_net_recv(ftp->data_sock, buffer + total, capacity - total)) > 0) {
        total += (size_t)n;
        if (total >= capacity - 1024) {
            capacity *= 2;
            char* newbuf = realloc(buffer, capacity);
            if (!newbuf) break;
            buffer = newbuf;
        }
    }

    tbos_net_close(ftp->data_sock);
    ftp->data_sock = -1;

    /* Read transfer complete response */
    ftp_read_response(ftp);

    *data = buffer;
    *len = total;

    return TBOS_FTP_SUCCESS;
}

int tbos_ftp_close(tbos_ftp_t* ftp) {
    if (!ftp) return TBOS_FTP_ERROR;

    if (ftp->logged_in && ftp->ctrl_sock >= 0) {
        ftp_send_cmd(ftp, "QUIT");
    }

    if (ftp->data_sock >= 0) {
        tbos_net_close(ftp->data_sock);
        ftp->data_sock = -1;
    }

    if (ftp->ctrl_sock >= 0) {
        tbos_net_close(ftp->ctrl_sock);
        ftp->ctrl_sock = -1;
    }

    ftp->logged_in = 0;
    return TBOS_FTP_SUCCESS;
}

int tbos_ftp_fetch(const char* host, uint16_t port,
                   const char* user, const char* pass,
                   const char* path, void** data, size_t* len) {
    tbos_ftp_t ftp;

    if (tbos_ftp_connect(&ftp, host, port) != TBOS_FTP_SUCCESS) {
        return TBOS_FTP_ERROR;
    }

    if (tbos_ftp_login(&ftp, user, pass) != TBOS_FTP_SUCCESS) {
        tbos_ftp_close(&ftp);
        return TBOS_FTP_AUTH_FAIL;
    }

    int result = tbos_ftp_get(&ftp, path, data, len);
    tbos_ftp_close(&ftp);

    return result;
}
