/**
 * Chameleon Ultra - All Missing RFID Features Header
 * 
 * Includes and initializes all features marked "Not yet implemented" in the
 * Technical Whitepaper. This header should be included after tag_emulation.h
 * in app_main.c or rfid_main.c
 * 
 * Implements:
 *   1-2. HF Emulation (DESFire EV1/EV2)
 *   3. Relay Attack Support  
 *   4-19. LF Card Emulation (T5577, EM4305, FDX-B, Indala, Keri, Paradox, etc.)
 */

#ifndef ALL_MISSING_FEATURES_H
#define ALL_MISSING_FEATURES_H

#include "nrf.h"
#include "app_util.h"
#include "tag_emulation.h"

/* =========================== Feature Initialization ========================== */

/**
 * Initialize all missing features subsystems
 * 
 * Should be called after tag_emulation_init() in main startup code
 */
void rfid_missing_features_complete_init(void);

/**
 * Finalize all missing features (cleanup)
 */
void rfid_missing_features_finalize(void);

/* =========================== Feature Support Check ========================== */

/**
 * Check which missing features are supported by hardware/firmware
 * 
 * @return Bitmask of enabled features:
 *   - 1 = HF DESFire EV1
 *   - 2 = HF DESFire EV2  
 *   - 4 = Relay Attack
 */
uint8_t rfid_missing_features_check_support(void);

/* =========================== Handler Selection Helpers ========================== */

/**
 * Select appropriate DESFire handler based on SAK response
 * 
 * @param sak_response SAK byte from card (0x89=EV1, 0x90=EV2)
 * @return Load callback function pointer, or -1 on error
 */
int rfid_desfire_select_handler(uint8_t sak_response);

/**
 * Auto-detect and select ASK card handler based on received data format
 * 
 * @param detected_type Detected tag type enum
 * @param buffer Data buffer
 * @return Bytes loaded from flash, or -1 on error
 */
int rfid_ask_auto_detect_handler(tag_specific_type_t detected_type, tag_data_buffer_t *buffer);

/**
 * Auto-detect and select PSK card handler based on received data format
 * 
 * @param detected_type Detected tag type enum
 * @param buffer Data buffer
 * @return Bytes loaded from flash, or -1 on error
 */
int rfid_psk_auto_detect_handler(tag_specific_type_t detected_type, tag_data_buffer_t *buffer);

/**
 * Auto-detect and select FSK card handler based on received data format
 * 
 * @param detected_type Detected tag type enum
 * @param buffer Data buffer  
 * @return Bytes loaded from flash, or -1 on error
 */
int rfid_fsk_auto_detect_handler(tag_specific_type_t detected_type, tag_data_buffer_t *buffer);

/* =========================== Legacy Compatibility Layer ========================== */

/**
 * Backward-compatible card emulation function for legacy code
 * 
 * Automatically routes to appropriate handler based on tag type
 * 
 * @param type Tag type enum (includes all new types)
 * @param data Output buffer for response data
 * @return True if emulation successful, false otherwise
 */
bool rfid_legacy_emulate_card(tag_specific_type_t type, uint8_t *data);

/* =========================== Type Definitions ========================== */

/** Feature bitmask constants */
#define FEATURE_HF_DESFIRE_EV1      (1 << 0)
#define FEATURE_HF_DESFIRE_EV2      (1 << 1)
#define FEATURE_RELAY_ATTACK        (1 << 2)

/** LF card protocol groups */
typedef enum {
    LF_CARD_GROUP_ASK,              // ASK protocol cards
    LF_CARD_GROUP_PSK,              // PSK protocol cards
    LF_CARD_GROUP_FSK,              // FSK protocol cards
    LF_CARD_GROUP_ALL               // All protocols supported
} lf_card_group_t;

/**
 * Get LF card group based on modulation type
 */
static inline lf_card_group_t get_lf_card_group(tag_specific_type_t type) {
    switch (type) {
        // ASK cards: T5577, EM4305, FDX-B and variants
        case TAG_TYPE_T5577:
        case TAG_TYPE_EM4305:
        case TAG_TYPE_EM4305_64:
        case TAG_TYPE_FD_X_B:
        case TAG_TYPE_FD_X_B_ELECTRA:
            return LF_CARD_GROUP_ASK;
            
        // PSK cards: Indala, Keri and variants
        case TAG_TYPE_INDALA:
        case TAG_TYPE_INDALA_20:
        case TAG_TYPE_KERI:
        case TAG_TYPE_KERI_V2:
            return LF_CARD_GROUP_PSK;
            
        // FSK cards: Paradox, AWD and variants
        case TAG_TYPE_PARADOX:
        case TAG_TYPE_FD_X_B_ELECTRA:  // Also handles AWD variant
            return LF_CARD_GROUP_FSK;
            
        default:
            return LF_CARD_GROUP_ALL;
    }
}

/**
 * Check if tag type is one of the new missing features types
 */
static inline bool is_new_feature_type(tag_specific_type_t type) {
    // HF DESFire types
    if (type == TAG_TYPE_MIFARE_DESFIRE_EV1 || 
        type == TAG_TYPE_MIFARE_DESFIRE_EV2) {
        return true;
    }
    
    // LF ASK types
    if (type == TAG_TYPE_T5577 ||
        type == TAG_TYPE_EM4305 ||
        type == TAG_TYPE_EM4305_64 ||
        type == TAG_TYPE_FD_X_B) {
        return true;
    }
    
    // LF PSK types
    if (type == TAG_TYPE_INDALA ||
        type == TAG_TYPE_KERI) {
        return true;
    }
    
    // LF FSK types  
    if (type == TAG_TYPE_PARADOX) {
        return true;
    }
    
    return false;
}

#endif /* ALL_MISSING_FEATURES_H */
