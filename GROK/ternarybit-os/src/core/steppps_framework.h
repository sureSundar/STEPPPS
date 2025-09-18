/**
 * @file steppps_framework.h
 * @brief STEPPPS Multi-Dimensional Computing Framework for TernaryBit OS
 *
 * Traceability:
 * - PRD: PR-020 (STEPPPS Framework Implementation)
 * - FRD: FR-STP-001 through FR-STP-007 (All STEPPPS dimensions)
 * - NFRD: NFR-PFM-001 through NFR-PFM-006 (Performance requirements)
 * - HLD: STEPPPS Framework Component
 * - LLD: steppps_manager_t structure
 * - PSC: InitializeSTEPPPS() and dimension algorithms
 *
 * STEPPPS Dimensions:
 * - SPACE: Universal Hardware Management
 * - TIME: Universal Scheduling
 * - EVENT: Universal I/O Management
 * - PSYCHOLOGY: Universal AI Integration
 * - PIXEL: Universal Display Management
 * - PROMPT: Universal Interface
 * - SCRIPT: Universal Automation
 *
 * @version 1.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#ifndef STEPPPS_FRAMEWORK_H
#define STEPPPS_FRAMEWORK_H

#include "hardware_detector.h"
#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct steppps_manager steppps_manager_t;
typedef struct space_manager space_manager_t;
typedef struct time_manager time_manager_t;
typedef struct event_manager event_manager_t;
typedef struct psychology_manager psychology_manager_t;
typedef struct pixel_manager pixel_manager_t;
typedef struct prompt_manager prompt_manager_t;
typedef struct script_manager script_manager_t;

// STEPPPS Framework Status
typedef enum {
    STEPPPS_STATUS_UNINITIALIZED = 0,
    STEPPPS_STATUS_INITIALIZING = 1,
    STEPPPS_STATUS_ACTIVE = 2,
    STEPPPS_STATUS_DEGRADED = 3,
    STEPPPS_STATUS_ERROR = 4
} steppps_status_t;

// STEPPPS Operation Modes
typedef enum {
    STEPPPS_MODE_MINIMAL = 0,    // Basic coordination only
    STEPPPS_MODE_STANDARD = 1,   // Standard multi-dimensional operation
    STEPPPS_MODE_FULL = 2,       // Full AI-enhanced operation
    STEPPPS_MODE_ADAPTIVE = 3    // Dynamically adaptive mode
} steppps_mode_t;

// =============================================================================
// SPACE DIMENSION - Universal Hardware Management
// =============================================================================

// Resource types
typedef enum {
    RESOURCE_CPU = 0,
    RESOURCE_MEMORY = 1,
    RESOURCE_STORAGE = 2,
    RESOURCE_NETWORK = 3,
    RESOURCE_DISPLAY = 4,
    RESOURCE_INPUT = 5,
    RESOURCE_CUSTOM = 6
} resource_type_t;

// Resource allocation request
typedef struct {
    resource_type_t type;
    uint64_t amount_requested;
    uint64_t amount_minimum;
    uint32_t priority;
    uint32_t timeout_ms;
    bool exclusive;
    void* requester_context;
} resource_request_t;

// Resource allocation result
typedef struct {
    bool success;
    uint64_t amount_allocated;
    void* resource_handle;
    uint32_t allocation_time_us;
} resource_allocation_t;

// SPACE Manager Structure
typedef struct space_manager {
    uint64_t total_resources[7];    // One for each resource type
    uint64_t available_resources[7];
    uint64_t allocated_resources[7];
    uint32_t allocation_count;
    uint32_t active_allocations;
    bool optimization_enabled;
    uint32_t efficiency_rating;
} space_manager_t;

// =============================================================================
// TIME DIMENSION - Universal Scheduling
// =============================================================================

// Task priority levels
typedef enum {
    PRIORITY_CRITICAL = 0,    // Real-time critical
    PRIORITY_HIGH = 1,        // High priority
    PRIORITY_NORMAL = 2,      // Normal priority
    PRIORITY_LOW = 3,         // Background tasks
    PRIORITY_IDLE = 4         // Idle time tasks
} task_priority_t;

// Task types
typedef enum {
    TASK_TYPE_SYSTEM = 0,
    TASK_TYPE_USER = 1,
    TASK_TYPE_BACKGROUND = 2,
    TASK_TYPE_REALTIME = 3
} task_type_t;

// Task context
typedef struct {
    uint32_t task_id;
    task_type_t type;
    task_priority_t priority;
    uint32_t cpu_time_us;
    uint32_t deadline_us;
    uint32_t period_us;
    bool is_periodic;
    void* task_function;
    void* task_data;
    uint32_t stack_size;
} task_context_t;

// TIME Manager Structure
typedef struct time_manager {
    uint32_t active_tasks;
    uint32_t max_tasks;
    uint32_t tick_frequency_hz;
    uint64_t system_uptime_us;
    uint32_t context_switches;
    uint32_t missed_deadlines;
    bool realtime_enabled;
    uint32_t scheduling_overhead_us;
} time_manager_t;

// =============================================================================
// EVENT DIMENSION - Universal I/O Management
// =============================================================================

// Event types
typedef enum {
    EVENT_HARDWARE_INTERRUPT = 0,
    EVENT_SOFTWARE_SIGNAL = 1,
    EVENT_USER_INPUT = 2,
    EVENT_NETWORK_PACKET = 3,
    EVENT_TIMER_EXPIRY = 4,
    EVENT_SYSTEM_CALL = 5,
    EVENT_CUSTOM = 6
} event_type_t;

// Event structure
typedef struct {
    event_type_t type;
    uint32_t source_id;
    uint32_t timestamp_us;
    uint32_t data_size;
    void* event_data;
    uint32_t priority;
    bool processed;
} event_t;

// EVENT Manager Structure
typedef struct event_manager {
    uint32_t events_processed;
    uint32_t events_pending;
    uint32_t max_queue_size;
    uint32_t average_processing_time_us;
    uint32_t missed_events;
    bool interrupt_mode;
    uint32_t correlation_cache_size;
} event_manager_t;

// =============================================================================
// PSYCHOLOGY DIMENSION - Universal AI Integration
// =============================================================================

// AI complexity levels
typedef enum {
    AI_LEVEL_NONE = 0,        // No AI features
    AI_LEVEL_HEURISTIC = 1,   // Simple heuristics
    AI_LEVEL_LEARNING = 2,    // Machine learning
    AI_LEVEL_ADAPTIVE = 3,    // Adaptive algorithms
    AI_LEVEL_INTELLIGENT = 4  // Full AI
} ai_level_t;

// Learning metrics
typedef struct {
    uint32_t optimization_cycles;
    uint32_t successful_predictions;
    uint32_t total_predictions;
    uint32_t adaptation_count;
    float efficiency_improvement;
    uint32_t learning_memory_kb;
} learning_metrics_t;

// PSYCHOLOGY Manager Structure
typedef struct psychology_manager {
    ai_level_t ai_level;
    learning_metrics_t metrics;
    bool learning_enabled;
    bool prediction_enabled;
    uint32_t optimization_interval_ms;
    uint32_t pattern_cache_size;
} psychology_manager_t;

// =============================================================================
// PIXEL DIMENSION - Universal Display Management
// =============================================================================

// Display modes
typedef enum {
    DISPLAY_MODE_NONE = 0,
    DISPLAY_MODE_TEXT = 1,
    DISPLAY_MODE_GRAPHICS_2D = 2,
    DISPLAY_MODE_GRAPHICS_3D = 3,
    DISPLAY_MODE_HOLOGRAPHIC = 4
} display_mode_t;

// Render context
typedef struct {
    display_mode_t mode;
    uint16_t width;
    uint16_t height;
    uint8_t color_depth;
    uint32_t frame_rate;
    uint32_t memory_usage_bytes;
    bool hardware_accelerated;
} render_context_t;

// PIXEL Manager Structure
typedef struct pixel_manager {
    render_context_t context;
    uint32_t frames_rendered;
    uint32_t average_frame_time_us;
    uint32_t dropped_frames;
    bool adaptive_quality;
    uint32_t ui_elements_active;
} pixel_manager_t;

// =============================================================================
// PROMPT DIMENSION - Universal Interface
// =============================================================================

// Interface types
typedef enum {
    INTERFACE_NONE = 0,
    INTERFACE_TEXT = 1,
    INTERFACE_VOICE = 2,
    INTERFACE_GESTURE = 3,
    INTERFACE_NEURAL = 4,
    INTERFACE_MULTIMODAL = 5
} interface_type_t;

// Input context
typedef struct {
    interface_type_t type;
    uint32_t input_length;
    void* input_data;
    uint32_t confidence_level;
    uint32_t processing_time_us;
    bool natural_language;
} input_context_t;

// PROMPT Manager Structure
typedef struct prompt_manager {
    interface_type_t active_interfaces[5];
    uint8_t interface_count;
    uint32_t commands_processed;
    uint32_t recognition_accuracy;
    uint32_t response_time_us;
    bool nlp_enabled;
} prompt_manager_t;

// =============================================================================
// SCRIPT DIMENSION - Universal Automation
// =============================================================================

// Script types
typedef enum {
    SCRIPT_TYPE_SYSTEM = 0,
    SCRIPT_TYPE_USER = 1,
    SCRIPT_TYPE_AUTOMATION = 2,
    SCRIPT_TYPE_OPTIMIZATION = 3
} script_type_t;

// Script execution context
typedef struct {
    script_type_t type;
    uint32_t script_id;
    uint32_t execution_count;
    uint32_t average_execution_time_us;
    uint32_t memory_usage_bytes;
    bool cross_platform;
    void* execution_context;
} script_execution_t;

// SCRIPT Manager Structure
typedef struct script_manager {
    uint32_t scripts_active;
    uint32_t scripts_executed;
    uint32_t automation_rules;
    uint32_t execution_efficiency;
    bool cross_platform_enabled;
    uint32_t translation_cache_size;
} script_manager_t;

// =============================================================================
// MAIN STEPPPS FRAMEWORK MANAGER
// =============================================================================

// Main STEPPPS Manager Structure
typedef struct steppps_manager {
    // Framework status
    steppps_status_t status;
    steppps_mode_t mode;
    hardware_info_t* hardware;

    // Dimension managers
    space_manager_t* space;
    time_manager_t* time;
    event_manager_t* event;
    psychology_manager_t* psychology;
    pixel_manager_t* pixel;
    prompt_manager_t* prompt;
    script_manager_t* script;

    // Framework metrics
    uint64_t initialization_time_us;
    uint64_t total_operations;
    uint32_t coordination_overhead_us;
    uint32_t efficiency_rating;
    bool adaptive_mode_enabled;

    // Memory allocation
    uint32_t memory_base_addr;
    uint32_t memory_size_bytes;
    uint32_t memory_used_bytes;

} steppps_manager_t;

// =============================================================================
// FUNCTION PROTOTYPES
// =============================================================================

/**
 * @brief Initialize STEPPPS Framework
 * @param hardware Hardware information from detection
 * @param mode Operating mode
 * @param memory_base Base address for STEPPPS memory
 * @param memory_size Size of allocated memory
 * @return steppps_manager_t* Initialized framework manager
 *
 * Implements PSC InitializeSTEPPPS() algorithm
 */
steppps_manager_t* initialize_steppps_framework(
    const hardware_info_t* hardware,
    steppps_mode_t mode,
    uint32_t memory_base,
    uint32_t memory_size
);

/**
 * @brief Coordinate all STEPPPS dimensions
 * @param manager STEPPPS framework manager
 * @return bool Success/failure status
 *
 * Implements PSC CoordinateSTEPPPS() algorithm
 * Target: <10μs coordination time (NFRD NFR-PFM-002)
 */
bool coordinate_steppps_dimensions(steppps_manager_t* manager);

/**
 * @brief Process framework operation cycle
 * @param manager STEPPPS framework manager
 * @return bool Success/failure status
 */
bool process_steppps_cycle(steppps_manager_t* manager);

/**
 * @brief Shutdown STEPPPS Framework
 * @param manager STEPPPS framework manager
 */
void shutdown_steppps_framework(steppps_manager_t* manager);

// SPACE Dimension Functions
space_manager_t* initialize_space_manager(const hardware_info_t* hardware);
resource_allocation_t allocate_resource(space_manager_t* space, const resource_request_t* request);
bool deallocate_resource(space_manager_t* space, void* resource_handle);
uint32_t get_resource_efficiency(const space_manager_t* space);

// TIME Dimension Functions
time_manager_t* initialize_time_manager(const hardware_info_t* hardware);
bool schedule_task(time_manager_t* time, const task_context_t* task);
bool process_time_slice(time_manager_t* time);
uint32_t get_scheduling_efficiency(const time_manager_t* time);

// EVENT Dimension Functions
event_manager_t* initialize_event_manager(const hardware_info_t* hardware);
bool process_event(event_manager_t* event, const event_t* evt);
bool register_event_handler(event_manager_t* event, event_type_t type, void* handler);
uint32_t get_event_processing_efficiency(const event_manager_t* event);

// PSYCHOLOGY Dimension Functions
psychology_manager_t* initialize_psychology_manager(const hardware_info_t* hardware);
bool optimize_system_behavior(psychology_manager_t* psychology);
bool learn_from_operation(psychology_manager_t* psychology, const void* operation_data);
uint32_t get_ai_efficiency(const psychology_manager_t* psychology);

// PIXEL Dimension Functions
pixel_manager_t* initialize_pixel_manager(const hardware_info_t* hardware);
bool render_adaptive_ui(pixel_manager_t* pixel, const void* ui_data);
bool update_display_mode(pixel_manager_t* pixel, display_mode_t mode);
uint32_t get_rendering_efficiency(const pixel_manager_t* pixel);

// PROMPT Dimension Functions
prompt_manager_t* initialize_prompt_manager(const hardware_info_t* hardware);
bool process_user_input(prompt_manager_t* prompt, const input_context_t* input);
bool register_interface(prompt_manager_t* prompt, interface_type_t type);
uint32_t get_interface_efficiency(const prompt_manager_t* prompt);

// SCRIPT Dimension Functions
script_manager_t* initialize_script_manager(const hardware_info_t* hardware);
bool execute_script(script_manager_t* script, const script_execution_t* execution);
bool register_automation_rule(script_manager_t* script, const void* rule);
uint32_t get_automation_efficiency(const script_manager_t* script);

// Utility Functions
const char* get_steppps_status_name(steppps_status_t status);
const char* get_steppps_mode_name(steppps_mode_t mode);
void print_steppps_metrics(const steppps_manager_t* manager);
bool validate_steppps_framework(const steppps_manager_t* manager);

// Global STEPPPS manager instance
extern steppps_manager_t* g_steppps_manager;

#endif // STEPPPS_FRAMEWORK_H