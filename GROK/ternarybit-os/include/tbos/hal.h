#pragma once

#include <stdint.h>

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

typedef struct {
    void (*init)(void);
    hal_capabilities_t (*capabilities)(void);
    hal_console_ops_t console;
    hal_input_ops_t input;
    hal_timer_ops_t timer;
    hal_storage_ops_t storage;
} hal_dispatch_table_t;

const hal_dispatch_table_t* hal_get_dispatch(void);
