#ifndef TBOS_COMMAND_PROVIDER_H
#define TBOS_COMMAND_PROVIDER_H

#include <stddef.h>

#define TBOS_SHELL_MAX_PROVIDERS 8

typedef enum {
    TBOS_COMMAND_NOT_HANDLED = 0,
    TBOS_COMMAND_HANDLED = 1
} tbos_command_handling_t;

typedef int (*tbos_command_provider_execute_fn)(
    void* context,
    const char* command,
    const char* arguments,
    int* command_status
);

typedef struct {
    const char* name;
    const char* capability;
    tbos_command_provider_execute_fn execute;
    void* context;
} tbos_command_provider_t;

/* Reset the provider registry. Providers are invoked in registration order. */
void tbos_command_providers_init(void);

/*
 * Register a provider whose descriptor and strings remain valid for the life
 * of the shell runtime. Returns zero on success.
 */
int tbos_command_provider_register(const tbos_command_provider_t* provider);

/*
 * Ask registered providers to handle a command.
 * Returns TBOS_COMMAND_HANDLED or TBOS_COMMAND_NOT_HANDLED.
 */
int tbos_command_provider_dispatch(
    const char* command,
    const char* arguments,
    int* command_status
);

size_t tbos_command_provider_count(void);
const tbos_command_provider_t* tbos_command_provider_at(size_t index);

#endif /* TBOS_COMMAND_PROVIDER_H */
