/*
 * TBOS v3.0 Shell Plugin System
 * Extensible plugin architecture for human and machine expandability
 * Supports dynamic loading, AI integration, and quantum operations
 */

#ifndef TBOS_SHELL_PLUGINS_H
#define TBOS_SHELL_PLUGINS_H

#include "tbos_shell_architecture.h"
#include <stdint.h>
#include <stdbool.h>

//=============================================================================
// PLUGIN SYSTEM ARCHITECTURE
//=============================================================================

typedef enum {
    PLUGIN_TYPE_COMMAND = 0,        // Command-line extensions
    PLUGIN_TYPE_GUI = 1,            // Graphical interface plugins
    PLUGIN_TYPE_AI = 2,             // AI/Machine learning plugins
    PLUGIN_TYPE_QUANTUM = 3,        // Quantum operation plugins
    PLUGIN_TYPE_NETWORK = 4,        // Network protocol plugins
    PLUGIN_TYPE_FILESYSTEM = 5,     // File system plugins
    PLUGIN_TYPE_DEVICE = 6,         // Device driver plugins
    PLUGIN_TYPE_CRYPTO = 7,         // Cryptographic plugins
    PLUGIN_TYPE_PERSONA = 8,        // Persona behavior plugins
    PLUGIN_TYPE_BRIDGE = 9,         // Cross-architecture bridge plugins
    PLUGIN_TYPE_COUNT = 10
} plugin_type_t;

typedef enum {
    PLUGIN_STATE_UNLOADED = 0,
    PLUGIN_STATE_LOADING = 1,
    PLUGIN_STATE_LOADED = 2,
    PLUGIN_STATE_RUNNING = 3,
    PLUGIN_STATE_SUSPENDED = 4,
    PLUGIN_STATE_ERROR = 5,
    PLUGIN_STATE_COUNT = 6
} plugin_state_t;

typedef enum {
    PLUGIN_CAPABILITY_REAL_TIME = (1 << 0),     // Real-time operations
    PLUGIN_CAPABILITY_QUANTUM = (1 << 1),       // Quantum computing
    PLUGIN_CAPABILITY_AI = (1 << 2),            // AI/ML integration
    PLUGIN_CAPABILITY_NETWORK = (1 << 3),       // Network operations
    PLUGIN_CAPABILITY_GPU = (1 << 4),           // GPU acceleration
    PLUGIN_CAPABILITY_SECURE = (1 << 5),        // Security operations
    PLUGIN_CAPABILITY_VR = (1 << 6),            // VR/AR support
    PLUGIN_CAPABILITY_MOBILE = (1 << 7),        // Mobile optimized
    PLUGIN_CAPABILITY_WEB = (1 << 8),           // Web interface
    PLUGIN_CAPABILITY_NEURAL = (1 << 9),        // Neural interface
    PLUGIN_CAPABILITY_DISTRIBUTED = (1 << 10),  // Distributed computing
    PLUGIN_CAPABILITY_ADAPTIVE = (1 << 11)      // Self-adapting behavior
} plugin_capability_t;

//=============================================================================
// PLUGIN INTERFACE DEFINITIONS
//=============================================================================

typedef struct plugin_context plugin_context_t;
typedef struct shell_plugin shell_plugin_t;

// Plugin API function signatures
typedef int (*plugin_init_fn)(plugin_context_t* context);
typedef int (*plugin_cleanup_fn)(plugin_context_t* context);
typedef int (*plugin_execute_fn)(plugin_context_t* context, int argc, char* argv[]);
typedef int (*plugin_update_fn)(plugin_context_t* context, uint64_t timestamp);
typedef int (*plugin_configure_fn)(plugin_context_t* context, const char* config);

// Plugin event handlers
typedef int (*plugin_on_load_fn)(plugin_context_t* context);
typedef int (*plugin_on_unload_fn)(plugin_context_t* context);
typedef int (*plugin_on_persona_switch_fn)(plugin_context_t* context, uint8_t new_persona);
typedef int (*plugin_on_command_fn)(plugin_context_t* context, const char* command, char* response, size_t response_size);
typedef int (*plugin_on_data_fn)(plugin_context_t* context, const void* data, size_t data_size);

// Plugin API structure
typedef struct {
    // Core functions
    plugin_init_fn init;
    plugin_cleanup_fn cleanup;
    plugin_execute_fn execute;
    plugin_update_fn update;
    plugin_configure_fn configure;

    // Event handlers
    plugin_on_load_fn on_load;
    plugin_on_unload_fn on_unload;
    plugin_on_persona_switch_fn on_persona_switch;
    plugin_on_command_fn on_command;
    plugin_on_data_fn on_data;

} plugin_api_t;

//=============================================================================
// PLUGIN METADATA AND REGISTRATION
//=============================================================================

typedef struct {
    char name[64];                  // Plugin name
    char version[16];               // Plugin version (e.g., "1.0.0")
    char author[64];                // Plugin author/organization
    char description[256];          // Plugin description
    char license[32];               // License (MIT, GPL, etc.)
    char website[128];              // Plugin website/repository

    // Technical specifications
    plugin_type_t type;
    uint32_t capabilities;          // Bitfield of plugin_capability_t
    uint8_t supported_personas;     // Bitfield of supported personas
    shell_layer_type_t target_layer; // Target shell layer

    // Dependencies
    char dependencies[512];         // Comma-separated list of required plugins
    char conflicts[256];            // Comma-separated list of conflicting plugins
    uint32_t min_tbos_version;      // Minimum TBOS version required

    // Security and validation
    char signature[128];            // Digital signature for validation
    uint32_t checksum;              // Plugin file checksum
    bool trusted;                   // Is plugin from trusted source

} plugin_metadata_t;

//=============================================================================
// PLUGIN CONTEXT AND RUNTIME
//=============================================================================

struct plugin_context {
    // Plugin identity
    uint32_t plugin_id;
    char plugin_name[64];
    shell_plugin_t* plugin;

    // Runtime state
    plugin_state_t state;
    uint64_t load_time;
    uint64_t last_update;
    uint32_t invocation_count;

    // Shell integration
    shell_layer_type_t layer;
    uint8_t current_persona;
    void* shell_manager;            // Reference to tbos_shell_manager_t

    // Plugin data
    void* private_data;             // Plugin's private data
    size_t private_data_size;
    void* shared_data;              // Shared data with other plugins
    size_t shared_data_size;

    // Resource management
    uint32_t memory_limit_kb;       // Memory usage limit
    uint32_t cpu_quota_percent;     // CPU usage quota (1-100%)
    uint32_t network_quota_bps;     // Network bandwidth quota
    bool real_time_priority;        // Real-time scheduling priority

    // Communication channels
    int input_fd;                   // Input file descriptor
    int output_fd;                  // Output file descriptor
    int error_fd;                   // Error file descriptor
    void* message_queue;            // Inter-plugin message queue

    // Security context
    uint32_t security_level;        // Security clearance level (0-10)
    bool sandboxed;                 // Is plugin sandboxed
    char allowed_paths[512];        // Allowed file system paths
    uint32_t allowed_syscalls;      // Bitfield of allowed system calls

};

//=============================================================================
// PLUGIN STRUCTURE
//=============================================================================

struct shell_plugin {
    // Plugin metadata
    plugin_metadata_t metadata;
    plugin_api_t api;

    // Plugin state
    plugin_state_t state;
    plugin_context_t context;

    // Dynamic loading
    void* handle;                   // dlopen handle for shared libraries
    char library_path[512];         // Path to plugin library
    bool is_builtin;                // Is this a built-in plugin?

    // Configuration
    char config_file[256];          // Plugin configuration file
    char config_data[1024];         // Plugin configuration data
    bool auto_start;                // Start plugin automatically

    // Performance metrics
    uint64_t total_runtime_us;      // Total execution time in microseconds
    uint32_t average_response_time_ms; // Average response time
    uint64_t total_invocations;     // Total number of invocations
    uint32_t error_count;           // Number of errors encountered

    // Plugin relationships
    uint32_t parent_plugin_id;      // Parent plugin (for sub-plugins)
    uint32_t child_plugins[16];     // Child plugin IDs
    uint32_t child_count;

};

//=============================================================================
// PLUGIN MANAGER
//=============================================================================

#define MAX_PLUGINS 256
#define MAX_PLUGIN_SEARCH_PATHS 16
#define MAX_PLUGIN_CATEGORIES 32

typedef struct {
    // Plugin registry
    shell_plugin_t plugins[MAX_PLUGINS];
    uint32_t plugin_count;
    uint32_t next_plugin_id;

    // Plugin search paths
    char search_paths[MAX_PLUGIN_SEARCH_PATHS][256];
    uint32_t search_path_count;

    // Plugin categories and organization
    char categories[MAX_PLUGIN_CATEGORIES][64];
    uint32_t category_plugins[MAX_PLUGIN_CATEGORIES][64];
    uint32_t category_counts[MAX_PLUGIN_CATEGORIES];
    uint32_t category_count;

    // Plugin security and validation
    bool signature_validation_enabled;
    bool sandbox_mode_enabled;
    uint32_t max_plugin_memory_mb;
    uint32_t max_concurrent_plugins;

    // Plugin communication
    void* message_bus;              // Inter-plugin message bus
    void* event_dispatcher;         // Plugin event dispatcher
    void* shared_memory_pool;       // Shared memory for plugins

    // Hot reloading and updates
    bool hot_reload_enabled;
    char plugin_update_server[128]; // URL for plugin updates
    uint64_t last_update_check;

    // AI and machine learning integration
    bool ai_plugin_discovery;       // AI-powered plugin discovery
    bool auto_plugin_optimization;  // Automatic plugin optimization
    void* ml_model_cache;           // Machine learning model cache

    // Quantum plugin support
    bool quantum_plugins_enabled;
    void* quantum_resource_manager;
    uint32_t quantum_plugin_count;

} plugin_manager_t;

//=============================================================================
// PLUGIN MANAGER API
//=============================================================================

// Plugin manager lifecycle
int tbos_plugin_manager_init(plugin_manager_t* manager);
int tbos_plugin_manager_cleanup(plugin_manager_t* manager);
int tbos_plugin_manager_start(plugin_manager_t* manager);
int tbos_plugin_manager_stop(plugin_manager_t* manager);

// Plugin discovery and loading
int tbos_plugin_discover_plugins(plugin_manager_t* manager, const char* search_path);
int tbos_plugin_load(plugin_manager_t* manager, const char* plugin_path);
int tbos_plugin_unload(plugin_manager_t* manager, uint32_t plugin_id);
int tbos_plugin_reload(plugin_manager_t* manager, uint32_t plugin_id);

// Plugin management
uint32_t tbos_plugin_register(plugin_manager_t* manager, shell_plugin_t* plugin);
int tbos_plugin_unregister(plugin_manager_t* manager, uint32_t plugin_id);
shell_plugin_t* tbos_plugin_get(plugin_manager_t* manager, uint32_t plugin_id);
shell_plugin_t* tbos_plugin_find_by_name(plugin_manager_t* manager, const char* name);

// Plugin execution
int tbos_plugin_execute(plugin_manager_t* manager, uint32_t plugin_id, int argc, char* argv[]);
int tbos_plugin_execute_by_name(plugin_manager_t* manager, const char* name, int argc, char* argv[]);
int tbos_plugin_send_command(plugin_manager_t* manager, uint32_t plugin_id, const char* command);

// Plugin configuration
int tbos_plugin_configure(plugin_manager_t* manager, uint32_t plugin_id, const char* config);
int tbos_plugin_get_config(plugin_manager_t* manager, uint32_t plugin_id, char* config, size_t config_size);
int tbos_plugin_save_config(plugin_manager_t* manager, uint32_t plugin_id);

// Plugin communication and events
int tbos_plugin_send_message(plugin_manager_t* manager, uint32_t from_id, uint32_t to_id, const void* data, size_t size);
int tbos_plugin_broadcast_event(plugin_manager_t* manager, const char* event_type, const void* data, size_t size);
int tbos_plugin_subscribe_event(plugin_manager_t* manager, uint32_t plugin_id, const char* event_type);

// Plugin security and validation
int tbos_plugin_validate_signature(plugin_manager_t* manager, const char* plugin_path);
int tbos_plugin_set_security_level(plugin_manager_t* manager, uint32_t plugin_id, uint32_t level);
int tbos_plugin_enable_sandbox(plugin_manager_t* manager, uint32_t plugin_id);

// Persona integration
int tbos_plugin_switch_persona(plugin_manager_t* manager, uint8_t new_persona);
int tbos_plugin_get_persona_plugins(plugin_manager_t* manager, uint8_t persona, uint32_t* plugin_ids, uint32_t max_plugins);

// AI and machine learning features
int tbos_plugin_enable_ai_discovery(plugin_manager_t* manager);
int tbos_plugin_optimize_performance(plugin_manager_t* manager, uint32_t plugin_id);
int tbos_plugin_predict_usage(plugin_manager_t* manager, uint32_t plugin_id, uint64_t* predicted_usage);

// Quantum plugin support
int tbos_plugin_enable_quantum_support(plugin_manager_t* manager);
int tbos_plugin_allocate_quantum_resources(plugin_manager_t* manager, uint32_t plugin_id, uint32_t qubit_count);

// Utility functions
const char* tbos_plugin_type_name(plugin_type_t type);
const char* tbos_plugin_state_name(plugin_state_t state);
void tbos_plugin_print_info(shell_plugin_t* plugin);
void tbos_plugin_print_manager_status(plugin_manager_t* manager);

// Error codes
#define TBOS_PLUGIN_SUCCESS                 0
#define TBOS_PLUGIN_ERROR_INVALID_PARAM    -1
#define TBOS_PLUGIN_ERROR_NOT_FOUND        -2
#define TBOS_PLUGIN_ERROR_LOAD_FAILED      -3
#define TBOS_PLUGIN_ERROR_INVALID_SIGNATURE -4
#define TBOS_PLUGIN_ERROR_SECURITY_VIOLATION -5
#define TBOS_PLUGIN_ERROR_DEPENDENCY_MISSING -6
#define TBOS_PLUGIN_ERROR_RESOURCE_EXHAUSTED -7
#define TBOS_PLUGIN_ERROR_QUANTUM_UNAVAILABLE -8

//=============================================================================
// BUILT-IN PLUGIN DECLARATIONS
//=============================================================================

// Core system plugins
extern shell_plugin_t tbos_plugin_file_manager;
extern shell_plugin_t tbos_plugin_process_manager;
extern shell_plugin_t tbos_plugin_network_manager;
extern shell_plugin_t tbos_plugin_device_manager;

// Persona-specific plugins
extern shell_plugin_t tbos_plugin_calculator_enhanced;
extern shell_plugin_t tbos_plugin_embedded_diagnostics;
extern shell_plugin_t tbos_plugin_desktop_widgets;
extern shell_plugin_t tbos_plugin_mobile_gestures;
extern shell_plugin_t tbos_plugin_quantum_simulator;
extern shell_plugin_t tbos_plugin_vr_interface;
extern shell_plugin_t tbos_plugin_neural_bridge;

// AI and machine learning plugins
extern shell_plugin_t tbos_plugin_ai_assistant;
extern shell_plugin_t tbos_plugin_ml_accelerator;
extern shell_plugin_t tbos_plugin_nlp_processor;
extern shell_plugin_t tbos_plugin_computer_vision;

// Web and browser integration plugins
extern shell_plugin_t tbos_plugin_web_server;
extern shell_plugin_t tbos_plugin_websocket_bridge;
extern shell_plugin_t tbos_plugin_browser_renderer;
extern shell_plugin_t tbos_plugin_rest_api;

#endif // TBOS_SHELL_PLUGINS_H