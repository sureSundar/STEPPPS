/**
 * @file steppps_entity_demo.c
 * @brief Demonstration of STEPPPS Universal Entity Framework
 *
 * Shows how every entity in TBOS is:
 * - Identifiable (unique 128-bit STEPPPS ID)
 * - Addressable (STEPPPS URI scheme)
 * - Invokable (action intents)
 * - Interactable (cross-entity communication)
 * - Authenticated (karma-based trust)
 * - Authorized (consciousness-based permissions)
 *
 * Build:
 *   gcc -DHOST_BUILD -o steppps_entity_demo demos/steppps_entity_demo.c \
 *       src/core/steppps/steppps_entity.c -Iinclude -lpthread
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/tbos/steppps_entity.h"

/* ========================================================================= */
/* DEMO: ENTITY TYPES                                                         */
/* ========================================================================= */

static void demo_entity_types(void) {
    printf("\n");
    printf("=========================================================\n");
    printf("  STEPPPS UNIVERSAL ENTITY FRAMEWORK DEMO\n");
    printf("=========================================================\n");
    printf("\n");
    printf("Every entity in TBOS is:\n");
    printf("  - Identifiable  : Unique 128-bit STEPPPS ID\n");
    printf("  - Addressable   : steppps://[type]/[id]?params\n");
    printf("  - Invokable     : Action intents with karma cost\n");
    printf("  - Interactable  : Message passing between entities\n");
    printf("  - Authenticated : Karma-based trust verification\n");
    printf("  - Authorized    : Consciousness-based permissions\n");
    printf("\n");

    printf("---------------------------------------------------------\n");
    printf("  Entity Types (16 types supported)\n");
    printf("---------------------------------------------------------\n");

    for (int i = 0; i <= ENTITY_TYPE_CUSTOM; i++) {
        printf("  0x%X: %s\n", i, steppps_entity_type_name((steppps_entity_type_t)i));
    }
    printf("\n");
}

/* ========================================================================= */
/* DEMO: ENTITY CREATION & IDENTIFICATION                                     */
/* ========================================================================= */

static void demo_entity_identification(void) {
    printf("---------------------------------------------------------\n");
    printf("  Entity Identification (128-bit STEPPPS ID)\n");
    printf("---------------------------------------------------------\n");

    /* Create various entity types */
    steppps_entity_t* kernel = steppps_entity_create(ENTITY_TYPE_SYSTEM, "kernel");
    steppps_entity_t* user = steppps_entity_create(ENTITY_TYPE_USER, "alice");
    steppps_entity_t* file = steppps_entity_create(ENTITY_TYPE_FILE, "config.txt");
    steppps_entity_t* process = steppps_entity_create(ENTITY_TYPE_PROCESS, "shell");
    steppps_entity_t* device = steppps_entity_create(ENTITY_TYPE_DEVICE, "display0");
    steppps_entity_t* sangha = steppps_entity_create(ENTITY_TYPE_SANGHA, "node.mobile.1");

    printf("\nCreated Entities:\n");

    /* Print IDs */
    char id_str[33];
    steppps_entity_t* entities[] = {kernel, user, file, process, device, sangha};
    const char* names[] = {"kernel", "alice", "config.txt", "shell", "display0", "sangha"};

    for (int i = 0; i < 6; i++) {
        steppps_id_to_string(&entities[i]->id, id_str, sizeof(id_str));
        printf("  %-12s ID: %s\n", names[i], id_str);
    }

    printf("\nID Structure (128-bit):\n");
    printf("  [60-63]: Entity type (4 bits)\n");
    printf("  [56-59]: Version (4 bits)\n");
    printf("  [48-55]: Hardware tier (8 bits)\n");
    printf("  [16-47]: Timestamp since TBOS epoch (32 bits)\n");
    printf("  [0-15]:  Space hash (location/device)\n");
    printf("  [low 48]: Sequence + Random (uniqueness)\n");
    printf("\n");

    /* Cleanup */
    for (int i = 0; i < 6; i++) {
        steppps_entity_destroy(entities[i]);
    }
}

/* ========================================================================= */
/* DEMO: URI ADDRESSING                                                       */
/* ========================================================================= */

static void demo_uri_addressing(void) {
    printf("---------------------------------------------------------\n");
    printf("  URI Addressing (steppps:// scheme)\n");
    printf("---------------------------------------------------------\n");

    printf("\nSTEPPPS URI Format:\n");
    printf("  steppps://[entity-type]/[id-or-name]?[STEPPPS-params]\n");
    printf("\n");

    printf("Query Parameters (STEPPPS dimensions):\n");
    printf("  s=space      : Location context\n");
    printf("  t=time       : Temporal context\n");
    printf("  e=event      : Event context\n");
    printf("  p=psychology : Consciousness context\n");
    printf("  action=      : Requested action\n");
    printf("\n");

    /* Parse example URIs */
    printf("Parsing Example URIs:\n\n");

    const char* uris[] = {
        "steppps://file/config.txt?s=local&action=read",
        "steppps://process/shell?action=execute&t=now",
        "steppps://sangha/node.mobile.12345?action=discover",
        "steppps://device/display0?p=aware&action=render",
        "steppps://user/alice?action=authenticate"
    };

    for (int i = 0; i < 5; i++) {
        steppps_uri_t uri;
        if (steppps_uri_parse(uris[i], &uri) == 0) {
            printf("  URI: %s\n", uris[i]);
            printf("    Type:   %s\n", steppps_entity_type_name(uri.type));
            printf("    Name:   %s\n", uri.name);
            if (uri.space[0]) printf("    Space:  %s\n", uri.space);
            if (uri.time[0]) printf("    Time:   %s\n", uri.time);
            if (uri.psychology[0]) printf("    Psych:  %s\n", uri.psychology);
            if (uri.action[0]) printf("    Action: %s\n", uri.action);
            printf("\n");
        }
    }
}

/* ========================================================================= */
/* DEMO: ACTION INTENTS (INVOCATION)                                          */
/* ========================================================================= */

static void demo_action_intents(void) {
    printf("---------------------------------------------------------\n");
    printf("  Action Intents (Invocation)\n");
    printf("---------------------------------------------------------\n");

    printf("\nEach entity advertises intents (what it can do):\n");
    printf("  - Intent name and description\n");
    printf("  - Required capabilities (CAPABILITY_READ, etc.)\n");
    printf("  - Minimum consciousness level\n");
    printf("  - Karma cost and reward\n\n");

    /* Create a file entity with intents */
    steppps_entity_t* file = steppps_entity_create(ENTITY_TYPE_FILE, "important.doc");

    /* Register read intent */
    steppps_intent_t read_intent = {
        .name = "read",
        .description = "Read file contents",
        .required_caps = CAPABILITY_READ,
        .min_consciousness = CONSCIOUSNESS_REACTIVE,
        .karma_cost = 0,
        .karma_reward = 1,
        .param_count = 0
    };
    steppps_register_intent(file, &read_intent);

    /* Register write intent (needs more karma) */
    steppps_intent_t write_intent = {
        .name = "write",
        .description = "Write to file",
        .required_caps = CAPABILITY_WRITE,
        .min_consciousness = CONSCIOUSNESS_AWARE,
        .karma_cost = 5,
        .karma_reward = 10,
        .param_count = 1
    };
    strncpy(write_intent.param_names[0], "content", 32);
    strncpy(write_intent.param_types[0], "string", 16);
    write_intent.param_required[0] = true;
    steppps_register_intent(file, &write_intent);

    /* Register delete intent (highest requirements) */
    steppps_intent_t delete_intent = {
        .name = "delete",
        .description = "Delete file",
        .required_caps = CAPABILITY_DELETE | CAPABILITY_ADMIN,
        .min_consciousness = CONSCIOUSNESS_MINDFUL,
        .karma_cost = 20,
        .karma_reward = 5,
        .param_count = 0
    };
    steppps_register_intent(file, &delete_intent);

    printf("File Entity: %s\n", file->name);
    printf("Registered Intents:\n");

    for (int i = 0; i < file->intent_count; i++) {
        steppps_intent_t* intent = &file->intents[i];
        printf("\n  Intent: %s\n", intent->name);
        printf("    Description: %s\n", intent->description);
        printf("    Requires:    %s consciousness\n",
               steppps_consciousness_name(intent->min_consciousness));
        printf("    Karma Cost:  %lld\n", (long long)intent->karma_cost);
        printf("    Karma Reward: %lld\n", (long long)intent->karma_reward);
    }
    printf("\n");

    steppps_entity_destroy(file);
}

/* ========================================================================= */
/* DEMO: AUTHENTICATION & AUTHORIZATION                                       */
/* ========================================================================= */

static void demo_auth(void) {
    printf("---------------------------------------------------------\n");
    printf("  Authentication & Authorization\n");
    printf("---------------------------------------------------------\n");

    printf("\nAuthentication is karma-based:\n");
    printf("  - Trust score derived from karma history\n");
    printf("  - Higher consciousness = more trust\n");
    printf("\n");

    printf("Authorization is capability-based:\n");
    printf("  - CAPABILITY_READ      = 0x0001\n");
    printf("  - CAPABILITY_WRITE     = 0x0002\n");
    printf("  - CAPABILITY_EXECUTE   = 0x0004\n");
    printf("  - CAPABILITY_DELETE    = 0x0008\n");
    printf("  - CAPABILITY_INVOKE    = 0x0010\n");
    printf("  - CAPABILITY_INTERACT  = 0x0020\n");
    printf("  - CAPABILITY_DISCOVER  = 0x0100\n");
    printf("  - CAPABILITY_NETWORK   = 0x0400\n");
    printf("  - CAPABILITY_SANGHA    = 0x0800\n");
    printf("  - CAPABILITY_ADMIN     = 0x1000\n");
    printf("\n");

    /* Create entities with different karma */
    steppps_entity_t* newbie = steppps_entity_create(ENTITY_TYPE_USER, "newbie");
    steppps_entity_t* veteran = steppps_entity_create(ENTITY_TYPE_USER, "veteran");
    steppps_entity_t* sage = steppps_entity_create(ENTITY_TYPE_USER, "sage");

    /* Award karma */
    for (int i = 0; i < 50; i++) {
        steppps_karma_award(newbie, 2, "action");
    }

    for (int i = 0; i < 200; i++) {
        steppps_karma_award(veteran, 10, "helping");
    }

    for (int i = 0; i < 500; i++) {
        steppps_karma_award(sage, 100, "enlightening");
    }

    /* Evolve consciousness */
    steppps_consciousness_evolve(newbie);
    steppps_consciousness_evolve(veteran);
    steppps_consciousness_evolve(veteran);
    steppps_consciousness_evolve(sage);
    steppps_consciousness_evolve(sage);
    steppps_consciousness_evolve(sage);
    steppps_consciousness_evolve(sage);
    steppps_consciousness_evolve(sage);

    printf("User Karma & Consciousness:\n\n");

    steppps_entity_t* users[] = {newbie, veteran, sage};
    for (int i = 0; i < 3; i++) {
        printf("  %-10s Karma: %-6lld  Trust: %.2f  Consciousness: %s\n",
               users[i]->name,
               (long long)users[i]->karma.total,
               users[i]->karma.trust_score,
               steppps_consciousness_name(users[i]->consciousness));
    }
    printf("\n");

    /* Create a restricted resource */
    steppps_entity_t* secret = steppps_entity_create(ENTITY_TYPE_FILE, "secret.key");
    secret->capabilities = CAPABILITY_READ | CAPABILITY_ADMIN;

    printf("Checking access to secret.key (requires CAPABILITY_ADMIN):\n");
    printf("  newbie:  %s\n", steppps_authorize(newbie, CAPABILITY_ADMIN) ? "GRANTED" : "DENIED");
    printf("  veteran: %s\n", steppps_authorize(veteran, CAPABILITY_ADMIN) ? "GRANTED" : "DENIED");

    /* Grant admin to sage */
    steppps_grant_capability(sage, CAPABILITY_ADMIN);
    printf("  sage:    %s (has ADMIN capability)\n",
           steppps_authorize(sage, CAPABILITY_ADMIN) ? "GRANTED" : "DENIED");
    printf("\n");

    /* Cleanup */
    steppps_entity_destroy(newbie);
    steppps_entity_destroy(veteran);
    steppps_entity_destroy(sage);
    steppps_entity_destroy(secret);
}

/* ========================================================================= */
/* DEMO: INVOCATION FLOW                                                      */
/* ========================================================================= */

static void demo_invocation(void) {
    printf("---------------------------------------------------------\n");
    printf("  Invocation Flow\n");
    printf("---------------------------------------------------------\n");

    /* Create target entity */
    steppps_entity_t* service = steppps_entity_create(ENTITY_TYPE_SERVICE, "calculator");

    /* Register add intent */
    steppps_intent_t add_intent = {
        .name = "add",
        .description = "Add two numbers",
        .required_caps = CAPABILITY_INVOKE,
        .min_consciousness = CONSCIOUSNESS_REACTIVE,
        .karma_cost = 1,
        .karma_reward = 2,
        .param_count = 2
    };
    strncpy(add_intent.param_names[0], "a", 32);
    strncpy(add_intent.param_names[1], "b", 32);
    strncpy(add_intent.param_types[0], "int", 16);
    strncpy(add_intent.param_types[1], "int", 16);
    add_intent.param_required[0] = true;
    add_intent.param_required[1] = true;
    steppps_register_intent(service, &add_intent);

    /* Create caller */
    steppps_entity_t* caller = steppps_entity_create(ENTITY_TYPE_PROCESS, "calc_client");
    steppps_karma_award(caller, 100, "initial");

    printf("\nInvocation Request:\n");
    printf("  Source: %s (karma: %lld, consciousness: %s)\n",
           caller->name, (long long)caller->karma.total,
           steppps_consciousness_name(caller->consciousness));
    printf("  Target: steppps://service/calculator?action=add\n");
    printf("  Intent: add(a=5, b=3)\n");
    printf("\n");

    /* Build URI */
    steppps_uri_t target_uri;
    steppps_uri_build(service, &target_uri);
    strncpy(target_uri.action, "add", sizeof(target_uri.action) - 1);

    /* Build invocation */
    steppps_invocation_t inv = {
        .source = caller->id,
        .target = service->id,
        .consciousness = caller->consciousness,
        .karma = caller->karma,
        .param_count = 2,
        .async = false
    };
    strncpy(inv.intent_name, "add", sizeof(inv.intent_name) - 1);
    strncpy(inv.param_values[0], "5", 256);
    strncpy(inv.param_values[1], "3", 256);

    /* Invoke */
    steppps_result_t result;
    int rc = steppps_invoke(&target_uri, &inv, &result);

    printf("Invocation Result:\n");
    printf("  Success: %s\n", result.success ? "Yes" : "No");
    if (!result.success) {
        printf("  Error:   %s\n", result.error_message);
    }
    printf("  Karma Change: %+lld\n", (long long)result.karma_change);
    printf("  Exec Time:    %llu us\n", (unsigned long long)result.execution_time_us);
    printf("\n");

    /* Try invocation with insufficient consciousness */
    steppps_intent_t admin_intent = {
        .name = "admin_op",
        .description = "Administrative operation",
        .required_caps = CAPABILITY_ADMIN,
        .min_consciousness = CONSCIOUSNESS_MINDFUL,
        .karma_cost = 50,
        .karma_reward = 100,
        .param_count = 0
    };
    steppps_register_intent(service, &admin_intent);

    printf("Attempting admin_op (requires Mindful consciousness):\n");
    strncpy(inv.intent_name, "admin_op", sizeof(inv.intent_name) - 1);
    inv.param_count = 0;

    rc = steppps_invoke(&target_uri, &inv, &result);
    printf("  Success: %s\n", result.success ? "Yes" : "No");
    printf("  Error:   %s\n", result.error_message);
    printf("\n");

    steppps_entity_destroy(service);
    steppps_entity_destroy(caller);
}

/* ========================================================================= */
/* DEMO: SANGHA DISCOVERY                                                     */
/* ========================================================================= */

static void demo_sangha_discovery(void) {
    printf("---------------------------------------------------------\n");
    printf("  Sangha Discovery (Cross-Device Interaction)\n");
    printf("---------------------------------------------------------\n");

    printf("\nSangha nodes can:\n");
    printf("  1. Announce themselves to the network\n");
    printf("  2. Discover other TBOS instances\n");
    printf("  3. Query available intents on remote nodes\n");
    printf("  4. Invoke intents across devices\n");
    printf("\n");

    printf("Sangha Discovery Protocol:\n");
    printf("  Port: 7805 (UDP broadcast)\n");
    printf("  Messages: announce, query, response\n");
    printf("\n");

    printf("Example Sangha nodes:\n");

    /* Simulate discovered nodes */
    sangha_node_t nodes[] = {
        {
            .name = "desktop-main",
            .device_type = "desktop",
            .address = "192.168.1.100",
            .port = 7805,
            .online = true,
            .consciousness = CONSCIOUSNESS_AWARE,
            .trust_score = 0.85f
        },
        {
            .name = "mobile-alice",
            .device_type = "mobile",
            .address = "192.168.1.101",
            .port = 7805,
            .online = true,
            .consciousness = CONSCIOUSNESS_ADAPTIVE,
            .trust_score = 0.72f
        },
        {
            .name = "server-api",
            .device_type = "server",
            .address = "192.168.1.10",
            .port = 7805,
            .online = true,
            .consciousness = CONSCIOUSNESS_ENLIGHTENED,
            .trust_score = 0.98f
        }
    };

    for (int i = 0; i < 3; i++) {
        printf("\n  Node: %s (%s)\n", nodes[i].name, nodes[i].device_type);
        printf("    Address:       %s:%d\n", nodes[i].address, nodes[i].port);
        printf("    Online:        %s\n", nodes[i].online ? "Yes" : "No");
        printf("    Consciousness: %s\n", steppps_consciousness_name(nodes[i].consciousness));
        printf("    Trust:         %.2f\n", nodes[i].trust_score);
    }
    printf("\n");

    printf("Cross-Device Intent Invocation:\n");
    printf("  steppps://sangha/mobile-alice?action=discover&intent=camera\n");
    printf("  steppps://sangha/server-api?action=invoke&intent=compute\n");
    printf("\n");
}

/* ========================================================================= */
/* DEMO: REGISTRY STATISTICS                                                  */
/* ========================================================================= */

static void demo_registry(void) {
    printf("---------------------------------------------------------\n");
    printf("  Entity Registry\n");
    printf("---------------------------------------------------------\n");

    printf("\nThe Entity Registry is a global index of all entities.\n");
    printf("Features:\n");
    printf("  - O(1) lookup by ID (hash-based)\n");
    printf("  - Name-based lookup\n");
    printf("  - Type-based filtering\n");
    printf("  - Cache hit/miss tracking\n");
    printf("\n");

    /* Create many entities */
    steppps_entity_t* entities[20];
    char name[32];
    for (int i = 0; i < 20; i++) {
        snprintf(name, sizeof(name), "entity_%d", i);
        entities[i] = steppps_entity_create(ENTITY_TYPE_RESOURCE, name);
    }

    /* Perform lookups */
    for (int i = 0; i < 50; i++) {
        int idx = i % 20;
        steppps_registry_lookup(&entities[idx]->id);
    }

    /* Print stats */
    steppps_registry_print_stats();
    printf("\n");

    /* Cleanup */
    for (int i = 0; i < 20; i++) {
        steppps_entity_destroy(entities[i]);
    }
}

/* ========================================================================= */
/* DEMO: COMPLETE ENTITY EXAMPLE                                              */
/* ========================================================================= */

static void demo_complete_entity(void) {
    printf("---------------------------------------------------------\n");
    printf("  Complete Entity Example\n");
    printf("---------------------------------------------------------\n");

    /* Create a fully-featured entity */
    steppps_entity_t* my_app = steppps_entity_create(ENTITY_TYPE_PROCESS, "my_tbos_app");
    strncpy(my_app->description, "A sample TBOS application", sizeof(my_app->description) - 1);
    strncpy(my_app->location, "local://desktop", sizeof(my_app->location) - 1);

    /* Build up karma */
    for (int i = 0; i < 100; i++) {
        steppps_karma_award(my_app, 10, "helpful_action");
    }

    /* Evolve consciousness */
    while (steppps_consciousness_evolve(my_app) == 1) {
        printf("  Consciousness evolved to: %s\n",
               steppps_consciousness_name(my_app->consciousness));
    }

    /* Grant capabilities */
    steppps_grant_capability(my_app, CAPABILITY_NETWORK);
    steppps_grant_capability(my_app, CAPABILITY_SANGHA);
    steppps_grant_capability(my_app, CAPABILITY_DISCOVER);

    /* Register intents */
    steppps_intent_t hello_intent = {
        .name = "hello",
        .description = "Say hello to the world",
        .required_caps = CAPABILITY_INVOKE,
        .min_consciousness = CONSCIOUSNESS_DORMANT,
        .karma_cost = 0,
        .karma_reward = 1,
        .param_count = 0
    };
    steppps_register_intent(my_app, &hello_intent);

    steppps_intent_t compute_intent = {
        .name = "compute",
        .description = "Perform a computation",
        .required_caps = CAPABILITY_EXECUTE,
        .min_consciousness = CONSCIOUSNESS_ADAPTIVE,
        .karma_cost = 5,
        .karma_reward = 15,
        .param_count = 1
    };
    strncpy(compute_intent.param_names[0], "expression", 32);
    strncpy(compute_intent.param_types[0], "string", 16);
    compute_intent.param_required[0] = true;
    steppps_register_intent(my_app, &compute_intent);

    printf("\n");
    steppps_entity_print(my_app);
    printf("\n");

    printf("Available Intents:\n");
    for (int i = 0; i < my_app->intent_count; i++) {
        printf("  - %s: %s\n", my_app->intents[i].name, my_app->intents[i].description);
    }
    printf("\n");

    steppps_entity_destroy(my_app);
}

/* ========================================================================= */
/* MAIN                                                                       */
/* ========================================================================= */

int main(void) {
    /* Initialize entity framework */
    if (steppps_entity_init() != 0) {
        fprintf(stderr, "Failed to initialize STEPPPS Entity Framework\n");
        return 1;
    }

    /* Run demos */
    demo_entity_types();
    demo_entity_identification();
    demo_uri_addressing();
    demo_action_intents();
    demo_auth();
    demo_invocation();
    demo_sangha_discovery();
    demo_registry();
    demo_complete_entity();

    printf("=========================================================\n");
    printf("  STEPPPS Universal Entity Framework Demo Complete\n");
    printf("=========================================================\n");
    printf("\n");
    printf("Key Takeaways:\n");
    printf("  1. Every TBOS entity has a unique 128-bit ID\n");
    printf("  2. Entities are addressed via steppps:// URIs\n");
    printf("  3. Actions are invoked through intent registration\n");
    printf("  4. Authentication is based on karma & consciousness\n");
    printf("  5. Authorization uses capability flags\n");
    printf("  6. Cross-device interaction via Sangha discovery\n");
    printf("\n");
    printf("\"100%% backward compatibility and resourcefulness\"\n");
    printf("Each resource is active, intelligent, and interactable.\n");
    printf("\n");

    /* Shutdown */
    steppps_entity_shutdown();

    return 0;
}
