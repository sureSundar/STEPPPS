/**
 * steppps.h - STEPPPS v2: Simple JSON + Smart Secure Runtime
 *
 * A STEPPPS is a self-contained, human-readable JSON with:
 * - 7 dimensions (Space, Time, Event, Psychology, Prompt, Pixel, Script)
 * - Cryptographic signature for trust
 * - Capability declaration for security
 * - Embedded executable code
 *
 * The runtime provides:
 * - Signature verification
 * - Karma-based trust
 * - Sandboxed execution
 * - Audit logging
 * - Sangha discovery
 */

#ifndef STEPPPS_H
#define STEPPPS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* CONSTANTS                                                                  */
/* ========================================================================= */

#define STEPPPS_VERSION         "2.0"
#define STEPPPS_MAX_JSON        (1024 * 1024)   /* 1MB max file size */
#define STEPPPS_MAX_FIELD       4096
#define STEPPPS_MAX_CODE        65536
#define STEPPPS_SIG_SIZE        64              /* Ed25519 signature */
#define STEPPPS_PUBKEY_SIZE     32              /* Ed25519 public key */
#define STEPPPS_HASH_SIZE       32              /* SHA-256 */

/* ========================================================================= */
/* SANDBOX LEVELS                                                             */
/* ========================================================================= */

typedef enum {
    SANDBOX_DISPLAY   = 0,  /* View only, no code execution */
    SANDBOX_PURE      = 1,  /* Pure computation, no I/O */
    SANDBOX_READ      = 2,  /* Read local files (declared paths) */
    SANDBOX_WRITE     = 3,  /* Write local files (declared paths) */
    SANDBOX_NETWORK   = 4,  /* Network access (declared hosts) */
    SANDBOX_SYSTEM    = 5,  /* System commands (declared cmds) */
    SANDBOX_TRUSTED   = 6   /* Unrestricted (requires high karma + sig) */
} steppps_sandbox_t;

/* Minimum karma required for each sandbox level */
static const int64_t SANDBOX_KARMA_MIN[] = {
    0,      /* DISPLAY: anyone */
    0,      /* PURE: anyone */
    100,    /* READ: some trust */
    500,    /* WRITE: moderate trust */
    1000,   /* NETWORK: high trust */
    5000,   /* SYSTEM: very high trust */
    50000   /* TRUSTED: enlightened only */
};

/* ========================================================================= */
/* CAPABILITY FLAGS                                                           */
/* ========================================================================= */

typedef enum {
    CAP_NONE        = 0x0000,
    CAP_READ_FS     = 0x0001,   /* Read filesystem */
    CAP_WRITE_FS    = 0x0002,   /* Write filesystem */
    CAP_EXEC        = 0x0004,   /* Execute programs */
    CAP_NET_LOCAL   = 0x0008,   /* Localhost network */
    CAP_NET_REMOTE  = 0x0010,   /* Remote network */
    CAP_SANGHA      = 0x0020,   /* Sangha discovery/messaging */
    CAP_SPAWN       = 0x0040,   /* Create child STEPPPS */
    CAP_CAMERA      = 0x0080,   /* Access camera */
    CAP_MIC         = 0x0100,   /* Access microphone */
    CAP_LOCATION    = 0x0200,   /* Access precise location */
    CAP_KEYCHAIN    = 0x0400,   /* Access secrets/keys */
    CAP_ROOT        = 0x8000    /* Root/admin access */
} steppps_cap_t;

/* ========================================================================= */
/* SECURITY CONTEXT                                                           */
/* ========================================================================= */

typedef struct {
    /* Signature */
    char author[128];                   /* Author identifier */
    uint8_t pubkey[STEPPPS_PUBKEY_SIZE];/* Ed25519 public key */
    uint8_t signature[STEPPPS_SIG_SIZE];/* Ed25519 signature */
    bool is_signed;
    bool sig_valid;

    /* Trust */
    int64_t author_karma;               /* Author's karma score */
    steppps_sandbox_t max_sandbox;      /* Max allowed sandbox level */

    /* Capabilities */
    steppps_cap_t requested;            /* What STEPPPS requests */
    steppps_cap_t granted;              /* What user approved */

    /* Paths/Hosts allowed */
    char allowed_paths[16][256];
    int allowed_path_count;
    char allowed_hosts[16][128];
    int allowed_host_count;
    char allowed_cmds[16][64];
    int allowed_cmd_count;

    /* Audit */
    char hash[STEPPPS_HASH_SIZE * 2 + 1]; /* Hex SHA-256 of content */
    uint64_t first_seen;
    uint64_t last_run;
    uint32_t run_count;
    bool blacklisted;
} steppps_security_t;

/* ========================================================================= */
/* 7 DIMENSIONS (Simple Data Structures)                                      */
/* ========================================================================= */

/* S - Space: Where in the multiverse */
typedef struct {
    char universe[32];          /* "earth", "metaverse", "simulation-42" */
    double lat, lon, alt;       /* Earth coordinates */
    char cosmic[128];           /* "sun:corona", "andromeda:m31" */
    char device[64];            /* Device identifier */
    char path[256];             /* Local path */
    char realm[16];             /* "physical", "virtual", "hybrid" */
} steppps_space_t;

/* T - Time: When (UTC for Earth, relative for others) */
typedef struct {
    char utc[32];               /* ISO 8601 */
    char created[32];
    char expires[32];
    char relative[64];          /* "+8min photon arrival" */
    char epoch[16];             /* "earth", "tbos", "cosmic" */
    uint64_t timestamp_ms;      /* Unix ms */
} steppps_time_t;

/* E - Event: What triggered this */
typedef struct {
    char type[32];              /* "cosmic", "user", "transaction", etc */
    char source[128];           /* What/who triggered */
    char description[512];
    char severity[16];          /* "info" to "cosmic" */
    char parent[128];           /* Parent STEPPPS ID */
    char chain[512];            /* Event chain JSON array */
} steppps_event_t;

/* P - Psychology: Context state */
typedef struct {
    /* System state */
    int health;                 /* 0-100 */
    int64_t karma;
    char consciousness[32];     /* "dormant" to "enlightened" */

    /* Environment */
    char weather[32];
    double temperature;
    char conditions[128];

    /* Beings nearby (JSON array) */
    char beings[512];

    /* Intent */
    char intent[256];
} steppps_psych_t;

/* P - Prompt: AI interaction */
typedef struct {
    char system[1024];          /* System prompt */
    char user[1024];            /* User-facing prompt */
    char next[1024];            /* Prompt for next STEPPPS */
    char interaction[512];      /* How to interact */
    char model[64];             /* Preferred AI model */
    double temperature;
} steppps_prompt_t;

/* P - Pixel: Display logic */
typedef struct {
    char type[16];              /* "text", "html", "terminal", "gui" */
    char template_str[2048];    /* Display template */
    char data[2048];            /* JSON data to render */
    char style[512];            /* CSS/style JSON */
    char actions[1024];         /* Available actions JSON */
} steppps_pixel_t;

/* S - Script: Executable code */
typedef struct {
    char lang[16];              /* "sh", "c", "python", "json", "lua" */
    char code[STEPPPS_MAX_CODE];/* The actual code */
    char entry[64];             /* Entry point */
    char deps[512];             /* Dependencies JSON array */
    steppps_sandbox_t sandbox;  /* Required sandbox level */
    char outputs[512];          /* Output STEPPPS IDs */
} steppps_script_t;

/* ========================================================================= */
/* STEPPPS STRUCTURE                                                          */
/* ========================================================================= */

typedef struct {
    /* Identity */
    char id[128];
    char name[256];
    char version[16];
    char kind[32];

    /* 7 Dimensions */
    steppps_space_t  space;     /* S */
    steppps_time_t   time;      /* T */
    steppps_event_t  event;     /* E */
    steppps_psych_t  psych;     /* P */
    steppps_prompt_t prompt;    /* P */
    steppps_pixel_t  pixel;     /* P */
    steppps_script_t script;    /* S */

    /* Security */
    steppps_security_t security;

    /* Meta */
    char author[64];
    char tags[256];
    int64_t karma_cost;
    int64_t karma_reward;

    /* Runtime state */
    bool loaded;
    bool verified;
    bool approved;
    int exit_code;
} steppps_t;

/* ========================================================================= */
/* RUNTIME API                                                                */
/* ========================================================================= */

/* Initialize runtime */
int steppps_runtime_init(void);
void steppps_runtime_shutdown(void);
int64_t steppps_get_user_karma(void);  /* Real, persisted karma balance (punyam - pavam) */
int64_t steppps_get_punyam(void);      /* Merit earned */
int64_t steppps_get_pavam(void);       /* Demerit spent */

/* Load & Parse */
int steppps_load(const char* path, steppps_t* s);
int steppps_parse(const char* json, steppps_t* s);
void steppps_free(steppps_t* s);

/* Security */
int steppps_verify(steppps_t* s);                           /* Verify signature */
int steppps_check_trust(steppps_t* s, int64_t user_karma);  /* Check karma threshold */
int steppps_request_caps(steppps_t* s);                     /* Prompt user for caps */
bool steppps_is_safe(const steppps_t* s);                   /* Quick safety check */

/* Execution */
int steppps_run(steppps_t* s);                              /* Execute in sandbox */
int steppps_display(const steppps_t* s);                    /* Show info */
int steppps_render(const steppps_t* s);                     /* Render pixel output */

/* Audit */
int steppps_audit_log(const steppps_t* s, const char* action, int result);
int steppps_audit_get(const char* id, char* log, size_t len);

/* Signing (for authors) */
int steppps_sign(steppps_t* s, const uint8_t* privkey);
int steppps_export(const steppps_t* s, char* json, size_t len);

/* Sangha */
int steppps_broadcast(const steppps_t* s);
int steppps_discover(const char* query, steppps_t* results, int max_results);

/* Utility */
const char* steppps_sandbox_name(steppps_sandbox_t level);
const char* steppps_cap_name(steppps_cap_t cap);
void steppps_print(const steppps_t* s);
void steppps_print_security(const steppps_t* s);

#ifdef __cplusplus
}
#endif

#endif /* STEPPPS_H */
