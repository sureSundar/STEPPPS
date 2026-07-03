/**
 * steppps_vfs.h - STEPPPS-Aware Virtual Filesystem
 *
 * Every file in TBOS has a STEPPPS identity:
 * - Minimal: Auto-generated (id, space, time, karma)
 * - Rich: Full 7D via companion .steppps file or PXFS header
 *
 * Security gate checks karma/consciousness before file operations.
 */

#ifndef TBOS_STEPPPS_VFS_H
#define TBOS_STEPPPS_VFS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* SECURITY LEVELS                                                            */
/* ========================================================================= */

typedef enum {
    STEPPPS_SECURITY_PERMISSIVE = 0,  /* Log only, allow all */
    STEPPPS_SECURITY_MODERATE   = 1,  /* Warn on low karma */
    STEPPPS_SECURITY_STRICT     = 2   /* Block if insufficient */
} steppps_security_level_t;

/* ========================================================================= */
/* MINIMAL STEPPPS (Auto-generated, ~64 bytes)                                */
/* ========================================================================= */

typedef struct {
    uint64_t id_high;           /* Hash of path + device */
    uint64_t id_low;            /* Hash of content + time */

    /* Space */
    char path[128];             /* File path */
    char device[32];            /* Device identifier */

    /* Time */
    uint64_t created;           /* Unix timestamp */
    uint64_t modified;          /* Unix timestamp */
    uint64_t accessed;          /* Unix timestamp */

    /* Security */
    int32_t karma;              /* Inherited from creator */
    uint8_t consciousness;      /* 0=dormant to 5=enlightened */
    uint8_t required_karma;     /* Minimum karma to access */
    uint8_t required_consciousness; /* Minimum level to access */
    bool is_signed;             /* Has valid signature */

    /* Flags */
    bool has_rich_steppps;      /* Has companion .steppps file */
    bool is_executable;         /* Contains executable script */
} steppps_minimal_t;

/* ========================================================================= */
/* FILE OPERATION TYPES                                                       */
/* ========================================================================= */

typedef enum {
    STEPPPS_OP_READ     = 0,
    STEPPPS_OP_WRITE    = 1,
    STEPPPS_OP_DELETE   = 2,
    STEPPPS_OP_EXECUTE  = 3,
    STEPPPS_OP_LIST     = 4,
    STEPPPS_OP_CREATE   = 5,
    STEPPPS_OP_RENAME   = 6,
    STEPPPS_OP_CHMOD    = 7
} steppps_op_t;

/* Karma cost per operation */
static const int STEPPPS_OP_KARMA_COST[] = {
    0,   /* READ: free */
    1,   /* WRITE: 1 karma */
    5,   /* DELETE: 5 karma */
    2,   /* EXECUTE: 2 karma */
    0,   /* LIST: free */
    1,   /* CREATE: 1 karma */
    2,   /* RENAME: 2 karma */
    3    /* CHMOD: 3 karma */
};

/* Minimum consciousness per operation */
static const uint8_t STEPPPS_OP_MIN_CONSCIOUSNESS[] = {
    0,   /* READ: dormant */
    1,   /* WRITE: reactive */
    3,   /* DELETE: aware */
    1,   /* EXECUTE: reactive */
    0,   /* LIST: dormant */
    1,   /* CREATE: reactive */
    2,   /* RENAME: adaptive */
    3    /* CHMOD: aware */
};

/* ========================================================================= */
/* CALLER CONTEXT                                                             */
/* ========================================================================= */

typedef struct {
    uint64_t id_high;           /* Caller's STEPPPS ID */
    uint64_t id_low;
    char name[64];              /* Caller name (user/process) */
    int32_t karma;              /* Caller's karma */
    uint8_t consciousness;      /* Caller's consciousness level */
    uint32_t capabilities;      /* Caller's capability flags */
    bool is_root;               /* Bypass security checks */
} steppps_caller_t;

/* ========================================================================= */
/* AUDIT LOG ENTRY                                                            */
/* ========================================================================= */

typedef struct {
    uint64_t timestamp;         /* When */
    steppps_op_t operation;     /* What operation */
    char path[256];             /* Target file */
    steppps_caller_t caller;    /* Who */
    int result;                 /* 0=allowed, -1=denied */
    char reason[128];           /* Why denied (if applicable) */
} steppps_audit_entry_t;

/* ========================================================================= */
/* STEPPPS VFS CONTEXT                                                        */
/* ========================================================================= */

typedef struct {
    /* Security configuration */
    steppps_security_level_t security_level;

    /* Current caller */
    steppps_caller_t current_caller;

    /* Statistics */
    uint64_t total_operations;
    uint64_t allowed_operations;
    uint64_t denied_operations;
    uint64_t warnings_issued;

    /* Audit */
    bool audit_enabled;
    char audit_path[256];

    /* STEPPPS companion file extension */
    char companion_ext[16];     /* Default: ".steppps" */

    /* Callbacks */
    void (*on_deny)(const steppps_audit_entry_t* entry);
    void (*on_warn)(const steppps_audit_entry_t* entry);
} steppps_vfs_ctx_t;

/* ========================================================================= */
/* API FUNCTIONS                                                              */
/* ========================================================================= */

/* Initialization */
int steppps_vfs_init(steppps_security_level_t level);
void steppps_vfs_shutdown(void);
steppps_vfs_ctx_t* steppps_vfs_get_context(void);

/* Caller management */
int steppps_vfs_set_caller(const steppps_caller_t* caller);
int steppps_vfs_get_caller(steppps_caller_t* caller);

/* Security configuration */
int steppps_vfs_set_security_level(steppps_security_level_t level);
steppps_security_level_t steppps_vfs_get_security_level(void);

/* Core security gate - call BEFORE every file operation */
int steppps_vfs_check(const char* path, steppps_op_t op);

/* STEPPPS metadata */
int steppps_vfs_get_minimal(const char* path, steppps_minimal_t* meta);
int steppps_vfs_set_minimal(const char* path, const steppps_minimal_t* meta);
int steppps_vfs_generate_minimal(const char* path, steppps_minimal_t* meta);

/* Companion file management */
int steppps_vfs_get_companion_path(const char* path, char* companion, size_t len);
bool steppps_vfs_has_companion(const char* path);
int steppps_vfs_read_companion(const char* path, char* json, size_t max_len);
int steppps_vfs_write_companion(const char* path, const char* json);

/* Karma operations */
int steppps_vfs_set_required_karma(const char* path, int32_t karma);
int steppps_vfs_set_required_consciousness(const char* path, uint8_t level);

/* Audit */
int steppps_vfs_audit_enable(const char* log_path);
int steppps_vfs_audit_disable(void);
int steppps_vfs_audit_read(steppps_audit_entry_t* entries, size_t max, size_t* count);

/* Statistics */
void steppps_vfs_print_stats(void);

/* Utility */
const char* steppps_op_name(steppps_op_t op);
const char* steppps_security_level_name(steppps_security_level_t level);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_STEPPPS_VFS_H */
