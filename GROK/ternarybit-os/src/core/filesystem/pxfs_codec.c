#include "fs/pxfs_codec.h"
#include "tbos/errno.h"
#include "tbos/libc.h"

#include <stdlib.h>
#include <stdio.h>

static int parse_pixel(const char* text, uint8_t* r, uint8_t* g, uint8_t* b, size_t* consumed) {
    if (!text || text[0] != '{') return -EINVAL;
    int ri, gi, bi;
    char closing;
    if (sscanf(text, "{%d,%d,%d%c", &ri, &gi, &bi, &closing) != 4 || closing != '}') {
        return -EINVAL;
    }
    if (ri < 0 || ri > 255 || gi < 0 || gi > 255 || bi < 0 || bi > 255) {
        return -EINVAL;
    }
    *r = (uint8_t)ri;
    *g = (uint8_t)gi;
    *b = (uint8_t)bi;
    const char* ptr = strchr(text, '}');
    if (!ptr) return -EINVAL;
    *consumed = (size_t)(ptr - text + 1);
    return 0;
}

int pxfs_parse(const char* path, pxfs_path_t* out) {
    if (!path || !*path || !out) return -EINVAL;
    memset(out, 0, sizeof(*out));

    size_t consumed = 0;
    if (parse_pixel(path, &out->r, &out->g, &out->b, &consumed) != 0) {
        return -EINVAL;
    }

    const char* cursor = path + consumed;
    if (!*cursor) return -EINVAL; /* need at least one component */

    size_t capacity = 4;
    out->components = malloc(sizeof(char*) * capacity);
    if (!out->components) return -ENOMEM;

    const char* segment_start = cursor;
    char pixel_token[32];
    snprintf(pixel_token, sizeof(pixel_token), "{%u,%u,%u}", out->r, out->g, out->b);
    size_t token_len = strlen(pixel_token);

    while (*cursor) {
        if (strncmp(cursor, pixel_token, token_len) == 0) {
            size_t seg_len = (size_t)(cursor - segment_start);
            if (seg_len == 0) {
                pxfs_free(out);
                return -EINVAL;
            }
            if (out->component_count == capacity) {
                capacity *= 2;
                char** resized = realloc(out->components, sizeof(char*) * capacity);
                if (!resized) {
                    pxfs_free(out);
                    return -ENOMEM;
                }
                out->components = resized;
            }
            char* dup = malloc(seg_len + 1);
            if (!dup) {
                pxfs_free(out);
                return -ENOMEM;
            }
            memcpy(dup, segment_start, seg_len);
            dup[seg_len] = '\0';
            out->components[out->component_count++] = dup;
            cursor += token_len;
            segment_start = cursor;
            continue;
        }
        cursor++;
    }

    if (cursor != segment_start) {
        size_t seg_len = (size_t)(cursor - segment_start);
        char* dup = malloc(seg_len + 1);
        if (!dup) {
            pxfs_free(out);
            return -ENOMEM;
        }
        memcpy(dup, segment_start, seg_len);
        dup[seg_len] = '\0';
        if (out->component_count == capacity) {
            capacity *= 2;
            char** resized = realloc(out->components, sizeof(char*) * capacity);
            if (!resized) {
                free(dup);
                pxfs_free(out);
                return -ENOMEM;
            }
            out->components = resized;
        }
        out->components[out->component_count++] = dup;
    }
    return 0;
}

void pxfs_free(pxfs_path_t* path) {
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

static int pxfs_append_base(const pxfs_path_t* path, const char* base, char* buffer, size_t buflen) {
    if (!path || !buffer || buflen == 0 || !base || base[0] != '/') {
        return -EINVAL;
    }
    size_t offset = 0;
    if (append_string(buffer, buflen, &offset, base) != 0) return -ENOSPC;
    if (buffer[offset - 1] != '/') {
        if (append_string(buffer, buflen, &offset, "/") != 0) return -ENOSPC;
    }
    char pixel_id[8];
    snprintf(pixel_id, sizeof(pixel_id), "%02X%02X%02X", path->r, path->g, path->b);
    if (append_string(buffer, buflen, &offset, pixel_id) != 0) return -ENOSPC;
    for (size_t i = 0; i < path->component_count; i++) {
        if (append_string(buffer, buflen, &offset, "/") != 0) return -ENOSPC;
        if (append_string(buffer, buflen, &offset, path->components[i]) != 0) return -ENOSPC;
    }
    return 0;
}

int pxfs_to_canonical(const pxfs_path_t* path, char* buffer, size_t buflen) {
    return pxfs_append_base(path, "/pxfs", buffer, buflen);
}

int pxfs_to_canonical_with_base(const pxfs_path_t* path, const char* base, char* buffer, size_t buflen) {
    return pxfs_append_base(path, base, buffer, buflen);
}
