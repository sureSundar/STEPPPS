/**
 * @file steppps_entity.c
 * @brief STEPPPS Universal Entity Framework Implementation
 *
 * Implements the Universal Entity Framework where every entity is:
 * - Identifiable (128-bit STEPPPS ID)
 * - Addressable (STEPPPS URI)
 * - Invokable (Action intents)
 * - Interactable (Message passing)
 * - Authenticated (Karma-based trust)
 * - Authorized (Consciousness-based permissions)
 *
 * @version 2.0
 */

#include "../../../include/tbos/steppps_entity.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef HOST_BUILD
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#else
/* Bare-metal stubs */
static uint64_t baremetal_time_us(void) { return 0; }
#endif

/* ========================================================================= */
/* GLOBAL STATE                                                               */
/* ========================================================================= */

static steppps_registry_t* g_registry = NULL;
static uint64_t g_entity_sequence = 0;
static uint64_t g_tbos_epoch = 1735689600;  /* 2025-01-01 00:00:00 UTC */
static int g_sangha_socket = -1;
static uint16_t g_sangha_port = 7805;       /* STEPPPS Sangha port */

/* ========================================================================= */
/* TIME HELPERS                                                               */
/* ========================================================================= */

static uint64_t get_time_us(void) {
#ifdef HOST_BUILD
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
#else
    return baremetal_time_us();
#endif
}

static uint32_t get_tbos_timestamp(void) {
#ifdef HOST_BUILD
    return (uint32_t)(time(NULL) - g_tbos_epoch);
#else
    return 0;
#endif
}

static uint64_t get_random_bits(void) {
#ifdef HOST_BUILD
    uint64_t r = 0;
    for (int i = 0; i < 8; i++) {
        r = (r << 8) | (rand() & 0xFF);
    }
    return r;
#else
    return g_entity_sequence;
#endif
}

/* ========================================================================= */
/* ID FUNCTIONS                                                               */
/* ========================================================================= */

int steppps_id_generate(steppps_entity_type_t type, steppps_id_t* id) {
    if (!id) return -1;

    uint32_t timestamp = get_tbos_timestamp();
    uint32_t space_hash = 0;  /* TODO: Hash device fingerprint */
    uint64_t sequence = __sync_fetch_and_add(&g_entity_sequence, 1);
    uint64_t random = get_random_bits();

    /* Build high bits: type(4) + version(4) + tier(8) + timestamp(32) + space(16) */
    id->high = ((uint64_t)(type & 0xF) << 60) |
               ((uint64_t)1 << 56) |           /* Version 1 */
               ((uint64_t)3 << 48) |           /* Tier 3 (desktop default) */
               ((uint64_t)timestamp << 16) |
               ((space_hash >> 16) & 0xFFFF);

    /* Build low bits: space_low(16) + sequence(24) + random(24) */
    id->low = ((uint64_t)(space_hash & 0xFFFF) << 48) |
              ((sequence & 0xFFFFFF) << 24) |
              (random & 0xFFFFFF);

    return 0;
}

int steppps_id_to_string(const steppps_id_t* id, char* buf, size_t len) {
    if (!id || !buf || len < 33) return -1;
    snprintf(buf, len, "%016llx%016llx",
             (unsigned long long)id->high,
             (unsigned long long)id->low);
    return 0;
}

int steppps_id_from_string(const char* str, steppps_id_t* id) {
    if (!str || !id || strlen(str) < 32) return -1;

    char high_str[17] = {0};
    char low_str[17] = {0};
    strncpy(high_str, str, 16);
    strncpy(low_str, str + 16, 16);

    id->high = strtoull(high_str, NULL, 16);
    id->low = strtoull(low_str, NULL, 16);
    return 0;
}

bool steppps_id_equals(const steppps_id_t* a, const steppps_id_t* b) {
    if (!a || !b) return false;
    return a->high == b->high && a->low == b->low;
}

/* ========================================================================= */
/* URI FUNCTIONS                                                              */
/* ========================================================================= */

static const char* entity_type_strings[] = {
    "system", "process", "file", "device", "network",
    "script", "user", "sangha", "service", "intent",
    "resource", "pixel", "event", "prompt", "stream", "custom"
};

int steppps_uri_parse(const char* uri_str, steppps_uri_t* uri) {
    if (!uri_str || !uri) return -1;

    memset(uri, 0, sizeof(steppps_uri_t));
    strncpy(uri->raw, uri_str, STEPPPS_URI_MAX_LENGTH - 1);

    /* Check scheme */
    if (strncmp(uri_str, "steppps://", 10) != 0) return -1;

    const char* p = uri_str + 10;

    /* Parse entity type */
    const char* slash = strchr(p, '/');
    if (!slash) return -1;

    char type_str[32] = {0};
    strncpy(type_str, p, slash - p);

    uri->type = ENTITY_TYPE_CUSTOM;
    for (int i = 0; i <= ENTITY_TYPE_CUSTOM; i++) {
        if (strcmp(type_str, entity_type_strings[i]) == 0) {
            uri->type = (steppps_entity_type_t)i;
            break;
        }
    }

    /* Parse name/id */
    p = slash + 1;
    const char* query = strchr(p, '?');
    if (query) {
        strncpy(uri->name, p, query - p);
    } else {
        strncpy(uri->name, p, sizeof(uri->name) - 1);
    }

    /* Try to parse ID from name if it looks like hex */
    if (strlen(uri->name) == 32) {
        steppps_id_from_string(uri->name, &uri->id);
    }

    /* Parse query parameters */
    if (query) {
        p = query + 1;
        while (*p) {
            char key[32] = {0};
            char value[64] = {0};

            const char* eq = strchr(p, '=');
            if (!eq) break;

            strncpy(key, p, eq - p);
            p = eq + 1;

            const char* amp = strchr(p, '&');
            if (amp) {
                strncpy(value, p, amp - p);
                p = amp + 1;
            } else {
                strncpy(value, p, sizeof(value) - 1);
                p += strlen(value);
            }

            /* Map to STEPPPS dimensions */
            if (strcmp(key, "s") == 0 || strcmp(key, "space") == 0) {
                strncpy(uri->space, value, sizeof(uri->space) - 1);
            } else if (strcmp(key, "t") == 0 || strcmp(key, "time") == 0) {
                strncpy(uri->time, value, sizeof(uri->time) - 1);
            } else if (strcmp(key, "e") == 0 || strcmp(key, "event") == 0) {
                strncpy(uri->event, value, sizeof(uri->event) - 1);
            } else if (strcmp(key, "p") == 0 || strcmp(key, "psychology") == 0) {
                strncpy(uri->psychology, value, sizeof(uri->psychology) - 1);
            } else if (strcmp(key, "action") == 0) {
                strncpy(uri->action, value, sizeof(uri->action) - 1);
            } else if (uri->param_count < STEPPPS_URI_MAX_PARAMS) {
                snprintf(uri->params[uri->param_count], 64, "%s=%s", key, value);
                uri->param_count++;
            }
        }
    }

    return 0;
}

int steppps_uri_build(const steppps_entity_t* entity, steppps_uri_t* uri) {
    if (!entity || !uri) return -1;

    memset(uri, 0, sizeof(steppps_uri_t));
    uri->type = entity->type;
    uri->id = entity->id;
    strncpy(uri->name, entity->name, sizeof(uri->name) - 1);
    strncpy(uri->space, entity->location, sizeof(uri->space) - 1);

    /* Build URI string */
    char id_str[33];
    steppps_id_to_string(&entity->id, id_str, sizeof(id_str));

    snprintf(uri->raw, STEPPPS_URI_MAX_LENGTH,
             "steppps://%s/%s",
             entity_type_strings[entity->type],
             id_str);

    return 0;
}

int steppps_uri_to_string(const steppps_uri_t* uri, char* buf, size_t len) {
    if (!uri || !buf) return -1;

    char id_str[33] = {0};
    if (uri->id.high != 0 || uri->id.low != 0) {
        steppps_id_to_string(&uri->id, id_str, sizeof(id_str));
    } else {
        strncpy(id_str, uri->name, sizeof(id_str) - 1);
    }

    int written = snprintf(buf, len, "steppps://%s/%s",
                           entity_type_strings[uri->type], id_str);

    /* Add query params */
    bool first = true;
    if (uri->space[0]) {
        written += snprintf(buf + written, len - written, "%cs=%s",
                           first ? '?' : '&', uri->space);
        first = false;
    }
    if (uri->time[0]) {
        written += snprintf(buf + written, len - written, "%ct=%s",
                           first ? '?' : '&', uri->time);
        first = false;
    }
    if (uri->action[0]) {
        written += snprintf(buf + written, len - written, "%caction=%s",
                           first ? '?' : '&', uri->action);
    }

    return 0;
}

steppps_entity_t* steppps_uri_resolve(const steppps_uri_t* uri) {
    if (!uri) return NULL;

    /* Try by ID first */
    if (uri->id.high != 0 || uri->id.low != 0) {
        return steppps_registry_lookup(&uri->id);
    }

    /* Then by name */
    return steppps_registry_lookup_by_name(uri->name);
}

/* ========================================================================= */
/* REGISTRY FUNCTIONS                                                         */
/* ========================================================================= */

int steppps_registry_init(size_t capacity) {
    if (g_registry) return 0;  /* Already initialized */

    g_registry = (steppps_registry_t*)calloc(1, sizeof(steppps_registry_t));
    if (!g_registry) return -1;

    g_registry->capacity = capacity > 0 ? capacity : ENTITY_REGISTRY_MAX_SIZE;
    g_registry->entries = (entity_registry_entry_t*)calloc(
        g_registry->capacity, sizeof(entity_registry_entry_t));
    if (!g_registry->entries) {
        free(g_registry);
        g_registry = NULL;
        return -1;
    }

    g_registry->count = 0;
    g_registry->created_at = get_time_us();

#ifdef HOST_BUILD
    srand((unsigned int)time(NULL));
#endif

    return 0;
}

int steppps_registry_register(steppps_entity_t* entity) {
    if (!g_registry || !entity) return -1;
    if (g_registry->count >= g_registry->capacity) return -1;

    /* Find empty slot */
    for (size_t i = 0; i < g_registry->capacity; i++) {
        if (!g_registry->entries[i].active) {
            g_registry->entries[i].id = entity->id;
            g_registry->entries[i].entity = entity;
            g_registry->entries[i].active = true;
            g_registry->entries[i].registered_at = get_time_us();
            g_registry->entries[i].last_access = g_registry->entries[i].registered_at;
            g_registry->entries[i].access_count = 0;
            g_registry->count++;
            g_registry->total_registrations++;
            return 0;
        }
    }

    return -1;
}

int steppps_registry_unregister(const steppps_id_t* id) {
    if (!g_registry || !id) return -1;

    for (size_t i = 0; i < g_registry->capacity; i++) {
        if (g_registry->entries[i].active &&
            steppps_id_equals(&g_registry->entries[i].id, id)) {
            g_registry->entries[i].active = false;
            g_registry->entries[i].entity = NULL;
            g_registry->count--;
            return 0;
        }
    }

    return -1;
}

steppps_entity_t* steppps_registry_lookup(const steppps_id_t* id) {
    if (!g_registry || !id) return NULL;
    g_registry->total_lookups++;

    for (size_t i = 0; i < g_registry->capacity; i++) {
        if (g_registry->entries[i].active &&
            steppps_id_equals(&g_registry->entries[i].id, id)) {
            g_registry->entries[i].last_access = get_time_us();
            g_registry->entries[i].access_count++;
            g_registry->cache_hits++;
            return g_registry->entries[i].entity;
        }
    }

    g_registry->cache_misses++;
    return NULL;
}

steppps_entity_t* steppps_registry_lookup_by_name(const char* name) {
    if (!g_registry || !name) return NULL;
    g_registry->total_lookups++;

    for (size_t i = 0; i < g_registry->capacity; i++) {
        if (g_registry->entries[i].active &&
            g_registry->entries[i].entity &&
            strcmp(g_registry->entries[i].entity->name, name) == 0) {
            g_registry->entries[i].last_access = get_time_us();
            g_registry->entries[i].access_count++;
            g_registry->cache_hits++;
            return g_registry->entries[i].entity;
        }
    }

    g_registry->cache_misses++;
    return NULL;
}

int steppps_registry_find(steppps_entity_type_t type, steppps_entity_t** results,
                          size_t max_results, size_t* found) {
    if (!g_registry || !results || !found) return -1;

    *found = 0;
    for (size_t i = 0; i < g_registry->capacity && *found < max_results; i++) {
        if (g_registry->entries[i].active &&
            g_registry->entries[i].entity &&
            g_registry->entries[i].entity->type == type) {
            results[*found] = g_registry->entries[i].entity;
            (*found)++;
        }
    }

    return 0;
}

int steppps_registry_stats(uint64_t* total, uint64_t* active, uint64_t* lookups) {
    if (!g_registry) return -1;
    if (total) *total = g_registry->total_registrations;
    if (active) *active = g_registry->count;
    if (lookups) *lookups = g_registry->total_lookups;
    return 0;
}

/* ========================================================================= */
/* ENTITY MANAGEMENT                                                          */
/* ========================================================================= */

int steppps_entity_init(void) {
    return steppps_registry_init(ENTITY_REGISTRY_MAX_SIZE);
}

int steppps_entity_shutdown(void) {
    if (g_registry) {
        if (g_registry->entries) {
            free(g_registry->entries);
        }
        free(g_registry);
        g_registry = NULL;
    }
    return 0;
}

steppps_entity_t* steppps_entity_create(steppps_entity_type_t type, const char* name) {
    steppps_entity_t* entity = (steppps_entity_t*)calloc(1, sizeof(steppps_entity_t));
    if (!entity) return NULL;

    /* Generate ID */
    steppps_id_generate(type, &entity->id);

    /* Set basic properties */
    entity->type = type;
    if (name) {
        strncpy(entity->name, name, sizeof(entity->name) - 1);
    }

    /* Initialize state */
    entity->active = true;
    entity->authenticated = false;
    entity->created_at = get_time_us();
    entity->last_active = entity->created_at;

    /* Default consciousness and karma */
    entity->consciousness = CONSCIOUSNESS_REACTIVE;
    entity->karma.total = 0;
    entity->karma.positive = 0;
    entity->karma.negative = 0;
    entity->karma.action_count = 0;
    entity->karma.helpful_actions = 0;
    entity->karma.trust_score = 0.5f;

    /* Default capabilities based on type */
    switch (type) {
        case ENTITY_TYPE_SYSTEM:
            entity->capabilities = CAPABILITY_ALL;
            entity->consciousness = CONSCIOUSNESS_ENLIGHTENED;
            break;
        case ENTITY_TYPE_USER:
            entity->capabilities = CAPABILITY_READ | CAPABILITY_WRITE |
                                   CAPABILITY_EXECUTE | CAPABILITY_INVOKE |
                                   CAPABILITY_INTERACT | CAPABILITY_DISCOVER;
            entity->consciousness = CONSCIOUSNESS_AWARE;
            break;
        case ENTITY_TYPE_PROCESS:
            entity->capabilities = CAPABILITY_READ | CAPABILITY_EXECUTE |
                                   CAPABILITY_INTERACT;
            break;
        case ENTITY_TYPE_FILE:
            entity->capabilities = CAPABILITY_READ;
            break;
        case ENTITY_TYPE_SANGHA:
            entity->capabilities = CAPABILITY_NETWORK | CAPABILITY_SANGHA |
                                   CAPABILITY_DISCOVER | CAPABILITY_BROADCAST |
                                   CAPABILITY_INTERACT;
            break;
        default:
            entity->capabilities = CAPABILITY_READ | CAPABILITY_INVOKE;
    }

    /* Register in global registry */
    if (g_registry) {
        steppps_registry_register(entity);
    }

    return entity;
}

void steppps_entity_destroy(steppps_entity_t* entity) {
    if (!entity) return;

    /* Unregister */
    if (g_registry) {
        steppps_registry_unregister(&entity->id);
    }

    /* Free intents */
    if (entity->intents) {
        free(entity->intents);
    }

    /* Free children array */
    if (entity->children) {
        free(entity->children);
    }

    /* Free user data */
    if (entity->user_data) {
        free(entity->user_data);
    }

    free(entity);
}

/* ========================================================================= */
/* AUTHENTICATION                                                             */
/* ========================================================================= */

int steppps_authenticate(steppps_entity_t* source, steppps_entity_t* target) {
    if (!source || !target) return -1;

    /* Authentication based on karma and consciousness */
    float source_trust = steppps_karma_to_trust(&source->karma);
    float target_trust = steppps_karma_to_trust(&target->karma);

    /* Require minimum trust level */
    if (source_trust < 0.3f) {
        return -1;  /* Untrusted source */
    }

    /* Higher consciousness entities can authenticate with lower */
    if (source->consciousness >= target->consciousness) {
        source->authenticated = true;
        return 0;
    }

    /* Otherwise, need mutual high trust */
    if (source_trust >= 0.7f && target_trust >= 0.5f) {
        source->authenticated = true;
        return 0;
    }

    return -1;
}

int steppps_verify_karma(steppps_entity_t* entity, int64_t min_karma) {
    if (!entity) return -1;
    return entity->karma.total >= min_karma ? 0 : -1;
}

int steppps_verify_consciousness(steppps_entity_t* entity, steppps_consciousness_t min_level) {
    if (!entity) return -1;
    return entity->consciousness >= min_level ? 0 : -1;
}

/* ========================================================================= */
/* AUTHORIZATION                                                              */
/* ========================================================================= */

bool steppps_authorize(steppps_entity_t* entity, steppps_capability_t required) {
    if (!entity) return false;
    return (entity->capabilities & required) == required;
}

int steppps_grant_capability(steppps_entity_t* entity, steppps_capability_t cap) {
    if (!entity) return -1;
    entity->capabilities |= cap;
    return 0;
}

int steppps_revoke_capability(steppps_entity_t* entity, steppps_capability_t cap) {
    if (!entity) return -1;
    entity->capabilities &= ~cap;
    return 0;
}

/* ========================================================================= */
/* INVOCATION                                                                 */
/* ========================================================================= */

int steppps_register_intent(steppps_entity_t* entity, const steppps_intent_t* intent) {
    if (!entity || !intent) return -1;

    /* Grow intents array */
    steppps_intent_t* new_intents = (steppps_intent_t*)realloc(
        entity->intents, (entity->intent_count + 1) * sizeof(steppps_intent_t));
    if (!new_intents) return -1;

    entity->intents = new_intents;
    memcpy(&entity->intents[entity->intent_count], intent, sizeof(steppps_intent_t));
    entity->intent_count++;

    return 0;
}

int steppps_invoke(const steppps_uri_t* target, const steppps_invocation_t* inv,
                   steppps_result_t* result) {
    if (!target || !inv || !result) return -1;

    memset(result, 0, sizeof(steppps_result_t));
    uint64_t start = get_time_us();

    /* Resolve target entity */
    steppps_entity_t* entity = steppps_uri_resolve(target);
    if (!entity) {
        result->success = false;
        result->error_code = -1;
        strncpy(result->error_message, "Entity not found", sizeof(result->error_message) - 1);
        return -1;
    }

    /* Find the intent */
    steppps_intent_t* intent = NULL;
    for (uint8_t i = 0; i < entity->intent_count; i++) {
        if (strcmp(entity->intents[i].name, inv->intent_name) == 0) {
            intent = &entity->intents[i];
            break;
        }
    }

    if (!intent) {
        result->success = false;
        result->error_code = -2;
        snprintf(result->error_message, sizeof(result->error_message),
                 "Intent '%s' not found", inv->intent_name);
        return -1;
    }

    /* Check consciousness level */
    if (inv->consciousness < intent->min_consciousness) {
        result->success = false;
        result->error_code = -3;
        snprintf(result->error_message, sizeof(result->error_message),
                 "Requires consciousness level: %s",
                 steppps_consciousness_name(intent->min_consciousness));
        return -1;
    }

    /* Check karma cost */
    if (intent->karma_cost > 0 && inv->karma.total < intent->karma_cost) {
        result->success = false;
        result->error_code = -4;
        snprintf(result->error_message, sizeof(result->error_message),
                 "Insufficient karma: need %lld, have %lld",
                 (long long)intent->karma_cost, (long long)inv->karma.total);
        return -1;
    }

    /* Execute via entity's ops if available */
    if (entity->ops && entity->ops->invoke) {
        steppps_invocation_t mutable_inv = *inv;
        int rc = entity->ops->invoke(entity, &mutable_inv, result);
        if (rc == 0) {
            result->karma_change = intent->karma_reward;
        }
        result->execution_time_us = get_time_us() - start;
        return rc;
    }

    /* Default success for entities without custom handler */
    result->success = true;
    result->karma_change = intent->karma_reward;
    result->execution_time_us = get_time_us() - start;
    return 0;
}

int steppps_invoke_async(const steppps_uri_t* target, const steppps_invocation_t* inv) {
    /* TODO: Queue for async processing */
    (void)target;
    (void)inv;
    return -1;  /* Not implemented */
}

/* ========================================================================= */
/* INTERACTION (MESSAGE PASSING)                                              */
/* ========================================================================= */

int steppps_send(steppps_entity_t* source, const steppps_uri_t* target,
                 const void* message, size_t len) {
    if (!source || !target || !message || len == 0) return -1;

    /* Check send capability */
    if (!steppps_authorize(source, CAPABILITY_INTERACT)) {
        return -1;
    }

    /* Resolve target */
    steppps_entity_t* target_entity = steppps_uri_resolve(target);
    if (!target_entity) {
        /* May be remote - try Sangha */
        return -1;  /* Not implemented yet */
    }

    /* Local delivery via ops */
    if (target_entity->ops && target_entity->ops->receive_message) {
        return target_entity->ops->receive_message(target_entity, source,
                                                    (void*)message, (size_t*)&len);
    }

    return 0;
}

int steppps_receive(steppps_entity_t* entity, void* buffer, size_t* len,
                    steppps_id_t* sender, uint32_t timeout_ms) {
    /* TODO: Implement message queue */
    (void)entity;
    (void)buffer;
    (void)len;
    (void)sender;
    (void)timeout_ms;
    return -1;
}

int steppps_broadcast(steppps_entity_t* source, steppps_entity_type_t target_type,
                      const void* message, size_t len) {
    if (!source || !message || len == 0) return -1;

    if (!steppps_authorize(source, CAPABILITY_BROADCAST)) {
        return -1;
    }

    /* Find all entities of type and send */
    steppps_entity_t* targets[256];
    size_t found = 0;
    steppps_registry_find(target_type, targets, 256, &found);

    int success_count = 0;
    for (size_t i = 0; i < found; i++) {
        if (targets[i] != source) {
            steppps_uri_t uri;
            steppps_uri_build(targets[i], &uri);
            if (steppps_send(source, &uri, message, len) == 0) {
                success_count++;
            }
        }
    }

    return success_count;
}

/* ========================================================================= */
/* SANGHA DISCOVERY                                                           */
/* ========================================================================= */

int sangha_init(uint16_t port) {
    g_sangha_port = port;

#ifdef HOST_BUILD
    g_sangha_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_sangha_socket < 0) return -1;

    /* Enable broadcast */
    int broadcast = 1;
    setsockopt(g_sangha_socket, SOL_SOCKET, SO_BROADCAST,
               &broadcast, sizeof(broadcast));

    /* Bind to port */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(g_sangha_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(g_sangha_socket);
        g_sangha_socket = -1;
        return -1;
    }
#endif

    return 0;
}

int sangha_announce(steppps_entity_t* entity) {
    if (!entity || g_sangha_socket < 0) return -1;

#ifdef HOST_BUILD
    /* Build announcement message */
    sangha_discovery_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.sender = entity->id;
    strncpy(msg.message_type, "announce", sizeof(msg.message_type) - 1);
    strncpy(msg.name, entity->name, sizeof(msg.name) - 1);
    strncpy(msg.device_type, "desktop", sizeof(msg.device_type) - 1);  /* TODO */
    msg.timestamp = get_time_us();

    /* Broadcast */
    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(g_sangha_port);
    broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;

    sendto(g_sangha_socket, &msg, sizeof(msg), 0,
           (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
#endif

    return 0;
}

int sangha_discover(steppps_entity_type_t type, const char* intent,
                    sangha_node_t* nodes, size_t max_nodes, size_t* found) {
    if (!nodes || !found) return -1;

    *found = 0;

#ifdef HOST_BUILD
    /* Send query */
    sangha_discovery_t query;
    memset(&query, 0, sizeof(query));
    strncpy(query.message_type, "query", sizeof(query.message_type) - 1);
    query.query_type = type;
    if (intent) {
        strncpy(query.query_intent, intent, sizeof(query.query_intent) - 1);
    }
    query.timestamp = get_time_us();

    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(g_sangha_port);
    broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;

    sendto(g_sangha_socket, &query, sizeof(query), 0,
           (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));

    /* Wait for responses (with timeout) */
    /* TODO: Implement async receive */
#else
    (void)type;
    (void)intent;
    (void)max_nodes;
#endif

    return 0;
}

int sangha_query(const steppps_id_t* target, const char* intent, steppps_result_t* result) {
    /* TODO: Send direct query to specific node */
    (void)target;
    (void)intent;
    (void)result;
    return -1;
}

int sangha_get_nodes(sangha_node_t* nodes, size_t max_nodes, size_t* count) {
    /* TODO: Return cached known nodes */
    (void)nodes;
    (void)max_nodes;
    if (count) *count = 0;
    return 0;
}

/* ========================================================================= */
/* KARMA FUNCTIONS                                                            */
/* ========================================================================= */

int steppps_karma_award(steppps_entity_t* entity, int64_t amount, const char* reason) {
    if (!entity || amount <= 0) return -1;

    entity->karma.total += amount;
    entity->karma.positive += amount;
    entity->karma.action_count++;

    /* Recalculate trust */
    entity->karma.trust_score = steppps_karma_to_trust(&entity->karma);

    /* Log reason */
    (void)reason;  /* TODO: Event logging */

    return 0;
}

int steppps_karma_deduct(steppps_entity_t* entity, int64_t amount, const char* reason) {
    if (!entity || amount <= 0) return -1;

    entity->karma.total -= amount;
    entity->karma.negative += amount;
    entity->karma.action_count++;

    /* Recalculate trust */
    entity->karma.trust_score = steppps_karma_to_trust(&entity->karma);

    (void)reason;

    return 0;
}

int steppps_karma_get(steppps_entity_t* entity, steppps_karma_t* karma) {
    if (!entity || !karma) return -1;
    *karma = entity->karma;
    return 0;
}

float steppps_karma_to_trust(const steppps_karma_t* karma) {
    if (!karma || karma->action_count == 0) return 0.5f;

    /* Trust = 0.5 + (positive - negative) / (total_actions * 2) */
    float ratio = (float)(karma->positive - karma->negative) /
                  (float)(karma->action_count * 2);

    float trust = 0.5f + ratio;
    if (trust < 0.0f) trust = 0.0f;
    if (trust > 1.0f) trust = 1.0f;

    return trust;
}

/* ========================================================================= */
/* CONSCIOUSNESS FUNCTIONS                                                    */
/* ========================================================================= */

int steppps_consciousness_evolve(steppps_entity_t* entity) {
    if (!entity) return -1;

    /* Evolution requirements */
    static const int64_t karma_thresholds[] = {0, 100, 500, 2000, 10000, 50000};
    static const float trust_thresholds[] = {0.0f, 0.3f, 0.5f, 0.7f, 0.85f, 0.95f};

    int next_level = entity->consciousness + 1;
    if (next_level > CONSCIOUSNESS_ENLIGHTENED) return 0;  /* Already max */

    /* Check requirements */
    if (entity->karma.total >= karma_thresholds[next_level] &&
        entity->karma.trust_score >= trust_thresholds[next_level]) {
        entity->consciousness = (steppps_consciousness_t)next_level;
        return 1;  /* Evolved */
    }

    return 0;  /* Not ready */
}

int steppps_consciousness_get_level(steppps_entity_t* entity, steppps_consciousness_t* level) {
    if (!entity || !level) return -1;
    *level = entity->consciousness;
    return 0;
}

const char* steppps_consciousness_name(steppps_consciousness_t level) {
    static const char* names[] = {
        "Dormant", "Reactive", "Adaptive", "Aware", "Mindful", "Enlightened"
    };
    if (level > CONSCIOUSNESS_ENLIGHTENED) return "Unknown";
    return names[level];
}

/* ========================================================================= */
/* UTILITY FUNCTIONS                                                          */
/* ========================================================================= */

const char* steppps_entity_type_name(steppps_entity_type_t type) {
    if (type > ENTITY_TYPE_CUSTOM) return "unknown";
    return entity_type_strings[type];
}

void steppps_entity_print(const steppps_entity_t* entity) {
    if (!entity) {
        printf("Entity: (null)\n");
        return;
    }

    char id_str[33];
    steppps_id_to_string(&entity->id, id_str, sizeof(id_str));

    printf("STEPPPS Entity:\n");
    printf("  ID:           %s\n", id_str);
    printf("  Type:         %s\n", steppps_entity_type_name(entity->type));
    printf("  Name:         %s\n", entity->name);
    printf("  Active:       %s\n", entity->active ? "Yes" : "No");
    printf("  Authenticated: %s\n", entity->authenticated ? "Yes" : "No");
    printf("  Consciousness: %s\n", steppps_consciousness_name(entity->consciousness));
    printf("  Karma:        %lld (trust: %.2f)\n",
           (long long)entity->karma.total, entity->karma.trust_score);
    printf("  Capabilities: 0x%04X\n", entity->capabilities);
    printf("  Intents:      %d\n", entity->intent_count);

    /* Print URI */
    steppps_uri_t uri;
    steppps_uri_build(entity, &uri);
    printf("  URI:          %s\n", uri.raw);
}

void steppps_registry_print_stats(void) {
    if (!g_registry) {
        printf("Registry: Not initialized\n");
        return;
    }

    printf("STEPPPS Entity Registry:\n");
    printf("  Capacity:     %zu\n", g_registry->capacity);
    printf("  Active:       %zu\n", g_registry->count);
    printf("  Total Reg:    %llu\n", (unsigned long long)g_registry->total_registrations);
    printf("  Lookups:      %llu\n", (unsigned long long)g_registry->total_lookups);
    printf("  Cache Hits:   %llu\n", (unsigned long long)g_registry->cache_hits);
    printf("  Cache Miss:   %llu\n", (unsigned long long)g_registry->cache_misses);

    if (g_registry->total_lookups > 0) {
        float hit_rate = 100.0f * g_registry->cache_hits / g_registry->total_lookups;
        printf("  Hit Rate:     %.1f%%\n", hit_rate);
    }
}
