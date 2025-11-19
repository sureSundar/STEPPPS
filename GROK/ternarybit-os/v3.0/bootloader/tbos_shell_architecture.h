/*
 * TBOS v3.0 Multi-Layered Shell Architecture
 * Consolidates all components into integrated shell layers
 */

#ifndef TBOS_SHELL_ARCHITECTURE_H
#define TBOS_SHELL_ARCHITECTURE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

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

struct tbos_shell_layer {
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
    int (*init_layer)(struct tbos_shell_layer* layer);
    int (*cleanup_layer)(struct tbos_shell_layer* layer);
    int (*process_command)(struct tbos_shell_layer* layer, const char* command, char* response, size_t response_size);
    int (*handle_event)(struct tbos_shell_layer* layer, uint32_t event_type, void* event_data);

};

//=============================================================================
// SHELL PLUGIN SYSTEM
//=============================================================================

// Plugin types defined in tbos_shell_plugins.h to avoid duplication

// Shell plugin structure - lightweight version for architecture.h
// Full definition in tbos_shell_plugins.h
struct tbos_shell_plugin {
    char name[64];
    char version[16];
    uint32_t type;                 // Use uint32_t to avoid circular dependency
    uint32_t layer_mask;
    uint32_t persona_mask;
    bool loaded;
    bool active;
    shell_privilege_level_t required_privilege;
    char description[256];
    char author[64];
    uint32_t api_version;
    int (*plugin_init)(struct tbos_shell_plugin* plugin);
    int (*plugin_cleanup)(struct tbos_shell_plugin* plugin);
    int (*plugin_execute)(struct tbos_shell_plugin* plugin, void* context);
    void* plugin_data;
    size_t plugin_data_size;
};

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

// Additional utility functions from implementation
uint64_t tbos_get_time_us(void);
uint64_t tbos_get_timestamp_us(void);
const char* tbos_get_persona_name(uint8_t persona);
uint8_t tbos_get_persona_id_by_name(const char* name);
void tbos_shell_parse_command(const char* command, char* cmd_name, char* cmd_args);
shell_command_entry_t* tbos_shell_find_command(tbos_shell_manager_t* manager, const char* cmd_name);

// Layer initialization functions
int tbos_shell_init_kernel_layer(tbos_shell_layer_t* layer);
int tbos_shell_init_system_layer(tbos_shell_layer_t* layer);
int tbos_shell_init_persona_layer(tbos_shell_layer_t* layer);
int tbos_shell_init_application_layer(tbos_shell_layer_t* layer);
int tbos_shell_init_user_layer(tbos_shell_layer_t* layer);
int tbos_shell_init_quantum_layer(tbos_shell_layer_t* layer);
int tbos_shell_init_bridge_layer(tbos_shell_layer_t* layer);

// Layer command processors
int tbos_shell_process_kernel_command(tbos_shell_layer_t* layer, const char* cmd);
int tbos_shell_process_system_command(tbos_shell_layer_t* layer, const char* cmd);
int tbos_shell_process_persona_command(tbos_shell_layer_t* layer, const char* cmd);
int tbos_shell_process_application_command(tbos_shell_layer_t* layer, const char* cmd);
int tbos_shell_process_user_command(tbos_shell_layer_t* layer, const char* cmd);
int tbos_shell_process_quantum_command(tbos_shell_layer_t* layer, const char* cmd);
int tbos_shell_process_bridge_command(tbos_shell_layer_t* layer, const char* cmd);

// Plugin initialization functions
int tbos_plugin_ai_init(tbos_shell_plugin_t* plugin);
int tbos_plugin_ai_execute(tbos_shell_plugin_t* plugin, void* context);
int tbos_plugin_security_init(tbos_shell_plugin_t* plugin);
int tbos_plugin_security_execute(tbos_shell_plugin_t* plugin, void* context);
int tbos_plugin_chemos_init(tbos_shell_plugin_t* plugin);
int tbos_plugin_chemos_execute(tbos_shell_plugin_t* plugin, void* context);
int tbos_plugin_gui_init(tbos_shell_plugin_t* plugin);
int tbos_plugin_gui_execute(tbos_shell_plugin_t* plugin, void* context);

// Command handlers
int tbos_cmd_help(const char* args, char* response, size_t response_size, shell_session_t* session);
int tbos_cmd_persona(const char* args, char* response, size_t response_size, shell_session_t* session);
int tbos_cmd_gui(const char* args, char* response, size_t response_size, shell_session_t* session);
int tbos_cmd_quantum(const char* args, char* response, size_t response_size, shell_session_t* session);
int tbos_cmd_bridge(const char* args, char* response, size_t response_size, shell_session_t* session);
int tbos_cmd_shell(const char* args, char* response, size_t response_size, shell_session_t* session);
int tbos_cmd_status(const char* args, char* response, size_t response_size, shell_session_t* session);
int tbos_cmd_exit(const char* args, char* response, size_t response_size, shell_session_t* session);

// Internal functions
void tbos_shell_save_persona_state(tbos_shell_manager_t* manager, uint8_t persona);
void tbos_shell_restore_persona_state(tbos_shell_manager_t* manager, uint8_t persona);
void tbos_shell_update_persona_theme(tbos_shell_manager_t* manager, uint8_t persona);
void tbos_shell_activate_session_layers(shell_session_t* session);
void tbos_shell_start_background_services(tbos_shell_manager_t* manager);

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