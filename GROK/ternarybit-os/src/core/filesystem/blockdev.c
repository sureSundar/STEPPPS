#include "tbos/blockdev.h"
#include "tbos/libc.h"

#define MAX_BLOCKDEVS 8

typedef struct {
    tbos_blockdev_t dev;
    int in_use;
} blockdev_entry_t;

typedef struct {
    uint8_t* data;
    size_t block_size;
} ramdisk_ctx_t;

static blockdev_entry_t registry[MAX_BLOCKDEVS];

void blockdev_init(void) {
    memset(registry, 0, sizeof(registry));
}

const tbos_blockdev_t* blockdev_register(const char* name,
                                         size_t block_size,
                                         uint64_t block_count,
                                         const tbos_blockdev_ops_t* ops,
                                         void* ctx) {
    if (!name || !ops || block_size == 0 || block_count == 0) {
        return NULL;
    }

    for (size_t i = 0; i < MAX_BLOCKDEVS; i++) {
        if (registry[i].in_use && strcmp(registry[i].dev.name, name) == 0) {
            return &registry[i].dev;
        }
    }

    for (size_t i = 0; i < MAX_BLOCKDEVS; i++) {
        if (!registry[i].in_use) {
            registry[i].in_use = 1;
            tbos_blockdev_t* dev = &registry[i].dev;
            memset(dev, 0, sizeof(*dev));
            strncpy(dev->name, name, sizeof(dev->name) - 1);
            dev->block_size = block_size;
            dev->block_count = block_count;
            dev->ops = ops;
            dev->ctx = ctx;
            return dev;
        }
    }

    return NULL;
}

const tbos_blockdev_t* blockdev_find(const char* name) {
    if (!name) return NULL;
    for (size_t i = 0; i < MAX_BLOCKDEVS; i++) {
        if (registry[i].in_use && strcmp(registry[i].dev.name, name) == 0) {
            return &registry[i].dev;
        }
    }
    return NULL;
}

int blockdev_read(const tbos_blockdev_t* dev, uint64_t lba, void* buffer, size_t block_count) {
    if (!dev || !dev->ops || !dev->ops->read || !buffer) return -1;
    if (lba + block_count > dev->block_count) return -1;
    return dev->ops->read(dev->ctx, lba, buffer, block_count);
}

int blockdev_write(const tbos_blockdev_t* dev, uint64_t lba, const void* buffer, size_t block_count) {
    if (!dev || !dev->ops || !dev->ops->write || !buffer) return -1;
    if (lba + block_count > dev->block_count) return -1;
    return dev->ops->write(dev->ctx, lba, buffer, block_count);
}

int blockdev_flush(const tbos_blockdev_t* dev) {
    if (!dev || !dev->ops || !dev->ops->flush) return 0;
    return dev->ops->flush(dev->ctx);
}

static int ramdisk_read(void* ctx, uint64_t lba, void* buffer, size_t blocks) {
    ramdisk_ctx_t* rd = (ramdisk_ctx_t*)ctx;
    memcpy(buffer, rd->data + lba * rd->block_size, blocks * rd->block_size);
    return 0;
}

static int ramdisk_write(void* ctx, uint64_t lba, const void* buffer, size_t blocks) {
    ramdisk_ctx_t* rd = (ramdisk_ctx_t*)ctx;
    memcpy(rd->data + lba * rd->block_size, buffer, blocks * rd->block_size);
    return 0;
}

static const tbos_blockdev_ops_t ramdisk_ops = {
    .read = ramdisk_read,
    .write = ramdisk_write,
    .flush = NULL,
};

const tbos_blockdev_t* blockdev_create_ramdisk(const char* name,
                                               size_t block_size,
                                               uint64_t block_count) {
    ramdisk_ctx_t* ctx = malloc(sizeof(ramdisk_ctx_t));
    if (!ctx) return NULL;
    ctx->block_size = block_size;
    ctx->data = malloc(block_size * block_count);
    if (!ctx->data) {
        free(ctx);
        return NULL;
    }
    memset(ctx->data, 0, block_size * block_count);
    return blockdev_register(name, block_size, block_count, &ramdisk_ops, ctx);
}
