/**
 * Keri Card Emulation for Chameleon Ultra  
 * 
 * PSK 125 kHz card - Variant of Indala protocol, used in security systems
 * 
 * Features:
 * - PSK modulation with different timing than Indala
 * - Variable UID lengths (56+8 or 48+8 bit formats)
 * - Two-phase protocol (ATQA followed by data)
 * 
 * Implementation notes:
 * - Compatible with Keri security card readers
 * - Uses similar PSK base clock but different timing parameters
 */

#include "nrf.h"
#include "app_util.h"
#include "missing_features_compat.h"
#include "fds_ids.h"
#include "fds_util.h"
#include "crc_utils.h"
#include "tag_emulation.h"
#include "tag_persistence.h"
#include "lf_tag_em.h"
#include <string.h>

#define NRF_LOG_MODULE_NAME keri
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

/* =========================== Keri Data Format ========================== */

/** Keri data structure - variable length with optional extended format */
typedef struct {
    uint8_t uid_length_1;           // First phase UID length (48 or 56 bits)
    uint8_t uid_length_2;           // Optional second phase length  
    uint8_t data_bits[64];          // Raw data bits
    uint8_t bit_count;              // Number of data bits stored
} keri_data_t;

/* =========================== Data Handler Functions ========================== */

/**
 * Load Keri slot data from flash memory
 */
int keri_data_loadcb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Loading Keri slot data");
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(tag_emulation_get_slot(), TAG_SENSE_LF, &map_info);
    
    int byte_count = -1;
    bool ret = fds_read_sync(map_info.id, map_info.key, NULL, buffer->buffer);
    if (ret) {
        byte_count = buffer->length;
        NRF_LOG_INFO("Loaded %d bytes of Keri data", byte_count);
        
        // Validate Keri format: 6-10 bytes
        if (byte_count < 6 || byte_count > 10) {
            NRF_LOG_WARNING("Invalid Keri length: %d", byte_count);
            byte_count = -1;
        }
    } else {
        NRF_LOG_INFO("Keri slot data not found in flash");
    }
    
    return byte_count;
}

/**
 * Save Keri slot data to flash memory
 */
int keri_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Saving Keri slot data");
    
    int byte_length = buffer->length;
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(tag_emulation_get_slot(), TAG_SENSE_LF, &map_info);
    
    // Store with CRC
    if (byte_length + 2 <= buffer->length) {
        uint16_t crc;
        calc_14a_crc_lut(buffer->buffer, byte_length, (uint8_t *)&crc);
        
        buffer->buffer[byte_length] = (crc >> 8) & 0xFF;
        buffer->buffer[byte_length + 1] = crc & 0xFF;
        
        bool ret = fds_write_sync(map_info.id, map_info.key, 
                                 byte_length + 2, buffer->buffer);
        if (ret) {
            NRF_LOG_INFO("Saved %d bytes of Keri data", byte_length);
        } else {
            NRF_LOG_ERROR("Failed to write Keri data to flash");
        }
    }
    
    return byte_length;
}

/**
 * Factory initialization for Keri slot data
 */
bool keri_data_factory(uint8_t slot, tag_specific_type_t type) {
    NRF_LOG_INFO("Creating Keri factory data");
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(slot, TAG_SENSE_LF, &map_info);
    
    if (fds_is_exists(map_info.id, map_info.key)) {
        return false;
    }
    
    // Create default Keri factory data with 56-bit UID format
    uint8_t factory_data[10];
    memset(factory_data, 0xAA, sizeof(factory_data));
    
    // Generate random 56-bit UID
    for (uint8_t i = 0; i < 7; i++) {
        factory_data[i] = nrf_rand() & 0xFF;
    }
    
    // Set UID length indicator (56 bits = 7 bytes)
    factory_data[7] = 0x38;  // 56 bits marker
    
    // Second phase data (optional extended format)
    for (uint8_t i = 8; i < 10; i++) {
        factory_data[i] = nrf_rand() & 0xFF;
    }
    
    uint16_t crc;
    calc_14a_crc_lut(factory_data, sizeof(factory_data), (uint8_t *)&crc);
    
    if (!fds_is_exists(map_info.id, map_info.key)) {
        bool ret = fds_write_sync(map_info.id, map_info.key, 
                                 sizeof(factory_data) + 2, factory_data);
        if (ret) {
            NRF_LOG_INFO("Created Keri factory data");
            return true;
        } else {
            return false;
        }
    }
    
    return false;
}

/* =========================== Emulation Handlers ========================== */

/**
 * Handle PSK field induction for Keri cards
 */
void keri_field_induction_handler(void) {
    NRF_LOG_INFO("Keri PSK field induction detected");
    
    tag_data_buffer_t *buffer = get_buffer_by_tag_type(TAG_TYPE_KERI);
    if (buffer == NULL || buffer->length < 6) {
        return;
    }
    
    // Keri uses two-phase protocol - wait for ATQA first
    nrf_hw_timer_delay_us(100UL);  // Small delay for PSK handshake
    
    // Transmit stored Keri data using PSK modulation
    transmit_psk_carrier(buffer->buffer, buffer->length, 20000UL);  // 1MHz base clock
}

/**
 * Respond to first-phase UID request (48 or 56 bits)
 */
bool keri_read_uid_phase1_handler(uint8_t *data_out, uint16_t max_len) {
    NRF_LOG_DEBUG("Keri read UID phase 1 handler");
    
    tag_data_buffer_t *buffer = get_buffer_by_tag_type(TAG_TYPE_KERI);
    if (buffer == NULL || buffer->length < 6) {
        return false;
    }
    
    // Return first phase UID (48 or 56 bits)
    uint16_t copy_len = min(7, max_len);  // 56 bits = 7 bytes max
    memcpy(data_out, buffer->buffer, copy_len);
    
    NRF_LOG_INFO("Sent %d bytes of Keri phase-1 UID", copy_len);
    return true;
}

/**
 * Respond to second-phase extended data request
 */
bool keri_read_uid_phase2_handler(uint8_t *data_out, uint16_t max_len) {
    NRF_LOG_DEBUG("Keri read UID phase 2 handler");
    
    tag_data_buffer_t *buffer = get_buffer_by_tag_type(TAG_TYPE_KERI);
    if (buffer == NULL || buffer->length < 6) {
        return false;
    }
    
    // Return second-phase extended data if available
    uint16_t copy_len = min(2, max_len);
    memcpy(data_out, &buffer->buffer[7], copy_len);
    
    NRF_LOG_INFO("Sent %d bytes of Keri phase-2 UID", copy_len);
    return true;
}

/* =========================== Utility Functions ========================== */

/**
 * Keri UID length constants
 */
#define KERI_UID_LENGTH_48   6     // 48 bits = 6 bytes
#define KERI_UID_LENGTH_56   7     // 56 bits = 7 bytes
#define KERI_PHASE2_OFFSET    7    // Second phase offset in buffer

/**
 * Calculate Keri checksum for PSK frame verification
 */
uint8_t keri_calc_checksum(const uint8_t *frame, uint16_t length) {
    NRF_LOG_DEBUG("Keri checksum calculation: len=%d", length);
    
    uint8_t checksum = 0;
    for (int i = 0; i < length; i++) {
        checksum ^= frame[i];
    }
    
    return checksum;
}

/**
 * Verify Keri PSK frame with checksum
 */
bool keri_verify_checksum(const uint8_t *frame, uint16_t length) {
    NRF_LOG_DEBUG("Keri checksum verification: len=%d", length);
    
    if (length < 1) {
        return false;
    }
    
    // Last byte is expected checksum
    uint8_t expected_checksum = keri_calc_checksum(frame, length - 1);
    return (expected_checksum == frame[length - 1]);
}

/**
 * Handle Keri write request (if supported)
 */
bool keri_write_handler(uint8_t *data_in, uint16_t length) {
    NRF_LOG_DEBUG("Keri write request: len=%d", length);
    
    tag_data_buffer_t *buffer = get_buffer_by_tag_type(TAG_TYPE_KERI);
    if (buffer == NULL || buffer->length < 6) {
        return false;
    }
    
    // Update data area with new values
    if (length <= buffer->length - 7) {
        memcpy(&buffer->buffer[7], data_in, length);
        buffer->length = length + 7;
        
        NRF_LOG_INFO("Updated Keri data with %d bytes", length);
    }
    
    return true;
}

/**
 * Transmit Keri UID using PSK modulation
 */
void transmit_keri_uid(uint8_t *uid, uint16_t byte_len) {
    NRF_LOG_DEBUG("Keri PSK transmission: len=%d bytes", byte_len);
    
    if (byte_len == 0) return;
    
    // Convert bytes to bits
    uint8_t bits[64];
    for (int i = 0; i < byte_len * 8 && i < 64; i++) {
        int byte_idx = i / 8;
        int bit_idx = i % 8;
        bits[i] = (uid[byte_idx] >> (7 - bit_idx)) & 1;
    }
    
    // Transmit each bit using PSK modulation
    uint32_t bit_duration_us = 1000UL / 20000UL;  // 50??s per bit for 1MHz base clock
    for (uint8_t i = 0; i < byte_len * 8 && i < 64; i++) {
        if (bits[i] == 1) {
            enable_psk_carrier(20000UL, bit_duration_us);
        } else {
            disable_psk_carrier();
        }
    }
}
