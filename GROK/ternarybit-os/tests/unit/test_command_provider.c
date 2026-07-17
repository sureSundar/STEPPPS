#include "shell/command_provider.h"

#include <assert.h>
#include <string.h>

typedef struct {
    int calls;
} test_context_t;

static int test_execute(
    void* opaque,
    const char* command,
    const char* arguments,
    int* command_status
) {
    test_context_t* context = (test_context_t*)opaque;

    context->calls++;
    if (strcmp(command, "host-info") != 0) {
        return TBOS_COMMAND_NOT_HANDLED;
    }

    assert(arguments != NULL);
    assert(strcmp(arguments, "--short") == 0);
    *command_status = 23;
    return TBOS_COMMAND_HANDLED;
}
int main(void) {
    test_context_t context = {0};
    const tbos_command_provider_t provider = {
        .name = "test-host",
        .capability = "host.test",
        .execute = test_execute,
        .context = &context,
    };
    int status = -1;

    tbos_command_providers_init();
    assert(tbos_command_provider_count() == 0);
    assert(tbos_command_provider_register(NULL) == -1);
    assert(tbos_command_provider_register(&provider) == 0);
    assert(tbos_command_provider_count() == 1);
    assert(tbos_command_provider_at(0) == &provider);

    assert(tbos_command_provider_dispatch("unknown", NULL, &status) ==
           TBOS_COMMAND_NOT_HANDLED);
    assert(context.calls == 1);

    assert(tbos_command_provider_dispatch("host-info", "--short", &status) ==
           TBOS_COMMAND_HANDLED);
    assert(status == 23);
    assert(context.calls == 2);

    tbos_command_providers_init();
    assert(tbos_command_provider_count() == 0);
    return 0;
}
