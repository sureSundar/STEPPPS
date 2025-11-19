#ifndef TBOS_FS_UCFS_CODEC_H
#define TBOS_FS_UCFS_CODEC_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t delimiter;        /* Base Unicode code point */
    char delimiter_utf8[8];    /* UTF-8 bytes for delimiter (+ variation selectors) */
    size_t delimiter_len;
    size_t component_count;
    char** components;         /* UTF-8 strings */
} ucfs_path_t;

int  ucfs_parse(const char* utf8_path, ucfs_path_t* out_path);
void ucfs_free(ucfs_path_t* path);
int  ucfs_to_canonical(const ucfs_path_t* path, char* buffer, size_t buflen);
int  ucfs_to_canonical_with_base(const ucfs_path_t* path, const char* base, char* buffer, size_t buflen);

#endif /* TBOS_FS_UCFS_CODEC_H */
