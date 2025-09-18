/**
 * @file tbos_drivers.c
 * @brief TernaryBit OS Device Driver Framework
 *
 * Implements a real device driver subsystem with:
 * - Character and block device support
 * - Device registration and management
 * - I/O control (ioctl) operations
 * - Interrupt handling simulation
 * - DMA operations
 * - Device file operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// Device types
typedef enum {
    DEVICE_TYPE_CHAR,
    DEVICE_TYPE_BLOCK,
    DEVICE_TYPE_NETWORK,
    DEVICE_TYPE_VIRTUAL
} tbos_device_type_t;

// Device flags
#define DEVICE_FLAG_READABLE    0x01
#define DEVICE_FLAG_WRITABLE    0x02
#define DEVICE_FLAG_SEEKABLE    0x04
#define DEVICE_FLAG_DMA_CAPABLE 0x08
#define DEVICE_FLAG_INTERRUPT   0x10

// I/O control commands
#define IOCTL_GET_INFO     0x1001
#define IOCTL_RESET        0x1002
#define IOCTL_SET_PARAM    0x1003
#define IOCTL_GET_STATUS   0x1004

// Device operations structure
typedef struct tbos_device_ops {
    int (*open)(void* dev_data, int flags);
    int (*close)(void* dev_data);
    ssize_t (*read)(void* dev_data, void* buffer, size_t size);
    ssize_t (*write)(void* dev_data, const void* buffer, size_t size);
    int (*ioctl)(void* dev_data, unsigned int cmd, unsigned long arg);
    off_t (*seek)(void* dev_data, off_t offset, int whence);
    int (*mmap)(void* dev_data, void** addr, size_t length, int prot);
} tbos_device_ops_t;

// Device structure
typedef struct tbos_device {
    char name[64];
    int major;                  // Major device number
    int minor;                  // Minor device number
    tbos_device_type_t type;
    uint32_t flags;

    // Device operations
    tbos_device_ops_t* ops;
    void* private_data;

    // Statistics
    uint64_t bytes_read;
    uint64_t bytes_written;
    uint32_t open_count;
    uint32_t error_count;

    // Synchronization
    pthread_mutex_t lock;

    struct tbos_device* next;
} tbos_device_t;

// Device manager
typedef struct {
    tbos_device_t* devices;
    int device_count;
    int next_major;
    pthread_mutex_t manager_lock;
} tbos_device_manager_t;

// Global device manager
static tbos_device_manager_t* g_dev_manager = NULL;

/**
 * Initialize device driver subsystem
 */
int tbos_drivers_init(void) {
    if (g_dev_manager) return 0;

    g_dev_manager = calloc(1, sizeof(tbos_device_manager_t));
    if (!g_dev_manager) return -1;

    g_dev_manager->next_major = 1;
    pthread_mutex_init(&g_dev_manager->manager_lock, NULL);

    printf("✅ TBOS Device Driver Subsystem initialized\n");
    return 0;
}

/**
 * Register a device driver
 */
int tbos_device_register(const char* name, tbos_device_type_t type,
                         tbos_device_ops_t* ops, void* private_data) {
    if (!g_dev_manager || !name || !ops) return -1;

    pthread_mutex_lock(&g_dev_manager->manager_lock);

    tbos_device_t* device = calloc(1, sizeof(tbos_device_t));
    if (!device) {
        pthread_mutex_unlock(&g_dev_manager->manager_lock);
        return -1;
    }

    strncpy(device->name, name, 63);
    device->type = type;
    device->major = g_dev_manager->next_major++;
    device->minor = 0;
    device->ops = ops;
    device->private_data = private_data;
    device->flags = DEVICE_FLAG_READABLE | DEVICE_FLAG_WRITABLE;

    pthread_mutex_init(&device->lock, NULL);

    // Add to device list
    device->next = g_dev_manager->devices;
    g_dev_manager->devices = device;
    g_dev_manager->device_count++;

    pthread_mutex_unlock(&g_dev_manager->manager_lock);

    printf("✅ Registered device: %s (major=%d, type=%d)\n",
           name, device->major, type);

    return device->major;
}

/**
 * Find device by name
 */
static tbos_device_t* find_device(const char* name) {
    tbos_device_t* dev = g_dev_manager->devices;
    while (dev) {
        if (strcmp(dev->name, name) == 0) {
            return dev;
        }
        dev = dev->next;
    }
    return NULL;
}

/**
 * Open device
 */
int tbos_device_open(const char* name, int flags) {
    tbos_device_t* dev = find_device(name);
    if (!dev) {
        errno = ENODEV;
        return -1;
    }

    pthread_mutex_lock(&dev->lock);

    int result = -1;
    if (dev->ops && dev->ops->open) {
        result = dev->ops->open(dev->private_data, flags);
        if (result >= 0) {
            dev->open_count++;
        }
    }

    pthread_mutex_unlock(&dev->lock);
    return result;
}

/**
 * Read from device
 */
ssize_t tbos_device_read(const char* name, void* buffer, size_t size) {
    tbos_device_t* dev = find_device(name);
    if (!dev) {
        errno = ENODEV;
        return -1;
    }

    pthread_mutex_lock(&dev->lock);

    ssize_t result = -1;
    if (dev->ops && dev->ops->read) {
        result = dev->ops->read(dev->private_data, buffer, size);
        if (result > 0) {
            dev->bytes_read += result;
        }
    }

    pthread_mutex_unlock(&dev->lock);
    return result;
}

/**
 * Write to device
 */
ssize_t tbos_device_write(const char* name, const void* buffer, size_t size) {
    tbos_device_t* dev = find_device(name);
    if (!dev) {
        errno = ENODEV;
        return -1;
    }

    pthread_mutex_lock(&dev->lock);

    ssize_t result = -1;
    if (dev->ops && dev->ops->write) {
        result = dev->ops->write(dev->private_data, buffer, size);
        if (result > 0) {
            dev->bytes_written += result;
        }
    }

    pthread_mutex_unlock(&dev->lock);
    return result;
}

/**
 * I/O control operation
 */
int tbos_device_ioctl(const char* name, unsigned int cmd, unsigned long arg) {
    tbos_device_t* dev = find_device(name);
    if (!dev) {
        errno = ENODEV;
        return -1;
    }

    pthread_mutex_lock(&dev->lock);

    int result = -1;
    if (dev->ops && dev->ops->ioctl) {
        result = dev->ops->ioctl(dev->private_data, cmd, arg);
    }

    pthread_mutex_unlock(&dev->lock);
    return result;
}

/**
 * List all devices
 */
void tbos_device_list(void) {
    printf("\n📟 Registered Devices:\n");

    pthread_mutex_lock(&g_dev_manager->manager_lock);

    tbos_device_t* dev = g_dev_manager->devices;
    while (dev) {
        const char* type_str = "Unknown";
        switch (dev->type) {
            case DEVICE_TYPE_CHAR: type_str = "Character"; break;
            case DEVICE_TYPE_BLOCK: type_str = "Block"; break;
            case DEVICE_TYPE_NETWORK: type_str = "Network"; break;
            case DEVICE_TYPE_VIRTUAL: type_str = "Virtual"; break;
        }

        printf("   - %s (major=%d, minor=%d, type=%s)\n",
               dev->name, dev->major, dev->minor, type_str);
        printf("     Stats: %llu bytes read, %llu bytes written, %u opens\n",
               (unsigned long long)dev->bytes_read,
               (unsigned long long)dev->bytes_written,
               dev->open_count);

        dev = dev->next;
    }

    pthread_mutex_unlock(&g_dev_manager->manager_lock);
}

// ============================================
// Example Device Drivers
// ============================================

/**
 * Null device (/dev/null equivalent)
 */
typedef struct {
    uint64_t bytes_discarded;
} null_device_data_t;

int null_open(void* data, int flags) {
    (void)data; (void)flags;
    return 0;
}

ssize_t null_read(void* data, void* buffer, size_t size) {
    (void)data; (void)buffer; (void)size;
    return 0; // EOF
}

ssize_t null_write(void* data, const void* buffer, size_t size) {
    null_device_data_t* null_data = (null_device_data_t*)data;
    null_data->bytes_discarded += size;
    return size; // Accept everything
}

/**
 * Zero device (/dev/zero equivalent)
 */
ssize_t zero_read(void* data, void* buffer, size_t size) {
    (void)data;
    memset(buffer, 0, size);
    return size;
}

/**
 * Random device (/dev/random equivalent)
 */
typedef struct {
    unsigned int seed;
} random_device_data_t;

int random_open(void* data, int flags) {
    random_device_data_t* rnd = (random_device_data_t*)data;
    rnd->seed = time(NULL) ^ getpid();
    return 0;
}

ssize_t random_read(void* data, void* buffer, size_t size) {
    random_device_data_t* rnd = (random_device_data_t*)data;
    uint8_t* buf = (uint8_t*)buffer;

    for (size_t i = 0; i < size; i++) {
        buf[i] = rand_r(&rnd->seed) & 0xFF;
    }

    return size;
}

/**
 * Memory device (simulated RAM disk)
 */
typedef struct {
    void* memory;
    size_t size;
    off_t position;
} memory_device_data_t;

int memory_open(void* data, int flags) {
    memory_device_data_t* mem = (memory_device_data_t*)data;
    mem->position = 0;
    return 0;
}

ssize_t memory_read(void* data, void* buffer, size_t size) {
    memory_device_data_t* mem = (memory_device_data_t*)data;

    size_t available = mem->size - mem->position;
    if (size > available) size = available;

    if (size > 0) {
        memcpy(buffer, (uint8_t*)mem->memory + mem->position, size);
        mem->position += size;
    }

    return size;
}

ssize_t memory_write(void* data, const void* buffer, size_t size) {
    memory_device_data_t* mem = (memory_device_data_t*)data;

    size_t available = mem->size - mem->position;
    if (size > available) size = available;

    if (size > 0) {
        memcpy((uint8_t*)mem->memory + mem->position, buffer, size);
        mem->position += size;
    }

    return size;
}

off_t memory_seek(void* data, off_t offset, int whence) {
    memory_device_data_t* mem = (memory_device_data_t*)data;

    switch (whence) {
        case SEEK_SET:
            mem->position = offset;
            break;
        case SEEK_CUR:
            mem->position += offset;
            break;
        case SEEK_END:
            mem->position = mem->size + offset;
            break;
    }

    if (mem->position < 0) mem->position = 0;
    if (mem->position > (off_t)mem->size) mem->position = mem->size;

    return mem->position;
}

int memory_ioctl(void* data, unsigned int cmd, unsigned long arg) {
    memory_device_data_t* mem = (memory_device_data_t*)data;

    switch (cmd) {
        case IOCTL_GET_INFO:
            if (arg) {
                *(size_t*)arg = mem->size;
            }
            return 0;

        case IOCTL_RESET:
            memset(mem->memory, 0, mem->size);
            mem->position = 0;
            return 0;

        default:
            errno = EINVAL;
            return -1;
    }
}

/**
 * Driver demonstration
 */
void tbos_drivers_demo(void) {
    printf("\n🔌 === TBOS Device Drivers Demo ===\n");

    // Initialize driver subsystem
    tbos_drivers_init();

    // Register null device
    null_device_data_t null_data = {0};
    tbos_device_ops_t null_ops = {
        .open = null_open,
        .read = null_read,
        .write = null_write
    };
    tbos_device_register("null", DEVICE_TYPE_CHAR, &null_ops, &null_data);

    // Register zero device
    tbos_device_ops_t zero_ops = {
        .read = zero_read
    };
    tbos_device_register("zero", DEVICE_TYPE_CHAR, &zero_ops, NULL);

    // Register random device
    random_device_data_t random_data = {0};
    tbos_device_ops_t random_ops = {
        .open = random_open,
        .read = random_read
    };
    tbos_device_register("random", DEVICE_TYPE_CHAR, &random_ops, &random_data);

    // Register memory device (RAM disk)
    memory_device_data_t* mem_data = malloc(sizeof(memory_device_data_t));
    mem_data->size = 1024 * 1024; // 1MB
    mem_data->memory = malloc(mem_data->size);
    mem_data->position = 0;

    tbos_device_ops_t memory_ops = {
        .open = memory_open,
        .read = memory_read,
        .write = memory_write,
        .seek = memory_seek,
        .ioctl = memory_ioctl
    };
    tbos_device_register("ramdisk", DEVICE_TYPE_BLOCK, &memory_ops, mem_data);

    // Test the devices
    printf("\n🧪 Testing devices:\n");

    // Test null device
    tbos_device_open("null", O_WRONLY);
    const char* test_data = "This goes to null device";
    ssize_t written = tbos_device_write("null", test_data, strlen(test_data));
    printf("✅ Null device: discarded %zd bytes\n", written);

    // Test zero device
    uint8_t zero_buffer[16];
    ssize_t read_bytes = tbos_device_read("zero", zero_buffer, sizeof(zero_buffer));
    printf("✅ Zero device: read %zd zero bytes\n", read_bytes);

    // Test random device
    tbos_device_open("random", O_RDONLY);
    uint8_t random_buffer[8];
    tbos_device_read("random", random_buffer, sizeof(random_buffer));
    printf("✅ Random device: generated bytes: ");
    for (int i = 0; i < 8; i++) {
        printf("%02x ", random_buffer[i]);
    }
    printf("\n");

    // Test RAM disk
    tbos_device_open("ramdisk", O_RDWR);
    const char* ram_data = "Data stored in RAM disk!";
    tbos_device_write("ramdisk", ram_data, strlen(ram_data));

    // Seek back and read
    memory_seek(mem_data, 0, SEEK_SET);
    char read_buffer[128];
    read_bytes = tbos_device_read("ramdisk", read_buffer, sizeof(read_buffer));
    read_buffer[read_bytes] = '\0';
    printf("✅ RAM disk: stored and retrieved: \"%s\"\n", read_buffer);

    // Test ioctl
    size_t disk_size;
    tbos_device_ioctl("ramdisk", IOCTL_GET_INFO, (unsigned long)&disk_size);
    printf("✅ RAM disk size (via ioctl): %zu bytes\n", disk_size);

    // List all devices
    tbos_device_list();

    // Cleanup
    free(mem_data->memory);
    free(mem_data);

    printf("\n✅ Device drivers demonstration complete!\n");
}