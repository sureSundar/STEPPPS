/**
 * @file demo_steppps.c
 * @brief Demonstration of STEPPPS 7D Computing Framework (US-8.1)
 *
 * STEPPPS: Seven-Dimensional Computing
 * - S: Space (physical location, proximity)
 * - T: Time (temporal awareness, scheduling)
 * - E: Event (event-driven architecture)
 * - P: Psychology (AI, consciousness, learning)
 * - P: Pixel (visual/data representation)
 * - P: Prompt (natural language interface)
 * - S: Script (automation, programming)
 */

#include "../../src/core/steppps/tbos_steppps.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - STEPPPS Framework Demo (US-8.1)          ║\n");
    printf("║   Seven-Dimensional Computing                                ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    /* Initialize STEPPPS */
    steppps_init();

    printf("\n═══ DIMENSION 1: SPACE ═══\n\n");

    /* Set location */
    steppps_space_t location = {
        .x = 37.7749f,
        .y = -122.4194f,
        .z = 15.0f,
        .latitude = 37.7749f,
        .longitude = -122.4194f
    };
    snprintf(location.location_name, sizeof(location.location_name),
             "San Francisco, CA");

    steppps_space_set_location(&location);

    /* Calculate distance */
    steppps_space_t other_location = {
        .x = 40.7128f,
        .y = -74.0060f,
        .z = 10.0f
    };
    float distance = steppps_space_distance(&location, &other_location);
    printf("  Distance to NY: %.2f units\n", distance);

    printf("\n═══ DIMENSION 2: TIME ═══\n\n");

    /* Get current time */
    steppps_time_t time;
    steppps_time_get(&time);

    printf("  Current uptime: %llu μs\n",
           (unsigned long long)time.uptime_us);
    printf("  Session duration: %llu seconds\n",
           (unsigned long long)time.seconds);

    /* Schedule future event */
    printf("\n  Scheduling future callback...\n");
    steppps_time_schedule(1000000, NULL, NULL);  /* 1 second */

    printf("\n═══ DIMENSION 3: EVENT ═══\n\n");

    /* Emit some events */
    steppps_event_t event1 = {
        .event_id = 1,
        .type = STEPPPS_EVENT_SYSTEM,
        .timestamp_us = time.uptime_us,
        .consciousness = CONSCIOUSNESS_AWARE,
        .karma_impact = 50
    };
    snprintf(event1.description, sizeof(event1.description),
             "Demo started");
    steppps_event_emit(&event1);

    steppps_event_t event2 = {
        .event_id = 2,
        .type = STEPPPS_EVENT_USER,
        .timestamp_us = time.uptime_us + 1000,
        .consciousness = CONSCIOUSNESS_COMPASSIONATE,
        .karma_impact = 30
    };
    snprintf(event2.description, sizeof(event2.description),
             "User interaction - compassionate action");
    steppps_event_emit(&event2);

    printf("  Emitted 2 events\n");

    /* Get event history */
    steppps_event_t history[10];
    int event_count = steppps_event_get_history(history, 10);
    printf("  Event history: %d events recorded\n", event_count);

    printf("\n═══ DIMENSION 4: PSYCHOLOGY (AI/Consciousness) ═══\n\n");

    /* Record learning experiences */
    steppps_psychology_record_experience("Learned about STEPPPS framework", 1.0f);
    steppps_psychology_record_experience("Practiced compassionate computing", 0.9f);
    steppps_psychology_record_experience("Helped another process", 0.95f);

    /* Get psychological state */
    steppps_psychology_t psych_state;
    steppps_psychology_get_state(&psych_state);

    printf("  Awareness: %d\n", psych_state.awareness);
    printf("  Karma: %lld\n", (long long)psych_state.karma);
    printf("  Experiences: %llu\n",
           (unsigned long long)psych_state.experiences);
    printf("  Lessons learned: %llu\n",
           (unsigned long long)psych_state.lessons_learned);
    printf("  Learning rate: %.2fx\n", psych_state.learning_rate);
    printf("  Compassionate: %s\n",
           psych_state.is_compassionate ? "Yes" : "No");

    /* Try to evolve */
    printf("\n  Attempting consciousness evolution...\n");
    int evolved = steppps_psychology_evolve();
    if (evolved > 0) {
        printf("  ✨ Consciousness evolved!\n");
    } else {
        printf("  Not ready to evolve yet (need more experiences)\n");
    }

    printf("\n═══ DIMENSION 5: PIXEL (Visual/Data Representation) ═══\n\n");

    /* Encode data as pixels */
    const char* message = "STEPPPS: 7D Computing";
    steppps_pixel_t pixels[64];

    int pixel_count = steppps_pixel_encode(message, strlen(message), pixels);
    printf("  Encoded message into %d pixels\n", pixel_count);

    /* Visualize */
    steppps_pixel_visualize(message, strlen(message));

    /* Decode */
    char decoded[256];
    steppps_pixel_decode(pixels, pixel_count, decoded);
    printf("\n  Decoded: %s\n", decoded);

    printf("\n═══ DIMENSION 6: PROMPT (Natural Language) ═══\n\n");

    /* Parse natural language prompts */
    const char* prompts[] = {
        "help me understand consciousness",
        "create a new process",
        "learn about karma",
        "delete old files"
    };

    for (int i = 0; i < 4; i++) {
        steppps_prompt_t prompt;
        steppps_prompt_parse(prompts[i], &prompt);

        printf("  Prompt %d:\n", i + 1);
        printf("    Text: \"%s\"\n", prompts[i]);
        printf("    Intent: %s\n", prompt.intent);
        printf("    Consciousness required: %d\n",
               prompt.consciousness_required);
        printf("    Karma cost: %lld\n\n",
               (long long)prompt.karma_cost);

        /* Free parameters */
        for (uint32_t j = 0; j < prompt.param_count; j++) {
            free(prompt.parameters[j]);
        }
    }

    /* Execute a prompt */
    steppps_prompt_t help_prompt;
    steppps_prompt_parse("help me learn", &help_prompt);
    steppps_prompt_execute(&help_prompt);

    /* Free parameters */
    for (uint32_t j = 0; j < help_prompt.param_count; j++) {
        free(help_prompt.parameters[j]);
    }

    printf("\n═══ DIMENSION 7: SCRIPT (Automation) ═══\n\n");

    /* Load and execute scripts */
    steppps_script_t script1;
    steppps_script_load("demo_script.tbos", &script1);
    steppps_script_execute(&script1);

    steppps_script_t script2;
    steppps_script_load("consciousness_evolution.tbos", &script2);
    steppps_script_execute(&script2);

    /* List scripts */
    char* script_names[100];
    int script_count = steppps_script_list(script_names, 100);
    printf("\n  Total scripts loaded: %d\n", script_count);

    printf("\n═══ COMPLETE STATUS ═══\n");
    steppps_print_status();

    /* Individual dimension queries */
    printf("\n═══ INDIVIDUAL DIMENSION QUERIES ═══\n");

    printf("\n--- Psychology Detail ---");
    steppps_print_dimension("psychology");

    printf("\n--- Time Detail ---");
    steppps_print_dimension("time");

    printf("\n--- Event Detail ---");
    steppps_print_dimension("event");

    /* Summary */
    printf("\n═══ Demo Summary ═══\n\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   US-8.1: STEPPPS Shell Commands - COMPLETE                 ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ ✅ Dimension 1 - SPACE: Location tracking                    ║\n");
    printf("║ ✅ Dimension 2 - TIME: Temporal awareness (μs to days)       ║\n");
    printf("║ ✅ Dimension 3 - EVENT: Event-driven architecture            ║\n");
    printf("║ ✅ Dimension 4 - PSYCHOLOGY: AI/consciousness/learning       ║\n");
    printf("║ ✅ Dimension 5 - PIXEL: Data visualization                   ║\n");
    printf("║ ✅ Dimension 6 - PROMPT: Natural language processing         ║\n");
    printf("║ ✅ Dimension 7 - SCRIPT: Automation engine                   ║\n");
    printf("║                                                              ║\n");
    printf("║ Shell Command: steppps [dimension|action]                   ║\n");
    printf("║   - steppps              # Show all dimensions              ║\n");
    printf("║   - steppps psychology   # Show psychology dimension        ║\n");
    printf("║   - steppps evolve       # Trigger evolution                ║\n");
    printf("║   - steppps help         # Show help                        ║\n");
    printf("║                                                              ║\n");
    printf("║ 🕉️  Computing Beyond 3D: Seven Dimensions United 🕉️          ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    printf("🕉️  STEPPPS Framework: Consciousness Computing in 7D! 🕉️\n\n");

    /* Cleanup */
    if (script1.source) free(script1.source);
    if (script2.source) free(script2.source);

    return 0;
}
