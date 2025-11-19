#include "fs/ucfs_codec.h"
#include "tbos/errno.h"
#include "tbos/libc.h"

#include <stdlib.h>

typedef struct {
    uint32_t codepoint;
    size_t bytes;
} utf8_decode_t;

static utf8_decode_t decode_utf8(const char* s) {
    utf8_decode_t result = {0, 0};
    if (!s) return result;
    unsigned char c0 = (unsigned char)s[0];
    if (c0 < 0x80) {
        result.codepoint = c0;
        result.bytes = 1;
    } else if ((c0 & 0xE0u) == 0xC0u) {
        result.codepoint = ((uint32_t)(c0 & 0x1Fu) << 6) |
                           (uint32_t)(s[1] & 0x3Fu);
        result.bytes = 2;
    } else if ((c0 & 0xF0u) == 0xE0u) {
        result.codepoint = ((uint32_t)(c0 & 0x0Fu) << 12) |
                           ((uint32_t)(s[1] & 0x3Fu) << 6) |
                           (uint32_t)(s[2] & 0x3Fu);
        result.bytes = 3;
    } else if ((c0 & 0xF8u) == 0xF0u) {
        result.codepoint = ((uint32_t)(c0 & 0x07u) << 18) |
                           ((uint32_t)(s[1] & 0x3Fu) << 12) |
                           ((uint32_t)(s[2] & 0x3Fu) << 6) |
                           (uint32_t)(s[3] & 0x3Fu);
        result.bytes = 4;
    }
    return result;
}

static char* dup_bytes(const char* start, size_t len) {
    char* out = malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

static int matches_delimiter_sequence(const ucfs_path_t* path, const char* cursor) {
    if (!cursor || cursor[0] != '[') {
        return 0;
    }
    cursor++;
    for (size_t i = 0; i < path->delimiter_len; i++) {
        if (cursor[i] == '\0' || cursor[i] != path->delimiter_utf8[i]) {
            return 0;
        }
    }
    cursor += path->delimiter_len;
    if (*cursor != ']') {
        return 0;
    }
    return 1;
}

int ucfs_parse(const char* utf8_path, ucfs_path_t* out_path) {
    if (!utf8_path || !*utf8_path || !out_path) {
        return -EINVAL;
    }
    memset(out_path, 0, sizeof(*out_path));

    /* Check if path starts with a Unicode character (not ASCII / or \) */
    unsigned char first_byte = (unsigned char)utf8_path[0];

    /* If it's ASCII '/' or '\', it's a regular POSIX path, not UCFS */
    if (first_byte == '/' || first_byte == '\\' || first_byte < 0x80) {
        return -EINVAL;  /* Not a UCFS path */
    }

    /* Decode the first Unicode character - this is the root delimiter */
    utf8_decode_t first = decode_utf8(utf8_path);
    if (first.bytes == 0) {
        return -EINVAL;
    }

    out_path->delimiter = first.codepoint;
    out_path->delimiter_len = first.bytes;
    if (out_path->delimiter_len >= sizeof(out_path->delimiter_utf8)) {
        return -EINVAL;
    }
    /* Copy the Unicode root character */
    memcpy(out_path->delimiter_utf8, utf8_path, first.bytes);
    out_path->delimiter_utf8[first.bytes] = '\0';

    /* Move cursor past the Unicode root */
    const char* cursor = utf8_path + first.bytes;

    /* Expect '/' after the Unicode root (or end of string for just the root) */
    if (*cursor != '\0' && *cursor != '/') {
        return -EINVAL;  /* Invalid format - must be emoji/ or emoji */
    }

    /* Skip the '/' if present */
    if (*cursor == '/') {
        cursor++;
    }

    /* Now parse the rest as a standard POSIX-style path */
    const char* segment_start = cursor;

    size_t capacity = 4;
    out_path->components = malloc(sizeof(char*) * capacity);
    if (!out_path->components) {
        return -ENOMEM;
    }

    /* Parse path components separated by '/' */
    while (*cursor) {
        if (*cursor == '/') {
            size_t seg_len = (size_t)(cursor - segment_start);
            if (seg_len == 0) {
                /* Skip empty components (e.g., from //) */
                cursor++;
                segment_start = cursor;
                continue;
            }
            if (out_path->component_count == capacity) {
                capacity *= 2;
                char** resized = realloc(out_path->components, sizeof(char*) * capacity);
                if (!resized) {
                    ucfs_free(out_path);
                    return -ENOMEM;
                }
                out_path->components = resized;
            }
            char* dup = dup_bytes(segment_start, seg_len);
            if (!dup) {
                ucfs_free(out_path);
                return -ENOMEM;
            }
            out_path->components[out_path->component_count++] = dup;
            cursor++;  /* skip the '/' */
            segment_start = cursor;
            continue;
        }
        /* Not a separator, just advance */
        cursor++;
    }

    if (cursor != segment_start) {
        size_t seg_len = (size_t)(cursor - segment_start);
        char* dup = dup_bytes(segment_start, seg_len);
        if (!dup) {
            ucfs_free(out_path);
            return -ENOMEM;
        }
        if (out_path->component_count == capacity) {
            capacity *= 2;
            char** resized = realloc(out_path->components, sizeof(char*) * capacity);
            if (!resized) {
                free(dup);
                ucfs_free(out_path);
                return -ENOMEM;
            }
            out_path->components = resized;
        }
        out_path->components[out_path->component_count++] = dup;
    }

    return 0;
}

void ucfs_free(ucfs_path_t* path) {
    if (!path) return;
    if (path->components) {
        for (size_t i = 0; i < path->component_count; i++) {
            free(path->components[i]);
        }
        free(path->components);
    }
    memset(path, 0, sizeof(*path));
}

static int append_string(char* buffer, size_t buflen, size_t* offset, const char* text) {
    size_t len = strlen(text);
    if (*offset + len >= buflen) {
        return -ENOSPC;
    }
    memcpy(buffer + *offset, text, len);
    *offset += len;
    buffer[*offset] = '\0';
    return 0;
}

static int ucfs_append_base(const ucfs_path_t* path, const char* base, char* buffer, size_t buflen) {
    if (!path || !buffer || buflen == 0 || !base || base[0] != '/') {
        return -EINVAL;
    }
    size_t offset = 0;
    if (append_string(buffer, buflen, &offset, base) != 0) {
        return -ENOSPC;
    }
    if (buffer[offset - 1] != '/') {
        if (append_string(buffer, buflen, &offset, "/") != 0) {
            return -ENOSPC;
        }
    }
    char codepoint_buf[16];
    int written = snprintf(codepoint_buf, sizeof(codepoint_buf), "U+%04X", path->delimiter);
    if (written <= 0 || (size_t)written >= sizeof(codepoint_buf)) {
        return -EINVAL;
    }
    if (append_string(buffer, buflen, &offset, codepoint_buf) != 0) {
        return -ENOSPC;
    }
    for (size_t i = 0; i < path->component_count; i++) {
        if (append_string(buffer, buflen, &offset, "/") != 0) {
            return -ENOSPC;
        }
        if (append_string(buffer, buflen, &offset, path->components[i]) != 0) {
            return -ENOSPC;
        }
    }
    return 0;
}

int ucfs_to_canonical_with_base(const ucfs_path_t* path, const char* base, char* buffer, size_t buflen) {
    return ucfs_append_base(path, base, buffer, buflen);
}

int ucfs_to_canonical(const ucfs_path_t* path, char* buffer, size_t buflen) {
    if (!path || !buffer || buflen == 0) {
        return -EINVAL;
    }
    return ucfs_append_base(path, "/ucfs", buffer, buflen);
}
