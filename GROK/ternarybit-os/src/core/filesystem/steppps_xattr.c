/**
 * @file steppps_xattr.c
 * @brief STEPPPS Extended Attributes Implementation
 *
 * Platform-native storage of STEPPPS 7 dimensions for FUSE filesystems.
 *
 * @version 1.0
 * @date 2026-07-16
 */

#include "steppps_xattr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

void steppps_get_timestamp(char* buf, size_t size) {
    time_t now = time(NULL);
    struct tm* tm = gmtime(&now);
    strftime(buf, size, "%Y-%m-%dT%H:%M:%SZ", tm);
}

static char* json_escape_string(const char* str) {
    if (!str) return strdup("");

    size_t len = strlen(str);
    char* escaped = malloc(len * 2 + 1);
    if (!escaped) return NULL;

    char* p = escaped;
    for (size_t i = 0; i < len; i++) {
        switch (str[i]) {
            case '"':  *p++ = '\\'; *p++ = '"'; break;
            case '\\': *p++ = '\\'; *p++ = '\\'; break;
            case '\n': *p++ = '\\'; *p++ = 'n'; break;
            case '\r': *p++ = '\\'; *p++ = 'r'; break;
            case '\t': *p++ = '\\'; *p++ = 't'; break;
            default:   *p++ = str[i]; break;
        }
    }
    *p = '\0';
    return escaped;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * LIFECYCLE
 * ═══════════════════════════════════════════════════════════════════════════ */

void steppps_xattr_init(steppps_xattr_t* xattr) {
    if (!xattr) return;
    memset(xattr, 0, sizeof(steppps_xattr_t));
}

void steppps_xattr_free(steppps_xattr_t* xattr) {
    if (!xattr) return;
    free(xattr->space);
    free(xattr->time);
    free(xattr->event);
    free(xattr->psychology);
    free(xattr->pixel);
    free(xattr->prompt);
    free(xattr->script);
    free(xattr->meta);
    memset(xattr, 0, sizeof(steppps_xattr_t));
}

/* ═══════════════════════════════════════════════════════════════════════════
 * AUTO-POPULATE ON CREATION
 * ═══════════════════════════════════════════════════════════════════════════ */

int steppps_xattr_create(steppps_xattr_t* xattr, const steppps_create_ctx_t* ctx) {
    if (!xattr || !ctx) return -EINVAL;

    steppps_xattr_init(xattr);

    char timestamp[32];
    steppps_get_timestamp(timestamp, sizeof(timestamp));

    char buf[STEPPPS_XATTR_MAX_VALUE];
    char* escaped_path = json_escape_string(ctx->path);

    /* S_space: Where the entity exists */
    snprintf(buf, sizeof(buf),
        "{"
        "\"realm\":\"%s\","
        "\"filesystem\":\"%s\","
        "\"path\":\"%s\""
        "%s%s%s"
        "%s%s%s"
        "}",
        ctx->realm ? ctx->realm : "bhulok",
        ctx->filesystem ? ctx->filesystem : "pxfs",
        escaped_path ? escaped_path : "",
        ctx->sangha ? ",\"sangha\":\"" : "",
        ctx->sangha ? ctx->sangha : "",
        ctx->sangha ? "\"" : "",
        ctx->nation ? ",\"nation\":\"" : "",
        ctx->nation ? ctx->nation : "",
        ctx->nation ? "\"" : ""
    );
    xattr->space = strdup(buf);
    free(escaped_path);

    /* T_time: When the entity exists */
    snprintf(buf, sizeof(buf),
        "{"
        "\"created\":\"%s\","
        "\"modified\":\"%s\","
        "\"accessed\":\"%s\""
        "}",
        timestamp, timestamp, timestamp
    );
    xattr->time = strdup(buf);

    /* E_event: What happened to the entity */
    snprintf(buf, sizeof(buf),
        "{"
        "\"creation_event\":\"steppps://event/create-%ld\","
        "\"last_modifier\":\"%s\""
        "}",
        (long)time(NULL),
        ctx->owner ? ctx->owner : "steppps://human/unknown"
    );
    xattr->event = strdup(buf);

    /* P_psychology: The entity's consciousness state */
    snprintf(buf, sizeof(buf),
        "{"
        "\"consciousness_level\":\"dormant\","
        "\"karma\":0,"
        "\"purpose\":\"%s\""
        "}",
        ctx->is_directory ? "container" : "data_storage"
    );
    xattr->psychology = strdup(buf);

    /* P_pixel: How the entity appears */
    if (ctx->is_directory) {
        snprintf(buf, sizeof(buf),
            "{"
            "\"form\":\"directory\","
            "\"child_count\":0,"
            "\"total_size_bytes\":0,"
            "\"display_logic\":\"Render as folder icon\""
            "}"
        );
    } else {
        snprintf(buf, sizeof(buf),
            "{"
            "\"form\":\"file\","
            "\"mime_type\":\"%s\","
            "\"size_bytes\":%zu,"
            "\"display_logic\":\"Render based on mime type\""
            "}",
            ctx->mime_type ? ctx->mime_type : "application/octet-stream",
            ctx->size
        );
    }
    xattr->pixel = strdup(buf);

    /* P_prompt: The entity's voice/intent */
    char* escaped_prompt = json_escape_string(ctx->creation_prompt);
    snprintf(buf, sizeof(buf),
        "{"
        "\"creation_prompt\":\"%s\","
        "\"next_genai_prompt\":\"This %s may be accessed by authorized entities\""
        "}",
        escaped_prompt ? escaped_prompt : "Created by user",
        ctx->is_directory ? "directory" : "file"
    );
    xattr->prompt = strdup(buf);
    free(escaped_prompt);

    /* S_script: What the entity can do */
    snprintf(buf, sizeof(buf),
        "{"
        "\"acts_on\":[\"space\",\"time\",\"event\"],"
        "\"permissions\":{"
        "\"read\":[\"%s\"],"
        "\"write\":[\"%s\"],"
        "\"execute\":%s"
        "},"
        "\"intents\":[\"%s\"]"
        "}",
        ctx->owner ? ctx->owner : "steppps://human/unknown",
        ctx->owner ? ctx->owner : "steppps://human/unknown",
        ctx->is_directory ? "[\"steppps://human/unknown\"]" : "[]",
        ctx->is_directory ? "container" : "data_preservation"
    );
    xattr->script = strdup(buf);

    /* meta: Governance chain */
    snprintf(buf, sizeof(buf),
        "{"
        "\"owner\":\"%s\","
        "\"authority\":\"steppps://authority/owner\","
        "\"nation\":\"%s\""
        "%s%s%s"
        "}",
        ctx->owner ? ctx->owner : "steppps://human/unknown",
        ctx->nation ? ctx->nation : "steppps://nation/bharat",
        ctx->sangha ? ",\"contract\":\"" : "",
        ctx->sangha ? ctx->sangha : "",
        ctx->sangha ? "-contract\"" : ""
    );
    xattr->meta = strdup(buf);

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * GET / SET / LIST / REMOVE
 * ═══════════════════════════════════════════════════════════════════════════ */

bool steppps_xattr_is_steppps(const char* name) {
    return name && strncmp(name, STEPPPS_XATTR_PREFIX,
                           strlen(STEPPPS_XATTR_PREFIX)) == 0;
}

static char** steppps_xattr_get_ptr(steppps_xattr_t* xattr, const char* name) {
    if (strcmp(name, STEPPPS_XATTR_SPACE) == 0) return &xattr->space;
    if (strcmp(name, STEPPPS_XATTR_TIME) == 0) return &xattr->time;
    if (strcmp(name, STEPPPS_XATTR_EVENT) == 0) return &xattr->event;
    if (strcmp(name, STEPPPS_XATTR_PSYCHOLOGY) == 0) return &xattr->psychology;
    if (strcmp(name, STEPPPS_XATTR_PIXEL) == 0) return &xattr->pixel;
    if (strcmp(name, STEPPPS_XATTR_PROMPT) == 0) return &xattr->prompt;
    if (strcmp(name, STEPPPS_XATTR_SCRIPT) == 0) return &xattr->script;
    if (strcmp(name, STEPPPS_XATTR_META) == 0) return &xattr->meta;
    return NULL;
}

ssize_t steppps_xattr_get(const steppps_xattr_t* xattr, const char* name,
                          char* value, size_t size) {
    if (!xattr || !name) return -EINVAL;

    char** ptr = steppps_xattr_get_ptr((steppps_xattr_t*)xattr, name);
    if (!ptr) return -ENODATA;

    const char* val = *ptr;
    if (!val) return -ENODATA;

    size_t len = strlen(val);

    if (size == 0) {
        /* Query size only */
        return len;
    }

    if (len > size) {
        return -ERANGE;
    }

    memcpy(value, val, len);
    return len;
}

int steppps_xattr_set(steppps_xattr_t* xattr, const char* name,
                      const char* value, size_t size, int flags) {
    if (!xattr || !name) return -EINVAL;
    if (size > STEPPPS_XATTR_MAX_VALUE) return -E2BIG;

    char** ptr = steppps_xattr_get_ptr(xattr, name);
    if (!ptr) {
        /* Not a known STEPPPS dimension */
        return -ENODATA;
    }

    /* Check flags */
    bool exists = (*ptr != NULL);

    /* XATTR_CREATE = 1, XATTR_REPLACE = 2 on Linux */
    if ((flags & 1) && exists) {
        return -EEXIST;
    }
    if ((flags & 2) && !exists) {
        return -ENODATA;
    }

    /* Set the value */
    free(*ptr);
    if (value && size > 0) {
        *ptr = malloc(size + 1);
        if (!*ptr) return -ENOMEM;
        memcpy(*ptr, value, size);
        (*ptr)[size] = '\0';
    } else {
        *ptr = NULL;
    }

    return 0;
}

ssize_t steppps_xattr_list(const steppps_xattr_t* xattr, char* list, size_t size) {
    if (!xattr) return -EINVAL;

    static const char* names[] = {
        STEPPPS_XATTR_SPACE,
        STEPPPS_XATTR_TIME,
        STEPPPS_XATTR_EVENT,
        STEPPPS_XATTR_PSYCHOLOGY,
        STEPPPS_XATTR_PIXEL,
        STEPPPS_XATTR_PROMPT,
        STEPPPS_XATTR_SCRIPT,
        STEPPPS_XATTR_META,
        NULL
    };

    const char* const* values[] = {
        (const char**)&xattr->space,
        (const char**)&xattr->time,
        (const char**)&xattr->event,
        (const char**)&xattr->psychology,
        (const char**)&xattr->pixel,
        (const char**)&xattr->prompt,
        (const char**)&xattr->script,
        (const char**)&xattr->meta,
        NULL
    };

    size_t total = 0;
    size_t pos = 0;

    for (int i = 0; names[i]; i++) {
        if (*values[i]) {
            size_t len = strlen(names[i]) + 1;  /* Include null terminator */
            total += len;

            if (list && pos + len <= size) {
                memcpy(list + pos, names[i], len);
                pos += len;
            }
        }
    }

    if (size == 0) {
        return total;
    }

    if (total > size) {
        return -ERANGE;
    }

    return total;
}

int steppps_xattr_remove(steppps_xattr_t* xattr, const char* name) {
    if (!xattr || !name) return -EINVAL;

    char** ptr = steppps_xattr_get_ptr(xattr, name);
    if (!ptr || !*ptr) return -ENODATA;

    free(*ptr);
    *ptr = NULL;

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TIME UPDATES
 * ═══════════════════════════════════════════════════════════════════════════ */

void steppps_xattr_update_mtime(steppps_xattr_t* xattr) {
    if (!xattr || !xattr->time) return;

    char timestamp[32];
    steppps_get_timestamp(timestamp, sizeof(timestamp));

    /* Simple update - in production, parse JSON properly */
    char buf[STEPPPS_XATTR_MAX_VALUE];
    snprintf(buf, sizeof(buf),
        "{"
        "\"created\":\"%s\","
        "\"modified\":\"%s\","
        "\"accessed\":\"%s\""
        "}",
        /* Extract created from existing - simplified */
        timestamp, timestamp, timestamp
    );

    free(xattr->time);
    xattr->time = strdup(buf);
}

void steppps_xattr_update_atime(steppps_xattr_t* xattr) {
    /* Similar to mtime - simplified for now */
    steppps_xattr_update_mtime(xattr);
}
