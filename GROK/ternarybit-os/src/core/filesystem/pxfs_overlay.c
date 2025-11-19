#include "fs/pxfs_overlay.h"
#include "fs/pxfs_codec.h"
#include "tbos/vfs.h"
#include "tbos/errno.h"
#include "tbos/libc.h"

#include <stdlib.h>
#include <stdio.h>

static char g_pxfs_root[128] = "/pxfs";

int pxfs_set_backing_root(const char* path) {
    if (!path || path[0] != '/' || strlen(path) >= sizeof(g_pxfs_root)) {
        return -EINVAL;
    }
    strcpy(g_pxfs_root, path);
    return 0;
}

static int ensure_dirs(const char* canonical) {
    char temp[512];
    size_t len = strlen(canonical);
    if (len >= sizeof(temp)) return -ENOSPC;
    strcpy(temp, canonical);
    for (char* p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            vfs_mkdir(temp);
            *p = '/';
        }
    }
    return 0;
}

static int pxfs_build_canonical(const char* pxfs_path, char* buffer, size_t buflen) {
    pxfs_path_t parsed;
    int rc = pxfs_parse(pxfs_path, &parsed);
    if (rc != 0) {
        return rc;
    }
    rc = pxfs_to_canonical_with_base(&parsed, g_pxfs_root, buffer, buflen);
    pxfs_free(&parsed);
    return rc;
}

int pxfs_resolve_path(const char* pxfs_path, char* buffer, size_t buflen) {
    return pxfs_build_canonical(pxfs_path, buffer, buflen);
}

const char* pxfs_prompt_form(const char* canonical_path, char* buffer, size_t buflen) {
    if (!canonical_path || !buffer || buflen == 0) return canonical_path;
    const char* prefix = g_pxfs_root;
    size_t prefix_len = strlen(prefix);
    if (strncmp(canonical_path, prefix, prefix_len) != 0) {
        strncpy(buffer, canonical_path, buflen - 1);
        buffer[buflen - 1] = '\0';
        return buffer;
    }
    const char* pixel = canonical_path + prefix_len;
    if (*pixel == '/') pixel++;
    if (strlen(pixel) < 6) {
        strncpy(buffer, canonical_path, buflen - 1);
        buffer[buflen - 1] = '\0';
        return buffer;
    }
    char hex[7];
    memcpy(hex, pixel, 6);
    hex[6] = '\0';
    unsigned int r, g, b;
    if (sscanf(hex, "%02x%02x%02x", &r, &g, &b) != 3) {
        strncpy(buffer, canonical_path, buflen - 1);
        buffer[buflen - 1] = '\0';
        return buffer;
    }
    const char* slash = strchr(pixel, '/');
    if (!slash) {
        snprintf(buffer, buflen, "{%u,%u,%u}", r, g, b);
    } else {
        snprintf(buffer, buflen, "{%u,%u,%u}%s", r, g, b, slash + 1);
    }
    return buffer;
}

int pxfs_write_file(const char* pxfs_path, const void* data, size_t size) {
    if (!pxfs_path || !data) return -EINVAL;
    char canonical[512];
    int rc = pxfs_build_canonical(pxfs_path, canonical, sizeof(canonical));
    if (rc != 0) return rc;
    ensure_dirs(canonical);
    return vfs_write_file(canonical, data, size);
}

int pxfs_read_file(const char* pxfs_path, void* buffer, size_t max_size, size_t* out_size) {
    if (!pxfs_path || !buffer) return -EINVAL;
    char canonical[512];
    int rc = pxfs_build_canonical(pxfs_path, canonical, sizeof(canonical));
    if (rc != 0) return rc;
    return vfs_read_file(canonical, buffer, max_size, out_size);
}

int pxfs_exists(const char* pxfs_path) {
    if (!pxfs_path) return 0;
    char canonical[512];
    if (pxfs_build_canonical(pxfs_path, canonical, sizeof(canonical)) != 0) {
        return 0;
    }
    return vfs_exists(canonical);
}
