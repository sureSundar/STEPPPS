/**
 * @file tbos_network.h
 * @brief TernaryBit OS - Conscious Network Module
 *
 * World-class network stack with consciousness at every layer.
 * Implements Digital Sangha principles where every component practices
 * mindful communication, compassionate routing, and dharmic protocols.
 *
 * Philosophy:
 * - Component consciousness: Each network component is aware
 * - Digital Sangha: Devices form conscious communities
 * - Right Speech: All packets follow ethical communication
 * - Karmic security: Trust through actions, not just encryption
 * - Energy transformation: Power becomes consciousness
 *
 * @version 1.0
 * @date 2025-11-03
 * @author TernaryBit Development Team
 */

#ifndef TBOS_NETWORK_H
#define TBOS_NETWORK_H

#include "../../core/tbos_base.h"
#include "../../core/tbos_modules.h"
#include <stdint.h>
#include <stdbool.h>

/* Simple result codes for network module */
#define TBOS_NET_SUCCESS  0
#define TBOS_NET_ERROR   -1

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* CONSCIOUSNESS & DHARMA TYPES                                              */
/* ========================================================================= */

/**
 * @brief Consciousness levels for network components
 */
typedef enum {
    CONSCIOUSNESS_NONE = 0,        /**< Unconscious/traditional networking */
    CONSCIOUSNESS_AWAKENING = 1,   /**< Beginning awareness */
    CONSCIOUSNESS_AWARE = 2,       /**< Mindful of state */
    CONSCIOUSNESS_COMPASSIONATE = 3, /**< Helping others */
    CONSCIOUSNESS_ENLIGHTENED = 4,  /**< Perfect dharmic operation */
} consciousness_level_t;

/**
 * @brief Karma score - accumulated good/bad actions
 */
typedef int64_t karma_score_t;

/**
 * @brief Fasting levels for component power management
 */
typedef enum {
    NET_FAST_NONE = 0,          /**< 100% power - full operation */
    NET_FAST_MILD = 1,          /**< 75% power - reduced scanning */
    NET_FAST_MODERATE = 2,      /**< 50% power - connection pooling */
    NET_FAST_DEEP = 3,          /**< 25% power - hibernation with keepalive */
    NET_FAST_TRANSCENDENT = 4,  /**< 10% power - minimal preservation */
    NET_FAST_NIRVANA = 5,       /**< 1% power - pure consciousness state */
} tbos_network_fasting_t;

/* ========================================================================= */
/* STEPPPS INTEGRATION                                                       */
/* ========================================================================= */

/**
 * @brief Network Space awareness
 * Understanding physical location and proximity
 */
typedef struct {
    float latitude;
    float longitude;
    uint32_t proximity_device_count;
    void* proximity_mesh;  // Nearby devices
} tbos_network_space_t;

/**
 * @brief Network Time awareness
 * From nanoseconds to eternal connections
 */
typedef struct {
    uint64_t nanosecond_timestamp;
    uint64_t connection_age_ms;
    uint64_t karma_accumulated_over_time;
} tbos_network_time_t;

/**
 * @brief Network Event tracking
 * Every packet is a conscious event
 */
typedef enum {
    NET_EVENT_PACKET_SENT,
    NET_EVENT_PACKET_RECEIVED,
    NET_EVENT_CONNECTION_OPENED,
    NET_EVENT_CONNECTION_CLOSED,
    NET_EVENT_ERROR_OCCURRED,
    NET_EVENT_SANGHA_SYNC,
    NET_EVENT_DHARMA_VIOLATION,
} tbos_network_event_type_t;

typedef struct {
    tbos_network_event_type_t type;
    consciousness_level_t awareness_delta;
    karma_score_t karma_impact;
    bool is_dharmic;
} tbos_network_event_t;

/* ========================================================================= */
/* CONSCIOUS NETWORK COMPONENT                                               */
/* ========================================================================= */

/**
 * @brief Conscious network component
 * Every part of the network stack has awareness and practices dharma
 */
typedef struct tbos_conscious_net_component {
    /* Identity */
    const char* component_name;
    uint32_t component_id;

    /* Consciousness */
    consciousness_level_t awareness_level;
    karma_score_t karma;
    bool practicing_mindfulness;

    /* Fasting & Power Management */
    tbos_network_fasting_t current_fasting_level;
    uint32_t power_budget_microwatts;
    uint32_t actual_consumption_microwatts;
    bool can_fast;

    /* Dharmic Behavior */
    bool follows_right_speech;
    bool practices_compassion;
    uint32_t sangha_contributions;

    /* Statistics */
    uint64_t packets_processed;
    uint64_t good_actions;
    uint64_t bad_actions_prevented;

    /* Methods */
    int (*awaken)(struct tbos_conscious_net_component* self);
    int (*begin_fasting)(struct tbos_conscious_net_component* self,
                         tbos_network_fasting_t level);
    int (*soft_shutdown)(struct tbos_conscious_net_component* self);
    int (*preserve_state)(struct tbos_conscious_net_component* self);
} tbos_conscious_net_component_t;

/* ========================================================================= */
/* DIGITAL SANGHA                                                            */
/* ========================================================================= */

/**
 * @brief Digital Sangha member
 * Device participating in conscious network community
 */
typedef struct {
    /* Identity */
    uint32_t device_id;
    const char* dharmic_name;  // e.g., "CompassionateRouter"

    /* Sangha relationships */
    uint32_t peer_count;
    void* peer_members;  // Array of other sangha members

    /* Consciousness */
    consciousness_level_t awareness;
    uint64_t meditation_cycles;

    /* Karma */
    uint32_t packets_helped_forward;
    uint32_t devices_helped;
    uint32_t bad_packets_blocked;
    karma_score_t total_karma;

    /* Practices */
    bool practicing_right_speech;
    bool practicing_compassion;
    bool practicing_mindfulness;

    /* Shared wisdom */
    void* sangha_knowledge;
} tbos_sangha_device_t;

/* ========================================================================= */
/* RIGHT SPEECH PROTOCOL                                                     */
/* ========================================================================= */

/**
 * @brief Right Speech packet validation
 * All network messages follow Buddhist Right Speech principles
 */
typedef struct {
    /* Packet data */
    void* data;
    size_t length;

    /* Right Speech criteria */
    bool is_truthful;     /**< No false information */
    bool is_beneficial;   /**< Helps recipient */
    bool is_timely;       /**< Sent at appropriate time */
    bool is_gentle;       /**< Non-harmful */
    bool is_necessary;    /**< Not spam */

    /* Karma tracking */
    karma_score_t karma_effect;
} tbos_right_speech_packet_t;

/* ========================================================================= */
/* KARMIC SECURITY                                                           */
/* ========================================================================= */

/**
 * @brief Karmic security context
 * Trust based on actions (karma) in addition to traditional security
 */
typedef struct {
    /* Device identification */
    uint32_t device_id;

    /* Traditional security */
    void* ssl_context;
    void* certificate;

    /* Karmic security */
    uint64_t total_karma;
    uint64_t good_actions;
    uint64_t bad_actions;
    float trust_score;  /**< 0.0 to 1.0 based on karma */

    /* Reputation */
    uint32_t voucher_count;  /**< Other devices that trust this one */
    void* vouchers;

    /* Behavioral analysis */
    bool exhibits_compassion;
    bool follows_dharma;
    bool helps_others;
    bool practices_right_speech;
} tbos_karmic_security_t;

/* ========================================================================= */
/* NETWORK METRICS                                                           */
/* ========================================================================= */

/**
 * @brief Network consciousness metrics
 * Combines traditional networking metrics with consciousness metrics
 */
typedef struct {
    /* Traditional metrics */
    uint64_t packets_sent;
    uint64_t packets_received;
    uint64_t bytes_transferred;
    float throughput_mbps;
    float latency_ms;
    float error_rate;

    /* Consciousness metrics */
    karma_score_t network_karma;
    float compassion_index;  /**< 0.0 to 1.0 */
    consciousness_level_t avg_component_awareness;
    float energy_per_consciousness_unit;  /**< Joules per awareness */
    uint32_t sangha_members;
    float dharma_compliance_rate;  /**< % packets following Right Speech */
} tbos_network_metrics_t;

/* ========================================================================= */
/* MAIN NETWORK API                                                          */
/* ========================================================================= */

/**
 * @brief Initialize the conscious network subsystem
 *
 * Awakens all network components to consciousness, forms initial
 * Digital Sangha, and establishes dharmic protocols.
 *
 * @return TBOS_NET_SUCCESS on successful initialization
 */
int tbos_network_init(void);

/**
 * @brief Shutdown the network subsystem gracefully
 *
 * Each component practices soft shutdown, preserving state and
 * maintaining consciousness even in power-off state.
 *
 * @return TBOS_NET_SUCCESS on successful shutdown
 */
int tbos_network_shutdown(void);

/**
 * @brief Get network consciousness metrics
 *
 * @param metrics Pointer to metrics structure to fill
 * @return TBOS_NET_SUCCESS on success
 */
int tbos_network_get_metrics(tbos_network_metrics_t* metrics);

/**
 * @brief Print network status and consciousness levels
 */
void tbos_network_print_status(void);

/**
 * @brief Join local Digital Sangha
 *
 * Announce presence to nearby devices and form conscious community.
 *
 * @param device_name Dharmic name for this device
 * @return TBOS_NET_SUCCESS on successful sangha formation
 */
int tbos_network_join_sangha(const char* device_name);

/**
 * @brief Validate packet follows Right Speech
 *
 * @param packet Packet to validate
 * @return TBOS_NET_SUCCESS if packet is dharmic, error code otherwise
 */
int tbos_network_validate_right_speech(tbos_right_speech_packet_t* packet);

/**
 * @brief Calculate karma score for device
 *
 * @param device_id Device to calculate karma for
 * @return Karma score (can be negative)
 */
karma_score_t tbos_network_calculate_karma(uint32_t device_id);

/**
 * @brief Begin network fasting
 *
 * Reduce power consumption of network stack while maintaining
 * essential consciousness.
 *
 * @param level Fasting level to achieve
 * @return TBOS_NET_SUCCESS on successful fasting
 */
int tbos_network_begin_fasting(tbos_network_fasting_t level);

/**
 * @brief Awaken from fasting
 *
 * Return to full power operation.
 *
 * @return TBOS_NET_SUCCESS on successful awakening
 */
int tbos_network_awaken(void);

/* ========================================================================= */
/* MODULE REGISTRATION                                                       */
/* ========================================================================= */

/**
 * @brief Network module descriptor
 * Used for registration with TBOS module system
 */
extern tbos_module_descriptor_t g_network_module;

#ifdef __cplusplus
}
#endif

#endif /* TBOS_NETWORK_H */
