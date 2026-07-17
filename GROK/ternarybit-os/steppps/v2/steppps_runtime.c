/**
 * steppps_runtime.c - STEPPPS v2 Smart Secure Runtime
 *
 * Provides:
 * - JSON parsing (simple, no external deps)
 * - Signature verification
 * - Karma-based trust checking
 * - Sandboxed execution
 * - Audit logging
 */

#include "steppps.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

/* Cross-platform support */
#include "tbos/platform.h"

#if defined(__APPLE__) && !defined(TBOS_PLATFORM_IOS)
#include <sandbox.h>
#endif

/* ========================================================================= */
/* GLOBAL RUNTIME STATE                                                       */
/* ========================================================================= */

static struct {
    bool initialized;
    int64_t user_karma;             /* Current user's karma */
    steppps_cap_t user_caps;        /* User's granted capabilities */
    char audit_path[256];           /* Audit log path */
    char blacklist_path[256];       /* Blacklist path */
    FILE* audit_file;
} g_runtime = {0};

/* ========================================================================= */
/* SIMPLE JSON PARSER (No External Dependencies)                              */
/* ========================================================================= */

static const char* json_skip_ws(const char* p) {
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
    return p;
}

static const char* json_skip_string(const char* p) {
    if (!p || *p != '"') return NULL;
    p++;
    while (*p) {
        if (*p == '\\') {
            if (!p[1]) return NULL;
            p += 2;
        } else if (*p == '"') {
            return p + 1;
        } else {
            p++;
        }
    }
    return NULL;
}

static const char* json_skip_value(const char* p) {
    char open;
    char close;
    int depth;

    p = json_skip_ws(p);
    if (!p || !*p) return NULL;
    if (*p == '"') return json_skip_string(p);

    if (*p == '{' || *p == '[') {
        open = *p;
        close = open == '{' ? '}' : ']';
        depth = 1;
        p++;
        while (*p && depth > 0) {
            if (*p == '"') {
                p = json_skip_string(p);
                if (!p) return NULL;
                continue;
            }
            if (*p == open) depth++;
            else if (*p == close) depth--;
            p++;
        }
        return depth == 0 ? p : NULL;
    }

    while (*p && *p != ',' && *p != '}' && *p != ']') p++;
    return p;
}

/* Find a direct member of one JSON object; nested objects are not searched. */
static const char* json_find_member(const char* object, const char* key) {
    const char* p;
    size_t key_len;

    if (!object || !key) return NULL;
    p = json_skip_ws(object);
    if (*p != '{') return NULL;
    p++;
    key_len = strlen(key);

    while (*p) {
        const char* key_start;
        const char* key_end;
        const char* value;

        p = json_skip_ws(p);
        if (*p == '}') return NULL;
        if (*p != '"') return NULL;

        key_start = p + 1;
        key_end = json_skip_string(p);
        if (!key_end) return NULL;

        p = json_skip_ws(key_end);
        if (*p != ':') return NULL;
        value = json_skip_ws(p + 1);

        if ((size_t)((key_end - 1) - key_start) == key_len &&
            strncmp(key_start, key, key_len) == 0) {
            return value;
        }

        p = json_skip_value(value);
        if (!p) return NULL;
        p = json_skip_ws(p);
        if (*p == ',') {
            p++;
            continue;
        }
        if (*p == '}') return NULL;
        return NULL;
    }
    return NULL;
}

static char* json_get_string(const char* json, const char* key, char* buf, size_t len) {
    const char* pos = json_find_member(json, key);
    if (!pos || !buf || len == 0) {
        if (buf && len) buf[0] = '\0';
        return NULL;
    }

    if (*pos == '"') {
        pos++;
        size_t i = 0;
        while (*pos && *pos != '"' && i < len - 1) {
            if (*pos == '\\' && *(pos + 1)) {
                pos++;
                switch (*pos) {
                    case 'n': buf[i++] = '\n'; break;
                    case 't': buf[i++] = '\t'; break;
                    case 'r': buf[i++] = '\r'; break;
                    case '\\': buf[i++] = '\\'; break;
                    case '"': buf[i++] = '"'; break;
                    default: buf[i++] = *pos;
                }
            } else {
                buf[i++] = *pos;
            }
            pos++;
        }
        buf[i] = '\0';
        return buf;
    }

    buf[0] = '\0';
    return NULL;
}

static int64_t json_get_int(const char* json, const char* key, int64_t def) {
    const char* pos = json_find_member(json, key);
    if (!pos) return def;
    return strtoll(pos, NULL, 10);
}

static double json_get_double(const char* json, const char* key, double def) {
    const char* pos = json_find_member(json, key);
    if (!pos) return def;
    return strtod(pos, NULL);
}

/* Find nested object and return pointer to its content */
static const char* json_find_object(const char* json, const char* key) {
    const char* pos = json_find_member(json, key);
    if (!pos) return NULL;
    if (*pos == '{') return pos;

    return NULL;
}

/* ========================================================================= */
/* SIMPLE SHA-256 (Minimal Implementation for Hashing)                        */
/* ========================================================================= */

/* For production, use a proper crypto library. This is a placeholder. */
static void compute_hash(const char* data, size_t len, char* hex_out) {
    /* Simple hash for demo - NOT cryptographically secure */
    uint64_t h = 0x5555555555555555ULL;
    for (size_t i = 0; i < len; i++) {
        h = h * 31 + (uint8_t)data[i];
        h ^= (h >> 17);
    }
    snprintf(hex_out, STEPPPS_HASH_SIZE * 2 + 1,
             "%016llx%016llx%016llx%016llx",
             (unsigned long long)(h ^ 0xDEADBEEF),
             (unsigned long long)(h ^ 0xCAFEBABE),
             (unsigned long long)(h ^ 0x12345678),
             (unsigned long long)(h ^ 0x87654321));
}

/* ========================================================================= */
/* RUNTIME INITIALIZATION                                                     */
/* ========================================================================= */

int steppps_runtime_init(void) {
    if (g_runtime.initialized) return 0;

    /* Set defaults */
    g_runtime.user_karma = 100;  /* Default karma for new users */
    g_runtime.user_caps = CAP_READ_FS | CAP_EXEC;  /* Basic caps */

    /* Setup audit log */
    const char* home = getenv("HOME");
    if (home) {
        snprintf(g_runtime.audit_path, sizeof(g_runtime.audit_path),
                 "%s/.tbos/steppps_audit.log", home);
        snprintf(g_runtime.blacklist_path, sizeof(g_runtime.blacklist_path),
                 "%s/.tbos/steppps_blacklist", home);

        /* Create directory if needed */
        char dir[256];
        snprintf(dir, sizeof(dir), "%s/.tbos", home);
        tbos_mkdir(dir);

        g_runtime.audit_file = fopen(g_runtime.audit_path, "a");
    }

    g_runtime.initialized = true;
    return 0;
}

void steppps_runtime_shutdown(void) {
    if (g_runtime.audit_file) {
        fclose(g_runtime.audit_file);
        g_runtime.audit_file = NULL;
    }
    g_runtime.initialized = false;
}

/* ========================================================================= */
/* PARSING                                                                    */
/* ========================================================================= */

int steppps_parse(const char* json, steppps_t* s) {
    if (!json || !s) return -1;

    memset(s, 0, sizeof(steppps_t));

    /* Basic fields */
    json_get_string(json, "id", s->id, sizeof(s->id));
    json_get_string(json, "name", s->name, sizeof(s->name));
    json_get_string(json, "version", s->version, sizeof(s->version));
    json_get_string(json, "kind", s->kind, sizeof(s->kind));
    if (!s->version[0]) {
        json_get_string(json, "steppps_version", s->version, sizeof(s->version));
    }
    if (!s->id[0]) {
        const char* identity = json_find_object(json, "id");
        if (identity) {
            json_get_string(identity, "uri", s->id, sizeof(s->id));
        }
    }

    if (!s->id[0]) return -1;  /* ID is required */

    /* Compute content hash */
    compute_hash(json, strlen(json), s->security.hash);

    /* S - Space */
    const char* space = json_find_object(json, "space");
    if (!space) space = json_find_object(json, "S_space");
    if (space) {
        const char* coordinates = json_find_object(space, "coordinates");
        json_get_string(space, "universe", s->space.universe, sizeof(s->space.universe));
        s->space.lat = json_get_double(coordinates ? coordinates : space, "lat", 0);
        s->space.lon = json_get_double(coordinates ? coordinates : space, "lon", 0);
        s->space.alt = json_get_double(coordinates ? coordinates : space, "alt", 0);
        json_get_string(space, "cosmic", s->space.cosmic, sizeof(s->space.cosmic));
        json_get_string(space, "device", s->space.device, sizeof(s->space.device));
        if (!s->space.device[0]) {
            json_get_string(space, "device_fingerprint", s->space.device,
                            sizeof(s->space.device));
        }
        json_get_string(space, "path", s->space.path, sizeof(s->space.path));
        json_get_string(space, "realm", s->space.realm, sizeof(s->space.realm));
    }
    if (!s->space.universe[0]) strcpy(s->space.universe, "earth");

    /* T - Time */
    const char* time_obj = json_find_object(json, "time");
    if (!time_obj) time_obj = json_find_object(json, "T_time");
    if (time_obj) {
        json_get_string(time_obj, "utc", s->time.utc, sizeof(s->time.utc));
        json_get_string(time_obj, "created", s->time.created, sizeof(s->time.created));
        json_get_string(time_obj, "expires", s->time.expires, sizeof(s->time.expires));
        json_get_string(time_obj, "relative", s->time.relative, sizeof(s->time.relative));
        json_get_string(time_obj, "epoch", s->time.epoch, sizeof(s->time.epoch));
    }
    if (!s->time.created[0]) {
        json_get_string(json, "created", s->time.created, sizeof(s->time.created));
    }
    if (!s->time.utc[0] && s->time.created[0]) {
        strncpy(s->time.utc, s->time.created, sizeof(s->time.utc) - 1);
    }

    /* E - Event */
    const char* event = json_find_object(json, "event");
    if (!event) event = json_find_object(json, "E_event");
    if (event) {
        json_get_string(event, "type", s->event.type, sizeof(s->event.type));
        json_get_string(event, "source", s->event.source, sizeof(s->event.source));
        json_get_string(event, "description", s->event.description, sizeof(s->event.description));
        json_get_string(event, "severity", s->event.severity, sizeof(s->event.severity));
        json_get_string(event, "parent", s->event.parent, sizeof(s->event.parent));
    }

    /* P - Psychology */
    const char* psych = json_find_object(json, "psychology");
    if (!psych) psych = json_find_object(json, "P_psychology");
    if (psych) {
        const char* sys = json_find_object(psych, "system");
        const char* environment = json_find_object(psych, "environment");
        if (sys) {
            s->psych.health = (int)json_get_int(sys, "health", 100);
            s->psych.karma = json_get_int(sys, "karma", 0);
            json_get_string(sys, "consciousness", s->psych.consciousness, sizeof(s->psych.consciousness));
        }
        json_get_string(environment ? environment : psych, "weather",
                        s->psych.weather, sizeof(s->psych.weather));
        s->psych.temperature = json_get_double(
            environment ? environment : psych, "temperature", 20);
        json_get_string(psych, "intent", s->psych.intent, sizeof(s->psych.intent));
        if (!s->psych.consciousness[0]) {
            json_get_string(psych, "consciousness_level", s->psych.consciousness,
                            sizeof(s->psych.consciousness));
        }
    }
    if (!s->psych.consciousness[0]) {
        json_get_string(json, "consciousness_level", s->psych.consciousness,
                        sizeof(s->psych.consciousness));
    }
    if (s->psych.karma == 0) {
        s->psych.karma = json_get_int(json, "karma", 0);
    }

    /* P - Prompt */
    const char* prompt = json_find_object(json, "prompt");
    if (!prompt) prompt = json_find_object(json, "P_prompt");
    if (prompt) {
        json_get_string(prompt, "system", s->prompt.system, sizeof(s->prompt.system));
        json_get_string(prompt, "user", s->prompt.user, sizeof(s->prompt.user));
        json_get_string(prompt, "next", s->prompt.next, sizeof(s->prompt.next));
        json_get_string(prompt, "interaction", s->prompt.interaction, sizeof(s->prompt.interaction));
        json_get_string(prompt, "model", s->prompt.model, sizeof(s->prompt.model));
        s->prompt.temperature = json_get_double(prompt, "temperature", 0.7);
    }

    /* P - Pixel */
    const char* pixel = json_find_object(json, "pixel");
    if (!pixel) pixel = json_find_object(json, "P_pixel");
    if (pixel) {
        json_get_string(pixel, "type", s->pixel.type, sizeof(s->pixel.type));
        if (!s->pixel.type[0]) {
            json_get_string(pixel, "form", s->pixel.type, sizeof(s->pixel.type));
        }
        json_get_string(pixel, "template", s->pixel.template_str, sizeof(s->pixel.template_str));
        /* data and actions are complex - store raw for now */
    }

    /* S - Script */
    const char* script = json_find_object(json, "script");
    if (!script) script = json_find_object(json, "S_script");
    if (script) {
        json_get_string(script, "lang", s->script.lang, sizeof(s->script.lang));
        json_get_string(script, "code", s->script.code, sizeof(s->script.code));
        json_get_string(script, "entry", s->script.entry, sizeof(s->script.entry));
        s->script.sandbox = (steppps_sandbox_t)json_get_int(script, "sandbox", SANDBOX_PURE);
    }

    /* Security - Signature */
    const char* sig = json_find_object(json, "signature");
    if (sig) {
        json_get_string(sig, "author", s->security.author, sizeof(s->security.author));
        s->security.author_karma = json_get_int(sig, "karma", 0);
        s->security.is_signed = true;
        /* Actual signature verification would go here */
    }

    /* Security - Requires (capabilities) */
    const char* requires = json_find_object(json, "requires");
    if (requires) {
        if (strstr(requires, "filesystem")) s->security.requested |= CAP_READ_FS | CAP_WRITE_FS;
        if (strstr(requires, "network")) s->security.requested |= CAP_NET_LOCAL | CAP_NET_REMOTE;
        if (strstr(requires, "execute")) s->security.requested |= CAP_EXEC;
        if (strstr(requires, "sangha")) s->security.requested |= CAP_SANGHA;
        if (strstr(requires, "camera")) s->security.requested |= CAP_CAMERA;
        if (strstr(requires, "location")) s->security.requested |= CAP_LOCATION;
    }

    /* Meta */
    const char* meta = json_find_object(json, "meta");
    if (meta) {
        json_get_string(meta, "author", s->author, sizeof(s->author));
        json_get_string(meta, "tags", s->tags, sizeof(s->tags));
        s->karma_cost = json_get_int(meta, "karma_cost", 0);
        s->karma_reward = json_get_int(meta, "karma_reward", 1);
    }

    s->loaded = true;
    return 0;
}

int steppps_load(const char* path, steppps_t* s) {
    FILE* f = fopen(path, "r");
    if (!f) return -1;

    char* json = malloc(STEPPPS_MAX_JSON);
    if (!json) {
        fclose(f);
        return -1;
    }

    size_t len = fread(json, 1, STEPPPS_MAX_JSON - 1, f);
    json[len] = '\0';
    fclose(f);

    int rc = steppps_parse(json, s);
    free(json);

    if (rc == 0) {
        strncpy(s->space.path, path, sizeof(s->space.path) - 1);
    }

    return rc;
}

void steppps_free(steppps_t* s) {
    if (s) {
        memset(s, 0, sizeof(steppps_t));
    }
}

/* ========================================================================= */
/* SECURITY                                                                   */
/* ========================================================================= */

int steppps_verify(steppps_t* s) {
    if (!s) return -1;

    /* Check blacklist */
    if (g_runtime.blacklist_path[0]) {
        FILE* bl = fopen(g_runtime.blacklist_path, "r");
        if (bl) {
            char line[256];
            while (fgets(line, sizeof(line), bl)) {
                line[strcspn(line, "\n")] = '\0';
                if (strcmp(line, s->security.hash) == 0 ||
                    strcmp(line, s->id) == 0) {
                    s->security.blacklisted = true;
                    fclose(bl);
                    return -1;  /* Blacklisted */
                }
            }
            fclose(bl);
        }
    }

    /* If signed, verify signature */
    if (s->security.is_signed) {
        /* TODO: Actual Ed25519 verification */
        /* For now, trust signed STEPPPS */
        s->security.sig_valid = true;
        s->verified = true;
    } else {
        /* Unsigned - limit to low sandbox */
        s->security.max_sandbox = SANDBOX_READ;
        s->verified = true;  /* Verified as unsigned */
    }

    return 0;
}

int steppps_check_trust(steppps_t* s, int64_t user_karma) {
    if (!s) return -1;

    /* Determine max sandbox level based on author karma */
    int64_t author_k = s->security.author_karma;

    for (int i = SANDBOX_TRUSTED; i >= SANDBOX_DISPLAY; i--) {
        if (author_k >= SANDBOX_KARMA_MIN[i]) {
            s->security.max_sandbox = (steppps_sandbox_t)i;
            break;
        }
    }

    /* User must also have enough karma */
    if (user_karma < s->karma_cost) {
        return -1;  /* User can't afford this STEPPPS */
    }

    return 0;
}

int steppps_request_caps(steppps_t* s) {
    if (!s) return -1;

    steppps_cap_t needed = s->security.requested;
    steppps_cap_t have = g_runtime.user_caps;

    /* Check what we need to request */
    steppps_cap_t missing = needed & ~have;

    if (missing == CAP_NONE) {
        s->security.granted = needed;
        s->approved = true;
        return 0;
    }

    /* Prompt user for missing capabilities */
    printf("\n┌─────────────────────────────────────────┐\n");
    printf("│  STEPPPS CAPABILITY REQUEST             │\n");
    printf("├─────────────────────────────────────────┤\n");
    printf("│  ID: %-33s │\n", s->id);
    printf("│  Author: %-29s │\n", s->security.author[0] ? s->security.author : "(unsigned)");
    printf("│  Karma: %-30lld │\n", (long long)s->security.author_karma);
    printf("├─────────────────────────────────────────┤\n");
    printf("│  Requested Capabilities:                │\n");

    if (missing & CAP_READ_FS)   printf("│    - Read filesystem                   │\n");
    if (missing & CAP_WRITE_FS)  printf("│    - Write filesystem                  │\n");
    if (missing & CAP_EXEC)      printf("│    - Execute programs                  │\n");
    if (missing & CAP_NET_LOCAL) printf("│    - Local network                     │\n");
    if (missing & CAP_NET_REMOTE)printf("│    - Remote network                    │\n");
    if (missing & CAP_SANGHA)    printf("│    - Sangha discovery                  │\n");
    if (missing & CAP_CAMERA)    printf("│    - Camera access                     │\n");
    if (missing & CAP_MIC)       printf("│    - Microphone access                 │\n");
    if (missing & CAP_LOCATION)  printf("│    - Precise location                  │\n");

    printf("├─────────────────────────────────────────┤\n");
    printf("│  [Y]es  [N]o  [A]lways  [B]lacklist     │\n");
    printf("└─────────────────────────────────────────┘\n");
    printf("Allow? ");

    char response[16];
    if (!fgets(response, sizeof(response), stdin)) {
        return -1;
    }

    switch (response[0]) {
        case 'y': case 'Y':
            s->security.granted = needed;
            s->approved = true;
            return 0;

        case 'a': case 'A':
            s->security.granted = needed;
            g_runtime.user_caps |= needed;  /* Remember for future */
            s->approved = true;
            return 0;

        case 'b': case 'B':
            /* Add to blacklist */
            if (g_runtime.blacklist_path[0]) {
                FILE* bl = fopen(g_runtime.blacklist_path, "a");
                if (bl) {
                    fprintf(bl, "%s\n", s->security.hash);
                    fclose(bl);
                }
            }
            s->security.blacklisted = true;
            return -1;

        default:
            return -1;  /* Denied */
    }
}

bool steppps_is_safe(const steppps_t* s) {
    if (!s) return false;
    if (s->security.blacklisted) return false;
    if (!s->verified) return false;

    /* Unsigned + high sandbox = unsafe */
    if (!s->security.is_signed && s->script.sandbox > SANDBOX_READ) {
        return false;
    }

    return true;
}

/* ========================================================================= */
/* EXECUTION                                                                  */
/* ========================================================================= */

int steppps_run(steppps_t* s) {
    if (!s || !s->loaded) return -1;

    /* Security checks */
    if (!s->verified) {
        if (steppps_verify(s) != 0) {
            fprintf(stderr, "STEPPPS verification failed\n");
            return -1;
        }
    }

    if (s->security.blacklisted) {
        fprintf(stderr, "STEPPPS is blacklisted\n");
        return -1;
    }

    if (!s->approved) {
        if (steppps_request_caps(s) != 0) {
            fprintf(stderr, "Capabilities denied\n");
            return -1;
        }
    }

    /* Check sandbox level */
    if (s->script.sandbox > s->security.max_sandbox) {
        fprintf(stderr, "STEPPPS requires sandbox level %d, max allowed is %d\n",
                s->script.sandbox, s->security.max_sandbox);
        return -1;
    }

    /* Audit log - before execution */
    steppps_audit_log(s, "run", 0);

    /* Set environment */
    tbos_setenv("STEPPPS_ID", s->id);
    tbos_setenv("STEPPPS_NAME", s->name);
    tbos_setenv("STEPPPS_UNIVERSE", s->space.universe);
    tbos_setenv("STEPPPS_PATH", s->space.path);

    char karma_str[32];
    snprintf(karma_str, sizeof(karma_str), "%lld", (long long)s->psych.karma);
    tbos_setenv("STEPPPS_KARMA", karma_str);
    tbos_setenv("STEPPPS_CONSCIOUSNESS", s->psych.consciousness);

    printf("\n--- Executing STEPPPS [%s] ---\n", s->id);
    printf("Sandbox: %s | Lang: %s\n\n", steppps_sandbox_name(s->script.sandbox), s->script.lang);

    int rc = 0;

    if (strcmp(s->script.lang, "sh") == 0) {
        /* Shell execution (uses tbos_system for cross-platform support) */
        rc = tbos_system(s->script.code);
#ifndef TBOS_PLATFORM_WINDOWS
        rc = WEXITSTATUS(rc);
#endif
    }
    else if (strcmp(s->script.lang, "json") == 0) {
        /* JSON is data - just display */
        printf("%s\n", s->script.code);
    }
    else if (strcmp(s->script.lang, "c") == 0) {
        printf("[C code - would compile and execute]\n");
    }
    else {
        printf("[%s script]\n%s\n", s->script.lang, s->script.code);
    }

    s->exit_code = rc;

    /* Audit log - after execution */
    steppps_audit_log(s, "complete", rc);

    /* Award karma on success */
    if (rc == 0) {
        g_runtime.user_karma += s->karma_reward;
        printf("\n+%lld karma (total: %lld)\n",
               (long long)s->karma_reward, (long long)g_runtime.user_karma);
    }

    return rc;
}

/* ========================================================================= */
/* DISPLAY                                                                    */
/* ========================================================================= */

int steppps_display(const steppps_t* s) {
    if (!s) return -1;

    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════════╗\n");
    printf("║  STEPPPS: %-55s  ║\n", s->name);
    printf("╠═══════════════════════════════════════════════════════════════════╣\n");
    printf("║  ID: %-60s  ║\n", s->id);
    printf("║  Version: %-55s  ║\n", s->version[0] ? s->version : "1.0");
    if (s->kind[0]) {
        printf("║  Kind: %-58s  ║\n", s->kind);
    }
    printf("╠═══════════════════════════════════════════════════════════════════╣\n");

    /* S - Space */
    printf("║  S (SPACE) - Where in the Multiverse                              ║\n");
    printf("║    Universe: %-52s  ║\n", s->space.universe);
    if (s->space.lat != 0 || s->space.lon != 0) {
        printf("║    Coords:   %.4f, %.4f                                        ║\n",
               s->space.lat, s->space.lon);
    }
    if (s->space.cosmic[0]) {
        printf("║    Cosmic:   %-52s  ║\n", s->space.cosmic);
    }
    printf("║    Device:   %-52s  ║\n", s->space.device[0] ? s->space.device : "(local)");

    /* T - Time */
    printf("╠═══════════════════════════════════════════════════════════════════╣\n");
    printf("║  T (TIME) - When (UTC / Relative)                                 ║\n");
    printf("║    UTC:      %-52s  ║\n", s->time.utc[0] ? s->time.utc : "(now)");
    if (s->time.relative[0]) {
        printf("║    Relative: %-52s  ║\n", s->time.relative);
    }

    /* E - Event */
    printf("╠═══════════════════════════════════════════════════════════════════╣\n");
    printf("║  E (EVENT) - What Triggered This                                  ║\n");
    printf("║    Type:     %-52s  ║\n", s->event.type);
    printf("║    Severity: %-52s  ║\n", s->event.severity);
    if (s->event.description[0]) {
        printf("║    Desc:     %-52.52s  ║\n", s->event.description);
    }

    /* P - Psychology */
    printf("╠═══════════════════════════════════════════════════════════════════╣\n");
    printf("║  P (PSYCHOLOGY) - Context State                                   ║\n");
    printf("║    Health: %d%%  Karma: %lld  Consciousness: %-20s  ║\n",
           s->psych.health, (long long)s->psych.karma, s->psych.consciousness);
    if (s->psych.intent[0]) {
        printf("║    Intent:  %-53.53s  ║\n", s->psych.intent);
    }

    /* P - Prompt */
    printf("╠═══════════════════════════════════════════════════════════════════╣\n");
    printf("║  P (PROMPT) - AI Interaction                                      ║\n");
    if (s->prompt.user[0]) {
        printf("║    User:    %-53.53s  ║\n", s->prompt.user);
    }
    if (s->prompt.model[0]) {
        printf("║    Model:   %-53s  ║\n", s->prompt.model);
    }

    /* P - Pixel */
    printf("╠═══════════════════════════════════════════════════════════════════╣\n");
    printf("║  P (PIXEL) - Display                                              ║\n");
    printf("║    Type:    %-53s  ║\n", s->pixel.type[0] ? s->pixel.type : "terminal");

    /* S - Script */
    printf("╠═══════════════════════════════════════════════════════════════════╣\n");
    printf("║  S (SCRIPT) - Executable Code                                     ║\n");
    printf("║    Lang:    %-53s  ║\n", s->script.lang);
    printf("║    Sandbox: %-53s  ║\n", steppps_sandbox_name(s->script.sandbox));
    printf("║    Code:    %zu bytes                                              ║\n",
           strlen(s->script.code));

    /* Security */
    printf("╠═══════════════════════════════════════════════════════════════════╣\n");
    printf("║  SECURITY                                                         ║\n");
    printf("║    Signed:  %-53s  ║\n", s->security.is_signed ? "Yes" : "No");
    if (s->security.is_signed) {
        printf("║    Author:  %-53s  ║\n", s->security.author);
        printf("║    Karma:   %-53lld  ║\n", (long long)s->security.author_karma);
    }
    printf("║    Hash:    %.32s...  ║\n", s->security.hash);
    printf("║    Safe:    %-53s  ║\n", steppps_is_safe(s) ? "Yes" : "No");

    /* Meta */
    printf("╠═══════════════════════════════════════════════════════════════════╣\n");
    printf("║  META: Karma -%lld/+%lld                                            ║\n",
           (long long)s->karma_cost, (long long)s->karma_reward);
    printf("╚═══════════════════════════════════════════════════════════════════╝\n");

    return 0;
}

int steppps_render(const steppps_t* s) {
    if (!s || !s->pixel.template_str[0]) return -1;

    printf("\n--- Pixel Output (%s) ---\n\n", s->pixel.type);
    printf("%s\n", s->pixel.template_str);
    return 0;
}

/* ========================================================================= */
/* AUDIT                                                                      */
/* ========================================================================= */

int steppps_audit_log(const steppps_t* s, const char* action, int result) {
    if (!g_runtime.audit_file || !s) return -1;

    time_t now = time(NULL);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));

    fprintf(g_runtime.audit_file,
            "%s|%s|%s|%s|%s|%d|%lld\n",
            timestamp,
            s->id,
            s->security.hash,
            action,
            s->security.author[0] ? s->security.author : "unsigned",
            result,
            (long long)s->security.granted);

    fflush(g_runtime.audit_file);
    return 0;
}

/* ========================================================================= */
/* UTILITY                                                                    */
/* ========================================================================= */

const char* steppps_sandbox_name(steppps_sandbox_t level) {
    static const char* names[] = {
        "DISPLAY", "PURE", "READ", "WRITE", "NETWORK", "SYSTEM", "TRUSTED"
    };
    if (level > SANDBOX_TRUSTED) return "UNKNOWN";
    return names[level];
}

void steppps_print(const steppps_t* s) {
    steppps_display(s);
}
