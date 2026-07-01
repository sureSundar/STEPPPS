/**
 * @file tbos_ftp.h
 * @brief TernaryBit OS - Simple FTP Client
 *
 * Minimal FTP client for wow://ftp:// support.
 * Supports: connect, login, retrieve, quit
 */

#ifndef TBOS_FTP_H
#define TBOS_FTP_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSTANTS
 * ═══════════════════════════════════════════════════════════════════════════ */

#define TBOS_FTP_PORT         21
#define TBOS_FTP_MAX_RESPONSE 1024

#define TBOS_FTP_SUCCESS      0
#define TBOS_FTP_ERROR       -1
#define TBOS_FTP_AUTH_FAIL   -2
#define TBOS_FTP_NOT_FOUND   -3

/* ═══════════════════════════════════════════════════════════════════════════
 * STRUCTURES
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    int ctrl_sock;       /* Control connection */
    int data_sock;       /* Data connection */
    char host[256];
    uint16_t port;
    int logged_in;
    int passive_mode;
    char last_response[TBOS_FTP_MAX_RESPONSE];
    int last_code;
} tbos_ftp_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Connect to FTP server
 */
int tbos_ftp_connect(tbos_ftp_t* ftp, const char* host, uint16_t port);

/**
 * @brief Login with username/password
 */
int tbos_ftp_login(tbos_ftp_t* ftp, const char* user, const char* pass);

/**
 * @brief Retrieve file to buffer
 * @param ftp FTP session
 * @param path Remote file path
 * @param data Output: allocated buffer (caller frees)
 * @param len Output: data length
 * @return TBOS_FTP_SUCCESS or error
 */
int tbos_ftp_get(tbos_ftp_t* ftp, const char* path, void** data, size_t* len);

/**
 * @brief Close FTP session
 */
int tbos_ftp_close(tbos_ftp_t* ftp);

/**
 * @brief Simple one-shot fetch
 */
int tbos_ftp_fetch(const char* host, uint16_t port,
                   const char* user, const char* pass,
                   const char* path, void** data, size_t* len);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_FTP_H */
