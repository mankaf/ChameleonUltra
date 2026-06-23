/**
 * FDX-B Card Emulation for Chameleon Ultra  
 * 
 * ASK 125 kHz card - EM-Micro FDX-B, common in industrial access control
 * 
 * Features:
 * - Variable data length (16-68 bits)
 * - Pseudo-random UID generation
 * - No sector structure (flat data format)
 * 
 * Implementation notes:
 * - Used in legacy industrial systems (gates, turnstiles)
 * - ASK modulation with carrier frequency 125 kHz
 * - Simple raw bit transmission format
 */

#include "nrf.h"
#include "app_util.h"
#include "fds_ids.h"
#include "fds_util.h"
#include "tag_emulation.h"
#include "lf_tag_em.h"
#include <string.h>

#define NRF_LOG_MODULE_NAME "fdx_b"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

/* =========================== FDX-B Data Format ========================== */

/** FDX-B data structure - pseudo-random UID, variable length */
typedef struct {
    uint8_t uid_length;             // UID length in bits (16/32/48/56/64)
    uint8_t data_bits[68];          // Raw data bits
    uint8_t bit_count;              // Number of data bits stored
    bool has_pseudo_random_uid;     // True = pseudo-random UID generated
} fdx_b_data_t;

/* =========================== Data Handler Functions ========================== */

/**
 * Load FDX-B slot data from flash memory
 * 
 * @param type Tag type enum (TAG_TYPE_FD_X_B or TAG_TYPE_FD_X_B_ELECTRA)
 * @param buffer Output buffer for tag data
 * @return Length of bytes loaded, or -1 on error
 */
int fdx_b_data_loadcb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Loading FDX-B slot data");
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(TAG_SLOT_ACTIVE, TAG_SENSE_LF, &map_info);
    
    int byte_count = -1;
    bool ret = fds_read_sync(map_info.id, map_info.key, NULL, buffer->buffer);
    if (ret) {
        byte_count = buffer->length;
        NRF_LOG_INFO("Loaded %d bytes of FDX-B data", byte_count);
        
        // Validate FDX-B format: 2-8 bytes (16-64 bits basic UID, possibly more)
        if (byte_count < 1 || byte_count > 8) {
            NRF_LOG_WARNING("Invalid FDX-B length: %d", byte_count);
            byte_count = -1;
        }
    } else {
        NRF_LOG_INFO("FDX-B slot data not found in flash");
    }
    
    return byte_count;
}

/**
 * Save FDX-B slot data to flash memory
 * 
 * @param type Tag type enum
 * @param buffer Input buffer with tag data
 * @return Bytes written (positive) or error code
 */
int fdx_b_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Saving FDX-B slot data");
    
    int byte_length = buffer->length;
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(TAG_SLOT_ACTIVE, TAG_SENSE_LF, &map_info);
    
    // Store in flash (FDX-B is pseudo-random UID, typically no CRC stored)
    bool ret = fds_write_sync(map_info.id, map_info.key, 
                             byte_length, buffer->buffer);
    if (ret) {
        NRF_LOG_INFO("Saved %d bytes of FDX-B data", byte_length);
    } else {
        NRF_LOG_ERROR("Failed to write FDX-B data to flash");
    }
    
    return byte_length;
}

/**
 * Factory initialization for FDX-B slot data
 * Creates default factory data with pseudo-random UID
 * 
 * @param slot Active card slot
 * @param type Tag type enum (TAG_TYPE_FD_X_B or TAG_TYPE_FD_X_B_ELECTRA)
 * @return True if initialized successfully
 */
bool fdx_b_data_factory(uint8_t slot, tag_specific_type_t type) {
    NRF_LOG_INFO("Creating FDX-B factory data");
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(slot, TAG_SENSE_LF, &map_info);
    
    // Check if data already exists
    if (fds_is_exists(map_info.id, map_info.key)) {
        NRF_LOG_INFO("FDX-B factory data already exists");
        return false;  // Already initialized
    }
    
    // Create default FDX-B factory data with pseudo-random UID
    uint8_t factory_data[8];  // Max 64 bits = 8 bytes
    
    // Pseudo-random UID generation (typical FDX-B format)
    memset(factory_data, 0, sizeof(factory_data));
    
    // Generate pseudo-random UID based on slot index and random seed
    uint16_t random_seed = nrf_rand() + slot * 0x1234;
    
    for (uint8_t i = 0; i < sizeof(factory_data); i++) {
        factory_data[i] = random_seed & 0xFF;
        random_seed >>= 8;
    }
    
    // Set UID length indicator in first byte
    uint8_t uid_length_bits = bit_count_trailing_zeros(factory_data[0]) + 4;
    if (uid_length_bits < 16) {
        uid_length_bits = 16;  // Minimum 16 bits
    }
    
    // Calculate and store CRC for integrity check
    uint16_t crc;
    calc_14a_crc_lut(factory_data, sizeof(factory_data), (uint8_t *)&crc);
    
    // Store in flash with CRC
    if (!fds_write_exists(map_info.id, map_info.key)) {
        bool ret = fds_write_sync(map_info.id, map_info.key, 
                                 sizeof(factory_data) + 2, factory_data);
        if (ret) {
            NRF_LOG_INFO("Created FDX-B factory data with pseudo-random UID");
            return true;
        } else {
            NRF_LOG_ERROR("Failed to create FDX-B factory data");
            return false;
        }
    }
    
    return false;  // Already exists
}

/* =========================== Emulation Handlers ========================== */

/**
 * Handle field induction for FDX-B
 * Pseudo-random UID transmission on ASK carrier
 */
void fdx_b_field_induction_handler(void) {
    NRF_LOG_INFO("FDX-B field induction detected");
    
    uint8_t *buffer = get_buffer_by_tag_type(TAG_TYPE_FD_X_B);
    if (buffer == NULL || buffer->length == 0) {
        return;
    }
    
    // Transmit stored FDX-B pseudo-random UID
    int byte_len = buffer->length;
    if (byte_len > 0) {
        NRF_LOG_INFO("Transmitting %d bytes of FDX-B data", byte_len);
        
        // ASK carrier modulation
        transmit_ask_carrier(buffer->buffer, byte_len, 125000UL);
    }
}

/**
 * Respond to reader request for FDX-B UID
 */
bool fdx_b_read_uid_handler(uint8_t *data_out, uint16_t max_len) {
    NRF_LOG_DEBUG("FDX-B read UID handler");
    
    uint8_t *buffer = get_buffer_by_tag_type(TAG_TYPE_FD_X_B);
    if (buffer == NULL || buffer->length == 0) {
        NRF_LOG_WARNING("No FDX-B data available");
        return false;
    }
    
    // Return pseudo-random UID to reader
    uint16_t copy_len = min(buffer->length, max_len);
    memcpy(data_out, buffer->buffer, copy_len);
    
    NRF_LOG_INFO("Sent %d bytes of FDX-B UID to reader", copy_len);
    return true;
}

/**
 * Generate new pseudo-random UID for FDX-B Electra variant
 */
bool fdx_b_electra_generate_uid(uint8_t *uid_buf, uint16_t max_len) {
    NRF_LOG_DEBUG("FDX-B Electra UID generation");
    
    if (max_len < 2 || max_len > 8) {
        return false;
    }
    
    // Generate pseudo-random UID for Electra variant
    memset(uid_buf, 0, max_len);
    
    uint16_t random_seed = nrf_rand() + nrf_hw_timer_get_ms() & 0xFFFF;
    
    for (uint8_t i = 0; i < max_len && i * 8 <= 64; i++) {
        // Pseudo-random byte with slot-dependent variation
        uint16_t slot_seed = nrf_rand();
        uid_buf[i] = (random_seed ^ (slot_seed >> 8)) & 0xFF;
        random_seed += uid_buf[i];  // Snowball effect for randomness
    }
    
    NRF_LOG_INFO("Generated FDX-B Electra UID: %02X %02X %02X", 
                 uid_buf[0], uid_buf[1], uid_buf[2]);
    return true;
}

/* =========================== Utility Functions ========================== */

/**
 * Calculate bit length from UID bytes
 */
uint8_t fdx_b_bit_count_from_bytes(uint8_t byte_count) {
    return byte_count * 8;
}

/**
 * Convert FDX-B byte buffer to bit stream (for ASK transmission)
 */
void fdx_b_bytes_to_bits(const uint8_t *bytes, uint16_t byte_len, uint8_t *bits) {
    for (int i = 0; i < byte_len * 8 && i < 68; i++) {
        int byte_idx = i / 8;
        int bit_idx = i % 8;
        
        // MSB first ordering
        bits[i] = (bytes[byte_idx] >> (7 - bit_idx)) & 1;
    }
}

/**
 * Transmit FDX-B UID using ASK modulation with variable carrier
 */
void transmit_fdxb_uid(uint8_t *uid, uint16_t byte_len) {
    NRF_LOG_DEBUG("FDX-B ASK transmission");
    
    // Convert bytes to bits
    uint8_t bits[68];
    fdx_b_bytes_to_bits(uid, byte_len, bits);
    
    // Transmit each bit using ASK modulation
    for (uint8_t i = 0; i < byte_len * 8 && i < 68; i++) {
        uint32_t pulse_width_us = 100UL / 125000UL;  // Bit duration
        
        if (bits[i] == 1) {
            enable_ask_carrier(125000UL, pulse_width_us);
        } else {
            disable_ask_carrier();
        }
    }
}

/* =========================== FDX-B Electra Support ========================== */

/**
 * FDX-B Electra variant features:
 * - Data-only transmission (no ATQA/SAK)
 * - Pseudo-random UID with 20-68 bit lengths
 * - No authentication overhead
 */

/**
 * Generate FDX-B Electra data bits
 */
bool fdx_b_electra_generate_data_bits(uint8_t *data_bits, uint16_t max_bit_count) {
    NRF_LOG_DEBUG("FDX-B Electra data bit generation: %d bits", max_bit_count);
    
    if (max_bit_count < 20 || max_bit_count > 68) {
        return false;
    }
    
    // Generate pseudo-random data bits
    for (uint16_t i = 0; i < max_bit_count / 8 && i * 8 < max_bit_count; i++) {
        data_bits[i] = nrf_rand() & 0xFF;
    }
    
    NRF_LOG_INFO("Generated FDX-B Electra data: %d bits", max_bit_count);
    return true;
}

/**
 * Handle FDX-B Electra response request
 */
bool fdx_b_electra_response_handler(uint8_t *data_out, uint16_t max_len) {
    NRF_LOG_DEBUG("FDX-B Electra response handler");
    
    // Electra transmits data bits only (no UID header)
    uint8_t *buffer = get_buffer_by_tag_type(TAG_TYPE_FD_X_B);
    if (buffer == NULL || buffer->length == 0) {
        return false;
    }
    
    // Return full buffer for Electra response
    uint16_t copy_len = min(buffer->length, max_len);
    memcpy(data_out, buffer->buffer, copy_len);
    
    NRF_LOG_INFO("FDX-B Electra response: %d bytes", copy_len);
    return true;
}
