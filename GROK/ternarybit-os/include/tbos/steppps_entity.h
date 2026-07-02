/**
 * @file steppps_entity.h
 * @brief STEPPPS Universal Entity Framework for TernaryBit OS
 *
 * Every entity in TBOS (bootloader, file, script, device, process) is:
 * - Identifiable: Unique STEPPPS ID (128-bit)
 * - Addressable: STEPPPS URI scheme
 * - Invokable: Can be called with actions
 * - Interactable: Can communicate with other entities
 * - Authenticated: Identity verified via karma/trust
 * - Authorized: Permissions based on consciousness level
 *
 * STEPPPS URI Format:
 *   steppps://[entity-type]/[id]?s=space&t=time&e=event&p=psych
 *
 * Examples:
 *   steppps://file/0x1234567890ABCDEF?s=local&t=now
 *   steppps://process/kernel.init?p=enlightened
 *   steppps://device/sangha.mobile.12345?action=discover
 *   steppps://bootloader/stage1?action=status
 *
 * @version 2.0
 * @date 2025
 */

#ifndef TBOS_STEPPPS_ENTITY_H
#define TBOS_STEPPPS_ENTITY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* STEPPPS ENTITY ID (128-bit Universal Identifier)                          */
/* ========================================================================= */

/**
 * @brief 128-bit STEPPPS Entity ID
 * Structure:
 *   [0-15]:  Entity type (4 bits) + Version (4 bits) + Tier (8 bits)
 *   [16-47]: Timestamp (32 bits, seconds since TBOS epoch)
 *   [48-79]: Space hash (32 bits, location/device fingerprint)
 *   [80-127]: Random/Sequence (48 bits, uniqueness)
 */
typedef struct {
    uint64_t high;  /* Type + Version + Tier + Timestamp + Space */
    uint64_t low;   /* Sequence + Random */
} steppps_id_t;

/* Entity Types (4 bits = 16 types) */
typedef enum {
    ENTITY_TYPE_SYSTEM      = 0x0,  /* Kernel, bootloader */
    ENTITY_TYPE_PROCESS     = 0x1,  /* Running process */
    ENTITY_TYPE_FILE        = 0x2,  /* File or directory */
    ENTITY_TYPE_DEVICE      = 0x3,  /* Hardware device */
    ENTITY_TYPE_NETWORK     = 0x4,  /* Network connection */
    ENTITY_TYPE_SCRIPT      = 0x5,  /* Automation script */
    ENTITY_TYPE_USER        = 0x6,  /* User account */
    ENTITY_TYPE_SANGHA      = 0x7,  /* Sangha node */
    ENTITY_TYPE_SERVICE     = 0x8,  /* System service */
    ENTITY_TYPE_INTENT      = 0x9,  /* Action intent */
    ENTITY_TYPE_RESOURCE    = 0xA,  /* Generic resource */
    ENTITY_TYPE_PIXEL       = 0xB,  /* PXFS pixel data */
    ENTITY_TYPE_EVENT       = 0xC,  /* Event object */
    ENTITY_TYPE_PROMPT      = 0xD,  /* Prompt/command */
    ENTITY_TYPE_STREAM      = 0xE,  /* Data stream */
    ENTITY_TYPE_CUSTOM      = 0xF   /* Custom entity */
} steppps_entity_type_t;

/* ========================================================================= */
/* STEPPPS URI                                                                */
/* ========================================================================= */

#define STEPPPS_URI_MAX_LENGTH 512
#define STEPPPS_URI_MAX_PARAMS 16

/**
 * @brief STEPPPS URI structure
 */
typedef struct {
    char raw[STEPPPS_URI_MAX_LENGTH];
    steppps_entity_type_t type;
    steppps_id_t id;
    char name[128];                     /* Human-readable name */

    /* Query parameters (STEPPPS dimensions) */
    char space[64];                     /* S: Location context */
    char time[64];                      /* T: Temporal context */
    char event[64];                     /* E: Event context */
    char psychology[64];                /* P: Consciousness context */
    char pixel[64];                     /* P: Visual context */
    char prompt[64];                    /* P: Interface context */
    char script[64];                    /* S: Automation context */

    /* Action */
    char action[64];                    /* Requested action */
    char params[STEPPPS_URI_MAX_PARAMS][64]; /* Additional parameters */
    uint8_t param_count;
} steppps_uri_t;

/* ========================================================================= */
/* CONSCIOUSNESS & KARMA (Authentication/Authorization)                       */
/* ========================================================================= */

/**
 * @brief Consciousness levels for authorization
 */
typedef enum {
    CONSCIOUSNESS_DORMANT      = 0,    /* Inactive, no awareness */
    CONSCIOUSNESS_REACTIVE     = 1,    /* Basic stimulus-response */
    CONSCIOUSNESS_ADAPTIVE     = 2,    /* Learning, adapting */
    CONSCIOUSNESS_AWARE        = 3,    /* Self-aware, contextual */
    CONSCIOUSNESS_MINDFUL      = 4,    /* Deliberate, thoughtful */
    CONSCIOUSNESS_ENLIGHTENED  = 5     /* Full awareness, wisdom */
} steppps_consciousness_t;

/**
 * @brief Karma score (trust metric)
 */
typedef struct {
    int64_t total;                      /* Accumulated karma */
    int64_t positive;                   /* Good actions */
    int64_t negative;                   /* Bad actions */
    uint64_t action_count;              /* Total actions */
    uint64_t helpful_actions;           /* Actions helping others */
    float trust_score;                  /* 0.0 - 1.0 */
} steppps_karma_t;

/**
 * @brief Entity capabilities (authorization)
 */
typedef enum {
    CAPABILITY_NONE            = 0x0000,
    CAPABILITY_READ            = 0x0001,
    CAPABILITY_WRITE           = 0x0002,
    CAPABILITY_EXECUTE         = 0x0004,
    CAPABILITY_DELETE          = 0x0008,
    CAPABILITY_INVOKE          = 0x0010,
    CAPABILITY_INTERACT        = 0x0020,
    CAPABILITY_AUTHENTICATE    = 0x0040,
    CAPABILITY_AUTHORIZE       = 0x0080,
    CAPABILITY_DISCOVER        = 0x0100,
    CAPABILITY_BROADCAST       = 0x0200,
    CAPABILITY_NETWORK         = 0x0400,
    CAPABILITY_SANGHA          = 0x0800,
    CAPABILITY_ADMIN           = 0x1000,
    CAPABILITY_ROOT            = 0x2000,
    CAPABILITY_ALL             = 0xFFFF
} steppps_capability_t;

/* ========================================================================= */
/* ACTION INTENTS (Dynamic Invocation)                                        */
/* ========================================================================= */

#define INTENT_MAX_PARAMS 16

/**
 * @brief Action intent (what an entity can do)
 */
typedef struct {
    char name[64];                      /* Intent name (e.g., "open", "send") */
    char description[256];              /* Human-readable description */
    steppps_capability_t required_caps; /* Required capabilities */
    steppps_consciousness_t min_consciousness; /* Minimum consciousness */
    int64_t karma_cost;                 /* Karma cost to execute */
    int64_t karma_reward;               /* Karma reward on success */

    /* Parameters */
    char param_names[INTENT_MAX_PARAMS][32];
    char param_types[INTENT_MAX_PARAMS][16];
    bool param_required[INTENT_MAX_PARAMS];
    uint8_t param_count;
} steppps_intent_t;

/**
 * @brief Intent invocation request
 */
typedef struct {
    steppps_id_t source;                /* Calling entity */
    steppps_id_t target;                /* Target entity */
    char intent_name[64];               /* Intent to invoke */

    /* Parameters */
    char param_values[INTENT_MAX_PARAMS][256];
    uint8_t param_count;

    /* Context (STEPPPS dimensions) */
    uint64_t timestamp;                 /* When */
    char location[64];                  /* Where */
    steppps_consciousness_t consciousness; /* Caller's level */
    steppps_karma_t karma;              /* Caller's karma */

    /* Response */
    bool async;                         /* Asynchronous invocation */
    steppps_id_t callback_entity;       /* For async response */
} steppps_invocation_t;

/**
 * @brief Intent execution result
 */
typedef struct {
    bool success;
    int error_code;
    char error_message[256];

    /* Results */
    void* data;
    size_t data_size;
    char result_type[32];

    /* Karma impact */
    int64_t karma_change;

    /* Metrics */
    uint64_t execution_time_us;
} steppps_result_t;

/* ========================================================================= */
/* STEPPPS ENTITY (The Universal Object)                                      */
/* ========================================================================= */

/* Forward declaration for entity operations */
struct steppps_entity;
typedef struct steppps_entity steppps_entity_t;

/**
 * @brief Entity operations (virtual table)
 */
typedef struct {
    /* Lifecycle */
    int (*init)(steppps_entity_t* self);
    int (*destroy)(steppps_entity_t* self);

    /* Identification */
    int (*get_id)(steppps_entity_t* self, steppps_id_t* id);
    int (*get_uri)(steppps_entity_t* self, steppps_uri_t* uri);
    int (*get_name)(steppps_entity_t* self, char* name, size_t len);

    /* Authentication */
    int (*authenticate)(steppps_entity_t* self, steppps_entity_t* peer);
    int (*verify_identity)(steppps_entity_t* self);
    int (*get_karma)(steppps_entity_t* self, steppps_karma_t* karma);

    /* Authorization */
    int (*get_capabilities)(steppps_entity_t* self, steppps_capability_t* caps);
    bool (*has_capability)(steppps_entity_t* self, steppps_capability_t cap);
    int (*get_consciousness)(steppps_entity_t* self, steppps_consciousness_t* level);

    /* Invocation */
    int (*list_intents)(steppps_entity_t* self, steppps_intent_t* intents, size_t max);
    int (*invoke)(steppps_entity_t* self, const steppps_invocation_t* inv, steppps_result_t* result);

    /* Interaction */
    int (*send_message)(steppps_entity_t* self, steppps_entity_t* target, const void* msg, size_t len);
    int (*receive_message)(steppps_entity_t* self, steppps_entity_t* source, void* msg, size_t* len);
    int (*broadcast)(steppps_entity_t* self, const void* msg, size_t len);

    /* Discovery */
    int (*discover)(steppps_entity_t* self, steppps_entity_type_t type,
                   steppps_entity_t** found, size_t max_count, size_t* found_count);
    int (*announce)(steppps_entity_t* self);

    /* STEPPPS Dimensions */
    int (*get_space)(steppps_entity_t* self, void* space);
    int (*get_time)(steppps_entity_t* self, void* time);
    int (*get_event_history)(steppps_entity_t* self, void* events, size_t max);
    int (*get_psychology)(steppps_entity_t* self, void* psych);
    int (*get_pixel)(steppps_entity_t* self, void* pixel);
    int (*get_prompt)(steppps_entity_t* self, void* prompt);
    int (*get_script)(steppps_entity_t* self, void* script);
} steppps_entity_ops_t;

/**
 * @brief STEPPPS Entity - The Universal Object
 */
struct steppps_entity {
    /* Identity */
    steppps_id_t id;
    steppps_entity_type_t type;
    char name[128];
    char description[256];

    /* Location (Space dimension) */
    uint64_t space_hash;                /* Device/location fingerprint */
    char location[64];

    /* State */
    bool active;
    bool authenticated;
    uint64_t created_at;
    uint64_t last_active;

    /* Consciousness & Karma */
    steppps_consciousness_t consciousness;
    steppps_karma_t karma;
    steppps_capability_t capabilities;

    /* Intents (what this entity can do) */
    steppps_intent_t* intents;
    uint8_t intent_count;

    /* Operations (virtual table) */
    steppps_entity_ops_t* ops;

    /* Parent/Children (hierarchy) */
    steppps_entity_t* parent;
    steppps_entity_t** children;
    uint16_t child_count;

    /* User data */
    void* user_data;
    size_t user_data_size;

    /* Internal */
    void* _internal;
};

/* ========================================================================= */
/* ENTITY REGISTRY (Global Entity Index)                                      */
/* ========================================================================= */

#define ENTITY_REGISTRY_MAX_SIZE 65536

/**
 * @brief Entity registry entry
 */
typedef struct {
    steppps_id_t id;
    steppps_entity_t* entity;
    bool active;
    uint64_t registered_at;
    uint64_t last_access;
    uint32_t access_count;
} entity_registry_entry_t;

/**
 * @brief Entity registry
 */
typedef struct {
    entity_registry_entry_t* entries;
    size_t capacity;
    size_t count;
    uint64_t created_at;

    /* Statistics */
    uint64_t total_registrations;
    uint64_t total_lookups;
    uint64_t cache_hits;
    uint64_t cache_misses;
} steppps_registry_t;

/* ========================================================================= */
/* SANGHA DISCOVERY (Cross-Device Interaction)                                */
/* ========================================================================= */

/**
 * @brief Sangha node (device in the community)
 */
typedef struct {
    steppps_id_t id;
    char name[64];
    char device_type[32];               /* mobile, desktop, server, etc. */

    /* Network */
    char address[64];                   /* IP or WOW address */
    uint16_t port;
    bool online;

    /* Capabilities */
    steppps_capability_t shared_caps;   /* What it shares */
    steppps_intent_t* shared_intents;   /* What it can do */
    uint8_t shared_intent_count;

    /* Trust */
    steppps_consciousness_t consciousness;
    steppps_karma_t karma;
    float trust_score;

    /* Metrics */
    uint64_t last_seen;
    uint64_t interaction_count;
    uint64_t successful_interactions;
} sangha_node_t;

/**
 * @brief Sangha discovery message
 */
typedef struct {
    steppps_id_t sender;
    char message_type[16];              /* announce, query, response */

    /* Sender info */
    char name[64];
    char device_type[32];
    steppps_intent_t* intents;
    uint8_t intent_count;

    /* Request (for query) */
    steppps_entity_type_t query_type;
    char query_intent[64];

    /* Timestamp */
    uint64_t timestamp;
} sangha_discovery_t;

/* ========================================================================= */
/* API FUNCTIONS                                                              */
/* ========================================================================= */

/* Entity Management */
int steppps_entity_init(void);
int steppps_entity_shutdown(void);

steppps_entity_t* steppps_entity_create(steppps_entity_type_t type, const char* name);
void steppps_entity_destroy(steppps_entity_t* entity);

/* ID Functions */
int steppps_id_generate(steppps_entity_type_t type, steppps_id_t* id);
int steppps_id_to_string(const steppps_id_t* id, char* buf, size_t len);
int steppps_id_from_string(const char* str, steppps_id_t* id);
bool steppps_id_equals(const steppps_id_t* a, const steppps_id_t* b);

/* URI Functions */
int steppps_uri_parse(const char* uri_str, steppps_uri_t* uri);
int steppps_uri_build(const steppps_entity_t* entity, steppps_uri_t* uri);
int steppps_uri_to_string(const steppps_uri_t* uri, char* buf, size_t len);
steppps_entity_t* steppps_uri_resolve(const steppps_uri_t* uri);

/* Registry Functions */
int steppps_registry_init(size_t capacity);
int steppps_registry_register(steppps_entity_t* entity);
int steppps_registry_unregister(const steppps_id_t* id);
steppps_entity_t* steppps_registry_lookup(const steppps_id_t* id);
steppps_entity_t* steppps_registry_lookup_by_name(const char* name);
int steppps_registry_find(steppps_entity_type_t type, steppps_entity_t** results,
                          size_t max_results, size_t* found);
int steppps_registry_stats(uint64_t* total, uint64_t* active, uint64_t* lookups);

/* Authentication */
int steppps_authenticate(steppps_entity_t* source, steppps_entity_t* target);
int steppps_verify_karma(steppps_entity_t* entity, int64_t min_karma);
int steppps_verify_consciousness(steppps_entity_t* entity, steppps_consciousness_t min_level);

/* Authorization */
bool steppps_authorize(steppps_entity_t* entity, steppps_capability_t required);
int steppps_grant_capability(steppps_entity_t* entity, steppps_capability_t cap);
int steppps_revoke_capability(steppps_entity_t* entity, steppps_capability_t cap);

/* Invocation */
int steppps_register_intent(steppps_entity_t* entity, const steppps_intent_t* intent);
int steppps_invoke(const steppps_uri_t* target, const steppps_invocation_t* inv,
                   steppps_result_t* result);
int steppps_invoke_async(const steppps_uri_t* target, const steppps_invocation_t* inv);

/* Interaction */
int steppps_send(steppps_entity_t* source, const steppps_uri_t* target,
                 const void* message, size_t len);
int steppps_receive(steppps_entity_t* entity, void* buffer, size_t* len,
                    steppps_id_t* sender, uint32_t timeout_ms);
int steppps_broadcast(steppps_entity_t* source, steppps_entity_type_t target_type,
                      const void* message, size_t len);

/* Sangha Discovery */
int sangha_init(uint16_t port);
int sangha_announce(steppps_entity_t* entity);
int sangha_discover(steppps_entity_type_t type, const char* intent,
                    sangha_node_t* nodes, size_t max_nodes, size_t* found);
int sangha_query(const steppps_id_t* target, const char* intent, steppps_result_t* result);
int sangha_get_nodes(sangha_node_t* nodes, size_t max_nodes, size_t* count);

/* Karma Functions */
int steppps_karma_award(steppps_entity_t* entity, int64_t amount, const char* reason);
int steppps_karma_deduct(steppps_entity_t* entity, int64_t amount, const char* reason);
int steppps_karma_get(steppps_entity_t* entity, steppps_karma_t* karma);
float steppps_karma_to_trust(const steppps_karma_t* karma);

/* Consciousness Functions */
int steppps_consciousness_evolve(steppps_entity_t* entity);
int steppps_consciousness_get_level(steppps_entity_t* entity, steppps_consciousness_t* level);
const char* steppps_consciousness_name(steppps_consciousness_t level);

/* Utility */
const char* steppps_entity_type_name(steppps_entity_type_t type);
void steppps_entity_print(const steppps_entity_t* entity);
void steppps_registry_print_stats(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_STEPPPS_ENTITY_H */
