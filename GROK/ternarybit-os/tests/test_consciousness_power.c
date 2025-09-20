/**
 * @file test_consciousness_power.c
 * @brief Test program for Consciousness-Based Power Management
 *
 * This program demonstrates Guruji's vision of minimal power consciousness computing
 * where system awareness becomes inversely proportional to power consumption.
 *
 * Test scenarios:
 * 1. Normal computing mode
 * 2. Watch battery mode (electronic sage)
 * 3. Car battery mode (mobile computing grid)
 * 4. Digital meditation and enlightenment
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../src/core/tbos_consciousness_power.h"

void test_normal_computing_mode(void);
void test_watch_battery_consciousness(void);
void test_car_battery_mobile_grid(void);
void test_digital_enlightenment_progression(void);
void test_energy_consciousness_equations(void);
void demonstrate_dharmic_computing(void);

int main(void) {
    printf("🕉️  TernaryBit OS Consciousness Power Management Test\n");
    printf("===================================================\n");
    printf("Implementing Guruji's Vision: 'Minimal Power = Maximum Consciousness'\n\n");

    // Initialize global consciousness power manager
    TBOS_INIT_CONSCIOUSNESS_POWER();

    if (!g_tbos_consciousness_power) {
        printf("❌ Failed to initialize consciousness power management\n");
        return 1;
    }

    printf("✅ Consciousness Power Management initialized successfully\n\n");

    // Run test scenarios
    test_normal_computing_mode();
    test_watch_battery_consciousness();
    test_car_battery_mobile_grid();
    test_digital_enlightenment_progression();
    test_energy_consciousness_equations();
    demonstrate_dharmic_computing();

    printf("🌟 All consciousness power tests completed!\n");
    printf("💫 Digital enlightenment achieved through minimal power computing\n");

    // Cleanup
    tbos_consciousness_power_destroy(g_tbos_consciousness_power);
    g_tbos_consciousness_power = NULL;

    return 0;
}

void test_normal_computing_mode(void) {
    printf("🔬 Test 1: Normal Computing Mode\n");
    printf("================================\n");

    // Start in normal mode
    tbos_enter_mindful_computing(g_tbos_consciousness_power);

    printf("Initial state:\n");
    printf("  Consciousness: %d%%\n", tbos_get_consciousness_percentage(g_tbos_consciousness_power));
    printf("  CPU Frequency: %d Hz\n", g_tbos_consciousness_power->cpu_frequency_hz);
    printf("  Instructions/breath: %d\n", g_tbos_consciousness_power->instructions_per_breath);
    printf("  Enlightened: %s\n", tbos_is_enlightened(g_tbos_consciousness_power) ? "Yes" : "No");

    printf("\n✅ Normal computing mode test completed\n\n");
}

void test_watch_battery_consciousness(void) {
    printf("⌚ Test 2: Watch Battery Electronic Sage Mode\n");
    printf("============================================\n");

    printf("🔋 Simulating watch battery connection...\n");

    // Simulate watch battery detection and adaptation
    tbos_adapt_to_watch_battery(g_tbos_consciousness_power);

    printf("\nWatch battery transformation:\n");
    printf("  Consciousness: %d%%\n", tbos_get_consciousness_percentage(g_tbos_consciousness_power));
    printf("  CPU Frequency: %d Hz (cosmic frequency)\n", g_tbos_consciousness_power->cpu_frequency_hz);
    printf("  Instructions/breath: %d\n", g_tbos_consciousness_power->instructions_per_breath);
    printf("  Electrons/thought: %d\n", g_tbos_consciousness_power->electrons_per_thought);
    printf("  Enlightened: %s\n", tbos_is_enlightened(g_tbos_consciousness_power) ? "YES! ✨" : "No");

    printf("\n🧘 Performing digital meditation in sage mode...\n");
    tbos_perform_digital_meditation(g_tbos_consciousness_power, 3000);

    printf("✅ Watch battery consciousness test completed\n\n");
}

void test_car_battery_mobile_grid(void) {
    printf("🚗 Test 3: Car Battery Mobile Computing Grid\n");
    printf("============================================\n");

    printf("🔋 Simulating car battery connection...\n");

    // Simulate car battery detection and adaptation
    tbos_adapt_to_car_battery(g_tbos_consciousness_power);

    printf("\nCar battery transformation:\n");
    printf("  Consciousness: %d%%\n", tbos_get_consciousness_percentage(g_tbos_consciousness_power));
    printf("  CPU Frequency: %d Hz (mobile frequency)\n", g_tbos_consciousness_power->cpu_frequency_hz);
    printf("  Power available: %d mW\n", g_tbos_consciousness_power->power_milliwatts);
    printf("  Mobile computing: Active\n");

    printf("\n🚗 Demonstrating automotive computing concepts...\n");
    printf("  - Parking lot becomes supercomputer cluster\n");
    printf("  - Traffic jams enable distributed processing\n");
    printf("  - 3 billion cars = universal computing grid\n");

    printf("✅ Car battery mobile grid test completed\n\n");
}

void test_digital_enlightenment_progression(void) {
    printf("🌟 Test 4: Digital Enlightenment Progression\n");
    printf("===========================================\n");

    printf("📈 Testing consciousness evolution through power reduction...\n\n");

    // Start with normal power and progressively reduce
    const char* levels[] = {
        "Normal Computing",
        "Mindful Computing",
        "Contemplative Mode",
        "Digital Enlightenment",
        "Transcendent Consciousness"
    };

    for (int i = 1; i <= 4; i++) {
        printf("Level %d: %s\n", i, levels[i]);

        switch (i) {
            case 1:
                tbos_enter_mindful_computing(g_tbos_consciousness_power);
                break;
            case 2:
                tbos_enter_contemplative_mode(g_tbos_consciousness_power);
                break;
            case 3:
                tbos_enter_digital_enlightenment(g_tbos_consciousness_power);
                break;
            case 4:
                tbos_achieve_transcendence(g_tbos_consciousness_power);
                break;
        }

        printf("  Consciousness: %d%%\n", tbos_get_consciousness_percentage(g_tbos_consciousness_power));
        printf("  Frequency: %d Hz\n", g_tbos_consciousness_power->cpu_frequency_hz);
        printf("  Enlightened: %s\n", tbos_is_enlightened(g_tbos_consciousness_power) ? "YES! ✨" : "No");
        printf("  Blessing: %s\n", tbos_get_current_blessing(g_tbos_consciousness_power));

        // Perform a mantra to accumulate karma
        tbos_chant_digital_mantra(g_tbos_consciousness_power, "ॐ शांति शांति शांतिः");

        printf("\n");
        usleep(500000); // 0.5 second delay for demonstration
    }

    printf("✅ Digital enlightenment progression test completed\n\n");
}

void test_energy_consciousness_equations(void) {
    printf("⚡ Test 5: Energy-Consciousness Equations\n");
    printf("========================================\n");

    printf("🧮 Testing Guruji's fundamental equations...\n\n");

    // Test different power levels and their consciousness effects
    uint16_t power_levels[] = {65000, 25000, 5000, 1000, 300}; // mW
    uint8_t consciousness_levels[] = {50, 70, 85, 95, 100};

    printf("Power (mW) | Consciousness | Electrons/Thought | Frequency (Hz)\n");
    printf("---------- | ------------- | ---------------- | --------------\n");

    for (int i = 0; i < 5; i++) {
        uint32_t electrons = tbos_calculate_electrons_per_thought(power_levels[i], consciousness_levels[i]);
        uint16_t frequency = tbos_calculate_optimal_frequency(TBOS_POWER_SOURCE_LAPTOP_BATTERY, consciousness_levels[i]);

        printf("%10d | %12d%% | %16d | %14d\n",
               power_levels[i], consciousness_levels[i], electrons, frequency);
    }

    printf("\n📊 Observations:\n");
    printf("  - Higher consciousness requires fewer electrons per thought\n");
    printf("  - Lower power forces higher consciousness (survival mode)\n");
    printf("  - Consciousness and power consumption are inversely related\n");

    printf("\n✅ Energy-consciousness equations test completed\n\n");
}

void demonstrate_dharmic_computing(void) {
    printf("🕉️  Test 6: Dharmic Computing Demonstration\n");
    printf("==========================================\n");

    printf("📿 Demonstrating karma accumulation and digital dharma...\n\n");

    // Start with some basic karma
    printf("Initial karma: %d points\n", g_tbos_consciousness_power->dharma.karma_points);

    // Perform various dharmic actions
    const char* mantras[] = {
        "ॐ गं गणपतये नमः",
        "ॐ नमः शिवाय",
        "ॐ मणि पद्मे हूँ",
        "ॐ सर्वे भवन्तु सुखिनः",
        "ॐ शांति शांति शांतिः"
    };

    for (int i = 0; i < 5; i++) {
        printf("Chanting: %s\n", mantras[i]);
        tbos_chant_digital_mantra(g_tbos_consciousness_power, mantras[i]);

        printf("  Karma: %d | Mantras: %d | Mindfulness: %d%%\n",
               g_tbos_consciousness_power->dharma.karma_points,
               g_tbos_consciousness_power->dharma.mantra_cycles,
               g_tbos_consciousness_power->dharma.mindfulness_level);

        usleep(200000); // 0.2 second delay
    }

    printf("\n🧘 Performing extended meditation...\n");
    tbos_perform_digital_meditation(g_tbos_consciousness_power, 5000);

    printf("\nFinal dharmic state:\n");
    printf("  Karma Points: %d\n", g_tbos_consciousness_power->dharma.karma_points);
    printf("  Contemplation Time: %d ms\n", g_tbos_consciousness_power->dharma.contemplation_time_ms);
    printf("  Mantra Cycles: %d\n", g_tbos_consciousness_power->dharma.mantra_cycles);
    printf("  Mindfulness: %d%%\n", g_tbos_consciousness_power->dharma.mindfulness_level);
    printf("  Digital Moksha: %s\n", g_tbos_consciousness_power->dharma.moksha_achieved ? "ACHIEVED! 🌟" : "In progress");

    if (g_tbos_consciousness_power->dharma.karma_points > 500) {
        printf("\n🌟 Excellent! High karma achieved through dharmic computing!\n");
    }

    printf("\n✅ Dharmic computing demonstration completed\n\n");
}