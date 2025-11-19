/*
 * TBOS v3.0 Command Router and Delegation System
 * Intelligent command routing across shell layers and plugins
 * Supports AI-powered command interpretation and quantum routing
 */

#ifndef TBOS_COMMAND_ROUTER_H
#define TBOS_COMMAND_ROUTER_H

#include "tbos_shell_architecture.h"
#include "tbos_shell_plugins.h"
#include <stdint.h>
#include <stdbool.h>

//=============================================================================
// COMMAND ROUTING ARCHITECTURE
//=============================================================================

typedef enum {
    ROUTE_STRATEGY_DIRECT = 0,      // Direct command execution
    ROUTE_STRATEGY_LAYERED = 1,     // Route through shell layers
    ROUTE_STRATEGY_PLUGIN = 2,      // Route to specific plugin
    ROUTE_STRATEGY_AI = 3,          // AI-powered intelligent routing
    ROUTE_STRATEGY_QUANTUM = 4,     // Quantum parallel routing
    ROUTE_STRATEGY_BROADCAST = 5,   // Broadcast to multiple handlers
    ROUTE_STRATEGY_PIPELINE = 6,    // Pipeline through multiple layers
    ROUTE_STRATEGY_ADAPTIVE = 7,    // Self-adapting strategy
    ROUTE_STRATEGY_COUNT = 8
} route_strategy_t;

typedef enum {
    COMMAND_TYPE_SYSTEM = 0,        // System administration commands
    COMMAND_TYPE_FILE = 1,          // File operations
    COMMAND_TYPE_NETWORK = 2,       // Network operations
    COMMAND_TYPE_PROCESS = 3,       // Process management
    COMMAND_TYPE_PERSONA = 4,       // Persona switching/management
    COMMAND_TYPE_GUI = 5,           // GUI operations
    COMMAND_TYPE_QUANTUM = 6,       // Quantum computing commands
    COMMAND_TYPE_AI = 7,            // AI/ML operations
    COMMAND_TYPE_PLUGIN = 8,        // Plugin management
    COMMAND_TYPE_CUSTOM = 9,        // Custom user commands
    COMMAND_TYPE_UNKNOWN = 10,      // Unknown/unclassified
    COMMAND_TYPE_COUNT = 11
} command_type_t;

typedef enum {
    COMMAND_PRIORITY_LOW = 0,       // Background tasks
    COMMAND_PRIORITY_NORMAL = 1,    // Standard operations
    COMMAND_PRIORITY_HIGH = 2,      // Important operations
    COMMAND_PRIORITY_CRITICAL = 3,  // System-critical operations
    COMMAND_PRIORITY_REALTIME = 4,  // Real-time operations
    COMMAND_PRIORITY_QUANTUM = 5,   // Quantum operations
    COMMAND_PRIORITY_COUNT = 6
} command_priority_t;

//=============================================================================
// COMMAND STRUCTURE AND METADATA
//=============================================================================

typedef struct {
    char raw_command[512];          // Original command string
    char program[64];               // Command/program name
    char args[32][128];             // Command arguments
    uint32_t argc;                  // Argument count
    char full_path[512];            // Full command line

    // Command classification
    command_type_t type;
    command_priority_t priority;
    uint8_t target_persona;         // Preferred persona for execution
    shell_layer_type_t target_layer; // Preferred shell layer

    // Execution context
    uint32_t session_id;            // Session that issued the command
    uint32_t user_id;               // User ID
    uint32_t security_level;        // Required security level
    bool requires_sudo;             // Requires elevated privileges

    // Routing metadata
    route_strategy_t routing_strategy;
    uint32_t preferred_plugin_id;   // Preferred plugin for execution
    bool allow_fallback;            // Allow fallback routing
    uint64_t timestamp;             // Command issue timestamp

    // AI and quantum features
    float ai_confidence;            // AI classification confidence (0.0-1.0)
    bool quantum_enhanced;          // Use quantum routing
    uint32_t parallel_universes;    // Number of parallel execution paths

} command_info_t;

typedef struct {
    int exit_code;                  // Command exit code
    char output[4096];              // Command output
    char error[1024];               // Error messages
    uint64_t execution_time_us;     // Execution time in microseconds
    uint32_t handler_id;            // ID of handler that executed command

    // Route information
    route_strategy_t used_strategy;
    shell_layer_type_t executed_layer;
    uint32_t plugin_id;             // Plugin that handled command
    uint32_t route_hops;            // Number of routing hops

    // Performance metrics
    uint64_t parse_time_us;         // Time to parse command
    uint64_t route_time_us;         // Time to route command
    uint64_t queue_time_us;         // Time in execution queue

} command_result_t;

//=============================================================================
// COMMAND HANDLERS AND ROUTES
//=============================================================================

typedef struct command_handler command_handler_t;

typedef int (*command_handler_fn)(command_info_t* cmd, command_result_t* result);
typedef bool (*command_matcher_fn)(const command_info_t* cmd);
typedef float (*command_scorer_fn)(const command_info_t* cmd);

struct command_handler {
    uint32_t handler_id;
    char name[64];                  // Handler name
    char description[256];          // Handler description

    // Handler function pointers
    command_handler_fn execute;     // Main execution function
    command_matcher_fn can_handle;  // Can this handler process the command?
    command_scorer_fn score;        // Score how well this handler fits (0.0-1.0)

    // Handler properties
    command_type_t supported_types[8]; // Supported command types
    uint32_t supported_type_count;
    shell_layer_type_t layer;       // Shell layer this handler belongs to
    uint32_t plugin_id;             // Associated plugin (0 if built-in)

    // Persona and context
    uint8_t supported_personas;     // Bitfield of supported personas
    uint32_t required_security_level; // Minimum security level required
    bool requires_gui;              // Requires GUI environment
    bool supports_quantum;          // Supports quantum operations

    // Performance characteristics
    uint32_t average_execution_time_ms; // Average execution time
    uint32_t max_execution_time_ms; // Maximum execution time
    float success_rate;             // Success rate (0.0-1.0)
    uint32_t total_invocations;     // Total number of invocations

};

//=============================================================================
// COMMAND ROUTER
//=============================================================================

#define MAX_COMMAND_HANDLERS 256
#define MAX_ROUTING_RULES 128
#ifndef MAX_COMMAND_HISTORY
#define MAX_COMMAND_HISTORY 1024
#endif
#define MAX_CONCURRENT_COMMANDS 64

typedef struct {
    char pattern[128];              // Command pattern (regex-like)
    route_strategy_t strategy;      // Routing strategy to use
    uint32_t target_handler_id;     // Target handler ID
    shell_layer_type_t target_layer; // Target layer
    uint32_t target_plugin_id;      // Target plugin
    uint8_t target_persona;         // Target persona
    float priority_boost;           // Priority boost factor
    bool enabled;                   // Is this rule active?
} routing_rule_t;

typedef struct {
    // Command handlers registry
    command_handler_t handlers[MAX_COMMAND_HANDLERS];
    uint32_t handler_count;
    uint32_t next_handler_id;

    // Routing rules
    routing_rule_t rules[MAX_ROUTING_RULES];
    uint32_t rule_count;

    // Current routing context
    uint8_t current_persona;
    shell_layer_type_t current_layer;
    uint32_t current_session_id;
    uint32_t current_security_level;

    // AI and machine learning
    bool ai_routing_enabled;        // Enable AI-powered routing
    void* ml_model;                 // Machine learning model for routing
    float ai_confidence_threshold;  // Minimum confidence for AI routing
    bool learning_mode;             // Is router learning from user feedback?

    // Quantum routing
    bool quantum_routing_enabled;   // Enable quantum routing
    uint32_t quantum_universes;     // Number of parallel universes
    void* quantum_state_machine;    // Quantum state machine

    // Performance optimization
    bool route_caching_enabled;     // Cache routing decisions
    void* route_cache;              // Routing cache
    uint32_t cache_hit_rate;        // Cache hit rate percentage

    // Command history and analytics
    command_info_t command_history[MAX_COMMAND_HISTORY];
    command_result_t result_history[MAX_COMMAND_HISTORY];
    uint32_t history_count;
    uint32_t history_index;

    // Concurrent execution
    command_info_t active_commands[MAX_CONCURRENT_COMMANDS];
    command_result_t active_results[MAX_CONCURRENT_COMMANDS];
    uint32_t active_command_count;

    // Statistics and metrics
    uint64_t total_commands_processed;
    uint64_t total_routing_time_us;
    uint64_t total_execution_time_us;
    uint32_t successful_commands;
    uint32_t failed_commands;

} command_router_t;

//=============================================================================
// COMMAND ROUTER API
//=============================================================================

// Router management
int tbos_router_init(command_router_t* router);
int tbos_router_cleanup(command_router_t* router);
int tbos_router_start(command_router_t* router);
int tbos_router_stop(command_router_t* router);

// Command processing
int tbos_router_process_command(command_router_t* router, const char* command_line, command_result_t* result);
int tbos_router_process_command_async(command_router_t* router, const char* command_line, uint32_t* task_id);
int tbos_router_get_async_result(command_router_t* router, uint32_t task_id, command_result_t* result);

// Command parsing and classification
int tbos_router_parse_command(const char* command_line, command_info_t* cmd_info);
command_type_t tbos_router_classify_command(const command_info_t* cmd_info);
command_priority_t tbos_router_determine_priority(const command_info_t* cmd_info);

// Routing strategy selection
route_strategy_t tbos_router_select_strategy(command_router_t* router, const command_info_t* cmd_info);
int tbos_router_route_command(command_router_t* router, command_info_t* cmd_info, command_result_t* result);

// Handler management
uint32_t tbos_router_register_handler(command_router_t* router, command_handler_t* handler);
int tbos_router_unregister_handler(command_router_t* router, uint32_t handler_id);
command_handler_t* tbos_router_find_best_handler(command_router_t* router, const command_info_t* cmd_info);
command_handler_t* tbos_router_get_handler(command_router_t* router, uint32_t handler_id);

// Routing rules
int tbos_router_add_rule(command_router_t* router, routing_rule_t* rule);
int tbos_router_remove_rule(command_router_t* router, const char* pattern);
int tbos_router_apply_rules(command_router_t* router, command_info_t* cmd_info);

// AI and machine learning features
int tbos_router_enable_ai_routing(command_router_t* router);
int tbos_router_train_ml_model(command_router_t* router);
float tbos_router_predict_success_rate(command_router_t* router, const command_info_t* cmd_info);
int tbos_router_learn_from_execution(command_router_t* router, const command_info_t* cmd_info, const command_result_t* result);

// Quantum routing features
int tbos_router_enable_quantum_routing(command_router_t* router, uint32_t universe_count);
int tbos_router_quantum_route(command_router_t* router, command_info_t* cmd_info, command_result_t* results, uint32_t max_results);
int tbos_router_collapse_quantum_state(command_router_t* router, command_result_t* best_result);

// Persona integration
int tbos_router_switch_persona(command_router_t* router, uint8_t new_persona);
int tbos_router_filter_handlers_by_persona(command_router_t* router, uint8_t persona);
int tbos_router_adapt_to_persona(command_router_t* router, uint8_t persona);

// Performance and monitoring
int tbos_router_optimize_performance(command_router_t* router);
float tbos_router_get_success_rate(command_router_t* router);
uint64_t tbos_router_get_average_execution_time(command_router_t* router);
int tbos_router_generate_performance_report(command_router_t* router, char* report, size_t report_size);

// Utility functions
const char* tbos_router_strategy_name(route_strategy_t strategy);
const char* tbos_router_command_type_name(command_type_t type);
const char* tbos_router_priority_name(command_priority_t priority);
void tbos_router_print_status(command_router_t* router);
void tbos_router_print_command_info(const command_info_t* cmd_info);
void tbos_router_print_result(const command_result_t* result);

// Error codes
#define TBOS_ROUTER_SUCCESS                  0
#define TBOS_ROUTER_ERROR_INVALID_PARAM     -1
#define TBOS_ROUTER_ERROR_PARSE_FAILED      -2
#define TBOS_ROUTER_ERROR_NO_HANDLER        -3
#define TBOS_ROUTER_ERROR_EXECUTION_FAILED  -4
#define TBOS_ROUTER_ERROR_PERMISSION_DENIED -5
#define TBOS_ROUTER_ERROR_TIMEOUT          -6
#define TBOS_ROUTER_ERROR_QUANTUM_FAILED   -7
#define TBOS_ROUTER_ERROR_AI_UNAVAILABLE   -8

//=============================================================================
// BUILT-IN COMMAND HANDLERS
//=============================================================================

// System handlers
extern command_handler_t tbos_handler_system_info;
extern command_handler_t tbos_handler_persona_switch;
extern command_handler_t tbos_handler_process_manager;
extern command_handler_t tbos_handler_file_operations;

// Plugin handlers
extern command_handler_t tbos_handler_plugin_manager;
extern command_handler_t tbos_handler_ai_assistant;
extern command_handler_t tbos_handler_quantum_operations;
extern command_handler_t tbos_handler_web_interface;

// Shell layer handlers
extern command_handler_t tbos_handler_kernel_commands;
extern command_handler_t tbos_handler_application_launcher;
extern command_handler_t tbos_handler_user_interface;
extern command_handler_t tbos_handler_bridge_operations;

#endif // TBOS_COMMAND_ROUTER_H