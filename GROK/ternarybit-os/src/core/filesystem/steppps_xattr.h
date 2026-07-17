/**
 * @file steppps_xattr.h
 * @brief STEPPPS Extended Attributes for FUSE Filesystems
 *
 * Stores the 7 STEPPPS dimensions as extended attributes:
 *   user.steppps.space      - S_space dimension
 *   user.steppps.time       - T_time dimension
 *   user.steppps.event      - E_event dimension
 *   user.steppps.psychology - P_psychology dimension
 *   user.steppps.pixel      - P_pixel dimension
 *   user.steppps.prompt     - P_prompt dimension
 *   user.steppps.script     - S_script dimension
 *   user.steppps.meta       - Governance metadata
 *
 * @version 1.0
 * @date 2026-07-16
 */

#ifndef STEPPPS_XATTR_H
#define STEPPPS_XATTR_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSTANTS
 * ═══════════════════════════════════════════════════════════════════════════ */

#define STEPPPS_XATTR_PREFIX      "user.steppps."
#define STEPPPS_XATTR_MAX_VALUE   4096   /* Max xattr value size */
#define STEPPPS_XATTR_MAX_NAME    64     /* Max xattr name length */

/* Dimension names */
#define STEPPPS_XATTR_SPACE       "user.steppps.space"
#define STEPPPS_XATTR_TIME        "user.steppps.time"
#define STEPPPS_XATTR_EVENT       "user.steppps.event"
#define STEPPPS_XATTR_PSYCHOLOGY  "user.steppps.psychology"
#define STEPPPS_XATTR_PIXEL       "user.steppps.pixel"
#define STEPPPS_XATTR_PROMPT      "user.steppps.prompt"
#define STEPPPS_XATTR_SCRIPT      "user.steppps.script"
#define STEPPPS_XATTR_META        "user.steppps.meta"

/* Number of STEPPPS dimensions */
#define STEPPPS_DIMENSION_COUNT   8

/* ═══════════════════════════════════════════════════════════════════════════
 * TYPES
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief STEPPPS xattr storage for a file/directory
 */
typedef struct {
    char* space;       /* S_space JSON */
    char* time;        /* T_time JSON */
    char* event;       /* E_event JSON */
    char* psychology;  /* P_psychology JSON */
    char* pixel;       /* P_pixel JSON */
    char* prompt;      /* P_prompt JSON */
    char* script;      /* S_script JSON */
    char* meta;        /* Governance metadata JSON */
} steppps_xattr_t;

/**
 * @brief Creation context for auto-populating STEPPPS
 */
typedef struct {
    const char* path;           /* File path */
    const char* filesystem;     /* pxfs, ucfs, rf2s, etc. */
    const char* realm;          /* bhulok, swarga, patala */
    const char* owner;          /* steppps://human/xxx */
    const char* sangha;         /* steppps://sangha/xxx */
    const char* nation;         /* steppps://nation/xxx */
    const char* creation_prompt; /* Why was this created */
    bool is_directory;          /* Directory vs file */
    mode_t mode;                /* Unix permissions */
    uid_t uid;                  /* Owner UID */
    gid_t gid;                  /* Owner GID */
    size_t size;                /* File size */
    const char* mime_type;      /* MIME type (files only) */
} steppps_create_ctx_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize a STEPPPS xattr structure
 */
void steppps_xattr_init(steppps_xattr_t* xattr);

/**
 * @brief Free all allocated STEPPPS xattr data
 */
void steppps_xattr_free(steppps_xattr_t* xattr);

/**
 * @brief Auto-populate STEPPPS dimensions for a new file/directory
 *
 * @param xattr Output structure to populate
 * @param ctx Creation context with file metadata
 * @return 0 on success, negative on error
 */
int steppps_xattr_create(steppps_xattr_t* xattr, const steppps_create_ctx_t* ctx);

/**
 * @brief Get a STEPPPS xattr value by name
 *
 * @param xattr The STEPPPS xattr storage
 * @param name The xattr name (e.g., "user.steppps.space")
 * @param value Output buffer
 * @param size Buffer size
 * @return Length of value, or negative on error
 */
ssize_t steppps_xattr_get(const steppps_xattr_t* xattr, const char* name,
                          char* value, size_t size);

/**
 * @brief Set a STEPPPS xattr value by name
 *
 * @param xattr The STEPPPS xattr storage
 * @param name The xattr name
 * @param value The value to set
 * @param size Value size
 * @param flags XATTR_CREATE, XATTR_REPLACE, or 0
 * @return 0 on success, negative on error
 */
int steppps_xattr_set(steppps_xattr_t* xattr, const char* name,
                      const char* value, size_t size, int flags);

/**
 * @brief List all STEPPPS xattr names
 *
 * @param xattr The STEPPPS xattr storage
 * @param list Output buffer for names (null-separated)
 * @param size Buffer size
 * @return Total length of names, or negative on error
 */
ssize_t steppps_xattr_list(const steppps_xattr_t* xattr, char* list, size_t size);

/**
 * @brief Remove a STEPPPS xattr by name
 *
 * @param xattr The STEPPPS xattr storage
 * @param name The xattr name to remove
 * @return 0 on success, negative on error
 */
int steppps_xattr_remove(steppps_xattr_t* xattr, const char* name);

/**
 * @brief Update T_time.modified timestamp
 */
void steppps_xattr_update_mtime(steppps_xattr_t* xattr);

/**
 * @brief Update T_time.accessed timestamp
 */
void steppps_xattr_update_atime(steppps_xattr_t* xattr);

/**
 * @brief Check if a name is a STEPPPS xattr
 */
bool steppps_xattr_is_steppps(const char* name);

/**
 * @brief Get ISO8601 timestamp string
 */
void steppps_get_timestamp(char* buf, size_t size);

#endif /* STEPPPS_XATTR_H */
