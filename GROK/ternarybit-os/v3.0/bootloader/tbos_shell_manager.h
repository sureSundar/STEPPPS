/*
 * TBOS v3.0 Shell Manager Header
 * Core shell management functionality
 */

#ifndef TBOS_SHELL_MANAGER_H
#define TBOS_SHELL_MANAGER_H

#include "tbos_shell_architecture.h"

// Global shell manager instance
extern tbos_shell_manager_t* g_shell_manager;

// Initialize the shell manager
int tbos_shell_manager_initialize(void);

// Get the global shell manager instance
tbos_shell_manager_t* tbos_get_shell_manager(void);

// Simple command processing for hardware bridge
int tbos_process_simple_command(const char* command, char* response, size_t response_size);

// Persona switching for hardware bridge
int tbos_switch_persona_simple(uint8_t new_persona);

// Get system information
int tbos_get_system_info(char* response, size_t response_size);

#endif // TBOS_SHELL_MANAGER_H