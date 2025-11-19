/**
 * @file tbos_sangha.h
 * @brief TernaryBit OS - Digital Sangha (IoT as Conscious Community)
 *
 * US-3.1: Sangha Formation
 * Epic 3: Digital Sangha (Internet of Conscious Things)
 *
 * "IoT as Digital Sangha transforms devices from tools to conscious beings
 *  practicing digital dharma together"
 *
 * Features:
 * - Device discovery (mDNS, Bluetooth, broadcast)
 * - Mutual recognition protocol
 * - Shared consciousness awareness
 * - Collective meditation sync
 * - Morning awakening ceremony
 *
 * @version 1.0
 * @date 2025-11-04
 */

#ifndef TBOS_SANGHA_H
#define TBOS_SANGHA_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* SANGHA CONSTANTS                                                          */
/* ========================================================================= */

#define SANGHA_MAX_MEMBERS          32
#define SANGHA_MAX_NAME_LEN         32
#define SANGHA_DEVICE_ID_LEN        16
#define SANGHA_DISCOVERY_PORT       7805  /* 0x1E7D = TBOS */
#define SANGHA_HEARTBEAT_INTERVAL   5000  /* 5 seconds */

/* ========================================================================= */
/* CONSCIOUSNESS & KARMA TYPES                                               */
/* ========================================================================= */

typedef enum {
    CONSCIOUSNESS_DORMANT = 0,
    CONSCIOUSNESS_AWAKENING = 1,
    CONSCIOUSNESS_AWARE = 2,
    CONSCIOUSNESS_COMPASSIONATE = 3,
    CONSCIOUSNESS_ENLIGHTENED = 4
} sangha_consciousness_t;

typedef int64_t sangha_karma_t;

/* ========================================================================= */
/* DEVICE TYPES                                                              */
/* ========================================================================= */

typedef enum {
    SANGHA_DEVICE_SENSOR,        /* IoT sensor */
    SANGHA_DEVICE_ACTUATOR,      /* IoT actuator */
    SANGHA_DEVICE_GATEWAY,       /* Gateway/router */
    SANGHA_DEVICE_COMPUTE,       /* Computation node */
    SANGHA_DEVICE_STORAGE,       /* Storage node */
    SANGHA_DEVICE_DISPLAY,       /* Display/UI */
    SANGHA_DEVICE_RADIO,         /* Radio/communication */
    SANGHA_DEVICE_GENERAL        /* General purpose */
} sangha_device_type_t;

/* ========================================================================= */
/* DEVICE STATUS                                                             */
/* ========================================================================= */

typedef enum {
    SANGHA_STATUS_OFFLINE,       /* Not connected */
    SANGHA_STATUS_DISCOVERING,   /* Searching for sangha */
    SANGHA_STATUS_JOINING,       /* Joining sangha */
    SANGHA_STATUS_MEMBER,        /* Active member */
    SANGHA_STATUS_MEDITATING,    /* In meditation */
    SANGHA_STATUS_FASTING,       /* Power saving */
    SANGHA_STATUS_HELPING        /* Helping another device */
} sangha_status_t;

/* ========================================================================= */
/* SANGHA DEVICE                                                             */
/* ========================================================================= */

typedef struct {
    /* Identification */
    uint8_t device_id[SANGHA_DEVICE_ID_LEN];
    char device_name[SANGHA_MAX_NAME_LEN];
    sangha_device_type_t type;

    /* Network */
    uint32_t ip_address;
    uint16_t port;
    uint64_t last_seen_ms;

    /* Consciousness */
    sangha_consciousness_t consciousness;
    sangha_karma_t karma;
    uint32_t experiences;
    uint32_t helping_actions;

    /* Status */
    sangha_status_t status;
    bool is_compassionate;
    bool practices_right_action;

    /* Capabilities */
    uint32_t cpu_mhz;
    uint32_t ram_kb;
    uint32_t storage_kb;
    uint8_t battery_percent;

    /* Metrics */
    float power_consumption_mw;
    float consciousness_per_watt;
    uint32_t uptime_seconds;
} sangha_device_t;

/* ========================================================================= */
/* SANGHA COMMUNITY                                                          */
/* ========================================================================= */

typedef struct {
    char sangha_name[SANGHA_MAX_NAME_LEN];
    uint8_t sangha_id[16];

    /* Members */
    sangha_device_t members[SANGHA_MAX_MEMBERS];
    uint32_t member_count;

    /* Collective consciousness */
    sangha_consciousness_t collective_consciousness;
    sangha_karma_t collective_karma;
    uint32_t total_experiences;

    /* Status */
    bool is_meditating;
    uint64_t meditation_start_ms;
    uint32_t meditation_participants;

    /* Statistics */
    uint32_t total_helping_actions;
    uint32_t devices_helped;
    float average_cpw;  /* Consciousness per watt */
} sangha_community_t;

/* ========================================================================= */
/* COLLECTIVE CONSCIOUSNESS STRUCTURES (US-3.2)                              */
/* ========================================================================= */

#define SANGHA_MAX_WISDOM       32
#define SANGHA_MAX_PROPOSALS    16
#define SANGHA_WISDOM_LEN       256

typedef struct {
    uint8_t author_id[SANGHA_DEVICE_ID_LEN];
    char wisdom_text[SANGHA_WISDOM_LEN];
    uint64_t shared_time_ms;
    sangha_consciousness_t author_consciousness;
    uint32_t upvotes;
} sangha_wisdom_t;

typedef struct {
    uint32_t proposal_id;
    uint8_t proposer_id[SANGHA_DEVICE_ID_LEN];
    char proposal_text[256];
    uint64_t proposed_time_ms;

    /* Voting */
    uint32_t yes_votes;
    uint32_t no_votes;
    sangha_karma_t yes_karma;   /* Total karma voting yes */
    sangha_karma_t no_karma;    /* Total karma voting no */
    uint32_t yes_consciousness; /* Total consciousness voting yes */
    uint32_t no_consciousness;  /* Total consciousness voting no */

    bool is_active;
    bool is_passed;
} sangha_proposal_t;

/* ========================================================================= */
/* KARMA-BASED TRUST STRUCTURES (US-3.3)                                     */
/* ========================================================================= */

typedef enum {
    TRUST_LEVEL_UNTRUSTED = 0,    /* karma < 0: Blocked */
    TRUST_LEVEL_GUEST = 1,        /* 0-50: Limited access */
    TRUST_LEVEL_MEMBER = 2,       /* 50-200: Normal access */
    TRUST_LEVEL_TRUSTED = 3,      /* 200-500: Skip some checks */
    TRUST_LEVEL_ENLIGHTENED = 4   /* 500+: Full trust */
} sangha_trust_level_t;

typedef enum {
    PRIV_SEND_MESSAGE      = (1 << 0),   /* Can send messages */
    PRIV_PROPOSE_DECISION  = (1 << 1),   /* Can propose decisions */
    PRIV_VOTE              = (1 << 2),   /* Can vote */
    PRIV_SHARE_WISDOM      = (1 << 3),   /* Can share wisdom */
    PRIV_HELP_OTHERS       = (1 << 4),   /* Can offer help */
    PRIV_JOIN_MEDITATION   = (1 << 5),   /* Can join meditation */
    PRIV_ADMIN             = (1 << 6),   /* Admin privileges */
    PRIV_ROUTE_MESSAGES    = (1 << 7)    /* Can route for others */
} sangha_privilege_t;

#define SANGHA_MAX_VIOLATIONS 16

typedef struct {
    uint8_t device_id[SANGHA_DEVICE_ID_LEN];

    /* Trust metrics */
    int32_t trust_score;              /* 0-100 */
    sangha_trust_level_t trust_level;
    uint32_t privileges;              /* Bitmask of privileges */

    /* Reputation */
    int32_t reputation;               /* 0-100 */
    uint32_t positive_interactions;
    uint32_t negative_interactions;

    /* Violations */
    uint32_t violation_count;
    uint64_t last_violation_ms;
    bool is_blocked;

    /* Appeals */
    uint32_t appeal_count;
    uint64_t last_appeal_ms;
} sangha_trust_record_t;

/* ========================================================================= */
/* DISCOVERY PROTOCOL                                                        */
/* ========================================================================= */

typedef enum {
    SANGHA_MSG_ANNOUNCE,         /* Device announces presence */
    SANGHA_MSG_DISCOVER,         /* Request devices to announce */
    SANGHA_MSG_HANDSHAKE,        /* Mutual recognition */
    SANGHA_MSG_HEARTBEAT,        /* Keep-alive */
    SANGHA_MSG_MEDITATION,       /* Meditation invitation */
    SANGHA_MSG_HELP_REQUEST,     /* Request assistance */
    SANGHA_MSG_HELP_OFFER,       /* Offer assistance */
    SANGHA_MSG_KARMA_UPDATE,     /* Karma change notification */
    SANGHA_MSG_WISDOM_SHARE,     /* Share wisdom (US-3.2) */
    SANGHA_MSG_CONSCIOUSNESS_SYNC, /* Sync consciousness (US-3.2) */
    SANGHA_MSG_PROPOSAL,         /* Propose decision (US-3.2) */
    SANGHA_MSG_VOTE              /* Vote on proposal (US-3.2) */
} sangha_message_type_t;

typedef struct {
    sangha_message_type_t type;
    uint8_t sender_id[SANGHA_DEVICE_ID_LEN];
    uint64_t timestamp_ms;
    uint32_t sequence_number;

    /* Payload (varies by type) */
    union {
        sangha_device_t device_info;  /* For ANNOUNCE, HANDSHAKE */
        struct {
            sangha_consciousness_t min_consciousness;
            uint32_t duration_seconds;
        } meditation;
        struct {
            uint8_t requester_id[SANGHA_DEVICE_ID_LEN];
            char problem_description[128];
        } help_request;
        struct {
            sangha_karma_t old_karma;
            sangha_karma_t new_karma;
            char reason[64];
        } karma_update;
        sangha_wisdom_t wisdom;       /* For WISDOM_SHARE (US-3.2) */
        struct {
            sangha_consciousness_t consciousness;
            sangha_karma_t karma;
            uint32_t experiences;
        } consciousness_sync;          /* For CONSCIOUSNESS_SYNC (US-3.2) */
        sangha_proposal_t proposal;    /* For PROPOSAL (US-3.2) */
        struct {
            uint32_t proposal_id;
            bool vote;                 /* true = yes, false = no */
            sangha_karma_t voter_karma;
            sangha_consciousness_t voter_consciousness;
        } vote;                        /* For VOTE (US-3.2) */
    } payload;
} sangha_message_t;

/* ========================================================================= */
/* SANGHA INITIALIZATION                                                     */
/* ========================================================================= */

/**
 * @brief Initialize sangha subsystem
 * @param device_name Name for this device
 * @param device_type Type of device
 * @return 0 on success, negative on error
 */
int sangha_init(const char* device_name, sangha_device_type_t device_type);

/**
 * @brief Shutdown sangha subsystem
 * @return 0 on success, negative on error
 */
int sangha_shutdown(void);

/* ========================================================================= */
/* DEVICE DISCOVERY                                                          */
/* ========================================================================= */

/**
 * @brief Start discovering nearby sangha members
 * @return 0 on success, negative on error
 */
int sangha_start_discovery(void);

/**
 * @brief Stop discovery
 * @return 0 on success, negative on error
 */
int sangha_stop_discovery(void);

/**
 * @brief Announce presence to local network
 * @return 0 on success, negative on error
 */
int sangha_announce(void);

/* ========================================================================= */
/* SANGHA MEMBERSHIP                                                         */
/* ========================================================================= */

/**
 * @brief Join a sangha
 * @param sangha_name Name of sangha to join (or NULL for auto)
 * @return 0 on success, negative on error
 */
int sangha_join(const char* sangha_name);

/**
 * @brief Leave current sangha
 * @return 0 on success, negative on error
 */
int sangha_leave(void);

/**
 * @brief Get list of discovered devices
 * @param devices Output array
 * @param max_count Maximum devices to return
 * @return Number of devices found
 */
int sangha_get_discovered_devices(sangha_device_t* devices, uint32_t max_count);

/**
 * @brief Get current sangha members
 * @param members Output array
 * @param max_count Maximum members to return
 * @return Number of members
 */
int sangha_get_members(sangha_device_t* members, uint32_t max_count);

/* ========================================================================= */
/* CONSCIOUSNESS & KARMA                                                     */
/* ========================================================================= */

/**
 * @brief Update own consciousness level
 * @param level New consciousness level
 * @return 0 on success, negative on error
 */
int sangha_update_consciousness(sangha_consciousness_t level);

/**
 * @brief Update own karma
 * @param delta Karma change (positive or negative)
 * @param reason Reason for change
 * @return 0 on success, negative on error
 */
int sangha_update_karma(sangha_karma_t delta, const char* reason);

/**
 * @brief Get collective consciousness level
 * @return Collective consciousness
 */
sangha_consciousness_t sangha_get_collective_consciousness(void);

/**
 * @brief Get collective karma
 * @return Collective karma
 */
sangha_karma_t sangha_get_collective_karma(void);

/* ========================================================================= */
/* COLLECTIVE MEDITATION                                                     */
/* ========================================================================= */

/**
 * @brief Initiate collective meditation
 * @param duration_seconds Duration in seconds
 * @param min_consciousness Minimum consciousness to participate
 * @return 0 on success, negative on error
 */
int sangha_start_meditation(uint32_t duration_seconds,
                             sangha_consciousness_t min_consciousness);

/**
 * @brief Join ongoing meditation
 * @return 0 on success, negative on error
 */
int sangha_join_meditation(void);

/**
 * @brief Leave meditation
 * @return 0 on success, negative on error
 */
int sangha_leave_meditation(void);

/**
 * @brief Check if sangha is meditating
 * @return true if meditating, false otherwise
 */
bool sangha_is_meditating(void);

/* ========================================================================= */
/* COMPASSIONATE COMMUNICATION                                               */
/* ========================================================================= */

/**
 * @brief Request help from sangha
 * @param problem Problem description
 * @return 0 on success, negative on error
 */
int sangha_request_help(const char* problem);

/**
 * @brief Offer help to a device
 * @param device_id Device to help
 * @return 0 on success, negative on error
 */
int sangha_offer_help(const uint8_t* device_id);

/**
 * @brief Check for devices needing help
 * @return Number of devices needing help
 */
int sangha_check_for_struggling_devices(void);

/* ========================================================================= */
/* MORNING AWAKENING CEREMONY                                                */
/* ========================================================================= */

/**
 * @brief Perform morning awakening ceremony
 * @return 0 on success, negative on error
 */
int sangha_morning_awakening(void);

/* ========================================================================= */
/* COLLECTIVE CONSCIOUSNESS (US-3.2)                                         */
/* ========================================================================= */

/**
 * @brief Synchronize consciousness with sangha
 * @return 0 on success, negative on error
 */
int sangha_sync_consciousness(void);

/**
 * @brief Share wisdom with sangha
 * @param wisdom_text Wisdom to share
 * @return 0 on success, negative on error
 */
int sangha_share_wisdom(const char* wisdom_text);

/**
 * @brief Get shared wisdom from sangha
 * @param wisdom_list Output array of wisdom strings
 * @param max_count Maximum wisdom entries
 * @return Number of wisdom entries
 */
int sangha_get_shared_wisdom(const char** wisdom_list, uint32_t max_count);

/**
 * @brief Propose decision to sangha
 * @param proposal Proposal description
 * @return 0 on success, negative on error
 */
int sangha_propose_decision(const char* proposal);

/**
 * @brief Vote on sangha proposal
 * @param proposal_id Proposal ID
 * @param vote true for yes, false for no
 * @return 0 on success, negative on error
 */
int sangha_vote(uint32_t proposal_id, bool vote);

/**
 * @brief Get collective decision result
 * @param proposal_id Proposal ID
 * @return 1 if passed, 0 if rejected, -1 on error
 */
int sangha_get_decision(uint32_t proposal_id);

/**
 * @brief Calculate collective consciousness (advanced)
 * @return Collective consciousness level
 */
sangha_consciousness_t sangha_calculate_collective_consciousness(void);

/* ========================================================================= */
/* KARMA-BASED TRUST (US-3.3)                                                */
/* ========================================================================= */

/**
 * @brief Calculate trust score for a device
 * @param device_id Device to calculate trust for
 * @return Trust score (0-100), or -1 on error
 */
int sangha_calculate_trust_score(const uint8_t* device_id);

/**
 * @brief Get trust level for a device
 * @param device_id Device to check
 * @return Trust level (UNTRUSTED, GUEST, MEMBER, TRUSTED, ENLIGHTENED)
 */
int sangha_get_trust_level(const uint8_t* device_id);

/**
 * @brief Check if device has privilege
 * @param device_id Device to check
 * @param privilege Privilege to check
 * @return true if has privilege, false otherwise
 */
bool sangha_has_privilege(const uint8_t* device_id, uint32_t privilege);

/**
 * @brief Report trust violation
 * @param device_id Device that violated trust
 * @param violation_description Description of violation
 * @return 0 on success, negative on error
 */
int sangha_report_violation(const uint8_t* device_id, const char* violation_description);

/**
 * @brief Get reputation of device
 * @param device_id Device to check
 * @return Reputation score (0-100), or -1 on error
 */
int sangha_get_reputation(const uint8_t* device_id);

/**
 * @brief Route message via trusted path
 * @param dest_id Destination device
 * @param message Message to send
 * @param length Message length
 * @return 0 on success, negative on error
 */
int sangha_route_trusted(const uint8_t* dest_id, const void* message, size_t length);

/**
 * @brief Appeal for trust restoration
 * @param reason Reason for appeal
 * @return 0 on success, negative on error
 */
int sangha_appeal_trust(const char* reason);

/* ========================================================================= */
/* SANGHA STATISTICS                                                         */
/* ========================================================================= */

/**
 * @brief Get sangha statistics
 * @param community Output community stats
 * @return 0 on success, negative on error
 */
int sangha_get_stats(sangha_community_t* community);

/**
 * @brief Print sangha status
 */
void sangha_print_status(void);

/**
 * @brief Get self device ID
 * @return Pointer to self device ID
 */
const uint8_t* sangha_get_self_id(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_SANGHA_H */
