#pragma once

#include <stdint.h>
#include <stddef.h>

/* HAL contracts must compile without hosted POSIX headers. */
typedef ptrdiff_t tbos_ssize_t;

typedef enum {
    HAL_DEVICE_CONSOLE,
    HAL_DEVICE_STORAGE,
    HAL_DEVICE_TIMER,
    HAL_DEVICE_INPUT,
    HAL_DEVICE_NETWORK
} hal_device_t;

typedef struct {
    uint32_t has_console : 1;
    uint32_t has_storage : 1;
    uint32_t has_timer : 1;
    uint32_t has_input : 1;
    uint32_t has_network : 1;
} hal_capabilities_t;

typedef struct {
    void (*put_char_xy)(uint32_t x, uint32_t y, char c, uint8_t color);
    void (*clear)(uint8_t color);
    void (*write_serial)(char c);
} hal_console_ops_t;

typedef struct {
    int (*read_key)(void);
    void (*set_callback)(void (*handler)(int));
} hal_input_ops_t;

typedef struct {
    void (*sleep_ms)(uint32_t milliseconds);
    void (*busy_wait)(uint32_t cycles);
} hal_timer_ops_t;

typedef struct {
    void (*read_sector)(uint32_t lba, void* buffer, uint32_t sectors);
    void (*write_sector)(uint32_t lba, const void* buffer, uint32_t sectors);
} hal_storage_ops_t;

/**
 * @brief HAL Network Operations
 * Platform-abstracted network socket operations for hosted and bare-metal modes
 */
typedef struct {
    int (*socket)(int domain, int type, int protocol);
    int (*connect)(int fd, const char* host, uint16_t port);
    int (*bind)(int fd, uint16_t port);
    int (*listen)(int fd, int backlog);
    int (*accept)(int fd, char* remote_addr, uint16_t* remote_port);
    tbos_ssize_t (*send)(int fd, const void* buf, size_t len);
    tbos_ssize_t (*recv)(int fd, void* buf, size_t len);
    int (*close)(int fd);
    int (*set_nonblocking)(int fd, int enabled);
    int (*get_local_ip)(char* buf, size_t len);
    tbos_ssize_t (*sendto)(int fd, const void* buf, size_t len,
                           const char* host, uint16_t port);
    tbos_ssize_t (*recvfrom)(int fd, void* buf, size_t len,
                             char* from_addr, uint16_t* from_port);
} hal_network_ops_t;

typedef struct {
    void (*init)(void);
    hal_capabilities_t (*capabilities)(void);
    hal_console_ops_t console;
    hal_input_ops_t input;
    hal_timer_ops_t timer;
    hal_storage_ops_t storage;
    hal_network_ops_t network;
} hal_dispatch_table_t;

const hal_dispatch_table_t* hal_get_dispatch(void);
