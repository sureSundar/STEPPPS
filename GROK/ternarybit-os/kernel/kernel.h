/*
 * TernaryBit OS Kernel Header
 * STEPPPS Framework Definitions
 */

#ifndef TERNARYBIT_KERNEL_H
#define TERNARYBIT_KERNEL_H

#include <stdint.h>
#include <stddef.h>

// Basic type definitions
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// STEPPPS Dimension Structures
typedef struct {
    int active;
    int cpu_count;
    u32 memory_size;
    int devices_found;
    int resource_pools_initialized;
} space_dimension_t;

typedef struct {
    int active;
    u32 system_uptime;
    u16 timer_frequency;
    u8 scheduler_quantum;
    int ai_scheduler_active;
} time_dimension_t;

typedef struct {
    int active;
    u32 events_processed;
    u32 interrupt_count;
    u32 error_count;
    u16 event_queue_size;
} event_dimension_t;

typedef struct {
    int active;
    u32 learning_cycles;
    float adaptation_rate;
    float confidence_level;
    int neural_net_active;
} psychology_dimension_t;

typedef struct {
    int active;
    int graphics_mode_active;
    u16 resolution_x;
    u16 resolution_y;
    u8 color_depth;
} pixel_dimension_t;

typedef struct {
    int active;
    u32 commands_processed;
    u32 ai_responses_generated;
    int language_model_active;
} prompt_dimension_t;

typedef struct {
    int active;
    u32 scripts_executed;
    u8 automation_level;
    int orchestrator_active;
} script_dimension_t;

// Main STEPPPS State Structure
typedef struct {
    space_dimension_t space;
    time_dimension_t time;
    event_dimension_t event;
    psychology_dimension_t psychology;
    pixel_dimension_t pixel;
    prompt_dimension_t prompt;
    script_dimension_t script;

    int orchestrator_active;
    u32 coordination_cycles;
} steppps_state_t;

// Terminal/Display
void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void kernel_printf(const char* format, ...);

// String functions
size_t strlen(const char* str);
int strcmp(const char* str1, const char* str2);
char* strstr(const char* haystack, const char* needle);
void int_to_string(int value, char* str);

// Input functions
void read_command(char* buffer);

// Hardware abstraction
int detect_cpu_count(void);
u32 detect_memory_size(void);
int detect_hardware_devices(void);

// System initialization
void init_memory_management(void);
void init_interrupt_handling(void);
void init_timer(void);

// STEPPPS Framework functions
void init_steppps_framework(void);
void init_space_dimension(void);
void init_time_dimension(void);
void init_event_dimension(void);
void init_psychology_dimension(void);
void init_pixel_dimension(void);
void init_prompt_dimension(void);
void init_script_dimension(void);
void start_steppps_orchestrator(void);
void update_steppps_dimensions(void);

// Command processing
void process_steppps_command(const char* command);
void process_ai_command(const char* command);
void show_help(void);
void show_steppps_status(void);
void show_system_stats(void);
void show_ai_status(void);

// Kernel functions
void kernel_main(void);
void kernel_main_loop(void);
void kernel_yield(void);
void kernel_reboot(void);

#endif // TERNARYBIT_KERNEL_H