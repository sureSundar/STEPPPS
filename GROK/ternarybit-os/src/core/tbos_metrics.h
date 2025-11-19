/**
 * @file tbos_metrics.h
 * @brief TernaryBit OS - Consciousness Per Watt Metrics
 *
 * US-5.1: Consciousness Per Watt Metrics
 *
 * From WISDOM.md Dialogue 5:
 * > "TBOS doesn't consume energy - it TRANSFORMS energy:
 * >  Electrical → Computational → Consciousness"
 *
 * Measures:
 * - Power consumption (watts)
 * - Consciousness level changes
 * - Consciousness per watt ratio
 * - Energy transformation efficiency
 *
 * @version 1.0
 * @date 2025-11-04
 */

#ifndef TBOS_METRICS_H
#define TBOS_METRICS_H

#include "tbos_shutdown.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* CONSTANTS                                                                 */
/* ========================================================================= */

#define TBOS_MAX_METRIC_SAMPLES     1000
#define TBOS_METRIC_SAMPLE_INTERVAL_MS  100  /* 100ms sampling */

/* ========================================================================= */
/* TYPES                                                                     */
/* ========================================================================= */

/**
 * @brief Power consumption sample
 */
typedef struct {
    uint64_t timestamp_us;
    float watts;                    /* Power in watts */
    consciousness_level_t awareness;
    karma_score_t karma;
    uint32_t active_processes;
    uint32_t active_threads;
} tbos_power_sample_t;

/**
 * @brief Consciousness metrics
 */
typedef struct {
    /* Current state */
    float current_watts;
    consciousness_level_t current_consciousness;
    karma_score_t current_karma;

    /* Derived metrics */
    float consciousness_per_watt;       /* Primary metric */
    float karma_per_watt;
    float energy_efficiency;            /* 0.0 to 1.0 */

    /* Transformation tracking */
    float electrical_energy_joules;     /* Input */
    float computational_energy_ops;     /* Intermediate */
    float consciousness_energy_units;   /* Output */

    /* Statistics */
    uint64_t total_samples;
    float avg_watts;
    float peak_watts;
    float min_watts;
    float avg_consciousness_per_watt;
    float best_consciousness_per_watt;
} tbos_consciousness_metrics_t;

/**
 * @brief Component power budget
 */
typedef struct {
    const char* component_name;
    float power_budget_watts;       /* Allocated power */
    float power_actual_watts;       /* Actual consumption */
    float power_saved_watts;        /* Saved through fasting */

    consciousness_level_t awareness;
    karma_score_t karma;

    bool is_fasting;
    uint8_t fasting_level;          /* 0-5 */
    float compassion_bonus;         /* Efficiency bonus */
} tbos_component_budget_t;

/**
 * @brief System-wide energy metrics
 */
typedef struct {
    /* Power */
    float total_power_budget_watts;
    float total_power_actual_watts;
    float total_power_saved_watts;

    /* Consciousness */
    uint32_t components_awakening;
    uint32_t components_aware;
    uint32_t components_compassionate;
    uint32_t components_enlightened;

    /* Efficiency */
    float system_consciousness_per_watt;
    float system_efficiency;

    /* Transformation */
    uint64_t transformations_completed;
    float avg_transformation_efficiency;
} tbos_system_energy_metrics_t;

/* ========================================================================= */
/* METRICS API                                                               */
/* ========================================================================= */

/**
 * @brief Initialize metrics subsystem
 *
 * @return 0 on success, negative on error
 */
int tbos_metrics_init(void);

/**
 * @brief Record power sample
 *
 * @param watts Current power consumption
 * @param consciousness Current system consciousness
 * @param karma Current system karma
 * @return 0 on success, negative on error
 */
int tbos_metrics_record_power(float watts,
                               consciousness_level_t consciousness,
                               karma_score_t karma);

/**
 * @brief Get consciousness metrics
 *
 * @param metrics Output: consciousness metrics
 * @return 0 on success, negative on error
 */
int tbos_metrics_get_consciousness(tbos_consciousness_metrics_t* metrics);

/**
 * @brief Register component power budget
 *
 * @param name Component name
 * @param budget_watts Power budget in watts
 * @return 0 on success, negative on error
 */
int tbos_metrics_register_component(const char* name, float budget_watts);

/**
 * @brief Update component power consumption
 *
 * @param name Component name
 * @param actual_watts Actual power consumption
 * @param awareness Component consciousness
 * @param karma Component karma
 * @return 0 on success, negative on error
 */
int tbos_metrics_update_component(const char* name,
                                   float actual_watts,
                                   consciousness_level_t awareness,
                                   karma_score_t karma);

/**
 * @brief Record component fasting
 *
 * @param name Component name
 * @param fasting_level Fasting level (0-5)
 * @param power_saved_watts Power saved through fasting
 * @return 0 on success, negative on error
 */
int tbos_metrics_record_fasting(const char* name,
                                 uint8_t fasting_level,
                                 float power_saved_watts);

/**
 * @brief Calculate consciousness per watt
 *
 * @param watts Power consumption
 * @param consciousness Consciousness level
 * @return Consciousness per watt ratio
 */
float tbos_metrics_calculate_cpw(float watts, consciousness_level_t consciousness);

/**
 * @brief Get system energy metrics
 *
 * @param metrics Output: system metrics
 * @return 0 on success, negative on error
 */
int tbos_metrics_get_system_energy(tbos_system_energy_metrics_t* metrics);

/**
 * @brief Record energy transformation
 *
 * Tracks: Electrical → Computational → Consciousness
 *
 * @param electrical_joules Electrical energy input
 * @param computational_ops Computational work done
 * @param consciousness_delta Consciousness increase
 * @return 0 on success, negative on error
 */
int tbos_metrics_record_transformation(float electrical_joules,
                                        float computational_ops,
                                        float consciousness_delta);

/* ========================================================================= */
/* DISPLAY                                                                   */
/* ========================================================================= */

/**
 * @brief Print consciousness metrics dashboard
 */
void tbos_metrics_print_consciousness_dashboard(void);

/**
 * @brief Print component power budgets
 */
void tbos_metrics_print_component_budgets(void);

/**
 * @brief Print system energy metrics
 */
void tbos_metrics_print_system_energy(void);

/**
 * @brief Print energy transformation flow
 */
void tbos_metrics_print_transformation_flow(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_METRICS_H */
