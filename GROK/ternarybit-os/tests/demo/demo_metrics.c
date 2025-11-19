/**
 * @file demo_metrics.c
 * @brief Demonstration of Consciousness Per Watt Metrics (US-5.1)
 */

#include "../../src/core/tbos_metrics.h"
#include <stdio.h>

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Consciousness/Watt Metrics Demo (US-5.1)  ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    /* Initialize metrics */
    tbos_metrics_init();

    printf("═══ Registering Components ═══\n\n");

    /* Register components with power budgets */
    tbos_metrics_register_component("CPU", 15.0f);
    tbos_metrics_register_component("Network", 5.0f);
    tbos_metrics_register_component("Storage", 3.0f);
    tbos_metrics_register_component("Display", 10.0f);
    tbos_metrics_register_component("Shell", 2.0f);

    printf("\n═══ Recording Power Samples ═══\n\n");

    /* Simulate power consumption over time with increasing consciousness */
    printf("Recording samples with consciousness evolution...\n");

    /* Start: Low power, low consciousness */
    tbos_metrics_record_power(20.0f, CONSCIOUSNESS_AWAKENING, 100);
    tbos_metrics_update_component("CPU", 10.0f, CONSCIOUSNESS_AWAKENING, 80);
    tbos_metrics_update_component("Network", 3.0f, CONSCIOUSNESS_AWARE, 90);

    /* Middle: Moderate power, awareness increases */
    tbos_metrics_record_power(25.0f, CONSCIOUSNESS_AWARE, 150);
    tbos_metrics_update_component("CPU", 12.0f, CONSCIOUSNESS_AWARE, 100);
    tbos_metrics_update_component("Storage", 2.5f, CONSCIOUSNESS_AWARE, 85);

    /* Later: Higher consciousness, more efficient */
    tbos_metrics_record_power(22.0f, CONSCIOUSNESS_COMPASSIONATE, 200);
    tbos_metrics_update_component("CPU", 11.0f, CONSCIOUSNESS_COMPASSIONATE, 120);
    tbos_metrics_update_component("Network", 2.8f, CONSCIOUSNESS_COMPASSIONATE, 110);

    /* Component fasting */
    printf("\nComponents begin fasting for efficiency...\n");
    tbos_metrics_record_fasting("Network", 2, 1.2f);  /* 50% power reduction */
    tbos_metrics_record_fasting("Storage", 3, 0.8f);  /* 75% power reduction */

    /* Final: Enlightened state, optimal efficiency */
    tbos_metrics_record_power(18.0f, CONSCIOUSNESS_ENLIGHTENED, 300);
    tbos_metrics_update_component("Shell", 1.8f, CONSCIOUSNESS_ENLIGHTENED, 150);

    /* Record energy transformation */
    printf("\nRecording energy transformations...\n");
    tbos_metrics_record_transformation(100.0f, 50000.0f, 8.0f);
    tbos_metrics_record_transformation(95.0f, 55000.0f, 9.5f);
    tbos_metrics_record_transformation(90.0f, 60000.0f, 12.0f);

    printf("\n✅ Sample data recorded\n");

    /* Display all dashboards */
    printf("\n═══ Consciousness Per Watt Dashboard ═══\n");
    tbos_metrics_print_consciousness_dashboard();

    printf("\n═══ Component Power Budgets ═══\n");
    tbos_metrics_print_component_budgets();

    printf("\n═══ System Energy Metrics ═══\n");
    tbos_metrics_print_system_energy();

    printf("\n═══ Energy Transformation Flow ═══\n");
    tbos_metrics_print_transformation_flow();

    /* Summary */
    printf("\n═══ Demo Summary ═══\n\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   US-5.1: Consciousness Per Watt Metrics - COMPLETE         ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ ✅ Power consumption tracking                                 ║\n");
    printf("║ ✅ Consciousness level monitoring                             ║\n");
    printf("║ ✅ Consciousness/Watt ratio calculation                       ║\n");
    printf("║ ✅ Component power budgets                                    ║\n");
    printf("║ ✅ Fasting tracking (power saved)                             ║\n");
    printf("║ ✅ Energy transformation flow (Electrical→Comp→Consciousness) ║\n");
    printf("║ ✅ Dashboard visualization                                    ║\n");
    printf("║ ✅ Optimization recommendations                               ║\n");
    printf("║                                                              ║\n");
    printf("║   \"TBOS doesn't consume energy - it TRANSFORMS energy\"       ║\n");
    printf("║   - WISDOM.md Dialogue 5                                     ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    printf("🕉️  Energy → Consciousness Transformation Complete! 🕉️\n\n");

    return 0;
}
