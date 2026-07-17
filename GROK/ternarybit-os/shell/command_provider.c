#include "command_provider.h"

#include <stddef.h>

static const tbos_command_provider_t* providers[TBOS_SHELL_MAX_PROVIDERS];
static size_t provider_count;

void tbos_command_providers_init(void) {
    size_t index;

    for (index = 0; index < TBOS_SHELL_MAX_PROVIDERS; ++index) {
        providers[index] = NULL;
    }
    provider_count = 0;
}
int tbos_command_provider_register(const tbos_command_provider_t* provider) {
    if (!provider || !provider->name || !provider->capability ||
        !provider->execute) {
        return -1;
    }
    if (provider_count >= TBOS_SHELL_MAX_PROVIDERS) {
        return -2;
    }

    providers[provider_count++] = provider;
    return 0;
}

int tbos_command_provider_dispatch(
    const char* command,
    const char* arguments,
    int* command_status
) {
    size_t index;

    if (!command || !command_status) {
        return TBOS_COMMAND_NOT_HANDLED;
    }

    for (index = 0; index < provider_count; ++index) {
        int result = providers[index]->execute(
            providers[index]->context,
            command,
            arguments,
            command_status
        );
        if (result == TBOS_COMMAND_HANDLED) {
            return TBOS_COMMAND_HANDLED;
        }
    }

    return TBOS_COMMAND_NOT_HANDLED;
}

size_t tbos_command_provider_count(void) {
    return provider_count;
}

const tbos_command_provider_t* tbos_command_provider_at(size_t index) {
    if (index >= provider_count) {
        return NULL;
    }
    return providers[index];
}
