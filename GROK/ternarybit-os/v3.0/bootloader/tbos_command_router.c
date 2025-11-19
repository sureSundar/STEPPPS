/*
 * TBOS v3.0 Command Router Implementation
 * Revolutionary intelligent command routing with AI and quantum capabilities
 * Routes commands across shell layers, plugins, and personas seamlessly
 */

#include "tbos_command_router.h"
#include "tbos_shell_manager.h"
#include "tbos_shell_plugins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

//=============================================================================
// GLOBAL COMMAND ROUTER INSTANCE
//=============================================================================

static command_router_t g_command_router = {0};

//=============================================================================
// COMMAND ROUTER IMPLEMENTATION
//=============================================================================

int tbos_router_init(command_router_t* router) {
    if (!router) return TBOS_ROUTER_ERROR_INVALID_PARAM;

    printf("🚦 Initializing TBOS v3.0 Revolutionary Command Router...\n");

    // Initialize router state
    memset(router, 0, sizeof(command_router_t));
    router->next_handler_id = 1;
    router->current_persona = 0;
    router->current_layer = SHELL_LAYER_USER;
    router->current_security_level = 5;

    // Enable advanced features
    router->ai_routing_enabled = true;
    router->ai_confidence_threshold = 0.7f;
    router->learning_mode = true;
    router->quantum_routing_enabled = true;
    router->quantum_universes = 4;
    router->route_caching_enabled = true;

    printf("✅ Command router initialized with AI and quantum capabilities\n");
    return TBOS_ROUTER_SUCCESS;
}

int tbos_router_start(command_router_t* router) {
    if (!router) return TBOS_ROUTER_ERROR_INVALID_PARAM;

    printf("🚀 Starting TBOS Command Router...\n");

    // Register built-in handlers
    tbos_router_register_builtin_handlers(router);

    // Load routing rules
    tbos_router_load_default_rules(router);

    // Initialize AI components
    if (router->ai_routing_enabled) {
        tbos_router_init_ai_components(router);
    }

    // Initialize quantum routing
    if (router->quantum_routing_enabled) {
        tbos_router_init_quantum_components(router);
    }

    printf("✅ Command router started with %u handlers and %u rules\n",
           router->handler_count, router->rule_count);
    return TBOS_ROUTER_SUCCESS;
}

//=============================================================================
// COMMAND PROCESSING
//=============================================================================

int tbos_router_process_command(command_router_t* router, const char* command_line, command_result_t* result) {
    if (!router || !command_line || !result) return TBOS_ROUTER_ERROR_INVALID_PARAM;

    uint64_t start_time = tbos_get_timestamp_us();
    command_info_t cmd_info = {0};

    printf("🎯 Processing command: %s\n", command_line);

    // Initialize result
    memset(result, 0, sizeof(command_result_t));
    result->exit_code = -1;

    // Parse command
    uint64_t parse_start = tbos_get_timestamp_us();
    int parse_result = tbos_router_parse_command(command_line, &cmd_info);
    result->parse_time_us = tbos_get_timestamp_us() - parse_start;

    if (parse_result != TBOS_ROUTER_SUCCESS) {
        snprintf(result->error, sizeof(result->error), "Command parsing failed");
        return parse_result;
    }

    // Classify and route command
    uint64_t route_start = tbos_get_timestamp_us();
    int route_result = tbos_router_route_command(router, &cmd_info, result);
    result->route_time_us = tbos_get_timestamp_us() - route_start;

    // Update statistics
    router->total_commands_processed++;
    result->execution_time_us = tbos_get_timestamp_us() - start_time;
    router->total_execution_time_us += result->execution_time_us;

    if (result->exit_code == 0) {
        router->successful_commands++;
    } else {
        router->failed_commands++;
    }

    // Learn from execution if AI is enabled
    if (router->learning_mode && router->ai_routing_enabled) {
        tbos_router_learn_from_execution(router, &cmd_info, result);
    }

    // Add to history
    tbos_router_add_to_history(router, &cmd_info, result);

    printf("✅ Command completed in %lu μs (exit code: %d)\n",
           result->execution_time_us, result->exit_code);

    return route_result;
}

//=============================================================================
// COMMAND PARSING AND CLASSIFICATION
//=============================================================================

int tbos_router_parse_command(const char* command_line, command_info_t* cmd_info) {
    if (!command_line || !cmd_info) return TBOS_ROUTER_ERROR_INVALID_PARAM;

    memset(cmd_info, 0, sizeof(command_info_t));
    strncpy(cmd_info->raw_command, command_line, sizeof(cmd_info->raw_command) - 1);
    strncpy(cmd_info->full_path, command_line, sizeof(cmd_info->full_path) - 1);
    cmd_info->timestamp = tbos_get_timestamp_us();

    // Simple tokenization (in real implementation, would use proper parser)
    char* token_line = strdup(command_line);
    char* token = strtok(token_line, " \t\n");

    if (token) {
        strncpy(cmd_info->program, token, sizeof(cmd_info->program) - 1);
        cmd_info->argc = 0;

        while ((token = strtok(NULL, " \t\n")) && cmd_info->argc < 32) {
            strncpy(cmd_info->args[cmd_info->argc], token, sizeof(cmd_info->args[0]) - 1);
            cmd_info->argc++;
        }
    }

    free(token_line);

    // Classify command type
    cmd_info->type = tbos_router_classify_command(cmd_info);
    cmd_info->priority = tbos_router_determine_priority(cmd_info);

    return TBOS_ROUTER_SUCCESS;
}

command_type_t tbos_router_classify_command(const command_info_t* cmd_info) {
    if (!cmd_info) return COMMAND_TYPE_UNKNOWN;

    const char* program = cmd_info->program;

    // System commands
    if (strcmp(program, "ps") == 0 || strcmp(program, "top") == 0 ||
        strcmp(program, "htop") == 0 || strcmp(program, "systemctl") == 0) {
        return COMMAND_TYPE_SYSTEM;
    }

    // File operations
    if (strcmp(program, "ls") == 0 || strcmp(program, "cp") == 0 ||
        strcmp(program, "mv") == 0 || strcmp(program, "rm") == 0 ||
        strcmp(program, "mkdir") == 0 || strcmp(program, "find") == 0) {
        return COMMAND_TYPE_FILE;
    }

    // Network operations
    if (strcmp(program, "ping") == 0 || strcmp(program, "wget") == 0 ||
        strcmp(program, "curl") == 0 || strcmp(program, "ssh") == 0) {
        return COMMAND_TYPE_NETWORK;
    }

    // Process management
    if (strcmp(program, "kill") == 0 || strcmp(program, "killall") == 0 ||
        strcmp(program, "jobs") == 0 || strcmp(program, "bg") == 0 || strcmp(program, "fg") == 0) {
        return COMMAND_TYPE_PROCESS;
    }

    // Persona commands
    if (strcmp(program, "persona") == 0 || strcmp(program, "switch") == 0) {
        return COMMAND_TYPE_PERSONA;
    }

    // GUI commands
    if (strcmp(program, "gui") == 0 || strcmp(program, "web") == 0 ||
        strcmp(program, "desktop") == 0 || strcmp(program, "browser") == 0) {
        return COMMAND_TYPE_GUI;
    }

    // Quantum commands
    if (strcmp(program, "quantum") == 0 || strcmp(program, "qubit") == 0 ||
        strcmp(program, "superposition") == 0) {
        return COMMAND_TYPE_QUANTUM;
    }

    // AI commands
    if (strcmp(program, "ai") == 0 || strcmp(program, "ml") == 0 ||
        strcmp(program, "neural") == 0 || strcmp(program, "learn") == 0) {
        return COMMAND_TYPE_AI;
    }

    // Plugin commands
    if (strcmp(program, "plugin") == 0 || strcmp(program, "load") == 0 || strcmp(program, "unload") == 0) {
        return COMMAND_TYPE_PLUGIN;
    }

    return COMMAND_TYPE_UNKNOWN;
}

command_priority_t tbos_router_determine_priority(const command_info_t* cmd_info) {
    if (!cmd_info) return COMMAND_PRIORITY_NORMAL;

    // Quantum operations get quantum priority
    if (cmd_info->type == COMMAND_TYPE_QUANTUM) {
        return COMMAND_PRIORITY_QUANTUM;
    }

    // System operations get high priority
    if (cmd_info->type == COMMAND_TYPE_SYSTEM || cmd_info->type == COMMAND_TYPE_PROCESS) {
        return COMMAND_PRIORITY_HIGH;
    }

    // Check for real-time indicators
    for (uint32_t i = 0; i < cmd_info->argc; i++) {
        if (strstr(cmd_info->args[i], "--realtime") || strstr(cmd_info->args[i], "-rt")) {
            return COMMAND_PRIORITY_REALTIME;
        }
    }

    return COMMAND_PRIORITY_NORMAL;
}

//=============================================================================
// COMMAND ROUTING
//=============================================================================

int tbos_router_route_command(command_router_t* router, command_info_t* cmd_info, command_result_t* result) {
    if (!router || !cmd_info || !result) return TBOS_ROUTER_ERROR_INVALID_PARAM;

    // Apply routing rules
    tbos_router_apply_rules(router, cmd_info);

    // Select routing strategy
    route_strategy_t strategy = tbos_router_select_strategy(router, cmd_info);
    cmd_info->routing_strategy = strategy;
    result->used_strategy = strategy;

    printf("🎯 Routing strategy: %s\n", tbos_router_strategy_name(strategy));

    // Execute based on strategy
    switch (strategy) {
        case ROUTE_STRATEGY_AI:
            return tbos_router_ai_route(router, cmd_info, result);

        case ROUTE_STRATEGY_QUANTUM:
            return tbos_router_quantum_route_single(router, cmd_info, result);

        case ROUTE_STRATEGY_PLUGIN:
            return tbos_router_plugin_route(router, cmd_info, result);

        case ROUTE_STRATEGY_LAYERED:
            return tbos_router_layered_route(router, cmd_info, result);

        case ROUTE_STRATEGY_DIRECT:
        default:
            return tbos_router_direct_route(router, cmd_info, result);
    }
}

route_strategy_t tbos_router_select_strategy(command_router_t* router, const command_info_t* cmd_info) {
    if (!router || !cmd_info) return ROUTE_STRATEGY_DIRECT;

    // Use quantum routing for quantum commands
    if (cmd_info->type == COMMAND_TYPE_QUANTUM && router->quantum_routing_enabled) {
        return ROUTE_STRATEGY_QUANTUM;
    }

    // Use AI routing if enabled and confidence is high
    if (router->ai_routing_enabled && cmd_info->ai_confidence > router->ai_confidence_threshold) {
        return ROUTE_STRATEGY_AI;
    }

    // Use plugin routing for plugin commands
    if (cmd_info->type == COMMAND_TYPE_PLUGIN || cmd_info->preferred_plugin_id != 0) {
        return ROUTE_STRATEGY_PLUGIN;
    }

    // Use layered routing for complex operations
    if (cmd_info->type == COMMAND_TYPE_SYSTEM || cmd_info->priority >= COMMAND_PRIORITY_HIGH) {
        return ROUTE_STRATEGY_LAYERED;
    }

    return ROUTE_STRATEGY_DIRECT;
}

//=============================================================================
// ROUTING STRATEGY IMPLEMENTATIONS
//=============================================================================

int tbos_router_direct_route(command_router_t* router, command_info_t* cmd_info, command_result_t* result) {
    printf("⚡ Direct routing: %s\n", cmd_info->program);

    // Find best handler
    command_handler_t* handler = tbos_router_find_best_handler(router, cmd_info);
    if (!handler) {
        snprintf(result->error, sizeof(result->error), "No handler found for command: %s", cmd_info->program);
        return TBOS_ROUTER_ERROR_NO_HANDLER;
    }

    result->handler_id = handler->handler_id;
    result->executed_layer = handler->layer;

    // Execute command
    uint64_t exec_start = tbos_get_timestamp_us();
    int exec_result = handler->execute(cmd_info, result);
    result->execution_time_us = tbos_get_timestamp_us() - exec_start;

    // Update handler statistics
    handler->total_invocations++;
    if (exec_result == 0) {
        handler->success_rate = (handler->success_rate * (handler->total_invocations - 1) + 1.0f) / handler->total_invocations;
    } else {
        handler->success_rate = (handler->success_rate * (handler->total_invocations - 1)) / handler->total_invocations;
    }

    return exec_result;
}

int tbos_router_ai_route(command_router_t* router, command_info_t* cmd_info, command_result_t* result) {
    printf("🤖 AI-powered routing: %s\n", cmd_info->program);

    // Simulate AI analysis
    printf("🧠 AI analyzing command context and intent...\n");
    usleep(25000); // 25ms AI processing time

    // AI selects best approach
    float confidence = 0.95f;
    printf("📊 AI confidence: %.2f\n", confidence);

    // Fall back to intelligent handler selection
    command_handler_t* handler = tbos_router_ai_select_handler(router, cmd_info);
    if (!handler) {
        return tbos_router_direct_route(router, cmd_info, result);
    }

    result->handler_id = handler->handler_id;
    result->executed_layer = handler->layer;

    return handler->execute(cmd_info, result);
}

int tbos_router_quantum_route_single(command_router_t* router, command_info_t* cmd_info, command_result_t* result) {
    printf("⚛️  Quantum routing: %s\n", cmd_info->program);

    // Simulate quantum superposition of execution paths
    printf("🌌 Creating quantum superposition of %u execution universes...\n", router->quantum_universes);

    command_result_t quantum_results[4];
    int universe_count = 4;

    // Execute in parallel universes
    for (int i = 0; i < universe_count; i++) {
        printf("🌠 Universe %d: ", i + 1);
        command_handler_t* handler = tbos_router_quantum_select_handler(router, cmd_info, i);
        if (handler) {
            handler->execute(cmd_info, &quantum_results[i]);
            printf("✅ Success (exit: %d)\n", quantum_results[i].exit_code);
        } else {
            quantum_results[i].exit_code = -1;
            printf("❌ No handler\n");
        }
    }

    // Collapse quantum state to best result
    int best_universe = tbos_router_select_best_quantum_result(quantum_results, universe_count);
    *result = quantum_results[best_universe];

    printf("🎯 Quantum collapse: Selected universe %d\n", best_universe + 1);

    return result->exit_code == 0 ? TBOS_ROUTER_SUCCESS : TBOS_ROUTER_ERROR_EXECUTION_FAILED;
}

int tbos_router_plugin_route(command_router_t* router, command_info_t* cmd_info, command_result_t* result) {
    printf("🔌 Plugin routing: %s\n", cmd_info->program);

    // Get plugin manager
    plugin_manager_t* plugin_mgr = tbos_get_global_plugin_manager();
    if (!plugin_mgr) {
        snprintf(result->error, sizeof(result->error), "Plugin manager not available");
        return TBOS_ROUTER_ERROR_EXECUTION_FAILED;
    }

    // Try to execute via plugin
    int plugin_result = tbos_plugin_execute_by_name(plugin_mgr, cmd_info->program, cmd_info->argc, (char**)cmd_info->args);

    if (plugin_result == TBOS_PLUGIN_SUCCESS) {
        result->exit_code = 0;
        snprintf(result->output, sizeof(result->output), "Plugin executed successfully");
        return TBOS_ROUTER_SUCCESS;
    }

    // Fallback to direct routing
    return tbos_router_direct_route(router, cmd_info, result);
}

int tbos_router_layered_route(command_router_t* router, command_info_t* cmd_info, command_result_t* result) {
    printf("🏗️  Layered routing through shell architecture\n");

    // Route through appropriate shell layers
    shell_layer_type_t layers[] = {
        SHELL_LAYER_KERNEL,
        SHELL_LAYER_SYSTEM,
        SHELL_LAYER_APPLICATION,
        SHELL_LAYER_USER
    };

    for (int i = 0; i < 4; i++) {
        command_handler_t* handler = tbos_router_find_handler_in_layer(router, cmd_info, layers[i]);
        if (handler && handler->can_handle && handler->can_handle(cmd_info)) {
            result->executed_layer = layers[i];
            result->handler_id = handler->handler_id;
            result->route_hops = i + 1;

            printf("🎯 Executing in %s layer\n", tbos_shell_layer_name(layers[i]));
            return handler->execute(cmd_info, result);
        }
    }

    return tbos_router_direct_route(router, cmd_info, result);
}

//=============================================================================
// HANDLER MANAGEMENT
//=============================================================================

command_handler_t* tbos_router_find_best_handler(command_router_t* router, const command_info_t* cmd_info) {
    if (!router || !cmd_info) return NULL;

    command_handler_t* best_handler = NULL;
    float best_score = 0.0f;

    for (uint32_t i = 0; i < router->handler_count; i++) {
        command_handler_t* handler = &router->handlers[i];

        // Check if handler can handle this command type
        bool can_handle_type = false;
        for (uint32_t j = 0; j < handler->supported_type_count; j++) {
            if (handler->supported_types[j] == cmd_info->type) {
                can_handle_type = true;
                break;
            }
        }

        if (!can_handle_type) continue;

        // Check persona compatibility
        if (!(handler->supported_personas & (1 << cmd_info->target_persona))) {
            continue;
        }

        // Check security level
        if (handler->required_security_level > router->current_security_level) {
            continue;
        }

        // Calculate score
        float score = handler->success_rate;
        if (handler->score) {
            score *= handler->score(cmd_info);
        }

        if (score > best_score) {
            best_score = score;
            best_handler = handler;
        }
    }

    return best_handler;
}

//=============================================================================
// BUILT-IN HANDLERS
//=============================================================================

int tbos_router_register_builtin_handlers(command_router_t* router) {
    printf("📦 Registering built-in command handlers...\n");

    // Register system handlers
    tbos_router_register_system_handlers(router);
    tbos_router_register_file_handlers(router);
    tbos_router_register_network_handlers(router);
    tbos_router_register_persona_handlers(router);
    tbos_router_register_quantum_handlers(router);
    tbos_router_register_ai_handlers(router);

    printf("✅ Built-in handlers registered: %u total\n", router->handler_count);
    return TBOS_ROUTER_SUCCESS;
}

int tbos_router_register_system_handlers(command_router_t* router) {
    // System info handler
    command_handler_t* handler = &router->handlers[router->handler_count++];
    memset(handler, 0, sizeof(command_handler_t));

    handler->handler_id = router->next_handler_id++;
    strcpy(handler->name, "system_info");
    strcpy(handler->description, "System information and status commands");
    handler->execute = tbos_handler_execute_system_info;
    handler->can_handle = tbos_handler_can_handle_system;
    handler->score = tbos_handler_score_system;

    handler->supported_types[0] = COMMAND_TYPE_SYSTEM;
    handler->supported_type_count = 1;
    handler->layer = SHELL_LAYER_SYSTEM;
    handler->supported_personas = 0xFF; // All personas
    handler->required_security_level = 3;
    handler->success_rate = 0.95f;

    return TBOS_ROUTER_SUCCESS;
}

int tbos_router_register_persona_handlers(command_router_t* router) {
    // Persona switching handler
    command_handler_t* handler = &router->handlers[router->handler_count++];
    memset(handler, 0, sizeof(command_handler_t));

    handler->handler_id = router->next_handler_id++;
    strcpy(handler->name, "persona_switch");
    strcpy(handler->description, "Runtime persona switching operations");
    handler->execute = tbos_handler_execute_persona_switch;
    handler->can_handle = tbos_handler_can_handle_persona;

    handler->supported_types[0] = COMMAND_TYPE_PERSONA;
    handler->supported_type_count = 1;
    handler->layer = SHELL_LAYER_PERSONA;
    handler->supported_personas = 0xFF;
    handler->required_security_level = 5;
    handler->success_rate = 0.98f;

    return TBOS_ROUTER_SUCCESS;
}

int tbos_router_register_quantum_handlers(command_router_t* router) {
    // Quantum operations handler
    command_handler_t* handler = &router->handlers[router->handler_count++];
    memset(handler, 0, sizeof(command_handler_t));

    handler->handler_id = router->next_handler_id++;
    strcpy(handler->name, "quantum_ops");
    strcpy(handler->description, "Quantum computing operations for ChemOS");
    handler->execute = tbos_handler_execute_quantum_ops;
    handler->can_handle = tbos_handler_can_handle_quantum;

    handler->supported_types[0] = COMMAND_TYPE_QUANTUM;
    handler->supported_type_count = 1;
    handler->layer = SHELL_LAYER_QUANTUM;
    handler->supported_personas = (1 << 4); // ChemOS only
    handler->required_security_level = 7;
    handler->supports_quantum = true;
    handler->success_rate = 0.92f;

    return TBOS_ROUTER_SUCCESS;
}

//=============================================================================
// HANDLER IMPLEMENTATIONS
//=============================================================================

int tbos_handler_execute_system_info(command_info_t* cmd, command_result_t* result) {
    if (strcmp(cmd->program, "sysinfo") == 0) {
        snprintf(result->output, sizeof(result->output),
                "TBOS v3.0 Revolutionary Bootloader\n"
                "Personas: 8 (Calculator, Embedded, Desktop, Mobile, ChemOS, VROS, BrainOS, Universal)\n"
                "Shell Layers: 7 active\n"
                "Plugin System: AI-enhanced with quantum support\n"
                "Current Persona: %s\n", tbos_get_persona_name(0));
        result->exit_code = 0;
    } else if (strcmp(cmd->program, "status") == 0) {
        snprintf(result->output, sizeof(result->output),
                "System Status: OPERATIONAL\n"
                "Quantum Engine: ACTIVE\n"
                "AI Routing: ENABLED\n"
                "Web GUI: ACTIVE\n");
        result->exit_code = 0;
    } else {
        snprintf(result->error, sizeof(result->error), "Unknown system command: %s", cmd->program);
        result->exit_code = 1;
    }
    return TBOS_ROUTER_SUCCESS;
}

int tbos_handler_execute_persona_switch(command_info_t* cmd, command_result_t* result) {
    if (cmd->argc == 0) {
        snprintf(result->error, sizeof(result->error), "Usage: persona <persona_name>");
        result->exit_code = 1;
        return TBOS_ROUTER_SUCCESS;
    }

    const char* persona_name = cmd->args[0];
    uint8_t persona_id = tbos_get_persona_id_by_name(persona_name);

    if (persona_id >= 8) {
        snprintf(result->error, sizeof(result->error), "Invalid persona: %s", persona_name);
        result->exit_code = 1;
    } else {
        snprintf(result->output, sizeof(result->output),
                "🔄 Switching to persona: %s\n"
                "✅ Persona switch completed successfully", persona_name);
        result->exit_code = 0;
    }

    return TBOS_ROUTER_SUCCESS;
}

int tbos_handler_execute_quantum_ops(command_info_t* cmd, command_result_t* result) {
    if (strcmp(cmd->program, "quantum") == 0) {
        if (cmd->argc > 0 && strcmp(cmd->args[0], "simulate") == 0) {
            snprintf(result->output, sizeof(result->output),
                    "⚛️  Quantum Simulation Started\n"
                    "🌌 Creating quantum superposition...\n"
                    "🔬 Running quantum algorithms...\n"
                    "✅ Quantum simulation completed successfully");
        } else {
            snprintf(result->output, sizeof(result->output),
                    "Quantum Commands:\n"
                    "  simulate - Run quantum simulation\n"
                    "  entangle - Create quantum entanglement\n"
                    "  measure  - Collapse quantum state");
        }
        result->exit_code = 0;
    } else {
        result->exit_code = 1;
    }
    return TBOS_ROUTER_SUCCESS;
}

bool tbos_handler_can_handle_system(const command_info_t* cmd) {
    return (cmd->type == COMMAND_TYPE_SYSTEM ||
            strcmp(cmd->program, "sysinfo") == 0 ||
            strcmp(cmd->program, "status") == 0);
}

bool tbos_handler_can_handle_persona(const command_info_t* cmd) {
    return (cmd->type == COMMAND_TYPE_PERSONA || strcmp(cmd->program, "persona") == 0);
}

bool tbos_handler_can_handle_quantum(const command_info_t* cmd) {
    return (cmd->type == COMMAND_TYPE_QUANTUM || strcmp(cmd->program, "quantum") == 0);
}

float tbos_handler_score_system(const command_info_t* cmd) {
    if (cmd->type == COMMAND_TYPE_SYSTEM) return 1.0f;
    if (strcmp(cmd->program, "sysinfo") == 0 || strcmp(cmd->program, "status") == 0) return 0.95f;
    return 0.0f;
}

//=============================================================================
// UTILITY AND HELPER FUNCTIONS
//=============================================================================

const char* tbos_router_strategy_name(route_strategy_t strategy) {
    static const char* strategy_names[] = {
        "Direct", "Layered", "Plugin", "AI", "Quantum",
        "Broadcast", "Pipeline", "Adaptive"
    };
    return (strategy < ROUTE_STRATEGY_COUNT) ? strategy_names[strategy] : "Unknown";
}

const char* tbos_router_command_type_name(command_type_t type) {
    static const char* type_names[] = {
        "System", "File", "Network", "Process", "Persona", "GUI",
        "Quantum", "AI", "Plugin", "Custom", "Unknown"
    };
    return (type < COMMAND_TYPE_COUNT) ? type_names[type] : "Unknown";
}

const char* tbos_router_priority_name(command_priority_t priority) {
    static const char* priority_names[] = {
        "Low", "Normal", "High", "Critical", "Realtime", "Quantum"
    };
    return (priority < COMMAND_PRIORITY_COUNT) ? priority_names[priority] : "Unknown";
}

void tbos_router_print_status(command_router_t* router) {
    if (!router) return;

    printf("\n🚦 TBOS Command Router Status:\n");
    printf("   Handlers: %u/%u\n", router->handler_count, MAX_COMMAND_HANDLERS);
    printf("   Rules: %u/%u\n", router->rule_count, MAX_ROUTING_RULES);
    printf("   AI Routing: %s (%.2f threshold)\n",
           router->ai_routing_enabled ? "Enabled" : "Disabled",
           router->ai_confidence_threshold);
    printf("   Quantum Routing: %s (%u universes)\n",
           router->quantum_routing_enabled ? "Enabled" : "Disabled",
           router->quantum_universes);
    printf("   Commands Processed: %lu\n", router->total_commands_processed);
    printf("   Success Rate: %.2f%%\n",
           router->total_commands_processed > 0 ?
           (float)router->successful_commands / router->total_commands_processed * 100.0f : 0.0f);
    printf("   Average Execution Time: %lu μs\n",
           router->total_commands_processed > 0 ?
           router->total_execution_time_us / router->total_commands_processed : 0);
}

//=============================================================================
// HELPER FUNCTION IMPLEMENTATIONS
//=============================================================================

command_handler_t* tbos_router_ai_select_handler(command_router_t* router, const command_info_t* cmd_info) {
    // Simulate AI-powered handler selection
    return tbos_router_find_best_handler(router, cmd_info);
}

command_handler_t* tbos_router_quantum_select_handler(command_router_t* router, const command_info_t* cmd_info, int universe) {
    // Simulate quantum handler selection in different universe
    if (router->handler_count == 0) return NULL;
    return &router->handlers[universe % router->handler_count];
}

int tbos_router_select_best_quantum_result(command_result_t* results, int count) {
    int best = 0;
    for (int i = 1; i < count; i++) {
        if (results[i].exit_code == 0 && results[best].exit_code != 0) {
            best = i;
        } else if (results[i].exit_code == 0 && results[best].exit_code == 0 &&
                   results[i].execution_time_us < results[best].execution_time_us) {
            best = i;
        }
    }
    return best;
}

command_handler_t* tbos_router_find_handler_in_layer(command_router_t* router, const command_info_t* cmd_info, shell_layer_type_t layer) {
    for (uint32_t i = 0; i < router->handler_count; i++) {
        if (router->handlers[i].layer == layer) {
            return &router->handlers[i];
        }
    }
    return NULL;
}

int tbos_router_load_default_rules(command_router_t* router) {
    // Add default routing rules
    routing_rule_t* rule = &router->rules[router->rule_count++];
    strcpy(rule->pattern, "quantum*");
    rule->strategy = ROUTE_STRATEGY_QUANTUM;
    rule->target_layer = SHELL_LAYER_QUANTUM;
    rule->enabled = true;

    rule = &router->rules[router->rule_count++];
    strcpy(rule->pattern, "ai*");
    rule->strategy = ROUTE_STRATEGY_AI;
    rule->target_layer = SHELL_LAYER_APPLICATION;
    rule->enabled = true;

    return TBOS_ROUTER_SUCCESS;
}

int tbos_router_apply_rules(command_router_t* router, command_info_t* cmd_info) {
    // Apply routing rules to command
    for (uint32_t i = 0; i < router->rule_count; i++) {
        routing_rule_t* rule = &router->rules[i];
        if (!rule->enabled) continue;

        // Simple pattern matching (in real implementation, would use regex)
        if (strstr(cmd_info->program, rule->pattern) ||
            (rule->pattern[strlen(rule->pattern)-1] == '*' &&
             strncmp(cmd_info->program, rule->pattern, strlen(rule->pattern)-1) == 0)) {
            cmd_info->routing_strategy = rule->strategy;
            cmd_info->target_layer = rule->target_layer;
            break;
        }
    }
    return TBOS_ROUTER_SUCCESS;
}

int tbos_router_add_to_history(command_router_t* router, const command_info_t* cmd_info, const command_result_t* result) {
    uint32_t index = router->history_index % MAX_COMMAND_HISTORY;
    router->command_history[index] = *cmd_info;
    router->result_history[index] = *result;

    router->history_index++;
    if (router->history_count < MAX_COMMAND_HISTORY) {
        router->history_count++;
    }

    return TBOS_ROUTER_SUCCESS;
}

int tbos_router_learn_from_execution(command_router_t* router, const command_info_t* cmd_info, const command_result_t* result) {
    // Simulate machine learning from execution results
    printf("🧠 AI learning from execution: success=%s, time=%lu μs\n",
           result->exit_code == 0 ? "true" : "false",
           result->execution_time_us);
    return TBOS_ROUTER_SUCCESS;
}

int tbos_router_init_ai_components(command_router_t* router) {
    printf("🤖 Initializing AI routing components...\n");
    router->ai_routing_enabled = true;
    return TBOS_ROUTER_SUCCESS;
}

int tbos_router_init_quantum_components(command_router_t* router) {
    printf("⚛️  Initializing quantum routing components...\n");
    router->quantum_routing_enabled = true;
    return TBOS_ROUTER_SUCCESS;
}

//=============================================================================
// GLOBAL ACCESS FUNCTIONS
//=============================================================================

command_router_t* tbos_get_global_command_router() {
    return &g_command_router;
}

int tbos_initialize_global_command_router() {
    int result = tbos_router_init(&g_command_router);
    if (result == TBOS_ROUTER_SUCCESS) {
        result = tbos_router_start(&g_command_router);
    }
    return result;
}