/**
 * steppps_vfs.c - STEPPPS-Aware Virtual Filesystem Implementation
 *
 * Security gate that checks karma/consciousness before file operations.
 * Supports companion .steppps files for rich metadata.
 */

#include "../../include/tbos/steppps_vfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#ifdef HOST_BUILD
#include <unistd.h>
#endif

/* ========================================================================= */
/* GLOBAL CONTEXT                                                             */
/* ========================================================================= */

static steppps_vfs_ctx_t g_ctx = {
    .security_level = STEPPPS_SECURITY_MODERATE,
    .audit_enabled = false,
    .companion_ext = ".steppps",
    .total_operations = 0,
    .allowed_operations = 0,
    .denied_operations = 0,
    .warnings_issued = 0
};

static FILE* g_audit_file = NULL;

/* ========================================================================= */
/* UTILITY                                                                    */
/* ========================================================================= */

const char* steppps_op_name(steppps_op_t op) {
    static const char* names[] = {
        "READ", "WRITE", "DELETE", "EXECUTE", "LIST", "CREATE", "RENAME", "CHMOD"
    };
    if (op > STEPPPS_OP_CHMOD) return "UNKNOWN";
    return names[op];
}

const char* steppps_security_level_name(steppps_security_level_t level) {
    static const char* names[] = {"PERMISSIVE", "MODERATE", "STRICT"};
    if (level > STEPPPS_SECURITY_STRICT) return "UNKNOWN";
    return names[level];
}

static uint64_t get_time_ms(void) {
#ifdef HOST_BUILD
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#else
    return 0;
#endif
}

/* Simple hash function for ID generation */
static uint64_t hash_string(const char* str) {
    uint64_t h = 0x5555555555555555ULL;
    while (*str) {
        h = h * 31 + (uint8_t)*str++;
        h ^= (h >> 17);
    }
    return h;
}

/* ========================================================================= */
/* INITIALIZATION                                                             */
/* ========================================================================= */

int steppps_vfs_init(steppps_security_level_t level) {
    memset(&g_ctx, 0, sizeof(g_ctx));
    g_ctx.security_level = level;
    strcpy(g_ctx.companion_ext, ".steppps");

    /* Default caller (unauthenticated) */
    g_ctx.current_caller.karma = 0;
    g_ctx.current_caller.consciousness = 0;
    strcpy(g_ctx.current_caller.name, "anonymous");

    return 0;
}

void steppps_vfs_shutdown(void) {
    if (g_audit_file) {
        fclose(g_audit_file);
        g_audit_file = NULL;
    }
}

steppps_vfs_ctx_t* steppps_vfs_get_context(void) {
    return &g_ctx;
}

/* ========================================================================= */
/* CALLER MANAGEMENT                                                          */
/* ========================================================================= */

int steppps_vfs_set_caller(const steppps_caller_t* caller) {
    if (!caller) return -1;
    memcpy(&g_ctx.current_caller, caller, sizeof(steppps_caller_t));
    return 0;
}

int steppps_vfs_get_caller(steppps_caller_t* caller) {
    if (!caller) return -1;
    memcpy(caller, &g_ctx.current_caller, sizeof(steppps_caller_t));
    return 0;
}

/* ========================================================================= */
/* SECURITY CONFIGURATION                                                     */
/* ========================================================================= */

int steppps_vfs_set_security_level(steppps_security_level_t level) {
    g_ctx.security_level = level;
    return 0;
}

steppps_security_level_t steppps_vfs_get_security_level(void) {
    return g_ctx.security_level;
}

/* ========================================================================= */
/* COMPANION FILE MANAGEMENT                                                  */
/* ========================================================================= */

int steppps_vfs_get_companion_path(const char* path, char* companion, size_t len) {
    if (!path || !companion) return -1;
    snprintf(companion, len, "%s%s", path, g_ctx.companion_ext);
    return 0;
}

bool steppps_vfs_has_companion(const char* path) {
    char companion[512];
    steppps_vfs_get_companion_path(path, companion, sizeof(companion));

#ifdef HOST_BUILD
    struct stat st;
    return (stat(companion, &st) == 0);
#else
    return false;
#endif
}

int steppps_vfs_read_companion(const char* path, char* json, size_t max_len) {
    char companion[512];
    steppps_vfs_get_companion_path(path, companion, sizeof(companion));

#ifdef HOST_BUILD
    FILE* f = fopen(companion, "r");
    if (!f) return -1;

    size_t len = fread(json, 1, max_len - 1, f);
    json[len] = '\0';
    fclose(f);
    return (int)len;
#else
    (void)json;
    (void)max_len;
    return -1;
#endif
}

int steppps_vfs_write_companion(const char* path, const char* json) {
    char companion[512];
    steppps_vfs_get_companion_path(path, companion, sizeof(companion));

#ifdef HOST_BUILD
    FILE* f = fopen(companion, "w");
    if (!f) return -1;

    fprintf(f, "%s", json);
    fclose(f);
    return 0;
#else
    (void)json;
    return -1;
#endif
}

/* ========================================================================= */
/* MINIMAL STEPPPS GENERATION                                                 */
/* ========================================================================= */

int steppps_vfs_generate_minimal(const char* path, steppps_minimal_t* meta) {
    if (!path || !meta) return -1;

    memset(meta, 0, sizeof(steppps_minimal_t));

    /* Generate ID from path */
    meta->id_high = hash_string(path);
    meta->id_low = get_time_ms() ^ hash_string(path);

    /* Space */
    strncpy(meta->path, path, sizeof(meta->path) - 1);
#ifdef HOST_BUILD
    gethostname(meta->device, sizeof(meta->device) - 1);
#else
    strcpy(meta->device, "tbos");
#endif

    /* Time */
    uint64_t now = get_time_ms() / 1000;
    meta->created = now;
    meta->modified = now;
    meta->accessed = now;

    /* Security - inherit from creator */
    meta->karma = g_ctx.current_caller.karma;
    meta->consciousness = g_ctx.current_caller.consciousness;
    meta->required_karma = 0;           /* Default: anyone can access */
    meta->required_consciousness = 0;   /* Default: dormant */
    meta->is_signed = false;

    /* Flags */
    meta->has_rich_steppps = steppps_vfs_has_companion(path);
    meta->is_executable = false;

    return 0;
}

int steppps_vfs_get_minimal(const char* path, steppps_minimal_t* meta) {
    if (!path || !meta) return -1;

    /* Try to read from companion file first */
    if (steppps_vfs_has_companion(path)) {
        char json[4096];
        if (steppps_vfs_read_companion(path, json, sizeof(json)) > 0) {
            /* Parse minimal fields from JSON */
            /* For now, generate fresh but mark has_rich_steppps */
            steppps_vfs_generate_minimal(path, meta);
            meta->has_rich_steppps = true;

            /* Extract required_karma from JSON if present */
            const char* karma_pos = strstr(json, "\"required_karma\"");
            if (karma_pos) {
                karma_pos = strchr(karma_pos, ':');
                if (karma_pos) {
                    meta->required_karma = (uint8_t)atoi(karma_pos + 1);
                }
            }

            /* Extract required_consciousness */
            const char* cons_pos = strstr(json, "\"required_consciousness\"");
            if (cons_pos) {
                cons_pos = strchr(cons_pos, ':');
                if (cons_pos) {
                    meta->required_consciousness = (uint8_t)atoi(cons_pos + 1);
                }
            }

            return 0;
        }
    }

    /* Generate minimal STEPPPS */
    return steppps_vfs_generate_minimal(path, meta);
}

int steppps_vfs_set_minimal(const char* path, const steppps_minimal_t* meta) {
    if (!path || !meta) return -1;

    /* Write as companion JSON */
    char json[2048];
    snprintf(json, sizeof(json),
        "{\n"
        "  \"id\": \"%016llx%016llx\",\n"
        "  \"space\": { \"path\": \"%s\", \"device\": \"%s\" },\n"
        "  \"time\": { \"created\": %llu, \"modified\": %llu },\n"
        "  \"security\": {\n"
        "    \"karma\": %d,\n"
        "    \"consciousness\": %d,\n"
        "    \"required_karma\": %d,\n"
        "    \"required_consciousness\": %d,\n"
        "    \"is_signed\": %s\n"
        "  }\n"
        "}\n",
        (unsigned long long)meta->id_high,
        (unsigned long long)meta->id_low,
        meta->path,
        meta->device,
        (unsigned long long)meta->created,
        (unsigned long long)meta->modified,
        meta->karma,
        meta->consciousness,
        meta->required_karma,
        meta->required_consciousness,
        meta->is_signed ? "true" : "false"
    );

    return steppps_vfs_write_companion(path, json);
}

/* ========================================================================= */
/* KARMA/CONSCIOUSNESS SETTERS                                                */
/* ========================================================================= */

int steppps_vfs_set_required_karma(const char* path, int32_t karma) {
    steppps_minimal_t meta;
    if (steppps_vfs_get_minimal(path, &meta) != 0) {
        steppps_vfs_generate_minimal(path, &meta);
    }
    meta.required_karma = (uint8_t)(karma > 255 ? 255 : karma);
    return steppps_vfs_set_minimal(path, &meta);
}

int steppps_vfs_set_required_consciousness(const char* path, uint8_t level) {
    steppps_minimal_t meta;
    if (steppps_vfs_get_minimal(path, &meta) != 0) {
        steppps_vfs_generate_minimal(path, &meta);
    }
    meta.required_consciousness = level > 5 ? 5 : level;
    return steppps_vfs_set_minimal(path, &meta);
}

/* ========================================================================= */
/* AUDIT                                                                      */
/* ========================================================================= */

static void audit_log(const steppps_audit_entry_t* entry) {
    if (!g_ctx.audit_enabled || !g_audit_file) return;

    fprintf(g_audit_file,
        "%llu|%s|%s|%s|%d|%d|%d|%s\n",
        (unsigned long long)entry->timestamp,
        steppps_op_name(entry->operation),
        entry->path,
        entry->caller.name,
        entry->caller.karma,
        entry->caller.consciousness,
        entry->result,
        entry->reason
    );
    fflush(g_audit_file);
}

int steppps_vfs_audit_enable(const char* log_path) {
    if (g_audit_file) {
        fclose(g_audit_file);
    }

    g_audit_file = fopen(log_path, "a");
    if (!g_audit_file) return -1;

    strncpy(g_ctx.audit_path, log_path, sizeof(g_ctx.audit_path) - 1);
    g_ctx.audit_enabled = true;
    return 0;
}

int steppps_vfs_audit_disable(void) {
    if (g_audit_file) {
        fclose(g_audit_file);
        g_audit_file = NULL;
    }
    g_ctx.audit_enabled = false;
    return 0;
}

/* ========================================================================= */
/* SECURITY GATE - THE CORE CHECK                                             */
/* ========================================================================= */

int steppps_vfs_check(const char* path, steppps_op_t op) {
    if (!path) return -1;

    g_ctx.total_operations++;

    steppps_audit_entry_t entry;
    memset(&entry, 0, sizeof(entry));
    entry.timestamp = get_time_ms();
    entry.operation = op;
    strncpy(entry.path, path, sizeof(entry.path) - 1);
    memcpy(&entry.caller, &g_ctx.current_caller, sizeof(steppps_caller_t));
    entry.result = 0;

    /* Root bypasses all checks */
    if (g_ctx.current_caller.is_root) {
        g_ctx.allowed_operations++;
        audit_log(&entry);
        return 0;
    }

    /* Get file's STEPPPS metadata */
    steppps_minimal_t meta;
    steppps_vfs_get_minimal(path, &meta);

    /* Check 1: Karma requirement */
    int required_karma = meta.required_karma;
    if (required_karma == 0) {
        /* Use operation default */
        required_karma = STEPPPS_OP_KARMA_COST[op];
    }

    if (g_ctx.current_caller.karma < required_karma) {
        snprintf(entry.reason, sizeof(entry.reason),
                 "Insufficient karma: have %d, need %d",
                 g_ctx.current_caller.karma, required_karma);

        if (g_ctx.security_level == STEPPPS_SECURITY_STRICT) {
            entry.result = -1;
            g_ctx.denied_operations++;
            audit_log(&entry);
            if (g_ctx.on_deny) g_ctx.on_deny(&entry);
            return -1;
        } else if (g_ctx.security_level == STEPPPS_SECURITY_MODERATE) {
            g_ctx.warnings_issued++;
            if (g_ctx.on_warn) g_ctx.on_warn(&entry);
            /* Fall through - allow but warn */
        }
        /* PERMISSIVE: just log */
    }

    /* Check 2: Consciousness requirement */
    int required_consciousness = meta.required_consciousness;
    if (required_consciousness == 0) {
        /* Use operation default */
        required_consciousness = STEPPPS_OP_MIN_CONSCIOUSNESS[op];
    }

    if (g_ctx.current_caller.consciousness < required_consciousness) {
        snprintf(entry.reason, sizeof(entry.reason),
                 "Insufficient consciousness: have %d, need %d",
                 g_ctx.current_caller.consciousness, required_consciousness);

        if (g_ctx.security_level == STEPPPS_SECURITY_STRICT) {
            entry.result = -1;
            g_ctx.denied_operations++;
            audit_log(&entry);
            if (g_ctx.on_deny) g_ctx.on_deny(&entry);
            return -1;
        } else if (g_ctx.security_level == STEPPPS_SECURITY_MODERATE) {
            g_ctx.warnings_issued++;
            if (g_ctx.on_warn) g_ctx.on_warn(&entry);
        }
    }

    /* Check 3: Signed file requires signed caller for write/delete */
    if (meta.is_signed && (op == STEPPPS_OP_WRITE || op == STEPPPS_OP_DELETE)) {
        /* TODO: Verify caller signature */
    }

    /* Allowed */
    g_ctx.allowed_operations++;
    entry.result = 0;
    entry.reason[0] = '\0';
    audit_log(&entry);

    return 0;
}

/* ========================================================================= */
/* STATISTICS                                                                 */
/* ========================================================================= */

void steppps_vfs_print_stats(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════╗\n");
    printf("║           STEPPPS VFS STATISTICS                  ║\n");
    printf("╠═══════════════════════════════════════════════════╣\n");
    printf("║  Security Level: %-30s  ║\n",
           steppps_security_level_name(g_ctx.security_level));
    printf("║  Current Caller: %-30s  ║\n", g_ctx.current_caller.name);
    printf("║  Caller Karma:   %-30d  ║\n", g_ctx.current_caller.karma);
    printf("║  Caller Consciousness: %-24d  ║\n", g_ctx.current_caller.consciousness);
    printf("╠═══════════════════════════════════════════════════╣\n");
    printf("║  Total Operations:   %-27llu  ║\n",
           (unsigned long long)g_ctx.total_operations);
    printf("║  Allowed:            %-27llu  ║\n",
           (unsigned long long)g_ctx.allowed_operations);
    printf("║  Denied:             %-27llu  ║\n",
           (unsigned long long)g_ctx.denied_operations);
    printf("║  Warnings:           %-27llu  ║\n",
           (unsigned long long)g_ctx.warnings_issued);
    printf("║  Audit Enabled:      %-27s  ║\n",
           g_ctx.audit_enabled ? "Yes" : "No");
    printf("╚═══════════════════════════════════════════════════╝\n");
}
