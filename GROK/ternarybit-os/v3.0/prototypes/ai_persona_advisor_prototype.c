/**
 * @file ai_persona_advisor_prototype.c
 * @brief TernaryBit OS v3.0 - AI Persona Advisor Engine Prototype
 *
 * This is a working prototype of the AI-powered persona recommendation system
 * that learns user preferences and provides intelligent hardware persona suggestions.
 *
 * Features:
 * - Simple neural network for persona recommendations
 * - Usage pattern learning
 * - Performance prediction
 * - Real-time adaptation
 *
 * @version 3.0.0
 * @date 2025-10-27
 * @author TernaryBit OS Team + Claude Code
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

// =============================================================================
// CONSTANTS AND ENUMERATIONS
// =============================================================================

#define MAX_PERSONAS 10
#define INPUT_FEATURES 16
#define HIDDEN_NEURONS 8
#define OUTPUT_NEURONS 4
#define MAX_HISTORY 1000
#define LEARNING_RATE 0.01f

typedef enum {
    PERSONA_CALC_4BIT = 0,
    PERSONA_CALC_16BIT = 1,
    PERSONA_RETRO_8BIT = 2,
    PERSONA_EMBEDDED_ARM = 3,
    PERSONA_MINI_X86 = 4,
    PERSONA_MODERN_X86 = 5,
    PERSONA_ARM_SBC = 6,
    PERSONA_RISCV_DEV = 7,
    PERSONA_WORKSTATION = 8,
    PERSONA_HPC_NODE = 9
} persona_id_t;

typedef enum {
    ARCH_CALCULATOR_4BIT = 0,
    ARCH_X86_16BIT = 1,
    ARCH_X86_32BIT = 2,
    ARCH_X86_64BIT = 3,
    ARCH_ARM64 = 4,
    ARCH_RISCV64 = 5,
    ARCH_EMBEDDED = 6
} cpu_arch_t;

// =============================================================================
// DATA STRUCTURES
// =============================================================================

typedef struct {
    float weights_ih[INPUT_FEATURES][HIDDEN_NEURONS];  // Input to hidden
    float weights_ho[HIDDEN_NEURONS][OUTPUT_NEURONS];  // Hidden to output
    float bias_hidden[HIDDEN_NEURONS];
    float bias_output[OUTPUT_NEURONS];

    // Training state
    float learning_rate;
    uint32_t training_epochs;
} neural_network_t;

typedef struct {
    cpu_arch_t architecture;
    uint8_t cpu_bits;
    uint32_t cpu_speed_mhz;
    uint8_t cpu_cores;
    uint64_t total_ram_bytes;
    uint64_t available_ram_bytes;
    bool has_mmu;
    bool has_fpu;
    uint32_t storage_speed_mbps;
    float power_level;  // 0.0 to 1.0
} hardware_capabilities_t;

typedef struct {
    float time_of_day;           // 0.0 to 1.0 (midnight to midnight)
    float day_of_week;           // 0.0 to 1.0 (Sunday to Saturday)
    float workload_intensity;    // Expected computational load
    float graphics_requirement;  // Graphics processing needs
    float network_requirement;   // Network usage expectation
} context_info_t;

typedef struct {
    uint32_t boot_count;
    uint32_t runtime_minutes;
    float user_satisfaction;     // 1.0 to 10.0
    float avg_performance;       // Performance score
    uint32_t last_used_timestamp;
} persona_usage_history_t;

typedef struct {
    persona_id_t persona_id;
    float confidence;            // 0.0 to 1.0
    float predicted_satisfaction;
    float estimated_performance;
    char rationale[256];
} persona_recommendation_t;

typedef struct {
    neural_network_t* network;
    persona_usage_history_t history[MAX_PERSONAS];

    // Feature normalization parameters
    float feature_means[INPUT_FEATURES];
    float feature_stds[INPUT_FEATURES];

    // Training data
    float training_inputs[MAX_HISTORY][INPUT_FEATURES];
    float training_outputs[MAX_HISTORY][OUTPUT_NEURONS];
    uint32_t training_samples;

    // Statistics
    uint32_t total_recommendations;
    uint32_t correct_predictions;
    float accuracy;
} ai_advisor_t;

// =============================================================================
// PERSONA DEFINITIONS
// =============================================================================

typedef struct {
    persona_id_t id;
    const char* name;
    const char* description;
    cpu_arch_t emulated_arch;
    uint8_t min_cpu_bits;
    uint64_t min_ram_bytes;
    float complexity_score;      // 0.0 to 1.0
} persona_definition_t;

static persona_definition_t persona_catalog[MAX_PERSONAS] = {
    {PERSONA_CALC_4BIT, "Calculator 4-bit", "Scientific calculator mode",
     ARCH_CALCULATOR_4BIT, 4, 1024, 0.1f},
    {PERSONA_CALC_16BIT, "Calculator 16-bit", "Advanced graphing calculator",
     ARCH_CALCULATOR_4BIT, 8, 65536, 0.2f},
    {PERSONA_RETRO_8BIT, "Retro 8-bit", "Classic computer emulation",
     ARCH_EMBEDDED, 8, 65536, 0.3f},
    {PERSONA_EMBEDDED_ARM, "Embedded ARM", "IoT microcontroller mode",
     ARCH_ARM64, 32, 262144, 0.4f},
    {PERSONA_MINI_X86, "Mini x86", "DOS-era PC emulation",
     ARCH_X86_16BIT, 16, 1048576, 0.5f},
    {PERSONA_MODERN_X86, "Modern x86", "Standard desktop PC",
     ARCH_X86_64BIT, 64, 4294967296ULL, 0.7f},
    {PERSONA_ARM_SBC, "ARM SBC", "Raspberry Pi mode",
     ARCH_ARM64, 64, 1073741824, 0.6f},
    {PERSONA_RISCV_DEV, "RISC-V Dev", "Development board mode",
     ARCH_RISCV64, 64, 2147483648ULL, 0.6f},
    {PERSONA_WORKSTATION, "Workstation", "Professional workstation",
     ARCH_X86_64BIT, 64, 34359738368ULL, 0.8f},
    {PERSONA_HPC_NODE, "HPC Node", "Supercomputer node",
     ARCH_X86_64BIT, 64, 1099511627776ULL, 0.9f}
};

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

float relu(float x) {
    return x > 0.0f ? x : 0.0f;
}

float random_float(void) {
    return (float)rand() / (float)RAND_MAX;
}

int min(int a, int b) {
    return a < b ? a : b;
}

float normalize_feature(float value, float min_val, float max_val) {
    if (max_val == min_val) return 0.5f;
    return (value - min_val) / (max_val - min_val);
}

uint32_t get_timestamp(void) {
    return (uint32_t)time(NULL);
}

// =============================================================================
// NEURAL NETWORK IMPLEMENTATION
// =============================================================================

neural_network_t* neural_network_init(void) {
    neural_network_t* nn = malloc(sizeof(neural_network_t));

    // Initialize weights with small random values (Xavier initialization)
    float weight_scale = sqrtf(2.0f / INPUT_FEATURES);

    for (int i = 0; i < INPUT_FEATURES; i++) {
        for (int j = 0; j < HIDDEN_NEURONS; j++) {
            nn->weights_ih[i][j] = (random_float() - 0.5f) * 2.0f * weight_scale;
        }
    }

    weight_scale = sqrtf(2.0f / HIDDEN_NEURONS);
    for (int i = 0; i < HIDDEN_NEURONS; i++) {
        for (int j = 0; j < OUTPUT_NEURONS; j++) {
            nn->weights_ho[i][j] = (random_float() - 0.5f) * 2.0f * weight_scale;
        }
    }

    // Initialize biases to zero
    for (int i = 0; i < HIDDEN_NEURONS; i++) {
        nn->bias_hidden[i] = 0.0f;
    }
    for (int i = 0; i < OUTPUT_NEURONS; i++) {
        nn->bias_output[i] = 0.0f;
    }

    nn->learning_rate = LEARNING_RATE;
    nn->training_epochs = 0;

    return nn;
}

void neural_network_forward(neural_network_t* nn, float* inputs, float* outputs) {
    float hidden[HIDDEN_NEURONS];

    // Input to hidden layer
    for (int i = 0; i < HIDDEN_NEURONS; i++) {
        hidden[i] = nn->bias_hidden[i];
        for (int j = 0; j < INPUT_FEATURES; j++) {
            hidden[i] += inputs[j] * nn->weights_ih[j][i];
        }
        hidden[i] = relu(hidden[i]);  // ReLU activation
    }

    // Hidden to output layer
    for (int i = 0; i < OUTPUT_NEURONS; i++) {
        outputs[i] = nn->bias_output[i];
        for (int j = 0; j < HIDDEN_NEURONS; j++) {
            outputs[i] += hidden[j] * nn->weights_ho[j][i];
        }
        outputs[i] = sigmoid(outputs[i]);  // Sigmoid activation for probabilities
    }
}

void neural_network_train_sample(neural_network_t* nn, float* inputs, float* targets) {
    float hidden[HIDDEN_NEURONS];
    float outputs[OUTPUT_NEURONS];

    // Forward pass
    neural_network_forward(nn, inputs, outputs);

    // Backward pass (simple gradient descent)
    float output_errors[OUTPUT_NEURONS];
    float hidden_errors[HIDDEN_NEURONS];

    // Calculate output layer errors
    for (int i = 0; i < OUTPUT_NEURONS; i++) {
        output_errors[i] = (targets[i] - outputs[i]) * outputs[i] * (1.0f - outputs[i]);
    }

    // Calculate hidden layer errors
    for (int i = 0; i < HIDDEN_NEURONS; i++) {
        hidden_errors[i] = 0.0f;
        for (int j = 0; j < OUTPUT_NEURONS; j++) {
            hidden_errors[i] += output_errors[j] * nn->weights_ho[i][j];
        }
        hidden_errors[i] *= (hidden[i] > 0.0f ? 1.0f : 0.0f);  // ReLU derivative
    }

    // Update weights and biases
    for (int i = 0; i < HIDDEN_NEURONS; i++) {
        for (int j = 0; j < OUTPUT_NEURONS; j++) {
            nn->weights_ho[i][j] += nn->learning_rate * output_errors[j] * hidden[i];
        }
    }

    for (int i = 0; i < INPUT_FEATURES; i++) {
        for (int j = 0; j < HIDDEN_NEURONS; j++) {
            nn->weights_ih[i][j] += nn->learning_rate * hidden_errors[j] * inputs[i];
        }
    }

    for (int i = 0; i < OUTPUT_NEURONS; i++) {
        nn->bias_output[i] += nn->learning_rate * output_errors[i];
    }

    for (int i = 0; i < HIDDEN_NEURONS; i++) {
        nn->bias_hidden[i] += nn->learning_rate * hidden_errors[i];
    }
}

// =============================================================================
// FEATURE EXTRACTION
// =============================================================================

void extract_features(hardware_capabilities_t* hw, context_info_t* context,
                     persona_usage_history_t* history, float* features) {
    int idx = 0;

    // Hardware features (normalized)
    features[idx++] = normalize_feature(log2f((float)hw->cpu_speed_mhz), 0, 15);  // 1 MHz to 32 GHz
    features[idx++] = normalize_feature(log2f((float)hw->available_ram_bytes), 10, 50);  // 1KB to 1PB
    features[idx++] = normalize_feature((float)hw->cpu_cores, 1, 128);
    features[idx++] = hw->has_mmu ? 1.0f : 0.0f;
    features[idx++] = hw->has_fpu ? 1.0f : 0.0f;
    features[idx++] = normalize_feature((float)hw->cpu_bits, 4, 128);
    features[idx++] = normalize_feature((float)hw->storage_speed_mbps, 1, 10000);
    features[idx++] = hw->power_level;

    // Context features
    features[idx++] = context->time_of_day;
    features[idx++] = context->day_of_week;
    features[idx++] = context->workload_intensity;
    features[idx++] = context->graphics_requirement;
    features[idx++] = context->network_requirement;

    // Usage history features (aggregate)
    float total_usage = 0;
    float weighted_satisfaction = 0;
    for (int i = 0; i < MAX_PERSONAS; i++) {
        total_usage += history[i].boot_count;
        weighted_satisfaction += history[i].user_satisfaction * history[i].boot_count;
    }

    features[idx++] = total_usage > 0 ? weighted_satisfaction / total_usage / 10.0f : 0.5f;
    features[idx++] = normalize_feature(total_usage, 0, 1000);
    features[idx++] = random_float(); // Noise feature for robustness
}

// =============================================================================
// AI ADVISOR IMPLEMENTATION
// =============================================================================

ai_advisor_t* ai_advisor_init(void) {
    ai_advisor_t* advisor = malloc(sizeof(ai_advisor_t));

    advisor->network = neural_network_init();
    advisor->training_samples = 0;
    advisor->total_recommendations = 0;
    advisor->correct_predictions = 0;
    advisor->accuracy = 0.0f;

    // Initialize usage history
    for (int i = 0; i < MAX_PERSONAS; i++) {
        advisor->history[i].boot_count = 0;
        advisor->history[i].runtime_minutes = 0;
        advisor->history[i].user_satisfaction = 5.0f;  // Neutral default
        advisor->history[i].avg_performance = 50.0f;
        advisor->history[i].last_used_timestamp = 0;
    }

    // Initialize feature normalization (will be updated during training)
    for (int i = 0; i < INPUT_FEATURES; i++) {
        advisor->feature_means[i] = 0.0f;
        advisor->feature_stds[i] = 1.0f;
    }

    return advisor;
}

void ai_advisor_add_training_sample(ai_advisor_t* advisor,
                                  hardware_capabilities_t* hw,
                                  context_info_t* context,
                                  persona_id_t chosen_persona,
                                  float satisfaction) {
    if (advisor->training_samples >= MAX_HISTORY) {
        // Remove oldest sample
        memmove(advisor->training_inputs, advisor->training_inputs + 1,
                sizeof(float) * INPUT_FEATURES * (MAX_HISTORY - 1));
        memmove(advisor->training_outputs, advisor->training_outputs + 1,
                sizeof(float) * OUTPUT_NEURONS * (MAX_HISTORY - 1));
        advisor->training_samples--;
    }

    // Extract features
    extract_features(hw, context, advisor->history,
                    advisor->training_inputs[advisor->training_samples]);

    // Create target output (one-hot encoding for top personas)
    for (int i = 0; i < OUTPUT_NEURONS; i++) {
        advisor->training_outputs[advisor->training_samples][i] = 0.0f;
    }

    // Set target for chosen persona (map to output neurons)
    int output_idx = chosen_persona % OUTPUT_NEURONS;
    advisor->training_outputs[advisor->training_samples][output_idx] = satisfaction / 10.0f;

    advisor->training_samples++;

    // Update usage history
    advisor->history[chosen_persona].boot_count++;
    advisor->history[chosen_persona].user_satisfaction =
        (advisor->history[chosen_persona].user_satisfaction + satisfaction) / 2.0f;
    advisor->history[chosen_persona].last_used_timestamp = get_timestamp();
}

void ai_advisor_train(ai_advisor_t* advisor, int epochs) {
    if (advisor->training_samples < 10) {
        printf("Not enough training samples (%d). Need at least 10.\n", advisor->training_samples);
        return;
    }

    printf("Training AI advisor with %d samples for %d epochs...\n",
           advisor->training_samples, epochs);

    for (int epoch = 0; epoch < epochs; epoch++) {
        float total_loss = 0.0f;

        // Train on all samples
        for (int i = 0; i < advisor->training_samples; i++) {
            neural_network_train_sample(advisor->network,
                                      advisor->training_inputs[i],
                                      advisor->training_outputs[i]);

            // Calculate loss for monitoring
            float outputs[OUTPUT_NEURONS];
            neural_network_forward(advisor->network, advisor->training_inputs[i], outputs);

            for (int j = 0; j < OUTPUT_NEURONS; j++) {
                float error = advisor->training_outputs[i][j] - outputs[j];
                total_loss += error * error;
            }
        }

        advisor->network->training_epochs++;

        if (epoch % 10 == 0) {
            printf("Epoch %d: Average loss = %.4f\n", epoch,
                   total_loss / (advisor->training_samples * OUTPUT_NEURONS));
        }
    }

    printf("Training completed. Total epochs: %d\n", advisor->network->training_epochs);
}

persona_recommendation_t* ai_advisor_get_recommendations(ai_advisor_t* advisor,
                                                       hardware_capabilities_t* hw,
                                                       context_info_t* context,
                                                       int* num_recommendations) {
    float features[INPUT_FEATURES];
    float outputs[OUTPUT_NEURONS];

    extract_features(hw, context, advisor->history, features);
    neural_network_forward(advisor->network, features, outputs);

    // Create recommendations based on network outputs
    persona_recommendation_t* recs = malloc(sizeof(persona_recommendation_t) * OUTPUT_NEURONS);

    for (int i = 0; i < OUTPUT_NEURONS; i++) {
        recs[i].persona_id = i; // Simple mapping for prototype
        recs[i].confidence = outputs[i];
        recs[i].predicted_satisfaction = outputs[i] * 10.0f;

        // Estimate performance based on hardware compatibility
        persona_definition_t* persona = &persona_catalog[recs[i].persona_id];
        if (hw->available_ram_bytes >= persona->min_ram_bytes &&
            hw->cpu_bits >= persona->min_cpu_bits) {
            recs[i].estimated_performance = 80.0f + outputs[i] * 20.0f;
        } else {
            recs[i].estimated_performance = 30.0f + outputs[i] * 30.0f;
        }

        snprintf(recs[i].rationale, sizeof(recs[i].rationale),
                "AI confidence: %.1f%%, based on %d previous uses of similar personas",
                outputs[i] * 100.0f, advisor->history[i].boot_count);
    }

    // Sort by confidence (simple bubble sort for prototype)
    for (int i = 0; i < OUTPUT_NEURONS - 1; i++) {
        for (int j = 0; j < OUTPUT_NEURONS - 1 - i; j++) {
            if (recs[j].confidence < recs[j + 1].confidence) {
                persona_recommendation_t temp = recs[j];
                recs[j] = recs[j + 1];
                recs[j + 1] = temp;
            }
        }
    }

    *num_recommendations = OUTPUT_NEURONS;
    advisor->total_recommendations++;

    return recs;
}

void ai_advisor_update_accuracy(ai_advisor_t* advisor, persona_id_t chosen_persona,
                               persona_recommendation_t* recommendations, int num_recs) {
    // Check if chosen persona is in top 3 recommendations
    bool correct = false;
    for (int i = 0; i < min(3, num_recs); i++) {
        if (recommendations[i].persona_id == chosen_persona) {
            correct = true;
            break;
        }
    }

    if (correct) {
        advisor->correct_predictions++;
    }

    advisor->accuracy = (float)advisor->correct_predictions / (float)advisor->total_recommendations;
}

// =============================================================================
// DEMONSTRATION AND TESTING
// =============================================================================

void print_hardware_info(hardware_capabilities_t* hw) {
    printf("Hardware Configuration:\n");
    printf("  CPU: %d-bit, %d cores, %d MHz\n", hw->cpu_bits, hw->cpu_cores, hw->cpu_speed_mhz);
    printf("  RAM: %lu MB available\n", (unsigned long)(hw->available_ram_bytes / 1024 / 1024));
    printf("  Features: MMU=%s, FPU=%s\n", hw->has_mmu ? "Yes" : "No", hw->has_fpu ? "Yes" : "No");
    printf("  Power: %.1f%%\n", hw->power_level * 100.0f);
}

void print_recommendations(persona_recommendation_t* recs, int num_recs) {
    printf("\nAI Persona Recommendations:\n");
    for (int i = 0; i < num_recs; i++) {
        persona_definition_t* persona = &persona_catalog[recs[i].persona_id];
        printf("  %d. %s\n", i + 1, persona->name);
        printf("     Confidence: %.1f%%\n", recs[i].confidence * 100.0f);
        printf("     Predicted Satisfaction: %.1f/10\n", recs[i].predicted_satisfaction);
        printf("     Estimated Performance: %.1f%%\n", recs[i].estimated_performance);
        printf("     Rationale: %s\n", recs[i].rationale);
        printf("\n");
    }
}

hardware_capabilities_t create_test_hardware(int test_case) {
    hardware_capabilities_t hw = {0};

    switch (test_case) {
        case 0: // Calculator
            hw.architecture = ARCH_CALCULATOR_4BIT;
            hw.cpu_bits = 4;
            hw.cpu_speed_mhz = 1;
            hw.cpu_cores = 1;
            hw.available_ram_bytes = 1024;
            hw.has_mmu = false;
            hw.has_fpu = false;
            hw.power_level = 0.8f;
            break;

        case 1: // Embedded ARM
            hw.architecture = ARCH_ARM64;
            hw.cpu_bits = 32;
            hw.cpu_speed_mhz = 100;
            hw.cpu_cores = 1;
            hw.available_ram_bytes = 256 * 1024;
            hw.has_mmu = false;
            hw.has_fpu = false;
            hw.power_level = 0.9f;
            break;

        case 2: // Raspberry Pi
            hw.architecture = ARCH_ARM64;
            hw.cpu_bits = 64;
            hw.cpu_speed_mhz = 1500;
            hw.cpu_cores = 4;
            hw.available_ram_bytes = 4ULL * 1024 * 1024 * 1024;
            hw.has_mmu = true;
            hw.has_fpu = true;
            hw.power_level = 1.0f;
            break;

        case 3: // Desktop PC
            hw.architecture = ARCH_X86_64BIT;
            hw.cpu_bits = 64;
            hw.cpu_speed_mhz = 3200;
            hw.cpu_cores = 8;
            hw.available_ram_bytes = 16ULL * 1024 * 1024 * 1024;
            hw.has_mmu = true;
            hw.has_fpu = true;
            hw.power_level = 1.0f;
            break;

        default:
            // Default to desktop PC
            hw = create_test_hardware(3);
            break;
    }

    return hw;
}

context_info_t create_test_context(int scenario) {
    context_info_t context = {0};

    switch (scenario) {
        case 0: // Morning work
            context.time_of_day = 0.33f;  // 8 AM
            context.day_of_week = 0.2f;   // Tuesday
            context.workload_intensity = 0.7f;
            context.graphics_requirement = 0.3f;
            context.network_requirement = 0.5f;
            break;

        case 1: // Evening gaming
            context.time_of_day = 0.83f;  // 8 PM
            context.day_of_week = 0.85f;  // Saturday
            context.workload_intensity = 0.9f;
            context.graphics_requirement = 0.9f;
            context.network_requirement = 0.8f;
            break;

        case 2: // Basic usage
            context.time_of_day = 0.5f;   // Noon
            context.day_of_week = 0.4f;   // Wednesday
            context.workload_intensity = 0.3f;
            context.graphics_requirement = 0.2f;
            context.network_requirement = 0.3f;
            break;

        default:
            context = create_test_context(2);
            break;
    }

    return context;
}

int main(void) {
    printf("=============================================================================\n");
    printf("TernaryBit OS v3.0 - AI Persona Advisor Engine Prototype\n");
    printf("=============================================================================\n\n");

    srand((unsigned int)time(NULL));

    // Initialize AI advisor
    ai_advisor_t* advisor = ai_advisor_init();

    printf("Phase 1: Generating synthetic training data...\n");

    // Generate synthetic training data
    for (int i = 0; i < 100; i++) {
        hardware_capabilities_t hw = create_test_hardware(i % 4);
        context_info_t context = create_test_context(i % 3);

        // Simulate user choices (with some logic)
        persona_id_t chosen_persona;
        float satisfaction;

        if (hw.available_ram_bytes < 1024 * 1024) {
            chosen_persona = PERSONA_CALC_4BIT;
            satisfaction = 8.0f + random_float() * 2.0f;
        } else if (hw.available_ram_bytes < 100 * 1024 * 1024) {
            chosen_persona = PERSONA_EMBEDDED_ARM;
            satisfaction = 7.0f + random_float() * 2.0f;
        } else if (hw.cpu_cores <= 2) {
            chosen_persona = PERSONA_ARM_SBC;
            satisfaction = 7.5f + random_float() * 2.0f;
        } else {
            chosen_persona = PERSONA_MODERN_X86;
            satisfaction = 8.5f + random_float() * 1.5f;
        }

        ai_advisor_add_training_sample(advisor, &hw, &context, chosen_persona, satisfaction);
    }

    printf("Generated %d training samples.\n\n", advisor->training_samples);

    printf("Phase 2: Training neural network...\n");
    ai_advisor_train(advisor, 50);
    printf("\n");

    printf("Phase 3: Testing recommendations...\n\n");

    // Test different hardware configurations
    const char* test_names[] = {"Calculator", "Embedded ARM", "Raspberry Pi", "Desktop PC"};

    for (int test = 0; test < 4; test++) {
        printf("Test Case %d: %s\n", test + 1, test_names[test]);
        printf("----------------------------------------\n");

        hardware_capabilities_t hw = create_test_hardware(test);
        context_info_t context = create_test_context(0); // Morning work scenario

        print_hardware_info(&hw);

        int num_recs;
        persona_recommendation_t* recs = ai_advisor_get_recommendations(advisor, &hw, &context, &num_recs);

        print_recommendations(recs, min(3, num_recs)); // Show top 3

        free(recs);
        printf("\n");
    }

    printf("Phase 4: Accuracy Testing...\n");

    // Test accuracy with new samples
    int correct = 0, total = 20;

    for (int i = 0; i < total; i++) {
        hardware_capabilities_t hw = create_test_hardware(i % 4);
        context_info_t context = create_test_context(i % 3);

        int num_recs;
        persona_recommendation_t* recs = ai_advisor_get_recommendations(advisor, &hw, &context, &num_recs);

        // Simulate "correct" choice based on hardware
        persona_id_t expected;
        if (hw.available_ram_bytes < 1024 * 1024) {
            expected = PERSONA_CALC_4BIT;
        } else if (hw.available_ram_bytes < 100 * 1024 * 1024) {
            expected = PERSONA_EMBEDDED_ARM;
        } else if (hw.cpu_cores <= 2) {
            expected = PERSONA_ARM_SBC;
        } else {
            expected = PERSONA_MODERN_X86;
        }

        // Check if expected is in top 3 recommendations
        bool found = false;
        for (int j = 0; j < min(3, num_recs); j++) {
            if (recs[j].persona_id == expected) {
                found = true;
                break;
            }
        }

        if (found) correct++;

        ai_advisor_update_accuracy(advisor, expected, recs, num_recs);
        free(recs);
    }

    printf("Accuracy Test Results:\n");
    printf("  Correct predictions: %d/%d (%.1f%%)\n", correct, total, (float)correct * 100.0f / total);
    printf("  Overall AI accuracy: %.1f%%\n", advisor->accuracy * 100.0f);
    printf("  Total recommendations made: %d\n", advisor->total_recommendations);
    printf("\n");

    printf("Phase 5: Performance Statistics...\n");
    printf("Neural Network Configuration:\n");
    printf("  Input features: %d\n", INPUT_FEATURES);
    printf("  Hidden neurons: %d\n", HIDDEN_NEURONS);
    printf("  Output neurons: %d\n", OUTPUT_NEURONS);
    printf("  Training epochs completed: %d\n", advisor->network->training_epochs);
    printf("  Learning rate: %.4f\n", advisor->network->learning_rate);
    printf("\nUsage History:\n");
    for (int i = 0; i < MAX_PERSONAS; i++) {
        if (advisor->history[i].boot_count > 0) {
            printf("  %s: %d boots, %.1f avg satisfaction\n",
                   persona_catalog[i].name,
                   advisor->history[i].boot_count,
                   advisor->history[i].user_satisfaction);
        }
    }

    printf("\n=============================================================================\n");
    printf("AI Persona Advisor Prototype Demo Complete!\n");
    printf("=============================================================================\n");

    // Cleanup
    free(advisor->network);
    free(advisor);

    return 0;
}