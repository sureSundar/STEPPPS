/*
 * TBOS v3.0 Consolidated Shell Manager Implementation
 * Integrates all shell layers with persona awareness and GUI support
 */

#include "tbos_shell_architecture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

// Global shell manager instance
static tbos_shell_manager_t g_shell_manager;
static bool g_manager_initialized = false;
static pthread_mutex_t g_manager_mutex = PTHREAD_MUTEX_INITIALIZER;

//=============================================================================
// SHELL MANAGER CORE OPERATIONS
//=============================================================================

int tbos_shell_manager_init(tbos_shell_manager_t* manager) {
    if (!manager) return TBOS_SHELL_ERROR_INVALID_PARAM;

    pthread_mutex_lock(&g_manager_mutex);

    if (g_manager_initialized) {
        pthread_mutex_unlock(&g_manager_mutex);
        return TBOS_SHELL_ERROR_ALREADY_EXISTS;
    }

    printf("🚀 Initializing TBOS v3.0 Consolidated Shell Manager...\n");

    // Initialize manager structure
    memset(manager, 0, sizeof(tbos_shell_manager_t));

    // Set default configuration
    manager->current_persona = 2; // x86 default
    manager->default_layer = SHELL_LAYER_USER;
    strcpy(manager->shell_prompt, "tbos> ");
    strcpy(manager->shell_theme, "revolutionary");
    manager->auto_completion_enabled = true;
    manager->command_suggestions_enabled = true;
    manager->persona_auto_switch = true;

    // Initialize capabilities
    manager->supported_interfaces = 0x3F; // All 6 interface types
    manager->supported_personas = 0xFF;   // All 8 personas
    manager->supported_layers = 0x7F;     // All 7 layers
    manager->quantum_capable = true;
    manager->bridge_capable = true;
    manager->ai_enhanced = true;

    // Initialize performance metrics
    manager->manager_start_time = (uint64_t)time(NULL);

    // Initialize default layers
    tbos_shell_register_default_layers(manager);

    // Load built-in plugins
    tbos_shell_load_builtin_plugins(manager);

    // Register default commands
    tbos_shell_register_default_commands(manager);

    manager->initialized = true;
    g_manager_initialized = true;

    printf("✅ Shell manager initialized with %d layers, %d plugins, %d commands\n",
           manager->layer_count, manager->plugin_count, manager->command_count);

    pthread_mutex_unlock(&g_manager_mutex);
    return TBOS_SHELL_SUCCESS;
}

int tbos_shell_manager_start(tbos_shell_manager_t* manager) {
    if (!manager || !manager->initialized) {
        return TBOS_SHELL_ERROR_NOT_INITIALIZED;
    }

    printf("\n🌟 Starting TBOS v3.0 Revolutionary Shell Manager\n");
    printf("=================================================\n");
    printf("🎭 Current Persona: %s\n", tbos_get_persona_name(manager->current_persona));
    printf("🔧 Active Layers: %d\n", manager->layer_count);
    printf("🔌 Loaded Plugins: %d\n", manager->plugin_count);
    printf("⚡ Available Commands: %d\n", manager->command_count);
    printf("🖥️ Supported Interfaces: CLI, GUI, TUI, API, VR, Neural\n");

    // Activate default layers for current persona
    tbos_shell_update_persona_layers(manager, manager->current_persona);

    // Start background services
    tbos_shell_start_background_services(manager);

    return TBOS_SHELL_SUCCESS;
}

//=============================================================================
// LAYER MANAGEMENT
//=============================================================================

int tbos_shell_register_default_layers(tbos_shell_manager_t* manager) {
    // Kernel Layer
    tbos_shell_layer_t kernel_layer = {
        .layer_type = SHELL_LAYER_KERNEL,
        .interface_type = SHELL_INTERFACE_CLI,
        .min_privilege = SHELL_PRIVILEGE_KERNEL,
        .capabilities = 0x01,
        .persona_mask = 0xFF, // All personas
        .active = true,
        .initialized = false,
        .init_layer = tbos_shell_init_kernel_layer,
        .process_command = tbos_shell_process_kernel_command
    };

    // System Layer
    tbos_shell_layer_t system_layer = {
        .layer_type = SHELL_LAYER_SYSTEM,
        .interface_type = SHELL_INTERFACE_CLI,
        .min_privilege = SHELL_PRIVILEGE_ADMIN,
        .capabilities = 0x02,
        .persona_mask = 0xFF,
        .active = true,
        .initialized = false,
        .init_layer = tbos_shell_init_system_layer,
        .process_command = tbos_shell_process_system_command
    };

    // Persona Layer
    tbos_shell_layer_t persona_layer = {
        .layer_type = SHELL_LAYER_PERSONA,
        .interface_type = SHELL_INTERFACE_GUI,
        .min_privilege = SHELL_PRIVILEGE_USER,
        .capabilities = 0x04,
        .persona_mask = 0xFF,
        .active = true,
        .initialized = false,
        .init_layer = tbos_shell_init_persona_layer,
        .process_command = tbos_shell_process_persona_command
    };

    // Application Layer
    tbos_shell_layer_t app_layer = {
        .layer_type = SHELL_LAYER_APPLICATION,
        .interface_type = SHELL_INTERFACE_GUI,
        .min_privilege = SHELL_PRIVILEGE_USER,
        .capabilities = 0x08,
        .persona_mask = 0xFF,
        .active = true,
        .initialized = false,
        .init_layer = tbos_shell_init_application_layer,
        .process_command = tbos_shell_process_application_command
    };

    // User Layer
    tbos_shell_layer_t user_layer = {
        .layer_type = SHELL_LAYER_USER,
        .interface_type = SHELL_INTERFACE_GUI,
        .min_privilege = SHELL_PRIVILEGE_USER,
        .capabilities = 0x10,
        .persona_mask = 0xFF,
        .active = true,
        .initialized = false,
        .init_layer = tbos_shell_init_user_layer,
        .process_command = tbos_shell_process_user_command
    };

    // Quantum Layer (ChemOS only)
    tbos_shell_layer_t quantum_layer = {
        .layer_type = SHELL_LAYER_QUANTUM,
        .interface_type = SHELL_INTERFACE_API,
        .min_privilege = SHELL_PRIVILEGE_QUANTUM,
        .capabilities = 0x20,
        .persona_mask = 0x80, // ChemOS only
        .active = false,
        .initialized = false,
        .init_layer = tbos_shell_init_quantum_layer,
        .process_command = tbos_shell_process_quantum_command
    };

    // Bridge Layer
    tbos_shell_layer_t bridge_layer = {
        .layer_type = SHELL_LAYER_BRIDGE,
        .interface_type = SHELL_INTERFACE_API,
        .min_privilege = SHELL_PRIVILEGE_POWER,
        .capabilities = 0x40,
        .persona_mask = 0xFF,
        .active = true,
        .initialized = false,
        .init_layer = tbos_shell_init_bridge_layer,
        .process_command = tbos_shell_process_bridge_command
    };

    // Register all layers
    manager->layers[0] = kernel_layer;
    manager->layers[1] = system_layer;
    manager->layers[2] = persona_layer;
    manager->layers[3] = app_layer;
    manager->layers[4] = user_layer;
    manager->layers[5] = quantum_layer;
    manager->layers[6] = bridge_layer;
    manager->layer_count = 7;

    // Initialize each layer
    for (int i = 0; i < manager->layer_count; i++) {
        if (manager->layers[i].init_layer) {
            manager->layers[i].init_layer(&manager->layers[i]);
            manager->layers[i].initialized = true;
        }
    }

    return TBOS_SHELL_SUCCESS;
}

//=============================================================================
// PLUGIN MANAGEMENT
//=============================================================================

int tbos_shell_load_builtin_plugins(tbos_shell_manager_t* manager) {
    // AI Enhancement Plugin
    tbos_shell_plugin_t ai_plugin = {
        .name = "AI_Advisor",
        .version = "3.0.0",
        .type = PLUGIN_TYPE_AI,
        .layer_mask = 0x7F,
        .persona_mask = 0xFF,
        .loaded = true,
        .active = true,
        .required_privilege = SHELL_PRIVILEGE_USER,
        .description = "AI-driven persona detection and optimization",
        .author = "TBOS Team",
        .api_version = 1,
        .plugin_init = tbos_plugin_ai_init,
        .plugin_execute = tbos_plugin_ai_execute
    };

    // Security Plugin
    tbos_shell_plugin_t security_plugin = {
        .name = "Security_Manager",
        .version = "3.0.0",
        .type = PLUGIN_TYPE_SECURITY,
        .layer_mask = 0x7F,
        .persona_mask = 0xFF,
        .loaded = true,
        .active = true,
        .required_privilege = SHELL_PRIVILEGE_ADMIN,
        .description = "RSA-4096/ECDSA-P384 security verification",
        .author = "TBOS Team",
        .api_version = 1,
        .plugin_init = tbos_plugin_security_init,
        .plugin_execute = tbos_plugin_security_execute
    };

    // ChemOS Plugin
    tbos_shell_plugin_t chemos_plugin = {
        .name = "ChemOS_Quantum",
        .version = "3.0.0",
        .type = PLUGIN_TYPE_EXTENSION,
        .layer_mask = 0x20, // Quantum layer only
        .persona_mask = 0x80, // ChemOS only
        .loaded = true,
        .active = true,
        .required_privilege = SHELL_PRIVILEGE_QUANTUM,
        .description = "Quantum chemical computing with fusion energy management",
        .author = "TBOS Team",
        .api_version = 1,
        .plugin_init = tbos_plugin_chemos_init,
        .plugin_execute = tbos_plugin_chemos_execute
    };

    // GUI Enhancement Plugin
    tbos_shell_plugin_t gui_plugin = {
        .name = "GUI_Framework",
        .version = "3.0.0",
        .type = PLUGIN_TYPE_EXTENSION,
        .layer_mask = 0x7C, // Persona, App, User layers
        .persona_mask = 0xFF,
        .loaded = true,
        .active = true,
        .required_privilege = SHELL_PRIVILEGE_USER,
        .description = "Advanced GUI framework with persona-aware themes",
        .author = "TBOS Team",
        .api_version = 1,
        .plugin_init = tbos_plugin_gui_init,
        .plugin_execute = tbos_plugin_gui_execute
    };

    // Register plugins
    manager->plugins[0] = ai_plugin;
    manager->plugins[1] = security_plugin;
    manager->plugins[2] = chemos_plugin;
    manager->plugins[3] = gui_plugin;
    manager->plugin_count = 4;

    // Initialize plugins
    for (int i = 0; i < manager->plugin_count; i++) {
        if (manager->plugins[i].plugin_init) {
            manager->plugins[i].plugin_init(&manager->plugins[i]);
        }
    }

    return TBOS_SHELL_SUCCESS;
}

//=============================================================================
// COMMAND MANAGEMENT
//=============================================================================

int tbos_shell_register_default_commands(tbos_shell_manager_t* manager) {
    shell_command_entry_t commands[] = {
        {"help", "?", COMMAND_SCOPE_GLOBAL, SHELL_LAYER_USER, SHELL_PRIVILEGE_GUEST,
         "Display available commands and usage information", "help [command]", "help\nhelp persona\nhelp quantum",
         0xFF, 0x7F, false, tbos_cmd_help},

        {"persona", "ps", COMMAND_SCOPE_PERSONA, SHELL_LAYER_PERSONA, SHELL_PRIVILEGE_USER,
         "Switch between hardware personas", "persona [persona_id]", "persona\npersona 7\npersona chemos",
         0xFF, 0x7F, false, tbos_cmd_persona},

        {"gui", "desktop", COMMAND_SCOPE_LAYER, SHELL_LAYER_USER, SHELL_PRIVILEGE_USER,
         "Launch graphical user interface", "gui [theme]", "gui\ngui dark\ngui quantum",
         0xFF, 0x7C, false, tbos_cmd_gui},

        {"quantum", "q", COMMAND_SCOPE_PERSONA, SHELL_LAYER_QUANTUM, SHELL_PRIVILEGE_QUANTUM,
         "Execute quantum operations", "quantum <operation> [args]", "quantum activate H\nquantum fusion start",
         0x80, 0x20, true, tbos_cmd_quantum},

        {"bridge", "br", COMMAND_SCOPE_BRIDGE, SHELL_LAYER_BRIDGE, SHELL_PRIVILEGE_POWER,
         "Manage cross-architecture bridges", "bridge <action> [personas]", "bridge status\nbridge connect x86 arm64",
         0xFF, 0x40, false, tbos_cmd_bridge},

        {"shell", "sh", COMMAND_SCOPE_LAYER, SHELL_LAYER_SYSTEM, SHELL_PRIVILEGE_USER,
         "Access different shell interfaces", "shell <interface>", "shell cli\nshell gui\nshell vr",
         0xFF, 0x7F, false, tbos_cmd_shell},

        {"status", "stat", COMMAND_SCOPE_GLOBAL, SHELL_LAYER_SYSTEM, SHELL_PRIVILEGE_USER,
         "Display system status and information", "status [component]", "status\nstatus persona\nstatus quantum",
         0xFF, 0x7F, false, tbos_cmd_status},

        {"exit", "quit", COMMAND_SCOPE_GLOBAL, SHELL_LAYER_USER, SHELL_PRIVILEGE_GUEST,
         "Exit the shell", "exit", "exit",
         0xFF, 0x7F, false, tbos_cmd_exit}
    };

    int command_count = sizeof(commands) / sizeof(shell_command_entry_t);

    for (int i = 0; i < command_count && i < MAX_SHELL_COMMANDS; i++) {
        manager->commands[i] = commands[i];
    }
    manager->command_count = command_count;

    return TBOS_SHELL_SUCCESS;
}

//=============================================================================
// SESSION MANAGEMENT
//=============================================================================

uint32_t tbos_shell_create_session(tbos_shell_manager_t* manager, shell_interface_type_t interface) {
    if (!manager || manager->session_count >= MAX_SHELL_SESSIONS) {
        return 0; // Invalid session ID
    }

    uint32_t session_id = manager->session_count + 1;
    shell_session_t* session = &manager->sessions[manager->session_count];

    // Initialize session
    memset(session, 0, sizeof(shell_session_t));
    session->session_id = session_id;
    session->interface_type = interface;
    session->privilege_level = SHELL_PRIVILEGE_USER;
    session->current_persona = manager->current_persona;
    session->authenticated = false;
    strcpy(session->username, "guest");
    strcpy(session->working_directory, "/home/user");
    session->session_start_time = (uint64_t)time(NULL);

    // Activate appropriate layers for interface
    tbos_shell_activate_session_layers(session);

    manager->session_count++;

    printf("📱 Created session %d with %s interface\n",
           session_id, tbos_shell_interface_name(interface));

    return session_id;
}

//=============================================================================
// PERSONA INTEGRATION
//=============================================================================

int tbos_shell_switch_persona(tbos_shell_manager_t* manager, uint8_t new_persona) {
    if (!manager || new_persona > 8) {
        return TBOS_SHELL_ERROR_INVALID_PARAM;
    }

    if (new_persona == manager->current_persona) {
        printf("Already running %s persona\n", tbos_get_persona_name(new_persona));
        return TBOS_SHELL_SUCCESS;
    }

    printf("\n🔄 Runtime Persona Switching: %s → %s\n",
           tbos_get_persona_name(manager->current_persona),
           tbos_get_persona_name(new_persona));

    printf("💾 Saving current shell state...\n");
    tbos_shell_save_persona_state(manager, manager->current_persona);

    printf("🧠 Updating layer configurations...\n");
    tbos_shell_update_persona_layers(manager, new_persona);

    printf("🎨 Switching interface themes...\n");
    tbos_shell_update_persona_theme(manager, new_persona);

    printf("⚡ Restoring persona-specific state...\n");
    tbos_shell_restore_persona_state(manager, new_persona);

    manager->current_persona = new_persona;

    printf("✅ Persona switch completed! Now running as %s\n",
           tbos_get_persona_name(new_persona));

    // Update all active sessions
    for (int i = 0; i < manager->session_count; i++) {
        manager->sessions[i].current_persona = new_persona;
    }

    return TBOS_SHELL_SUCCESS;
}

//=============================================================================
// COMMAND EXECUTION
//=============================================================================

int tbos_shell_execute_command(tbos_shell_manager_t* manager, uint32_t session_id, const char* command) {
    if (!manager || !command || session_id == 0) {
        return TBOS_SHELL_ERROR_INVALID_PARAM;
    }

    shell_session_t* session = tbos_shell_get_session(manager, session_id);
    if (!session) {
        return TBOS_SHELL_ERROR_SESSION_INVALID;
    }

    // Parse command
    char cmd_name[64];
    char cmd_args[512];
    tbos_shell_parse_command(command, cmd_name, cmd_args);

    // Find command
    shell_command_entry_t* cmd = tbos_shell_find_command(manager, cmd_name);
    if (!cmd) {
        printf("Unknown command: %s\n", cmd_name);
        printf("Type 'help' for available commands.\n");
        return TBOS_SHELL_ERROR_NOT_FOUND;
    }

    // Check persona support
    if (!(cmd->persona_mask & (1 << session->current_persona))) {
        printf("Command '%s' not supported in %s persona\n",
               cmd_name, tbos_get_persona_name(session->current_persona));
        return TBOS_SHELL_ERROR_PERMISSION;
    }

    // Check privilege level
    if (session->privilege_level < cmd->required_privilege) {
        printf("Insufficient privilege for command '%s'\n", cmd_name);
        return TBOS_SHELL_ERROR_PERMISSION;
    }

    // Execute command
    char response[2048];
    uint64_t start_time = tbos_get_time_us();

    int result = cmd->handler(cmd_args, response, sizeof(response), session);

    uint64_t execution_time = tbos_get_time_us() - start_time;

    // Update metrics
    session->commands_executed++;
    manager->total_commands_processed++;
    manager->total_execution_time_us += execution_time;

    if (result != 0) {
        session->errors_encountered++;
        manager->total_errors++;
    }

    // Add to history
    if (session->history_count < MAX_COMMAND_HISTORY) {
        strcpy(session->command_history[session->history_count], command);
        session->history_count++;
    }

    // Display response if any
    if (strlen(response) > 0) {
        printf("%s\n", response);
    }

    return result;
}

//=============================================================================
// UTILITY FUNCTIONS
//=============================================================================

const char* tbos_get_persona_name(uint8_t persona) {
    switch (persona) {
        case 0: return "Calculator";
        case 1: return "Embedded";
        case 2: return "x86";
        case 4: return "ARM64";
        case 5: return "RISC-V";
        case 6: return "Supercomputer";
        case 7: return "ChemOS";
        case 8: return "Universal";
        default: return "Unknown";
    }
}

const char* tbos_shell_layer_name(shell_layer_type_t layer) {
    switch (layer) {
        case SHELL_LAYER_KERNEL: return "Kernel";
        case SHELL_LAYER_SYSTEM: return "System";
        case SHELL_LAYER_PERSONA: return "Persona";
        case SHELL_LAYER_APPLICATION: return "Application";
        case SHELL_LAYER_USER: return "User";
        case SHELL_LAYER_QUANTUM: return "Quantum";
        case SHELL_LAYER_BRIDGE: return "Bridge";
        default: return "Unknown";
    }
}

const char* tbos_shell_interface_name(shell_interface_type_t interface) {
    switch (interface) {
        case SHELL_INTERFACE_CLI: return "CLI";
        case SHELL_INTERFACE_GUI: return "GUI";
        case SHELL_INTERFACE_TUI: return "TUI";
        case SHELL_INTERFACE_API: return "API";
        case SHELL_INTERFACE_VR: return "VR";
        case SHELL_INTERFACE_NEURAL: return "Neural";
        default: return "Unknown";
    }
}

void tbos_shell_print_status(tbos_shell_manager_t* manager) {
    printf("\n🖥️ TBOS v3.0 Shell Manager Status\n");
    printf("==================================\n");
    printf("🎭 Current Persona: %s\n", tbos_get_persona_name(manager->current_persona));
    printf("🔧 Active Layers: %d\n", manager->layer_count);
    printf("🔌 Loaded Plugins: %d\n", manager->plugin_count);
    printf("⚡ Available Commands: %d\n", manager->command_count);
    printf("📱 Active Sessions: %d\n", manager->session_count);
    printf("📊 Commands Processed: %lu\n", manager->total_commands_processed);
    printf("⏱️ Total Execution Time: %lu µs\n", manager->total_execution_time_us);
    printf("❌ Total Errors: %d\n", manager->total_errors);
    printf("🕒 Uptime: %lu seconds\n", (uint64_t)time(NULL) - manager->manager_start_time);
}

//=============================================================================
// MAIN DEMO FUNCTION
//=============================================================================

int main() {
    printf("🚀 TBOS v3.0 Consolidated Shell Manager Demo\n");
    printf("============================================\n\n");

    // Initialize shell manager
    int result = tbos_shell_manager_init(&g_shell_manager);
    if (result != TBOS_SHELL_SUCCESS) {
        printf("❌ Failed to initialize shell manager: %d\n", result);
        return 1;
    }

    // Start shell manager
    result = tbos_shell_manager_start(&g_shell_manager);
    if (result != TBOS_SHELL_SUCCESS) {
        printf("❌ Failed to start shell manager: %d\n", result);
        return 1;
    }

    // Create demo session
    uint32_t session_id = tbos_shell_create_session(&g_shell_manager, SHELL_INTERFACE_GUI);
    if (session_id == 0) {
        printf("❌ Failed to create session\n");
        return 1;
    }

    // Demo command execution
    printf("\n🧪 Demo Command Execution:\n");
    printf("==========================\n");

    const char* demo_commands[] = {
        "help",
        "status",
        "persona",
        "gui quantum",
        "persona 7",
        "quantum activate H",
        "bridge status",
        "shell vr"
    };

    for (int i = 0; i < 8; i++) {
        printf("\n> %s\n", demo_commands[i]);
        tbos_shell_execute_command(&g_shell_manager, session_id, demo_commands[i]);
        usleep(500000); // 0.5 second delay
    }

    // Print final status
    printf("\n");
    tbos_shell_print_status(&g_shell_manager);

    printf("\n✅ TBOS v3.0 Shell Manager demo completed successfully!\n");
    printf("🎯 Ready for GUI layer integration and advanced features.\n");

    return 0;
}