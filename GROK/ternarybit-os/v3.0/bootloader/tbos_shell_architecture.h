/*
 * TBOS v3.0 Multi-Layered Shell Architecture
 * Consolidates all components into integrated shell layers
 */

#ifndef TBOS_SHELL_ARCHITECTURE_H
#define TBOS_SHELL_ARCHITECTURE_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct tbos_shell_manager tbos_shell_manager_t;
typedef struct tbos_shell_layer tbos_shell_layer_t;
typedef struct tbos_shell_plugin tbos_shell_plugin_t;

//=============================================================================
// SHELL LAYER DEFINITIONS
//=============================================================================

typedef enum {
    SHELL_LAYER_KERNEL = 0,        // Kernel-level shell (bootloader integration)
    SHELL_LAYER_SYSTEM = 1,        // System services shell
    SHELL_LAYER_PERSONA = 2,       // Persona-aware shell
    SHELL_LAYER_APPLICATION = 3,   // Application shell interface
    SHELL_LAYER_USER = 4,          // User interface shell
    SHELL_LAYER_QUANTUM = 5,       // Quantum operations shell (ChemOS)
    SHELL_LAYER_BRIDGE = 6,        // Cross-architecture bridge shell
    SHELL_LAYER_COUNT = 7
} shell_layer_type_t;

typedef enum {
    SHELL_INTERFACE_CLI = 0,       // Command-line interface
    SHELL_INTERFACE_GUI = 1,       // Graphical user interface
    SHELL_INTERFACE_TUI = 2,       // Terminal user interface
    SHELL_INTERFACE_API = 3,       // Programming API
    SHELL_INTERFACE_VR = 4,        // Virtual reality interface
    SHELL_INTERFACE_NEURAL = 5,    // Neural interface (future)
    SHELL_INTERFACE_COUNT = 6
} shell_interface_type_t;

typedef enum {
    SHELL_PRIVILEGE_GUEST = 0,     // Guest access
    SHELL_PRIVILEGE_USER = 1,      // Regular user
    SHELL_PRIVILEGE_POWER = 2,     // Power user
    SHELL_PRIVILEGE_ADMIN = 3,     // System administrator
    SHELL_PRIVILEGE_KERNEL = 4,    // Kernel-level access
    SHELL_PRIVILEGE_QUANTUM = 5,   // Quantum operations
    SHELL_PRIVILEGE_COUNT = 6
} shell_privilege_level_t;

//=============================================================================
// SHELL LAYER STRUCTURE
//=============================================================================

typedef struct {
    shell_layer_type_t layer_type;
    shell_interface_type_t interface_type;
    shell_privilege_level_t min_privilege;

    // Layer capabilities
    uint32_t capabilities;
    uint32_t persona_mask;         // Which personas support this layer

    // Layer state
    bool active;
    bool initialized;
    uint32_t session_count;

    // Performance metrics
    uint64_t commands_processed;
    uint64_t total_execution_time_us;
    uint32_t error_count;

    // Layer-specific data
    void* layer_data;
    size_t layer_data_size;

    // Function pointers for layer operations
    int (*init_layer)(tbos_shell_layer_t* layer);
    int (*cleanup_layer)(tbos_shell_layer_t* layer);
    int (*process_command)(tbos_shell_layer_t* layer, const char* command, char* response, size_t response_size);
    int (*handle_event)(tbos_shell_layer_t* layer, uint32_t event_type, void* event_data);

} tbos_shell_layer_t;

//=============================================================================
// SHELL PLUGIN SYSTEM
//=============================================================================

typedef enum {
    PLUGIN_TYPE_COMMAND = 0,       // Command handler plugin
    PLUGIN_TYPE_FILTER = 1,        // Input/output filter
    PLUGIN_TYPE_EXTENSION = 2,     // Shell extension
    PLUGIN_TYPE_INTEGRATION = 3,   // External system integration
    PLUGIN_TYPE_AI = 4,            // AI enhancement plugin
    PLUGIN_TYPE_SECURITY = 5,      // Security plugin
    PLUGIN_TYPE_COUNT = 6
} shell_plugin_type_t;

typedef struct {
    char name[64];
    char version[16];
    shell_plugin_type_t type;
    uint32_t layer_mask;           // Which layers this plugin supports
    uint32_t persona_mask;         // Which personas this plugin supports

    // Plugin state
    bool loaded;
    bool active;
    shell_privilege_level_t required_privilege;

    // Plugin metadata
    char description[256];
    char author[64];
    uint32_t api_version;

    // Plugin functions
    int (*plugin_init)(tbos_shell_plugin_t* plugin);
    int (*plugin_cleanup)(tbos_shell_plugin_t* plugin);
    int (*plugin_execute)(tbos_shell_plugin_t* plugin, const char* input, char* output, size_t output_size);
    int (*plugin_configure)(tbos_shell_plugin_t* plugin, const char* config);

    // Plugin data
    void* plugin_data;
    size_t plugin_data_size;

} tbos_shell_plugin_t;

//=============================================================================
// SHELL COMMAND ROUTING
//=============================================================================

typedef enum {
    COMMAND_SCOPE_LAYER = 0,       // Layer-specific command
    COMMAND_SCOPE_GLOBAL = 1,      // Global system command
    COMMAND_SCOPE_PERSONA = 2,     // Persona-specific command
    COMMAND_SCOPE_BRIDGE = 3,      // Cross-layer bridge command
    COMMAND_SCOPE_PLUGIN = 4,      // Plugin-provided command
    COMMAND_SCOPE_COUNT = 5
} command_scope_t;

typedef struct {
    char command[64];
    char alias[32];
    command_scope_t scope;
    shell_layer_type_t target_layer;
    shell_privilege_level_t required_privilege;

    // Command metadata
    char description[256];
    char usage[512];
    char examples[1024];

    // Command routing
    uint32_t persona_mask;
    uint32_t layer_mask;
    bool requires_quantum;

    // Command handler
    int (*handler)(const char* args, char* response, size_t response_size, void* context);

} shell_command_entry_t;

//=============================================================================
// SHELL MANAGER STRUCTURE
//=============================================================================

#define MAX_SHELL_LAYERS 8
#define MAX_SHELL_PLUGINS 64
#define MAX_SHELL_COMMANDS 256
#define MAX_SHELL_SESSIONS 32
#define MAX_COMMAND_HISTORY 1000

typedef struct {
    uint32_t session_id;
    shell_layer_type_t active_layers[MAX_SHELL_LAYERS];
    uint32_t active_layer_count;
    shell_interface_type_t interface_type;
    shell_privilege_level_t privilege_level;
    uint8_t current_persona;

    // Session state
    bool authenticated;
    char username[64];
    char working_directory[512];

    // Session history
    char command_history[MAX_COMMAND_HISTORY][256];
    uint32_t history_count;
    uint32_t history_index;

    // Session metrics
    uint64_t session_start_time;
    uint32_t commands_executed;
    uint32_t errors_encountered;

} shell_session_t;

struct tbos_shell_manager {
    // Core components
    tbos_shell_layer_t layers[MAX_SHELL_LAYERS];
    uint32_t layer_count;

    tbos_shell_plugin_t plugins[MAX_SHELL_PLUGINS];
    uint32_t plugin_count;

    shell_command_entry_t commands[MAX_SHELL_COMMANDS];
    uint32_t command_count;

    shell_session_t sessions[MAX_SHELL_SESSIONS];
    uint32_t session_count;

    // Shell manager state
    bool initialized;
    uint8_t current_persona;
    shell_layer_type_t default_layer;

    // Integration components
    void* ai_advisor;              // AI persona advisor integration
    void* steppps_framework;       // STEPPPS framework integration
    void* security_manager;        // Security verification integration
    void* energy_manager;          // ChemOS energy management integration
    void* bridge_manager;          // Cross-architecture bridge integration

    // Global shell configuration
    char shell_prompt[128];
    char shell_theme[64];
    bool auto_completion_enabled;
    bool command_suggestions_enabled;
    bool persona_auto_switch;

    // Performance monitoring
    uint64_t total_commands_processed;
    uint64_t total_execution_time_us;
    uint32_t total_errors;
    uint64_t manager_start_time;

    // Shell capabilities
    uint32_t supported_interfaces;
    uint32_t supported_personas;
    uint32_t supported_layers;
    bool quantum_capable;
    bool bridge_capable;
    bool ai_enhanced;
};

//=============================================================================
// SHELL ARCHITECTURE API
//=============================================================================

// Shell manager operations
int tbos_shell_manager_init(tbos_shell_manager_t* manager);
int tbos_shell_manager_cleanup(tbos_shell_manager_t* manager);
int tbos_shell_manager_start(tbos_shell_manager_t* manager);
int tbos_shell_manager_stop(tbos_shell_manager_t* manager);

// Layer management
int tbos_shell_register_layer(tbos_shell_manager_t* manager, tbos_shell_layer_t* layer);
int tbos_shell_activate_layer(tbos_shell_manager_t* manager, shell_layer_type_t layer_type);
int tbos_shell_deactivate_layer(tbos_shell_manager_t* manager, shell_layer_type_t layer_type);
tbos_shell_layer_t* tbos_shell_get_layer(tbos_shell_manager_t* manager, shell_layer_type_t layer_type);

// Plugin management
int tbos_shell_load_plugin(tbos_shell_manager_t* manager, const char* plugin_path);
int tbos_shell_unload_plugin(tbos_shell_manager_t* manager, const char* plugin_name);
int tbos_shell_activate_plugin(tbos_shell_manager_t* manager, const char* plugin_name);
tbos_shell_plugin_t* tbos_shell_get_plugin(tbos_shell_manager_t* manager, const char* plugin_name);

// Command management
int tbos_shell_register_command(tbos_shell_manager_t* manager, shell_command_entry_t* command);
int tbos_shell_execute_command(tbos_shell_manager_t* manager, uint32_t session_id, const char* command);
shell_command_entry_t* tbos_shell_find_command(tbos_shell_manager_t* manager, const char* command);
int tbos_shell_get_command_suggestions(tbos_shell_manager_t* manager, const char* partial, char** suggestions, int max_suggestions);

// Session management
uint32_t tbos_shell_create_session(tbos_shell_manager_t* manager, shell_interface_type_t interface);
int tbos_shell_destroy_session(tbos_shell_manager_t* manager, uint32_t session_id);
shell_session_t* tbos_shell_get_session(tbos_shell_manager_t* manager, uint32_t session_id);
int tbos_shell_authenticate_session(tbos_shell_manager_t* manager, uint32_t session_id, const char* credentials);

// Persona integration
int tbos_shell_switch_persona(tbos_shell_manager_t* manager, uint8_t new_persona);
int tbos_shell_get_persona_capabilities(tbos_shell_manager_t* manager, uint8_t persona, uint32_t* capabilities);
int tbos_shell_update_persona_layers(tbos_shell_manager_t* manager, uint8_t persona);

// Cross-architecture bridging
int tbos_shell_bridge_command(tbos_shell_manager_t* manager, uint8_t target_persona, const char* command);
int tbos_shell_get_bridge_status(tbos_shell_manager_t* manager, uint8_t persona1, uint8_t persona2);
int tbos_shell_establish_bridge(tbos_shell_manager_t* manager, uint8_t persona1, uint8_t persona2);

// Integration interfaces
int tbos_shell_integrate_ai_advisor(tbos_shell_manager_t* manager, void* ai_advisor);
int tbos_shell_integrate_steppps(tbos_shell_manager_t* manager, void* steppps_framework);
int tbos_shell_integrate_security(tbos_shell_manager_t* manager, void* security_manager);
int tbos_shell_integrate_energy(tbos_shell_manager_t* manager, void* energy_manager);

// Utility functions
const char* tbos_shell_layer_name(shell_layer_type_t layer);
const char* tbos_shell_interface_name(shell_interface_type_t interface);
const char* tbos_shell_privilege_name(shell_privilege_level_t privilege);
void tbos_shell_print_status(tbos_shell_manager_t* manager);
void tbos_shell_print_layer_info(tbos_shell_layer_t* layer);
void tbos_shell_print_plugin_info(tbos_shell_plugin_t* plugin);

// Error codes
#define TBOS_SHELL_SUCCESS                 0
#define TBOS_SHELL_ERROR_INVALID_PARAM    -1
#define TBOS_SHELL_ERROR_NOT_INITIALIZED  -2
#define TBOS_SHELL_ERROR_ALREADY_EXISTS   -3
#define TBOS_SHELL_ERROR_NOT_FOUND        -4
#define TBOS_SHELL_ERROR_PERMISSION       -5
#define TBOS_SHELL_ERROR_QUOTA_EXCEEDED   -6
#define TBOS_SHELL_ERROR_LAYER_CONFLICT   -7
#define TBOS_SHELL_ERROR_PLUGIN_FAILED    -8
#define TBOS_SHELL_ERROR_COMMAND_FAILED   -9
#define TBOS_SHELL_ERROR_SESSION_INVALID  -10
#define TBOS_SHELL_ERROR_BRIDGE_FAILED    -11
#define TBOS_SHELL_ERROR_QUANTUM_REQUIRED -12

#endif // TBOS_SHELL_ARCHITECTURE_H