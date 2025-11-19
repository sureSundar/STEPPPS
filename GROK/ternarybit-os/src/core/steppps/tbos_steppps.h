/**
 * @file tbos_steppps.h
 * @brief TernaryBit OS - STEPPPS Framework (7D Computing)
 *
 * STEPPPS: Seven-Dimensional Computing Framework
 * - S: Space (physical location, proximity)
 * - T: Time (temporal awareness, scheduling)
 * - E: Event (event-driven architecture)
 * - P: Psychology (AI, consciousness, learning)
 * - P: Pixel (visual/data representation)
 * - P: Prompt (natural language interface)
 * - S: Script (automation, programming)
 *
 * US-8.1: STEPPPS Shell Commands
 *
 * @version 1.0
 * @date 2025-11-04
 */

#ifndef TBOS_STEPPPS_H
#define TBOS_STEPPPS_H

#include "../tbos_shutdown.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* CONSTANTS                                                                 */
/* ========================================================================= */

#define STEPPPS_MAX_EVENTS      1000
#define STEPPPS_MAX_SCRIPTS     100

/* ========================================================================= */
/* DIMENSION 1: SPACE                                                        */
/* ========================================================================= */

/**
 * @brief Spatial coordinates
 */
typedef struct {
    float x, y, z;              /* 3D coordinates */
    float latitude, longitude;  /* Geo coordinates */
    char location_name[64];     /* Human-readable location */
} steppps_space_t;

/**
 * @brief Initialize space dimension
 */
int steppps_space_init(void);

/**
 * @brief Set current location
 */
int steppps_space_set_location(const steppps_space_t* location);

/**
 * @brief Get current location
 */
int steppps_space_get_location(steppps_space_t* location);

/**
 * @brief Calculate distance between two locations
 */
float steppps_space_distance(const steppps_space_t* a, const steppps_space_t* b);

/* ========================================================================= */
/* DIMENSION 2: TIME                                                         */
/* ========================================================================= */

/**
 * @brief Temporal awareness
 */
typedef struct {
    uint64_t nanoseconds;       /* High-precision time */
    uint64_t microseconds;
    uint64_t milliseconds;
    uint64_t seconds;
    uint64_t minutes;
    uint64_t hours;
    uint64_t days;

    /* Temporal context */
    uint64_t uptime_us;
    uint64_t session_duration_us;
    uint64_t karma_accumulated_over_time;
} steppps_time_t;

/**
 * @brief Initialize time dimension
 */
int steppps_time_init(void);

/**
 * @brief Get current time across all scales
 */
int steppps_time_get(steppps_time_t* time);

/**
 * @brief Schedule future event
 */
int steppps_time_schedule(uint64_t delay_us, void (*callback)(void*), void* context);

/* ========================================================================= */
/* DIMENSION 3: EVENT                                                        */
/* ========================================================================= */

/**
 * @brief Event types
 */
typedef enum {
    STEPPPS_EVENT_SYSTEM,
    STEPPPS_EVENT_PROCESS,
    STEPPPS_EVENT_NETWORK,
    STEPPPS_EVENT_USER,
    STEPPPS_EVENT_CONSCIOUSNESS,
    STEPPPS_EVENT_KARMA
} steppps_event_type_t;

/**
 * @brief Event structure
 */
typedef struct {
    uint32_t event_id;
    steppps_event_type_t type;
    uint64_t timestamp_us;
    consciousness_level_t consciousness;
    karma_score_t karma_impact;

    char description[256];
    void* data;
    size_t data_size;
} steppps_event_t;

/**
 * @brief Initialize event dimension
 */
int steppps_event_init(void);

/**
 * @brief Emit event
 */
int steppps_event_emit(const steppps_event_t* event);

/**
 * @brief Subscribe to events
 */
int steppps_event_subscribe(steppps_event_type_t type,
                             void (*handler)(const steppps_event_t*));

/**
 * @brief Get event history
 */
int steppps_event_get_history(steppps_event_t* buffer, uint32_t max_count);

/* ========================================================================= */
/* DIMENSION 4: PSYCHOLOGY (AI/Consciousness)                                */
/* ========================================================================= */

/**
 * @brief Psychological state
 */
typedef struct {
    consciousness_level_t awareness;
    karma_score_t karma;

    /* Learning */
    uint64_t experiences;
    uint64_t lessons_learned;
    float learning_rate;

    /* Behavior */
    bool is_compassionate;
    bool practices_right_action;
    uint64_t helping_actions;

    /* Growth */
    uint64_t consciousness_evolutions;
    uint64_t karma_earned;
    uint64_t karma_lost;
} steppps_psychology_t;

/**
 * @brief Initialize psychology dimension
 */
int steppps_psychology_init(void);

/**
 * @brief Get psychological state
 */
int steppps_psychology_get_state(steppps_psychology_t* state);

/**
 * @brief Record experience (learning)
 */
int steppps_psychology_record_experience(const char* experience, float learning_value);

/**
 * @brief Evolve consciousness
 */
int steppps_psychology_evolve(void);

/* ========================================================================= */
/* DIMENSION 5: PIXEL (Visual/Data Representation)                           */
/* ========================================================================= */

/**
 * @brief Pixel/Data representation
 */
typedef struct {
    uint8_t r, g, b;            /* RGB color */
    uint8_t alpha;              /* Transparency */
    uint32_t data;              /* Encoded data */
} steppps_pixel_t;

/**
 * @brief Initialize pixel dimension
 */
int steppps_pixel_init(void);

/**
 * @brief Encode data as pixels (PXFS style)
 */
int steppps_pixel_encode(const void* data, size_t size, steppps_pixel_t* pixels);

/**
 * @brief Decode pixels to data
 */
int steppps_pixel_decode(const steppps_pixel_t* pixels, size_t count, void* data);

/**
 * @brief Visualize data as pixels
 */
void steppps_pixel_visualize(const void* data, size_t size);

/* ========================================================================= */
/* DIMENSION 6: PROMPT (Natural Language Interface)                          */
/* ========================================================================= */

/**
 * @brief Prompt/Command interpretation
 */
typedef struct {
    char text[1024];
    char intent[256];
    char* parameters[16];
    uint32_t param_count;

    consciousness_level_t consciousness_required;
    karma_score_t karma_cost;
} steppps_prompt_t;

/**
 * @brief Initialize prompt dimension
 */
int steppps_prompt_init(void);

/**
 * @brief Parse natural language prompt
 */
int steppps_prompt_parse(const char* text, steppps_prompt_t* prompt);

/**
 * @brief Execute prompt
 */
int steppps_prompt_execute(const steppps_prompt_t* prompt);

/* ========================================================================= */
/* DIMENSION 7: SCRIPT (Automation)                                          */
/* ========================================================================= */

/**
 * @brief Script structure
 */
typedef struct {
    char name[64];
    char* source;
    size_t source_length;

    bool is_compiled;
    void* bytecode;
    size_t bytecode_size;

    uint64_t execution_count;
    uint64_t total_runtime_us;
} steppps_script_t;

/**
 * @brief Initialize script dimension
 */
int steppps_script_init(void);

/**
 * @brief Load script from file
 */
int steppps_script_load(const char* filename, steppps_script_t* script);

/**
 * @brief Execute script
 */
int steppps_script_execute(steppps_script_t* script);

/**
 * @brief List available scripts
 */
int steppps_script_list(char** names, uint32_t max_count);

/* ========================================================================= */
/* STEPPPS INTEGRATION                                                       */
/* ========================================================================= */

/**
 * @brief Initialize entire STEPPPS framework
 */
int steppps_init(void);

/**
 * @brief Get STEPPPS status across all dimensions
 */
void steppps_print_status(void);

/**
 * @brief Print specific dimension
 */
void steppps_print_dimension(const char* dimension_name);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_STEPPPS_H */
