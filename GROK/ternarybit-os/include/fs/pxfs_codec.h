#ifndef TBOS_FS_PXFS_CODEC_H
#define TBOS_FS_PXFS_CODEC_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t r, g, b;
    size_t component_count;
    char** components;
} pxfs_path_t;

int  pxfs_parse(const char* path, pxfs_path_t* out);
void pxfs_free(pxfs_path_t* path);
int  pxfs_to_canonical(const pxfs_path_t* path, char* buffer, size_t buflen);
int  pxfs_to_canonical_with_base(const pxfs_path_t* path, const char* base, char* buffer, size_t buflen);

#endif /* TBOS_FS_PXFS_CODEC_H */
