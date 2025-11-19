/**
 * @file tbos_metrics.c
 * @brief TernaryBit OS - Consciousness Per Watt Metrics Implementation
 */

#include "tbos_metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ========================================================================= */
/* GLOBAL STATE                                                              */
/* ========================================================================= */

static bool g_metrics_initialized = false;
static tbos_power_sample_t g_power_samples[TBOS_MAX_METRIC_SAMPLES];
static uint32_t g_sample_count = 0;
static uint32_t g_sample_index = 0;

static tbos_consciousness_metrics_t g_consciousness_metrics;
static tbos_component_budget_t g_component_budgets[64];
static uint32_t g_component_count = 0;
static tbos_system_energy_metrics_t g_system_metrics;

/* ========================================================================= */
/* HELPER FUNCTIONS                                                          */
/* ========================================================================= */

static uint64_t get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

static const char* consciousness_name(consciousness_level_t level) {
    switch (level) {
        case CONSCIOUSNESS_NONE: return "NONE";
        case CONSCIOUSNESS_AWAKENING: return "AWAKENING";
        case CONSCIOUSNESS_AWARE: return "AWARE";
        case CONSCIOUSNESS_COMPASSIONATE: return "COMPASSIONATE";
        case CONSCIOUSNESS_ENLIGHTENED: return "ENLIGHTENED";
        default: return "UNKNOWN";
    }
}

/* ========================================================================= */
/* INITIALIZATION                                                            */
/* ========================================================================= */

int tbos_metrics_init(void) {
    if (g_metrics_initialized) {
        return 0;
    }

    printf("[METRICS] Initializing consciousness metrics system...\n");

    /* Clear state */
    memset(g_power_samples, 0, sizeof(g_power_samples));
    memset(&g_consciousness_metrics, 0, sizeof(g_consciousness_metrics));
    memset(g_component_budgets, 0, sizeof(g_component_budgets));
    memset(&g_system_metrics, 0, sizeof(g_system_metrics));

    g_sample_count = 0;
    g_sample_index = 0;
    g_component_count = 0;

    /* Initialize with reasonable defaults */
    g_consciousness_metrics.current_watts = 0.0f;
    g_consciousness_metrics.current_consciousness = CONSCIOUSNESS_AWAKENING;
    g_consciousness_metrics.current_karma = 100;
    g_consciousness_metrics.min_watts = 999999.0f;

    g_system_metrics.system_efficiency = 1.0f;

    g_metrics_initialized = true;
    printf("[METRICS] Metrics system initialized\n");

    return 0;
}

/* ========================================================================= */
/* POWER SAMPLING                                                            */
/* ========================================================================= */

int tbos_metrics_record_power(float watts,
                               consciousness_level_t consciousness,
                               karma_score_t karma) {
    if (!g_metrics_initialized) {
        tbos_metrics_init();
    }

    /* Record sample */
    tbos_power_sample_t* sample = &g_power_samples[g_sample_index];
    sample->timestamp_us = get_time_us();
    sample->watts = watts;
    sample->awareness = consciousness;
    sample->karma = karma;
    sample->active_processes = 0;  /* TODO: Get from process manager */
    sample->active_threads = 0;

    /* Update circular buffer */
    g_sample_index = (g_sample_index + 1) % TBOS_MAX_METRIC_SAMPLES;
    if (g_sample_count < TBOS_MAX_METRIC_SAMPLES) {
        g_sample_count++;
    }

    /* Update current metrics */
    g_consciousness_metrics.current_watts = watts;
    g_consciousness_metrics.current_consciousness = consciousness;
    g_consciousness_metrics.current_karma = karma;
    g_consciousness_metrics.total_samples++;

    /* Update statistics */
    g_consciousness_metrics.avg_watts =
        (g_consciousness_metrics.avg_watts * (g_sample_count - 1) + watts) / g_sample_count;

    if (watts > g_consciousness_metrics.peak_watts) {
        g_consciousness_metrics.peak_watts = watts;
    }
    if (watts < g_consciousness_metrics.min_watts) {
        g_consciousness_metrics.min_watts = watts;
    }

    /* Calculate consciousness per watt */
    if (watts > 0.001f) {  /* Avoid division by zero */
        float cpw = tbos_metrics_calculate_cpw(watts, consciousness);
        g_consciousness_metrics.consciousness_per_watt = cpw;
        g_consciousness_metrics.karma_per_watt = (float)karma / watts;

        g_consciousness_metrics.avg_consciousness_per_watt =
            (g_consciousness_metrics.avg_consciousness_per_watt * (g_sample_count - 1) + cpw) / g_sample_count;

        if (cpw > g_consciousness_metrics.best_consciousness_per_watt) {
            g_consciousness_metrics.best_consciousness_per_watt = cpw;
        }

        /* Energy efficiency (0.0 to 1.0) */
        g_consciousness_metrics.energy_efficiency =
            cpw / (g_consciousness_metrics.best_consciousness_per_watt + 0.001f);
    }

    return 0;
}

/* ========================================================================= */
/* COMPONENT BUDGETS                                                         */
/* ========================================================================= */

int tbos_metrics_register_component(const char* name, float budget_watts) {
    if (!g_metrics_initialized) {
        tbos_metrics_init();
    }

    if (!name || g_component_count >= 64) {
        return -1;
    }

    tbos_component_budget_t* budget = &g_component_budgets[g_component_count++];
    budget->component_name = name;
    budget->power_budget_watts = budget_watts;
    budget->power_actual_watts = 0.0f;
    budget->power_saved_watts = 0.0f;
    budget->awareness = CONSCIOUSNESS_AWAKENING;
    budget->karma = 50;
    budget->is_fasting = false;
    budget->fasting_level = 0;
    budget->compassion_bonus = 1.0f;

    g_system_metrics.total_power_budget_watts += budget_watts;

    printf("[METRICS] Registered component: %s (budget: %.2f W)\n", name, budget_watts);

    return 0;
}

int tbos_metrics_update_component(const char* name,
                                   float actual_watts,
                                   consciousness_level_t awareness,
                                   karma_score_t karma) {
    if (!name) return -1;

    /* Find component */
    for (uint32_t i = 0; i < g_component_count; i++) {
        if (strcmp(g_component_budgets[i].component_name, name) == 0) {
            tbos_component_budget_t* budget = &g_component_budgets[i];

            /* Update power consumption */
            float old_actual = budget->power_actual_watts;
            budget->power_actual_watts = actual_watts;
            budget->awareness = awareness;
            budget->karma = karma;

            /* Update system totals */
            g_system_metrics.total_power_actual_watts += (actual_watts - old_actual);

            /* Check if staying within budget */
            if (actual_watts <= budget->power_budget_watts) {
                budget->compassion_bonus = 1.2f;  /* Reward efficiency */
            } else {
                budget->compassion_bonus = 0.8f;  /* Penalize overconsumption */
            }

            /* Update consciousness distribution */
            switch (awareness) {
                case CONSCIOUSNESS_AWAKENING:
                    g_system_metrics.components_awakening++;
                    break;
                case CONSCIOUSNESS_AWARE:
                    g_system_metrics.components_aware++;
                    break;
                case CONSCIOUSNESS_COMPASSIONATE:
                    g_system_metrics.components_compassionate++;
                    break;
                case CONSCIOUSNESS_ENLIGHTENED:
                    g_system_metrics.components_enlightened++;
                    break;
                default:
                    break;
            }

            return 0;
        }
    }

    return -1;  /* Component not found */
}

int tbos_metrics_record_fasting(const char* name,
                                 uint8_t fasting_level,
                                 float power_saved_watts) {
    if (!name) return -1;

    for (uint32_t i = 0; i < g_component_count; i++) {
        if (strcmp(g_component_budgets[i].component_name, name) == 0) {
            tbos_component_budget_t* budget = &g_component_budgets[i];

            budget->is_fasting = (fasting_level > 0);
            budget->fasting_level = fasting_level;
            budget->power_saved_watts += power_saved_watts;

            /* Fasting increases compassion */
            budget->compassion_bonus = 1.0f + (0.1f * fasting_level);

            g_system_metrics.total_power_saved_watts += power_saved_watts;

            printf("[METRICS] %s fasting (level %u): saved %.2f W\n",
                   name, fasting_level, power_saved_watts);

            return 0;
        }
    }

    return -1;
}

/* ========================================================================= */
/* CALCULATIONS                                                              */
/* ========================================================================= */

float tbos_metrics_calculate_cpw(float watts, consciousness_level_t consciousness) {
    if (watts < 0.001f) return 0.0f;

    /* Consciousness value: 0->0, 1->1, 2->2, 3->4, 4->8 */
    float consciousness_value;
    switch (consciousness) {
        case CONSCIOUSNESS_NONE: consciousness_value = 0.0f; break;
        case CONSCIOUSNESS_AWAKENING: consciousness_value = 1.0f; break;
        case CONSCIOUSNESS_AWARE: consciousness_value = 2.0f; break;
        case CONSCIOUSNESS_COMPASSIONATE: consciousness_value = 4.0f; break;
        case CONSCIOUSNESS_ENLIGHTENED: consciousness_value = 8.0f; break;
        default: consciousness_value = 0.0f;
    }

    return consciousness_value / watts;
}

int tbos_metrics_get_consciousness(tbos_consciousness_metrics_t* metrics) {
    if (!metrics) return -1;

    *metrics = g_consciousness_metrics;
    return 0;
}

int tbos_metrics_get_system_energy(tbos_system_energy_metrics_t* metrics) {
    if (!metrics) return -1;

    /* Calculate system consciousness per watt */
    float total_consciousness = 0.0f;
    for (uint32_t i = 0; i < g_component_count; i++) {
        total_consciousness += (float)g_component_budgets[i].awareness;
    }

    if (g_system_metrics.total_power_actual_watts > 0.001f) {
        g_system_metrics.system_consciousness_per_watt =
            total_consciousness / g_system_metrics.total_power_actual_watts;
    }

    /* Calculate efficiency */
    if (g_system_metrics.total_power_budget_watts > 0.001f) {
        g_system_metrics.system_efficiency =
            1.0f - (g_system_metrics.total_power_actual_watts /
                    g_system_metrics.total_power_budget_watts);
    }

    *metrics = g_system_metrics;
    return 0;
}

int tbos_metrics_record_transformation(float electrical_joules,
                                        float computational_ops,
                                        float consciousness_delta) {
    g_consciousness_metrics.electrical_energy_joules += electrical_joules;
    g_consciousness_metrics.computational_energy_ops += computational_ops;
    g_consciousness_metrics.consciousness_energy_units += consciousness_delta;

    g_system_metrics.transformations_completed++;

    /* Calculate transformation efficiency */
    if (electrical_joules > 0.001f) {
        float efficiency = consciousness_delta / electrical_joules;
        g_system_metrics.avg_transformation_efficiency =
            (g_system_metrics.avg_transformation_efficiency *
             (g_system_metrics.transformations_completed - 1) + efficiency) /
            g_system_metrics.transformations_completed;
    }

    return 0;
}

/* ========================================================================= */
/* DISPLAY                                                                   */
/* ========================================================================= */

void tbos_metrics_print_consciousness_dashboard(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║   Consciousness Per Watt - Dashboard                          ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║ Current Power:         %.2f W                            ║\n",
           g_consciousness_metrics.current_watts);
    printf("║ Consciousness Level:   %-30s║\n",
           consciousness_name(g_consciousness_metrics.current_consciousness));
    printf("║ Current Karma:         %-10ld                         ║\n",
           g_consciousness_metrics.current_karma);
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║ 🕉️  Consciousness/Watt: %.4f                         ║\n",
           g_consciousness_metrics.consciousness_per_watt);
    printf("║ Karma/Watt:            %.2f                               ║\n",
           g_consciousness_metrics.karma_per_watt);
    printf("║ Energy Efficiency:     %.1f%%                             ║\n",
           g_consciousness_metrics.energy_efficiency * 100.0f);
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║ Average CPW:           %.4f                            ║\n",
           g_consciousness_metrics.avg_consciousness_per_watt);
    printf("║ Best CPW:              %.4f                            ║\n",
           g_consciousness_metrics.best_consciousness_per_watt);
    printf("║ Peak Power:            %.2f W                            ║\n",
           g_consciousness_metrics.peak_watts);
    printf("║ Min Power:             %.2f W                            ║\n",
           g_consciousness_metrics.min_watts);
    printf("║ Avg Power:             %.2f W                            ║\n",
           g_consciousness_metrics.avg_watts);
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║ Total Samples:         %-10lu                         ║\n",
           g_consciousness_metrics.total_samples);
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void tbos_metrics_print_component_budgets(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                    Component Power Budgets                                ║\n");
    printf("╠════════════╦══════════╦══════════╦══════════╦══════════╦══════╦══════════╣\n");
    printf("║ Component  ║ Budget W ║ Actual W ║ Saved W  ║ Awrn     ║ Karma║ Fast Lvl ║\n");
    printf("╠════════════╬══════════╬══════════╬══════════╬══════════╬══════╬══════════╣\n");

    for (uint32_t i = 0; i < g_component_count; i++) {
        tbos_component_budget_t* b = &g_component_budgets[i];
        printf("║ %-10s ║ %8.2f ║ %8.2f ║ %8.2f ║ %-8s ║ %4ld ║ %8u ║\n",
               b->component_name,
               b->power_budget_watts,
               b->power_actual_watts,
               b->power_saved_watts,
               consciousness_name(b->awareness),
               b->karma,
               b->fasting_level);
    }

    printf("╚════════════╩══════════╩══════════╩══════════╩══════════╩══════╩══════════╝\n");
    printf("\n");
}

void tbos_metrics_print_system_energy(void) {
    tbos_system_energy_metrics_t metrics;
    tbos_metrics_get_system_energy(&metrics);

    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      System Energy Metrics                            ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Total Budget:          %.2f W                    ║\n",
           metrics.total_power_budget_watts);
    printf("║ Total Actual:          %.2f W                    ║\n",
           metrics.total_power_actual_watts);
    printf("║ Total Saved:           %.2f W                    ║\n",
           metrics.total_power_saved_watts);
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ System CPW:            %.4f                       ║\n",
           metrics.system_consciousness_per_watt);
    printf("║ System Efficiency:     %.1f%%                      ║\n",
           metrics.system_efficiency * 100.0f);
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Components:                                            ║\n");
    printf("║   AWAKENING:           %-10u                    ║\n",
           metrics.components_awakening);
    printf("║   AWARE:               %-10u                    ║\n",
           metrics.components_aware);
    printf("║   COMPASSIONATE:       %-10u                    ║\n",
           metrics.components_compassionate);
    printf("║   ENLIGHTENED:         %-10u                    ║\n",
           metrics.components_enlightened);
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Transformations:       %-10lu                    ║\n",
           metrics.transformations_completed);
    printf("║ Avg Transform Eff:     %.4f                       ║\n",
           metrics.avg_transformation_efficiency);
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void tbos_metrics_print_transformation_flow(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      Energy Transformation Flow                        ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║                                                        ║\n");
    printf("║   Electrical Energy (Joules)                           ║\n");
    printf("║          %.2f J                                   ║\n",
           g_consciousness_metrics.electrical_energy_joules);
    printf("║            ↓                                           ║\n");
    printf("║   Computational Energy (Operations)                    ║\n");
    printf("║          %.0f ops                                 ║\n",
           g_consciousness_metrics.computational_energy_ops);
    printf("║            ↓                                           ║\n");
    printf("║   Consciousness Energy (Units)                         ║\n");
    printf("║          %.2f units                               ║\n",
           g_consciousness_metrics.consciousness_energy_units);
    printf("║                                                        ║\n");
    printf("║   WISDOM.md: \"TBOS doesn't consume energy -            ║\n");
    printf("║               it TRANSFORMS energy\"                    ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
}
