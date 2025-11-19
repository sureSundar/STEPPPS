#include "fs/ucfs_overlay.h"
#include "fs/ucfs_codec.h"
#include "tbos/vfs.h"
#include "tbos/errno.h"
#include "tbos/libc.h"

#include <stdlib.h>

static char g_backing_root[128] = "/ucfs";

int ucfs_set_backing_root(const char* path) {
    if (!path || path[0] != '/' || strlen(path) >= sizeof(g_backing_root)) {
        return -EINVAL;
    }
    strcpy(g_backing_root, path);
    return 0;
}

static int ensure_directories(const char* canonical_path) {
    char temp[256];
    size_t len = strlen(canonical_path);
    if (len >= sizeof(temp)) return -ENOSPC;
    strcpy(temp, canonical_path);
    for (char* p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            vfs_mkdir(temp);
            *p = '/';
        }
    }
    return 0;
}

static int ucfs_build_canonical(const char* ucfs_path, char* buffer, size_t buflen) {
    ucfs_path_t parsed;
    int rc = ucfs_parse(ucfs_path, &parsed);
    if (rc != 0) {
        return rc;
    }
    rc = ucfs_to_canonical_with_base(&parsed, g_backing_root, buffer, buflen);
    ucfs_free(&parsed);
    return rc;
}

int ucfs_write_file_uc(const char* ucfs_path, const void* data, size_t size) {
    if (!ucfs_path || !data) return -EINVAL;
    char canonical[512];
    int rc = ucfs_build_canonical(ucfs_path, canonical, sizeof(canonical));
    if (rc != 0) return rc;
    ensure_directories(canonical);
    return vfs_write_file(canonical, data, size);
}

int ucfs_read_file_uc(const char* ucfs_path, void* buffer, size_t max_size, size_t* out_size) {
    if (!ucfs_path || !buffer) return -EINVAL;
    char canonical[512];
    int rc = ucfs_build_canonical(ucfs_path, canonical, sizeof(canonical));
    if (rc != 0) return rc;
    return vfs_read_file(canonical, buffer, max_size, out_size);
}

int ucfs_exists_uc(const char* ucfs_path) {
    if (!ucfs_path) return 0;
    char canonical[512];
    if (ucfs_build_canonical(ucfs_path, canonical, sizeof(canonical)) != 0) {
        return 0;
    }
    return vfs_exists(canonical);
}

int ucfs_resolve_path(const char* ucfs_path, char* buffer, size_t buflen) {
    return ucfs_build_canonical(ucfs_path, buffer, buflen);
}

static size_t utf8_encode(uint32_t cp, char* out) {
    if (cp <= 0x7F) {
        out[0] = (char)cp;
        return 1;
    } else if (cp <= 0x7FF) {
        out[0] = (char)(0xC0 | ((cp >> 6) & 0x1F));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    } else if (cp <= 0xFFFF) {
        out[0] = (char)(0xE0 | ((cp >> 12) & 0x0F));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    } else {
        out[0] = (char)(0xF0 | ((cp >> 18) & 0x07));
        out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[3] = (char)(0x80 | (cp & 0x3F));
        return 4;
    }
}

const char* ucfs_prompt_form(const char* canonical_path, char* buffer, size_t buflen) {
    if (!canonical_path || !buffer || buflen == 0) return canonical_path;
    const char* prefix = g_backing_root;
    size_t prefix_len = strlen(prefix);
    if (strncmp(canonical_path, prefix, prefix_len) != 0) {
        strncpy(buffer, canonical_path, buflen - 1);
        buffer[buflen - 1] = '\0';
        return buffer;
    }
    const char* delimiter = canonical_path + prefix_len;
    if (*delimiter == '/') delimiter++;
    if (strncmp(delimiter, "U+", 2) != 0) {
        strncpy(buffer, canonical_path, buflen - 1);
        buffer[buflen - 1] = '\0';
        return buffer;
    }
    const char* slash = strchr(delimiter, '/');
    size_t hex_len = slash ? (size_t)(slash - (delimiter + 2)) : strlen(delimiter + 2);
    if (hex_len == 0 || hex_len >= 9) {
        strncpy(buffer, canonical_path, buflen - 1);
        buffer[buflen - 1] = '\0';
        return buffer;
    }
    char hex[9];
    memcpy(hex, delimiter + 2, hex_len);
    hex[hex_len] = '\0';
    uint32_t codepoint = (uint32_t)strtoul(hex, NULL, 16);
    char utf8[5] = {0};
    size_t len = utf8_encode(codepoint, utf8);
    utf8[len] = '\0';
    if (!slash) {
        snprintf(buffer, buflen, "[%s]", utf8);
    } else {
        snprintf(buffer, buflen, "[%s]%s", utf8, slash + 1);
    }
    return buffer;
}
