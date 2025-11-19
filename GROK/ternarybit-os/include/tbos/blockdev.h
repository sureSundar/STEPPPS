#ifndef TBOS_BLOCKDEV_H
#define TBOS_BLOCKDEV_H

#include <stddef.h>
#include <stdint.h>

typedef struct tbos_blockdev tbos_blockdev_t;

typedef struct {
    int (*read)(void* ctx, uint64_t lba, void* buffer, size_t block_count);
    int (*write)(void* ctx, uint64_t lba, const void* buffer, size_t block_count);
    int (*flush)(void* ctx);
} tbos_blockdev_ops_t;

struct tbos_blockdev {
    char name[32];
    size_t block_size;
    uint64_t block_count;
    const tbos_blockdev_ops_t* ops;
    void* ctx;
};

void blockdev_init(void);
const tbos_blockdev_t* blockdev_register(const char* name,
                                         size_t block_size,
                                         uint64_t block_count,
                                         const tbos_blockdev_ops_t* ops,
                                         void* ctx);
const tbos_blockdev_t* blockdev_find(const char* name);
int blockdev_read(const tbos_blockdev_t* dev, uint64_t lba, void* buffer, size_t block_count);
int blockdev_write(const tbos_blockdev_t* dev, uint64_t lba, const void* buffer, size_t block_count);
int blockdev_flush(const tbos_blockdev_t* dev);

/* Helper: create an in-memory RAM block device for testing */
const tbos_blockdev_t* blockdev_create_ramdisk(const char* name,
                                               size_t block_size,
                                               uint64_t block_count);

#endif /* TBOS_BLOCKDEV_H */
