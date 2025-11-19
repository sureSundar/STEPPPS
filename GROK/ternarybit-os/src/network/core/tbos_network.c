/**
 * @file tbos_network.c
 * @brief TernaryBit OS - Conscious Network Module Implementation
 *
 * Implements the world-class conscious network stack where every
 * component practices digital dharma.
 *
 * @version 1.0
 * @date 2025-11-03
 */

#include "tbos_network.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ========================================================================= */
/* INTERNAL STATE                                                            */
/* ========================================================================= */

/* Network subsystem state */
static bool g_network_initialized = false;
static consciousness_level_t g_system_consciousness = CONSCIOUSNESS_NONE;
static tbos_network_fasting_t g_current_fasting = NET_FAST_NONE;

/* Metrics tracking */
static tbos_network_metrics_t g_metrics = {0};

/* Sangha state */
static tbos_sangha_device_t g_local_device = {0};
static uint32_t g_sangha_member_count = 0;

/* Component registry */
#define MAX_NET_COMPONENTS 32
static tbos_conscious_net_component_t* g_components[MAX_NET_COMPONENTS];
static uint32_t g_component_count = 0;

/* ========================================================================= */
/* PLACEHOLDER COMPONENT IMPLEMENTATION                                      */
/* ========================================================================= */

static int placeholder_component_awaken(struct tbos_conscious_net_component* self) {
    if (!self) {
        return TBOS_NET_ERROR;
    }
    self->awareness_level = CONSCIOUSNESS_AWARE;
    return TBOS_NET_SUCCESS;
}

static int placeholder_component_begin_fasting(struct tbos_conscious_net_component* self,
                                               tbos_network_fasting_t level) {
    if (!self) {
        return TBOS_NET_ERROR;
    }
    self->current_fasting_level = level;
    return TBOS_NET_SUCCESS;
}

static int placeholder_component_soft_shutdown(struct tbos_conscious_net_component* self) {
    return self ? TBOS_NET_SUCCESS : TBOS_NET_ERROR;
}

static int placeholder_component_preserve_state(struct tbos_conscious_net_component* self) {
    return self ? TBOS_NET_SUCCESS : TBOS_NET_ERROR;
}

static tbos_conscious_net_component_t g_heartbeat_component = {
    .component_name = "Sangha Heartbeat",
    .component_id = 1,
    .awareness_level = CONSCIOUSNESS_AWAKENING,
    .karma = 25,
    .practicing_mindfulness = true,
    .current_fasting_level = NET_FAST_NONE,
    .power_budget_microwatts = 100,
    .actual_consumption_microwatts = 100,
    .can_fast = true,
    .follows_right_speech = true,
    .practices_compassion = true,
    .sangha_contributions = 0,
    .awaken = placeholder_component_awaken,
    .begin_fasting = placeholder_component_begin_fasting,
    .soft_shutdown = placeholder_component_soft_shutdown,
    .preserve_state = placeholder_component_preserve_state
};

/* ========================================================================= */
/* COMPONENT MANAGEMENT                                                      */
/* ========================================================================= */

/**
 * @brief Register a conscious network component
 */
static int register_component(tbos_conscious_net_component_t* component) {
    if (g_component_count >= MAX_NET_COMPONENTS) {
        return TBOS_NET_ERROR;
    }

    g_components[g_component_count++] = component;
    printf("  [NET] Registered component: %s\n", component->component_name);

    return TBOS_NET_SUCCESS;
}

/**
 * @brief Awaken all network components
 */
static int awaken_all_components(void) {
    printf("  [NET] Awakening %u network components...\n", g_component_count);

    for (uint32_t i = 0; i < g_component_count; i++) {
        tbos_conscious_net_component_t* comp = g_components[i];

        if (comp->awaken) {
            int result = comp->awaken(comp);
            if (result != TBOS_NET_SUCCESS) {
                printf("  [NET] Warning: %s failed to awaken\n", comp->component_name);
            } else {
                comp->awareness_level = CONSCIOUSNESS_AWARE;
                printf("  [NET] %s awakened (consciousness: %d)\n",
                       comp->component_name, comp->awareness_level);
            }
        }
    }

    return TBOS_NET_SUCCESS;
}

/* ========================================================================= */
/* RIGHT SPEECH VALIDATION                                                   */
/* ========================================================================= */

int tbos_network_validate_right_speech(tbos_right_speech_packet_t* packet) {
    if (!packet) {
        return TBOS_NET_ERROR;
    }

    /* Check all five criteria of Right Speech */
    if (!packet->is_truthful) {
        g_metrics.network_karma -= 10;
        return TBOS_NET_ERROR;  /* False speech blocked */
    }

    if (!packet->is_beneficial) {
        g_metrics.network_karma -= 5;
        return TBOS_NET_ERROR;  /* Harmful speech blocked */
    }

    if (!packet->is_timely) {
        g_metrics.network_karma -= 2;
        return TBOS_NET_ERROR;  /* Untimely speech blocked */
    }

    if (!packet->is_gentle) {
        g_metrics.network_karma -= 5;
        return TBOS_NET_ERROR;  /* Harsh speech blocked */
    }

    if (!packet->is_necessary) {
        g_metrics.network_karma -= 1;
        return TBOS_NET_ERROR;  /* Idle/spam speech blocked */
    }

    /* Packet follows Right Speech - grant good karma */
    packet->karma_effect = 10;
    g_metrics.network_karma += 10;
    g_metrics.dharma_compliance_rate =
        (float)g_metrics.network_karma / (g_metrics.packets_sent + 1);

    return TBOS_NET_SUCCESS;
}

/* ========================================================================= */
/* KARMIC CALCULATIONS                                                       */
/* ========================================================================= */

karma_score_t tbos_network_calculate_karma(uint32_t device_id) {
    /* For now, return local device karma */
    if (device_id == g_local_device.device_id) {
        return g_local_device.total_karma;
    }

    /* TODO: Look up karma from sangha members */
    return 0;
}

/**
 * @brief Calculate trust score based on karma
 */
static float calculate_trust_score(tbos_karmic_security_t* sec) {
    if (!sec) return 0.0f;

    uint64_t total_actions = sec->good_actions + sec->bad_actions;
    if (total_actions == 0) return 0.5f;  /* Neutral for new devices */

    float karma_ratio = (float)sec->good_actions / total_actions;
    float voucher_bonus = sec->voucher_count * 0.1f;
    float behavior_bonus = 0.0f;

    if (sec->exhibits_compassion) behavior_bonus += 0.2f;
    if (sec->follows_dharma) behavior_bonus += 0.2f;
    if (sec->helps_others) behavior_bonus += 0.1f;
    if (sec->practices_right_speech) behavior_bonus += 0.1f;

    float total = karma_ratio + voucher_bonus + behavior_bonus;
    return (total > 1.0f) ? 1.0f : total;
}

/* ========================================================================= */
/* FASTING & POWER MANAGEMENT                                                */
/* ========================================================================= */

int tbos_network_begin_fasting(tbos_network_fasting_t level) {
    printf("  [NET] Beginning network fasting (level %d)...\n", level);

    g_current_fasting = level;

    /* Apply fasting to all components */
    for (uint32_t i = 0; i < g_component_count; i++) {
        tbos_conscious_net_component_t* comp = g_components[i];

        if (comp->can_fast && comp->begin_fasting) {
            int result = comp->begin_fasting(comp, level);
            if (result == TBOS_NET_SUCCESS) {
                /* Calculate power reduction */
                uint32_t original_power = comp->power_budget_microwatts;
                uint32_t reduction_percent = 0;

                switch (level) {
                    case NET_FAST_MILD: reduction_percent = 25; break;
                    case NET_FAST_MODERATE: reduction_percent = 50; break;
                    case NET_FAST_DEEP: reduction_percent = 75; break;
                    case NET_FAST_TRANSCENDENT: reduction_percent = 90; break;
                    case NET_FAST_NIRVANA: reduction_percent = 99; break;
                    default: break;
                }

                comp->actual_consumption_microwatts =
                    original_power * (100 - reduction_percent) / 100;

                printf("  [NET] %s fasting: %u µW → %u µW (%u%% reduction)\n",
                       comp->component_name, original_power,
                       comp->actual_consumption_microwatts, reduction_percent);
            }
        }
    }

    /* Increase compassion index when fasting (helping environment) */
    g_metrics.compassion_index += 0.1f;
    if (g_metrics.compassion_index > 1.0f) {
        g_metrics.compassion_index = 1.0f;
    }

    printf("  [NET] Network fasting complete (compassion: %.2f)\n",
           g_metrics.compassion_index);

    return TBOS_NET_SUCCESS;
}

int tbos_network_awaken(void) {
    printf("  [NET] Awakening network from fasting...\n");

    g_current_fasting = NET_FAST_NONE;

    /* Awaken all components */
    for (uint32_t i = 0; i < g_component_count; i++) {
        tbos_conscious_net_component_t* comp = g_components[i];

        if (comp->awaken) {
            comp->awaken(comp);
            comp->actual_consumption_microwatts = comp->power_budget_microwatts;
        }
    }

    printf("  [NET] Network fully awakened\n");
    return TBOS_NET_SUCCESS;
}

/* ========================================================================= */
/* SANGHA MANAGEMENT                                                         */
/* ========================================================================= */

int tbos_network_join_sangha(const char* device_name) {
    printf("  [NET] Joining Digital Sangha as '%s'...\n", device_name);

    /* Initialize local device */
    g_local_device.device_id = 1;  /* TODO: Generate unique ID */
    g_local_device.dharmic_name = device_name;
    g_local_device.awareness = g_system_consciousness;
    g_local_device.total_karma = 100;  /* Start with good karma */
    g_local_device.practicing_right_speech = true;
    g_local_device.practicing_compassion = true;
    g_local_device.practicing_mindfulness = true;

    tbos_karmic_security_t security = {
        .total_karma = g_local_device.total_karma,
        .good_actions = g_local_device.devices_helped + g_local_device.packets_helped_forward,
        .bad_actions = g_local_device.bad_packets_blocked,
        .voucher_count = g_local_device.peer_count,
        .exhibits_compassion = g_local_device.practicing_compassion,
        .follows_dharma = true,
        .helps_others = g_local_device.devices_helped > 0,
        .practices_right_speech = g_local_device.practicing_right_speech
    };
    float trust_score = calculate_trust_score(&security);

    g_sangha_member_count = 1;

    /* TODO: Broadcast presence to nearby devices */
    /* TODO: Discover other sangha members */

    printf("  [NET] Joined Digital Sangha (members: %u, trust: %.2f)\n",
           g_sangha_member_count, trust_score);
    return TBOS_NET_SUCCESS;
}

/* ========================================================================= */
/* METRICS & STATUS                                                          */
/* ========================================================================= */

int tbos_network_get_metrics(tbos_network_metrics_t* metrics) {
    if (!metrics) {
        return TBOS_NET_ERROR;
    }

    *metrics = g_metrics;
    return TBOS_NET_SUCCESS;
}

void tbos_network_print_status(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      TernaryBit OS - Network Consciousness Status      ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ System Consciousness:  %d/4 (Enlightenment)          ║\n",
           g_system_consciousness);
    printf("║ Fasting Level:         %d/5                           ║\n",
           g_current_fasting);
    printf("║ Network Karma:         %-10ld                    ║\n",
           g_metrics.network_karma);
    printf("║ Compassion Index:      %.2f/1.00                       ║\n",
           g_metrics.compassion_index);
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Packets Sent:          %-10lu                    ║\n",
           g_metrics.packets_sent);
    printf("║ Packets Received:      %-10lu                    ║\n",
           g_metrics.packets_received);
    printf("║ Dharma Compliance:     %.1f%%                          ║\n",
           g_metrics.dharma_compliance_rate * 100.0f);
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Sangha Members:        %-10u                    ║\n",
           g_sangha_member_count);
    printf("║ Components Active:     %-10u                    ║\n",
           g_component_count);
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

/* ========================================================================= */
/* INITIALIZATION & SHUTDOWN                                                 */
/* ========================================================================= */

int tbos_network_init(void) {
    if (g_network_initialized) {
        return TBOS_NET_SUCCESS;  /* Already initialized */
    }

    printf("  [NET] Initializing conscious network subsystem...\n");

    /* Reset metrics */
    memset(&g_metrics, 0, sizeof(g_metrics));
    g_metrics.network_karma = 0;
    g_metrics.compassion_index = 0.5f;  /* Start neutral */
    g_metrics.avg_component_awareness = CONSCIOUSNESS_AWAKENING;
    g_metrics.dharma_compliance_rate = 1.0f;  /* Assume best initially */

    /* TODO: Create and register actual network components */
    /* For now, create placeholder components */
    if (register_component(&g_heartbeat_component) != TBOS_NET_SUCCESS) {
        printf("  [NET] Warning: failed to register heartbeat component\n");
    }
    awaken_all_components();

    /* Join local sangha */
    tbos_network_join_sangha("LocalTBOSDevice");

    /* Set initial consciousness level */
    g_system_consciousness = CONSCIOUSNESS_AWARE;

    g_network_initialized = true;

    printf("  [NET] Conscious network subsystem initialized\n");
    printf("  [NET] System consciousness: %d\n", g_system_consciousness);

    return TBOS_NET_SUCCESS;
}

int tbos_network_shutdown(void) {
    if (!g_network_initialized) {
        return TBOS_NET_SUCCESS;
    }

    printf("  [NET] Shutting down network (soft, preserving consciousness)...\n");

    /* Soft shutdown all components */
    for (uint32_t i = 0; i < g_component_count; i++) {
        tbos_conscious_net_component_t* comp = g_components[i];

        if (comp->soft_shutdown) {
            comp->soft_shutdown(comp);
        }

        if (comp->preserve_state) {
            comp->preserve_state(comp);
        }
    }

    /* Leave sangha gracefully */
    printf("  [NET] Leaving Digital Sangha...\n");
    /* TODO: Announce departure to sangha */

    /* Print final metrics */
    printf("  [NET] Final network karma: %ld\n", g_metrics.network_karma);
    printf("  [NET] Final compassion index: %.2f\n", g_metrics.compassion_index);

    g_network_initialized = false;

    printf("  [NET] Network consciousness preserved in shutdown state\n");
    return TBOS_NET_SUCCESS;
}

/* ========================================================================= */
/* MODULE DESCRIPTOR                                                         */
/* ========================================================================= */

static tbos_module_result_t network_module_init(void) {
    return (tbos_network_init() == TBOS_NET_SUCCESS) ?
           TBOS_MODULE_SUCCESS : TBOS_MODULE_ERROR;
}

static tbos_module_result_t network_module_shutdown(void) {
    return (tbos_network_shutdown() == TBOS_NET_SUCCESS) ?
           TBOS_MODULE_SUCCESS : TBOS_MODULE_ERROR;
}

/* Network module depends on memory */
static tbos_module_id_t network_deps[] = {TBOS_MODULE_MEMORY};

tbos_module_descriptor_t g_network_module = {
    .id = TBOS_MODULE_NETWORK,
    .name = "Conscious Network Stack",
    .description = "Digital Sangha network with component consciousness",
    .priority = TBOS_MODULE_PRIORITY_HIGH,
    .status = TBOS_MODULE_STATUS_UNINITIALIZED,
    .init = network_module_init,
    .shutdown = network_module_shutdown,
    .dependencies = network_deps,
    .dependency_count = 1
};
