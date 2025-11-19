#define _POSIX_C_SOURCE 200809L

#include "tbos_karma_ledger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#define TBOS_KARMA_LEDGER_PATH "artifacts/karma_ledger.log"
#define TBOS_KARMA_LEDGER_MAX_PERSONAS 32

typedef struct {
    char persona[64];
    karma_score_t total;
    int initialized;
} ledger_total_t;

static ledger_total_t g_totals[TBOS_KARMA_LEDGER_MAX_PERSONAS];
static int g_ledger_loaded = 0;

static void ensure_artifacts_dir(void) {
    struct stat st;
    if (stat("artifacts", &st) != 0) {
        mkdir("artifacts", 0775);
    }
}

static ledger_total_t* find_total(const char* persona, int create) {
    if (!persona) persona = "default";
    for (size_t i = 0; i < TBOS_KARMA_LEDGER_MAX_PERSONAS; ++i) {
        if (g_totals[i].initialized &&
            strcmp(g_totals[i].persona, persona) == 0) {
            return &g_totals[i];
        }
    }
    if (!create) return NULL;
    for (size_t i = 0; i < TBOS_KARMA_LEDGER_MAX_PERSONAS; ++i) {
        if (!g_totals[i].initialized) {
            strncpy(g_totals[i].persona, persona, sizeof(g_totals[i].persona) - 1);
            g_totals[i].persona[sizeof(g_totals[i].persona) - 1] = '\0';
            g_totals[i].total = 0;
            g_totals[i].initialized = 1;
            return &g_totals[i];
        }
    }
    return NULL;
}

static void ledger_load(void) {
    if (g_ledger_loaded) return;
    FILE* fp = fopen(TBOS_KARMA_LEDGER_PATH, "r");
    if (!fp) {
        g_ledger_loaded = 1;
        return;
    }
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char *persona, *total_str;
        strtok(line, "|"); /* timestamp */
        persona = strtok(NULL, "|");
        strtok(NULL, "|"); /* delta */
        total_str = strtok(NULL, "|");
        if (!persona || !total_str) {
            continue;
        }
        ledger_total_t* entry = find_total(persona, 1);
        if (!entry) continue;
        entry->total = (karma_score_t)strtoll(total_str, NULL, 10);
    }
    fclose(fp);
    g_ledger_loaded = 1;
}

void tbos_karma_ledger_init(void) {
    ledger_load();
}

int tbos_karma_ledger_get_total(const char* persona, karma_score_t* out_total) {
    ledger_total_t* entry = find_total(persona, 0);
    if (!entry || !out_total) {
        return -1;
    }
    *out_total = entry->total;
    return 0;
}

static void ledger_update_total(const char* persona, karma_score_t total) {
    ledger_total_t* entry = find_total(persona, 1);
    if (!entry) return;
    entry->total = total;
}

void tbos_karma_ledger_record(const char* persona,
                              karma_score_t delta,
                              karma_score_t new_total,
                              const char* reason) {
    ensure_artifacts_dir();
    FILE* fp = fopen(TBOS_KARMA_LEDGER_PATH, "a");
    if (!fp) {
        return;
    }
    time_t now = time(NULL);
    struct tm tm_now;
    gmtime_r(&now, &tm_now);
    char iso[32];
    strftime(iso, sizeof(iso), "%Y-%m-%dT%H:%M:%SZ", &tm_now);
    fprintf(fp, "%s|%s|%lld|%lld|%s\n",
            iso,
            persona ? persona : "default",
            (long long)delta,
            (long long)new_total,
            reason ? reason : "");
    fclose(fp);
    ledger_update_total(persona, new_total);
}

void tbos_karma_ledger_print(const char* persona, size_t max_entries) {
    FILE* fp = fopen(TBOS_KARMA_LEDGER_PATH, "r");
    if (!fp) {
        printf("No karma ledger entries yet.\n");
        return;
    }
    struct entry {
        char ts[32];
        char persona[64];
        karma_score_t delta;
        karma_score_t total;
        char reason[256];
    };
    struct entry buffer[64];
    if (max_entries > 64) max_entries = 64;
    size_t count = 0;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char *ts, *p, *delta_str, *total_str, *reason;
        ts = strtok(line, "|");
        p = strtok(NULL, "|");
        delta_str = strtok(NULL, "|");
        total_str = strtok(NULL, "|");
        reason = strtok(NULL, "\n");
        if (!ts || !p || !delta_str || !total_str) continue;
        if (persona && persona[0] != '\0' && strcmp(persona, p) != 0) continue;
        struct entry e;
        strncpy(e.ts, ts, sizeof(e.ts) - 1);
        e.ts[sizeof(e.ts) - 1] = '\0';
        strncpy(e.persona, p, sizeof(e.persona) - 1);
        e.persona[sizeof(e.persona) - 1] = '\0';
        e.delta = (karma_score_t)strtoll(delta_str, NULL, 10);
        e.total = (karma_score_t)strtoll(total_str, NULL, 10);
        if (reason) {
            strncpy(e.reason, reason, sizeof(e.reason) - 1);
            e.reason[sizeof(e.reason) - 1] = '\0';
        } else {
            e.reason[0] = '\0';
        }
        if (count < max_entries) {
            buffer[count++] = e;
        } else {
            memmove(buffer, buffer + 1, sizeof(struct entry) * (max_entries - 1));
            buffer[max_entries - 1] = e;
        }
    }
    fclose(fp);
    if (count == 0) {
        printf("No karma ledger entries for persona '%s'.\n",
               persona ? persona : "default");
        return;
    }
    printf("Karma ledger (last %zu entries for %s):\n",
           count,
           persona ? persona : "default");
    for (size_t i = 0; i < count; ++i) {
        printf("  %s | Δ=%lld | total=%lld | %s\n",
               buffer[i].ts,
               (long long)buffer[i].delta,
               (long long)buffer[i].total,
               buffer[i].reason);
    }
}
