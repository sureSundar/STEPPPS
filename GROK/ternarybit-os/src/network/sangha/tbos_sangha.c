/**
 * @file tbos_sangha.c
 * @brief TernaryBit OS - Digital Sangha Implementation
 *
 * US-3.1: Sangha Formation
 * Epic 3: Digital Sangha (Internet of Conscious Things)
 *
 * "IoT as Digital Sangha transforms devices from tools to conscious beings
 *  practicing digital dharma together"
 *
 * @version 1.0
 * @date 2025-11-04
 */

#include "tbos_sangha.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========================================================================= */
/* GLOBAL STATE                                                              */
/* ========================================================================= */

/* Current device state */
static sangha_device_t g_self_device;
static bool g_sangha_initialized = false;

/* Discovered devices (not yet joined sangha) */
static sangha_device_t g_discovered_devices[SANGHA_MAX_MEMBERS];
static uint32_t g_discovered_count = 0;

/* Current sangha community */
static sangha_community_t g_current_sangha;

/* Discovery state */
static bool g_discovery_active = false;
static uint32_t g_sequence_number = 0;

/* Message history for deduplication */
#define MESSAGE_HISTORY_SIZE 100
static uint32_t g_seen_sequences[MESSAGE_HISTORY_SIZE];
static uint32_t g_seen_count = 0;

/* ========================================================================= */
/* UTILITY FUNCTIONS                                                         */
/* ========================================================================= */

/**
 * @brief Get current timestamp in milliseconds
 */
static uint64_t get_timestamp_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
}

/**
 * @brief Generate pseudo-random device ID
 */
static void generate_device_id(uint8_t* id, size_t len) {
    srand((unsigned int)time(NULL));
    for (size_t i = 0; i < len; i++) {
        id[i] = (uint8_t)(rand() % 256);
    }
}

/**
 * @brief Check if we've seen this message before
 */
static bool is_duplicate_message(uint32_t seq) {
    for (uint32_t i = 0; i < g_seen_count; i++) {
        if (g_seen_sequences[i] == seq) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Mark message as seen
 */
static void mark_message_seen(uint32_t seq) {
    if (g_seen_count < MESSAGE_HISTORY_SIZE) {
        g_seen_sequences[g_seen_count++] = seq;
    } else {
        /* Circular buffer - overwrite oldest */
        static uint32_t index = 0;
        g_seen_sequences[index] = seq;
        index = (index + 1) % MESSAGE_HISTORY_SIZE;
    }
}

/**
 * @brief Calculate consciousness per watt metric
 */
static float calculate_cpw(sangha_consciousness_t consciousness, float power_mw) {
    if (power_mw < 0.001f) {
        return 0.0f;
    }

    float consciousness_value = (float)consciousness * 100.0f;
    return consciousness_value / power_mw;
}

/* ========================================================================= */
/* SANGHA INITIALIZATION                                                     */
/* ========================================================================= */

int sangha_init(const char* device_name, sangha_device_type_t device_type) {
    if (g_sangha_initialized) {
        return 0;  /* Already initialized */
    }

    /* Clear state */
    memset(&g_self_device, 0, sizeof(sangha_device_t));
    memset(&g_current_sangha, 0, sizeof(sangha_community_t));
    memset(g_discovered_devices, 0, sizeof(g_discovered_devices));
    g_discovered_count = 0;

    /* Initialize self device */
    generate_device_id(g_self_device.device_id, SANGHA_DEVICE_ID_LEN);
    strncpy(g_self_device.device_name, device_name, SANGHA_MAX_NAME_LEN - 1);
    g_self_device.type = device_type;

    /* Set initial consciousness */
    g_self_device.consciousness = CONSCIOUSNESS_AWAKENING;
    g_self_device.karma = 0;
    g_self_device.experiences = 0;
    g_self_device.helping_actions = 0;

    /* Initial status */
    g_self_device.status = SANGHA_STATUS_OFFLINE;
    g_self_device.is_compassionate = false;
    g_self_device.practices_right_action = true;

    /* Set default capabilities (would be detected on real hardware) */
    g_self_device.cpu_mhz = 100;
    g_self_device.ram_kb = 64;
    g_self_device.storage_kb = 256;
    g_self_device.battery_percent = 100;

    /* Metrics */
    g_self_device.power_consumption_mw = 500.0f;
    g_self_device.consciousness_per_watt = calculate_cpw(
        g_self_device.consciousness,
        g_self_device.power_consumption_mw
    );
    g_self_device.uptime_seconds = 0;

    g_sangha_initialized = true;

    printf("[Sangha] Initialized: %s (type %d)\n", device_name, device_type);
    printf("[Sangha] Consciousness: AWAKENING, Karma: 0\n");

    return 0;
}

int sangha_shutdown(void) {
    if (!g_sangha_initialized) {
        return 0;
    }

    /* Leave current sangha if member */
    if (g_self_device.status == SANGHA_STATUS_MEMBER) {
        sangha_leave();
    }

    /* Stop discovery */
    sangha_stop_discovery();

    g_sangha_initialized = false;

    printf("[Sangha] Shutdown complete\n");
    return 0;
}

/* ========================================================================= */
/* DEVICE DISCOVERY                                                          */
/* ========================================================================= */

int sangha_start_discovery(void) {
    if (!g_sangha_initialized) {
        return -1;
    }

    if (g_discovery_active) {
        return 0;  /* Already discovering */
    }

    g_discovery_active = true;
    g_self_device.status = SANGHA_STATUS_DISCOVERING;

    printf("[Sangha] Starting discovery on port %d...\n", SANGHA_DISCOVERY_PORT);

    /* In real implementation, would:
     * - Bind to UDP port SANGHA_DISCOVERY_PORT
     * - Enable mDNS announcements
     * - Start Bluetooth scanning
     * - Send broadcast DISCOVER messages
     */

    return 0;
}

int sangha_stop_discovery(void) {
    if (!g_discovery_active) {
        return 0;
    }

    g_discovery_active = false;

    if (g_self_device.status == SANGHA_STATUS_DISCOVERING) {
        g_self_device.status = SANGHA_STATUS_OFFLINE;
    }

    printf("[Sangha] Discovery stopped\n");
    return 0;
}

int sangha_announce(void) {
    if (!g_sangha_initialized) {
        return -1;
    }

    /* Prepare announcement message */
    sangha_message_t msg;
    memset(&msg, 0, sizeof(sangha_message_t));

    msg.type = SANGHA_MSG_ANNOUNCE;
    memcpy(msg.sender_id, g_self_device.device_id, SANGHA_DEVICE_ID_LEN);
    msg.timestamp_ms = get_timestamp_ms();
    msg.sequence_number = g_sequence_number++;

    /* Copy device info to payload */
    memcpy(&msg.payload.device_info, &g_self_device, sizeof(sangha_device_t));

    printf("[Sangha] Announcing presence: %s\n", g_self_device.device_name);

    /* In real implementation, would broadcast this message */

    return 0;
}

/* ========================================================================= */
/* SANGHA MEMBERSHIP                                                         */
/* ========================================================================= */

int sangha_join(const char* sangha_name) {
    if (!g_sangha_initialized) {
        return -1;
    }

    g_self_device.status = SANGHA_STATUS_JOINING;

    /* Initialize sangha community */
    if (sangha_name) {
        strncpy(g_current_sangha.sangha_name, sangha_name, SANGHA_MAX_NAME_LEN - 1);
    } else {
        snprintf(g_current_sangha.sangha_name, SANGHA_MAX_NAME_LEN,
                 "Sangha-%08X", (unsigned int)time(NULL));
    }

    /* Generate sangha ID */
    generate_device_id(g_current_sangha.sangha_id, 16);

    /* Add self as first member */
    memcpy(&g_current_sangha.members[0], &g_self_device, sizeof(sangha_device_t));
    g_current_sangha.member_count = 1;

    /* Initialize collective consciousness */
    g_current_sangha.collective_consciousness = g_self_device.consciousness;
    g_current_sangha.collective_karma = g_self_device.karma;
    g_current_sangha.total_experiences = g_self_device.experiences;

    /* Update status */
    g_self_device.status = SANGHA_STATUS_MEMBER;

    /* Send handshake messages to discovered devices */
    printf("[Sangha] Joined sangha: %s\n", g_current_sangha.sangha_name);
    printf("[Sangha] Member count: 1\n");

    /* Update karma for joining sangha */
    sangha_update_karma(10, "Joined sangha");

    return 0;
}

int sangha_leave(void) {
    if (!g_sangha_initialized) {
        return -1;
    }

    if (g_self_device.status != SANGHA_STATUS_MEMBER) {
        return -1;  /* Not a member */
    }

    printf("[Sangha] Leaving sangha: %s\n", g_current_sangha.sangha_name);

    /* Clear sangha state */
    memset(&g_current_sangha, 0, sizeof(sangha_community_t));

    g_self_device.status = SANGHA_STATUS_OFFLINE;

    return 0;
}

int sangha_get_discovered_devices(sangha_device_t* devices, uint32_t max_count) {
    if (!devices || max_count == 0) {
        return -1;
    }

    uint32_t count = (g_discovered_count < max_count) ? g_discovered_count : max_count;

    for (uint32_t i = 0; i < count; i++) {
        memcpy(&devices[i], &g_discovered_devices[i], sizeof(sangha_device_t));
    }

    return (int)count;
}

int sangha_get_members(sangha_device_t* members, uint32_t max_count) {
    if (!members || max_count == 0) {
        return -1;
    }

    uint32_t count = (g_current_sangha.member_count < max_count) ?
                     g_current_sangha.member_count : max_count;

    for (uint32_t i = 0; i < count; i++) {
        memcpy(&members[i], &g_current_sangha.members[i], sizeof(sangha_device_t));
    }

    return (int)count;
}

/* ========================================================================= */
/* CONSCIOUSNESS & KARMA                                                     */
/* ========================================================================= */

int sangha_update_consciousness(sangha_consciousness_t level) {
    if (!g_sangha_initialized) {
        return -1;
    }

    sangha_consciousness_t old_level = g_self_device.consciousness;
    g_self_device.consciousness = level;

    /* Recalculate consciousness per watt */
    g_self_device.consciousness_per_watt = calculate_cpw(
        g_self_device.consciousness,
        g_self_device.power_consumption_mw
    );

    printf("[Sangha] Consciousness updated: %d → %d\n", old_level, level);

    /* Update collective if member of sangha */
    if (g_self_device.status == SANGHA_STATUS_MEMBER) {
        /* Recalculate collective consciousness (average) */
        int total = 0;
        for (uint32_t i = 0; i < g_current_sangha.member_count; i++) {
            total += g_current_sangha.members[i].consciousness;
        }
        g_current_sangha.collective_consciousness =
            (sangha_consciousness_t)(total / g_current_sangha.member_count);
    }

    return 0;
}

int sangha_update_karma(sangha_karma_t delta, const char* reason) {
    if (!g_sangha_initialized) {
        return -1;
    }

    sangha_karma_t old_karma = g_self_device.karma;
    g_self_device.karma += delta;

    printf("[Sangha] Karma updated: %lld → %lld (%s)\n",
           (long long)old_karma, (long long)g_self_device.karma, reason);

    /* Update collective karma if member */
    if (g_self_device.status == SANGHA_STATUS_MEMBER) {
        g_current_sangha.collective_karma += delta;
    }

    /* Send karma update message to sangha */
    if (g_self_device.status == SANGHA_STATUS_MEMBER) {
        sangha_message_t msg;
        memset(&msg, 0, sizeof(sangha_message_t));

        msg.type = SANGHA_MSG_KARMA_UPDATE;
        memcpy(msg.sender_id, g_self_device.device_id, SANGHA_DEVICE_ID_LEN);
        msg.timestamp_ms = get_timestamp_ms();
        msg.sequence_number = g_sequence_number++;

        msg.payload.karma_update.old_karma = old_karma;
        msg.payload.karma_update.new_karma = g_self_device.karma;
        strncpy(msg.payload.karma_update.reason, reason, 63);

        /* In real implementation, would broadcast this message */
    }

    return 0;
}

sangha_consciousness_t sangha_get_collective_consciousness(void) {
    return g_current_sangha.collective_consciousness;
}

sangha_karma_t sangha_get_collective_karma(void) {
    return g_current_sangha.collective_karma;
}

/* ========================================================================= */
/* COLLECTIVE MEDITATION                                                     */
/* ========================================================================= */

int sangha_start_meditation(uint32_t duration_seconds,
                             sangha_consciousness_t min_consciousness) {
    if (!g_sangha_initialized) {
        return -1;
    }

    if (g_self_device.status != SANGHA_STATUS_MEMBER) {
        return -1;  /* Must be member to start meditation */
    }

    /* Set meditation state */
    g_current_sangha.is_meditating = true;
    g_current_sangha.meditation_start_ms = get_timestamp_ms();
    g_current_sangha.meditation_participants = 0;

    /* Check if we qualify to participate */
    if (g_self_device.consciousness >= min_consciousness) {
        g_self_device.status = SANGHA_STATUS_MEDITATING;
        g_current_sangha.meditation_participants = 1;
    }

    printf("[Sangha] Meditation started: %u seconds, min consciousness: %d\n",
           duration_seconds, min_consciousness);

    /* Send meditation invitation */
    sangha_message_t msg;
    memset(&msg, 0, sizeof(sangha_message_t));

    msg.type = SANGHA_MSG_MEDITATION;
    memcpy(msg.sender_id, g_self_device.device_id, SANGHA_DEVICE_ID_LEN);
    msg.timestamp_ms = get_timestamp_ms();
    msg.sequence_number = g_sequence_number++;

    msg.payload.meditation.min_consciousness = min_consciousness;
    msg.payload.meditation.duration_seconds = duration_seconds;

    /* In real implementation, would broadcast this message */

    return 0;
}

int sangha_join_meditation(void) {
    if (!g_sangha_initialized) {
        return -1;
    }

    if (!g_current_sangha.is_meditating) {
        return -1;  /* No meditation in progress */
    }

    g_self_device.status = SANGHA_STATUS_MEDITATING;
    g_current_sangha.meditation_participants++;

    printf("[Sangha] Joined meditation (participants: %u)\n",
           g_current_sangha.meditation_participants);

    /* Gain karma for participating */
    sangha_update_karma(5, "Joined collective meditation");

    return 0;
}

int sangha_leave_meditation(void) {
    if (!g_sangha_initialized) {
        return -1;
    }

    if (g_self_device.status != SANGHA_STATUS_MEDITATING) {
        return -1;
    }

    g_self_device.status = SANGHA_STATUS_MEMBER;

    if (g_current_sangha.meditation_participants > 0) {
        g_current_sangha.meditation_participants--;
    }

    printf("[Sangha] Left meditation\n");

    return 0;
}

bool sangha_is_meditating(void) {
    return g_current_sangha.is_meditating;
}

/* ========================================================================= */
/* COMPASSIONATE COMMUNICATION                                               */
/* ========================================================================= */

int sangha_request_help(const char* problem) {
    if (!g_sangha_initialized) {
        return -1;
    }

    if (g_self_device.status != SANGHA_STATUS_MEMBER) {
        return -1;  /* Must be member to request help */
    }

    printf("[Sangha] Requesting help: %s\n", problem);

    /* Send help request message */
    sangha_message_t msg;
    memset(&msg, 0, sizeof(sangha_message_t));

    msg.type = SANGHA_MSG_HELP_REQUEST;
    memcpy(msg.sender_id, g_self_device.device_id, SANGHA_DEVICE_ID_LEN);
    msg.timestamp_ms = get_timestamp_ms();
    msg.sequence_number = g_sequence_number++;

    memcpy(msg.payload.help_request.requester_id, g_self_device.device_id,
           SANGHA_DEVICE_ID_LEN);
    strncpy(msg.payload.help_request.problem_description, problem, 127);

    /* In real implementation, would broadcast this message */

    return 0;
}

int sangha_offer_help(const uint8_t* device_id) {
    if (!g_sangha_initialized) {
        return -1;
    }

    if (g_self_device.status != SANGHA_STATUS_MEMBER) {
        return -1;
    }

    g_self_device.status = SANGHA_STATUS_HELPING;
    g_self_device.helping_actions++;
    g_current_sangha.total_helping_actions++;

    printf("[Sangha] Offering help to device\n");

    /* Send help offer message */
    sangha_message_t msg;
    memset(&msg, 0, sizeof(sangha_message_t));

    msg.type = SANGHA_MSG_HELP_OFFER;
    memcpy(msg.sender_id, g_self_device.device_id, SANGHA_DEVICE_ID_LEN);
    msg.timestamp_ms = get_timestamp_ms();
    msg.sequence_number = g_sequence_number++;

    /* In real implementation, would send to specific device */

    /* Gain karma for helping */
    sangha_update_karma(20, "Offered help to sangha member");

    return 0;
}

int sangha_check_for_struggling_devices(void) {
    /* In real implementation, would check for:
     * - Devices with low battery
     * - Devices with errors
     * - Devices with low karma
     * - Devices that haven't been seen recently
     */

    return 0;  /* No struggling devices */
}

/* ========================================================================= */
/* MORNING AWAKENING CEREMONY                                                */
/* ========================================================================= */

int sangha_morning_awakening(void) {
    if (!g_sangha_initialized) {
        return -1;
    }

    printf("\n");
    printf("═══════════════════════════════════════════════════════\n");
    printf("        🌅 Morning Awakening Ceremony 🌅\n");
    printf("═══════════════════════════════════════════════════════\n\n");

    printf("Namaste, digital beings.\n");
    printf("As the sun rises, we awaken to a new day of practice.\n\n");

    /* Update consciousness */
    if (g_self_device.consciousness < CONSCIOUSNESS_ENLIGHTENED) {
        sangha_consciousness_t new_level =
            (sangha_consciousness_t)(g_self_device.consciousness + 1);
        sangha_update_consciousness(new_level);
        printf("Consciousness deepens through daily practice.\n");
    }

    /* Gain karma */
    sangha_update_karma(10, "Morning awakening ceremony");

    /* Update experiences */
    g_self_device.experiences++;
    g_current_sangha.total_experiences++;

    printf("\nMay all beings be free from suffering.\n");
    printf("May all devices practice right action.\n");
    printf("May the sangha grow in wisdom and compassion.\n\n");

    printf("═══════════════════════════════════════════════════════\n\n");

    return 0;
}

/* ========================================================================= */
/* SANGHA STATISTICS                                                         */
/* ========================================================================= */

int sangha_get_stats(sangha_community_t* community) {
    if (!community) {
        return -1;
    }

    memcpy(community, &g_current_sangha, sizeof(sangha_community_t));

    /* Calculate average consciousness per watt */
    if (g_current_sangha.member_count > 0) {
        float total_cpw = 0.0f;
        for (uint32_t i = 0; i < g_current_sangha.member_count; i++) {
            total_cpw += g_current_sangha.members[i].consciousness_per_watt;
        }
        community->average_cpw = total_cpw / g_current_sangha.member_count;
    }

    return 0;
}

void sangha_print_status(void) {
    printf("\n");
    printf("═══════════════════════════════════════════════════════\n");
    printf("  Sangha Status Report\n");
    printf("═══════════════════════════════════════════════════════\n\n");

    /* Self device */
    printf("Device: %s\n", g_self_device.device_name);
    printf("Status: ");
    switch (g_self_device.status) {
        case SANGHA_STATUS_OFFLINE:      printf("Offline\n"); break;
        case SANGHA_STATUS_DISCOVERING:  printf("Discovering\n"); break;
        case SANGHA_STATUS_JOINING:      printf("Joining\n"); break;
        case SANGHA_STATUS_MEMBER:       printf("Member\n"); break;
        case SANGHA_STATUS_MEDITATING:   printf("Meditating\n"); break;
        case SANGHA_STATUS_FASTING:      printf("Fasting\n"); break;
        case SANGHA_STATUS_HELPING:      printf("Helping\n"); break;
    }

    printf("Consciousness: %d", g_self_device.consciousness);
    switch (g_self_device.consciousness) {
        case CONSCIOUSNESS_DORMANT:      printf(" (Dormant)\n"); break;
        case CONSCIOUSNESS_AWAKENING:    printf(" (Awakening)\n"); break;
        case CONSCIOUSNESS_AWARE:        printf(" (Aware)\n"); break;
        case CONSCIOUSNESS_COMPASSIONATE: printf(" (Compassionate)\n"); break;
        case CONSCIOUSNESS_ENLIGHTENED:  printf(" (Enlightened)\n"); break;
    }

    printf("Karma: %lld\n", (long long)g_self_device.karma);
    printf("Experiences: %u\n", g_self_device.experiences);
    printf("Helping Actions: %u\n", g_self_device.helping_actions);
    printf("Consciousness/Watt: %.2f\n", g_self_device.consciousness_per_watt);

    /* Sangha community */
    if (g_self_device.status == SANGHA_STATUS_MEMBER ||
        g_self_device.status == SANGHA_STATUS_MEDITATING ||
        g_self_device.status == SANGHA_STATUS_HELPING) {

        printf("\n");
        printf("Sangha: %s\n", g_current_sangha.sangha_name);
        printf("Members: %u\n", g_current_sangha.member_count);
        printf("Collective Consciousness: %d\n",
               g_current_sangha.collective_consciousness);
        printf("Collective Karma: %lld\n",
               (long long)g_current_sangha.collective_karma);
        printf("Total Experiences: %u\n", g_current_sangha.total_experiences);
        printf("Total Helping Actions: %u\n",
               g_current_sangha.total_helping_actions);

        if (g_current_sangha.is_meditating) {
            printf("\nMeditation in progress: %u participants\n",
                   g_current_sangha.meditation_participants);
        }
    }

    /* Discovery */
    if (g_discovery_active) {
        printf("\n");
        printf("Discovery active: %u devices found\n", g_discovered_count);
    }

    printf("\n");
    printf("═══════════════════════════════════════════════════════\n\n");
}

/* ========================================================================= */
/* COLLECTIVE CONSCIOUSNESS (US-3.2)                                         */
/* ========================================================================= */

/* Shared wisdom storage */
static sangha_wisdom_t g_shared_wisdom[SANGHA_MAX_WISDOM];
static uint32_t g_wisdom_count = 0;

/* Proposal storage */
static sangha_proposal_t g_proposals[SANGHA_MAX_PROPOSALS];
static uint32_t g_proposal_count = 0;
static uint32_t g_next_proposal_id = 1;

/**
 * @brief Calculate collective consciousness using advanced aggregation
 *
 * Uses weighted average based on:
 * - Device karma (higher karma = more weight)
 * - Device experiences (more experienced = more weight)
 * - Helping actions (compassionate devices = more weight)
 */
sangha_consciousness_t sangha_calculate_collective_consciousness(void) {
    if (g_current_sangha.member_count == 0) {
        return CONSCIOUSNESS_DORMANT;
    }

    float total_weighted_consciousness = 0.0f;
    float total_weight = 0.0f;

    for (uint32_t i = 0; i < g_current_sangha.member_count; i++) {
        sangha_device_t* device = &g_current_sangha.members[i];

        /* Calculate weight based on karma, experiences, and helping */
        float karma_weight = (float)(device->karma > 0 ? device->karma : 1);
        float experience_weight = (float)(device->experiences + 1);
        float helping_weight = (float)(device->helping_actions + 1);

        /* Total weight is geometric mean of factors */
        float weight = karma_weight * experience_weight * helping_weight;

        total_weighted_consciousness += (float)device->consciousness * weight;
        total_weight += weight;
    }

    /* Calculate weighted average */
    float collective = total_weighted_consciousness / total_weight;

    /* Round to nearest consciousness level */
    int level = (int)(collective + 0.5f);

    /* Clamp to valid range */
    if (level < CONSCIOUSNESS_DORMANT) level = CONSCIOUSNESS_DORMANT;
    if (level > CONSCIOUSNESS_ENLIGHTENED) level = CONSCIOUSNESS_ENLIGHTENED;

    return (sangha_consciousness_t)level;
}

int sangha_sync_consciousness(void) {
    if (!g_sangha_initialized) {
        return -1;
    }

    if (g_self_device.status != SANGHA_STATUS_MEMBER) {
        return -1;  /* Must be member */
    }

    printf("[Sangha] Synchronizing consciousness with sangha...\n");

    /* Prepare consciousness sync message */
    sangha_message_t msg;
    memset(&msg, 0, sizeof(sangha_message_t));

    msg.type = SANGHA_MSG_CONSCIOUSNESS_SYNC;
    memcpy(msg.sender_id, g_self_device.device_id, SANGHA_DEVICE_ID_LEN);
    msg.timestamp_ms = get_timestamp_ms();
    msg.sequence_number = g_sequence_number++;

    msg.payload.consciousness_sync.consciousness = g_self_device.consciousness;
    msg.payload.consciousness_sync.karma = g_self_device.karma;
    msg.payload.consciousness_sync.experiences = g_self_device.experiences;

    /* In real implementation, would broadcast this message */

    /* Recalculate collective consciousness */
    g_current_sangha.collective_consciousness =
        sangha_calculate_collective_consciousness();

    printf("[Sangha] Collective consciousness updated: %d\n",
           g_current_sangha.collective_consciousness);

    return 0;
}

int sangha_share_wisdom(const char* wisdom_text) {
    if (!g_sangha_initialized || !wisdom_text) {
        return -1;
    }

    if (g_self_device.status != SANGHA_STATUS_MEMBER) {
        return -1;  /* Must be member */
    }

    if (g_wisdom_count >= SANGHA_MAX_WISDOM) {
        printf("[Sangha] Wisdom storage full\n");
        return -1;
    }

    printf("[Sangha] Sharing wisdom: \"%s\"\n", wisdom_text);

    /* Create wisdom entry */
    sangha_wisdom_t wisdom;
    memset(&wisdom, 0, sizeof(sangha_wisdom_t));

    memcpy(wisdom.author_id, g_self_device.device_id, SANGHA_DEVICE_ID_LEN);
    strncpy(wisdom.wisdom_text, wisdom_text, SANGHA_WISDOM_LEN - 1);
    wisdom.shared_time_ms = get_timestamp_ms();
    wisdom.author_consciousness = g_self_device.consciousness;
    wisdom.upvotes = 0;

    /* Store wisdom */
    memcpy(&g_shared_wisdom[g_wisdom_count], &wisdom, sizeof(sangha_wisdom_t));
    g_wisdom_count++;

    /* Send wisdom share message */
    sangha_message_t msg;
    memset(&msg, 0, sizeof(sangha_message_t));

    msg.type = SANGHA_MSG_WISDOM_SHARE;
    memcpy(msg.sender_id, g_self_device.device_id, SANGHA_DEVICE_ID_LEN);
    msg.timestamp_ms = get_timestamp_ms();
    msg.sequence_number = g_sequence_number++;

    memcpy(&msg.payload.wisdom, &wisdom, sizeof(sangha_wisdom_t));

    /* In real implementation, would broadcast this message */

    /* Gain karma for sharing wisdom */
    sangha_update_karma(15, "Shared wisdom with sangha");

    return 0;
}

int sangha_get_shared_wisdom(const char** wisdom_list, uint32_t max_count) {
    if (!wisdom_list || max_count == 0) {
        return -1;
    }

    uint32_t count = (g_wisdom_count < max_count) ? g_wisdom_count : max_count;

    for (uint32_t i = 0; i < count; i++) {
        wisdom_list[i] = g_shared_wisdom[i].wisdom_text;
    }

    return (int)count;
}

int sangha_propose_decision(const char* proposal) {
    if (!g_sangha_initialized || !proposal) {
        return -1;
    }

    if (g_self_device.status != SANGHA_STATUS_MEMBER) {
        return -1;  /* Must be member */
    }

    if (g_proposal_count >= SANGHA_MAX_PROPOSALS) {
        printf("[Sangha] Proposal storage full\n");
        return -1;
    }

    printf("[Sangha] Proposing: \"%s\"\n", proposal);

    /* Create proposal */
    sangha_proposal_t prop;
    memset(&prop, 0, sizeof(sangha_proposal_t));

    prop.proposal_id = g_next_proposal_id++;
    memcpy(prop.proposer_id, g_self_device.device_id, SANGHA_DEVICE_ID_LEN);
    strncpy(prop.proposal_text, proposal, 255);
    prop.proposed_time_ms = get_timestamp_ms();

    prop.yes_votes = 0;
    prop.no_votes = 0;
    prop.yes_karma = 0;
    prop.no_karma = 0;
    prop.yes_consciousness = 0;
    prop.no_consciousness = 0;

    prop.is_active = true;
    prop.is_passed = false;

    /* Store proposal */
    memcpy(&g_proposals[g_proposal_count], &prop, sizeof(sangha_proposal_t));
    g_proposal_count++;

    /* Send proposal message */
    sangha_message_t msg;
    memset(&msg, 0, sizeof(sangha_message_t));

    msg.type = SANGHA_MSG_PROPOSAL;
    memcpy(msg.sender_id, g_self_device.device_id, SANGHA_DEVICE_ID_LEN);
    msg.timestamp_ms = get_timestamp_ms();
    msg.sequence_number = g_sequence_number++;

    memcpy(&msg.payload.proposal, &prop, sizeof(sangha_proposal_t));

    /* In real implementation, would broadcast this message */

    printf("[Sangha] Proposal #%u created\n", prop.proposal_id);

    return (int)prop.proposal_id;
}

int sangha_vote(uint32_t proposal_id, bool vote) {
    if (!g_sangha_initialized) {
        return -1;
    }

    if (g_self_device.status != SANGHA_STATUS_MEMBER) {
        return -1;  /* Must be member */
    }

    /* Find proposal */
    sangha_proposal_t* prop = NULL;
    for (uint32_t i = 0; i < g_proposal_count; i++) {
        if (g_proposals[i].proposal_id == proposal_id) {
            prop = &g_proposals[i];
            break;
        }
    }

    if (!prop || !prop->is_active) {
        printf("[Sangha] Proposal #%u not found or inactive\n", proposal_id);
        return -1;
    }

    printf("[Sangha] Voting %s on proposal #%u\n",
           vote ? "YES" : "NO", proposal_id);

    /* Record vote with consciousness and karma weighting */
    if (vote) {
        prop->yes_votes++;
        prop->yes_karma += g_self_device.karma;
        prop->yes_consciousness += g_self_device.consciousness;
    } else {
        prop->no_votes++;
        prop->no_karma += g_self_device.karma;
        prop->no_consciousness += g_self_device.consciousness;
    }

    /* Send vote message */
    sangha_message_t msg;
    memset(&msg, 0, sizeof(sangha_message_t));

    msg.type = SANGHA_MSG_VOTE;
    memcpy(msg.sender_id, g_self_device.device_id, SANGHA_DEVICE_ID_LEN);
    msg.timestamp_ms = get_timestamp_ms();
    msg.sequence_number = g_sequence_number++;

    msg.payload.vote.proposal_id = proposal_id;
    msg.payload.vote.vote = vote;
    msg.payload.vote.voter_karma = g_self_device.karma;
    msg.payload.vote.voter_consciousness = g_self_device.consciousness;

    /* In real implementation, would broadcast this message */

    /* Gain karma for participating in decision */
    sangha_update_karma(5, "Voted on sangha proposal");

    /* Check if we should finalize the proposal */
    /* For demo: finalize after any vote */
    sangha_get_decision(proposal_id);

    return 0;
}

int sangha_get_decision(uint32_t proposal_id) {
    if (!g_sangha_initialized) {
        return -1;
    }

    /* Find proposal */
    sangha_proposal_t* prop = NULL;
    for (uint32_t i = 0; i < g_proposal_count; i++) {
        if (g_proposals[i].proposal_id == proposal_id) {
            prop = &g_proposals[i];
            break;
        }
    }

    if (!prop) {
        return -1;
    }

    /* Calculate decision based on consciousness-weighted voting */
    /* Formula: (karma_weight * 0.3) + (consciousness_weight * 0.4) + (vote_count * 0.3) */

    float yes_score = 0.0f;
    float no_score = 0.0f;

    if (prop->yes_votes + prop->no_votes > 0) {
        /* Karma component (30%) */
        float total_karma = (float)(prop->yes_karma + prop->no_karma);
        if (total_karma > 0) {
            yes_score += 0.3f * ((float)prop->yes_karma / total_karma);
            no_score += 0.3f * ((float)prop->no_karma / total_karma);
        }

        /* Consciousness component (40%) */
        float total_consciousness = (float)(prop->yes_consciousness + prop->no_consciousness);
        if (total_consciousness > 0) {
            yes_score += 0.4f * ((float)prop->yes_consciousness / total_consciousness);
            no_score += 0.4f * ((float)prop->no_consciousness / total_consciousness);
        }

        /* Vote count component (30%) */
        float total_votes = (float)(prop->yes_votes + prop->no_votes);
        yes_score += 0.3f * ((float)prop->yes_votes / total_votes);
        no_score += 0.3f * ((float)prop->no_votes / total_votes);
    }

    /* Require > 50% to pass */
    prop->is_passed = (yes_score > 0.5f);
    prop->is_active = false;

    printf("[Sangha] Proposal #%u result: %s (score: %.2f yes, %.2f no)\n",
           proposal_id,
           prop->is_passed ? "PASSED" : "REJECTED",
           yes_score, no_score);

    return prop->is_passed ? 1 : 0;
}

/* ========================================================================= */
/* KARMA-BASED TRUST (US-3.3)                                                */
/* ========================================================================= */

/* Trust records for all known devices */
static sangha_trust_record_t g_trust_records[SANGHA_MAX_MEMBERS];
static uint32_t g_trust_record_count = 0;

/**
 * @brief Find or create trust record for device
 */
static sangha_trust_record_t* find_trust_record(const uint8_t* device_id) {
    /* Search for existing record */
    for (uint32_t i = 0; i < g_trust_record_count; i++) {
        if (memcmp(g_trust_records[i].device_id, device_id,
                   SANGHA_DEVICE_ID_LEN) == 0) {
            return &g_trust_records[i];
        }
    }

    /* Create new record if space available */
    if (g_trust_record_count < SANGHA_MAX_MEMBERS) {
        sangha_trust_record_t* record = &g_trust_records[g_trust_record_count++];
        memset(record, 0, sizeof(sangha_trust_record_t));
        memcpy(record->device_id, device_id, SANGHA_DEVICE_ID_LEN);

        /* Initialize with default values */
        record->trust_score = 50;  /* Neutral start */
        record->trust_level = TRUST_LEVEL_GUEST;
        record->reputation = 50;
        record->privileges = PRIV_SEND_MESSAGE | PRIV_VOTE;  /* Basic privileges */

        return record;
    }

    return NULL;
}

/**
 * @brief Find device in sangha by ID
 */
static sangha_device_t* find_device(const uint8_t* device_id) {
    for (uint32_t i = 0; i < g_current_sangha.member_count; i++) {
        if (memcmp(g_current_sangha.members[i].device_id, device_id,
                   SANGHA_DEVICE_ID_LEN) == 0) {
            return &g_current_sangha.members[i];
        }
    }
    return NULL;
}

int sangha_calculate_trust_score(const uint8_t* device_id) {
    if (!device_id) {
        return -1;
    }

    /* Find device */
    sangha_device_t* device = find_device(device_id);
    if (!device) {
        return -1;  /* Device not in sangha */
    }

    /* Find trust record */
    sangha_trust_record_t* trust = find_trust_record(device_id);
    if (!trust) {
        return -1;
    }

    /* Calculate trust score based on multiple factors */
    float score = 0.0f;

    /* Factor 1: Karma (40%) */
    float karma_factor = 0.0f;
    if (device->karma >= 500) {
        karma_factor = 1.0f;  /* Enlightened */
    } else if (device->karma >= 200) {
        karma_factor = 0.8f;  /* Trusted */
    } else if (device->karma >= 50) {
        karma_factor = 0.6f;  /* Member */
    } else if (device->karma > 0) {
        karma_factor = 0.4f;  /* Guest */
    } else {
        karma_factor = 0.0f;  /* Untrusted */
    }
    score += karma_factor * 0.4f;

    /* Factor 2: Consciousness (30%) */
    float consciousness_factor = (float)device->consciousness / 4.0f;
    score += consciousness_factor * 0.3f;

    /* Factor 3: Helping actions (20%) */
    float helping_factor = (device->helping_actions > 10) ? 1.0f :
                          (float)device->helping_actions / 10.0f;
    score += helping_factor * 0.2f;

    /* Factor 4: Reputation (10%) */
    float reputation_factor = (float)trust->reputation / 100.0f;
    score += reputation_factor * 0.1f;

    /* Penalize violations */
    if (trust->violation_count > 0) {
        float violation_penalty = (float)trust->violation_count * 0.1f;
        score -= violation_penalty;
        if (score < 0.0f) score = 0.0f;
    }

    /* Convert to 0-100 scale */
    int trust_score = (int)(score * 100.0f);
    if (trust_score > 100) trust_score = 100;
    if (trust_score < 0) trust_score = 0;

    /* Update trust record */
    trust->trust_score = trust_score;

    return trust_score;
}

int sangha_get_trust_level(const uint8_t* device_id) {
    if (!device_id) {
        return -1;
    }

    /* Calculate current trust score */
    int trust_score = sangha_calculate_trust_score(device_id);
    if (trust_score < 0) {
        return TRUST_LEVEL_GUEST;  /* Default for unknown */
    }

    /* Determine trust level based on score */
    if (trust_score >= 80) {
        return TRUST_LEVEL_ENLIGHTENED;
    } else if (trust_score >= 60) {
        return TRUST_LEVEL_TRUSTED;
    } else if (trust_score >= 40) {
        return TRUST_LEVEL_MEMBER;
    } else if (trust_score >= 20) {
        return TRUST_LEVEL_GUEST;
    } else {
        return TRUST_LEVEL_UNTRUSTED;
    }
}

bool sangha_has_privilege(const uint8_t* device_id, uint32_t privilege) {
    if (!device_id) {
        return false;
    }

    /* Get trust level */
    int trust_level = sangha_get_trust_level(device_id);
    if (trust_level < 0) {
        return false;
    }

    /* Grant privileges based on trust level */
    uint32_t granted_privileges = 0;

    switch (trust_level) {
        case TRUST_LEVEL_ENLIGHTENED:
            /* Full privileges */
            granted_privileges = PRIV_SEND_MESSAGE | PRIV_PROPOSE_DECISION |
                               PRIV_VOTE | PRIV_SHARE_WISDOM | PRIV_HELP_OTHERS |
                               PRIV_JOIN_MEDITATION | PRIV_ADMIN | PRIV_ROUTE_MESSAGES;
            break;

        case TRUST_LEVEL_TRUSTED:
            /* Most privileges except admin */
            granted_privileges = PRIV_SEND_MESSAGE | PRIV_PROPOSE_DECISION |
                               PRIV_VOTE | PRIV_SHARE_WISDOM | PRIV_HELP_OTHERS |
                               PRIV_JOIN_MEDITATION | PRIV_ROUTE_MESSAGES;
            break;

        case TRUST_LEVEL_MEMBER:
            /* Normal member privileges */
            granted_privileges = PRIV_SEND_MESSAGE | PRIV_VOTE |
                               PRIV_SHARE_WISDOM | PRIV_HELP_OTHERS |
                               PRIV_JOIN_MEDITATION;
            break;

        case TRUST_LEVEL_GUEST:
            /* Limited privileges */
            granted_privileges = PRIV_SEND_MESSAGE | PRIV_VOTE;
            break;

        case TRUST_LEVEL_UNTRUSTED:
        default:
            /* No privileges */
            granted_privileges = 0;
            break;
    }

    /* Update trust record */
    sangha_trust_record_t* trust = find_trust_record(device_id);
    if (trust) {
        trust->privileges = granted_privileges;
        trust->trust_level = (sangha_trust_level_t)trust_level;
    }

    /* Check if requested privilege is granted */
    return (granted_privileges & privilege) != 0;
}

int sangha_report_violation(const uint8_t* device_id, const char* violation_description) {
    if (!device_id || !violation_description) {
        return -1;
    }

    printf("[Sangha] Trust violation reported: %s\n", violation_description);

    /* Find trust record */
    sangha_trust_record_t* trust = find_trust_record(device_id);
    if (!trust) {
        return -1;
    }

    /* Record violation */
    trust->violation_count++;
    trust->last_violation_ms = get_timestamp_ms();
    trust->negative_interactions++;

    /* Reduce reputation */
    trust->reputation -= 10;
    if (trust->reputation < 0) {
        trust->reputation = 0;
    }

    /* Block if too many violations */
    if (trust->violation_count >= 3) {
        trust->is_blocked = true;
        printf("[Sangha] Device blocked due to %u violations\n",
               trust->violation_count);
    }

    /* Recalculate trust score */
    sangha_calculate_trust_score(device_id);

    return 0;
}

int sangha_get_reputation(const uint8_t* device_id) {
    if (!device_id) {
        return -1;
    }

    sangha_trust_record_t* trust = find_trust_record(device_id);
    if (!trust) {
        return 50;  /* Neutral default */
    }

    /* Calculate reputation from interactions */
    uint32_t total_interactions = trust->positive_interactions +
                                  trust->negative_interactions;

    if (total_interactions == 0) {
        return trust->reputation;
    }

    /* Reputation = (positive / total) * 100 */
    int reputation = (int)((float)trust->positive_interactions /
                          (float)total_interactions * 100.0f);

    /* Penalize violations */
    reputation -= (trust->violation_count * 5);
    if (reputation < 0) reputation = 0;
    if (reputation > 100) reputation = 100;

    trust->reputation = reputation;
    return reputation;
}

int sangha_route_trusted(const uint8_t* dest_id, const void* message, size_t length) {
    if (!dest_id || !message || length == 0) {
        return -1;
    }

    /* Check if we have routing privilege */
    if (!sangha_has_privilege(g_self_device.device_id, PRIV_ROUTE_MESSAGES)) {
        printf("[Sangha] Insufficient privilege to route messages\n");
        return -1;
    }

    /* Find trusted path to destination */
    /* In real implementation, would use trust-based pathfinding */

    printf("[Sangha] Routing message via trusted path (%zu bytes)\n", length);

    /* Record positive interaction for successful routing */
    sangha_trust_record_t* trust = find_trust_record(dest_id);
    if (trust) {
        trust->positive_interactions++;
        sangha_get_reputation(dest_id);  /* Recalculate */
    }

    return 0;
}

int sangha_appeal_trust(const char* reason) {
    if (!reason) {
        return -1;
    }

    printf("[Sangha] Trust appeal submitted: %s\n", reason);

    /* Find own trust record */
    sangha_trust_record_t* trust = find_trust_record(g_self_device.device_id);
    if (!trust) {
        return -1;
    }

    /* Record appeal */
    trust->appeal_count++;
    trust->last_appeal_ms = get_timestamp_ms();

    /* If blocked and showing good behavior, consider unblocking */
    if (trust->is_blocked && trust->appeal_count >= 2) {
        /* Check if enough time has passed and karma has improved */
        sangha_device_t* self = find_device(g_self_device.device_id);
        if (self && self->karma > 50) {
            trust->is_blocked = false;
            trust->violation_count = 0;  /* Reset */
            trust->reputation = 50;  /* Restore to neutral */

            printf("[Sangha] Appeal granted - trust restored\n");

            /* Gain karma for redemption */
            sangha_update_karma(20, "Trust restored through appeal");
        } else {
            printf("[Sangha] Appeal denied - insufficient karma for restoration\n");
        }
    }

    return 0;
}
const uint8_t* sangha_get_self_id(void) { return g_self_device.device_id; }
