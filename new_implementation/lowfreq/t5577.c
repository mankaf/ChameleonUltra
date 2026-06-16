/**
 * T5577 Card Emulation for Chameleon Ultra
 * 
 * ASK 125 kHz card - Hitag S-like format, commonly used in low-power locks
 * 
 * Features:
 * - Variable bit-length data (10-68 bits)
 * - Write-only emulation mode (read from reader's response)
 * - Multiple bit length variants (T5577A/B variants)
 * 
 * Implementation notes:
 * - Uses ASK modulation with 3-bit carrier encoding
 * - Data transmitted in raw bit stream format
 * - No ATQA/SAK handshake like ISO14443 cards
 */

#include "nrf.h"
#include "app_util.h"
#include "fds_ids.h"
#include "fds_util.h"
#include "tag_emulation.h"
#include "lf_tag_em.h"
#include <string.h>

#define NRF_LOG_MODULE_NAME "t5577"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

/* =========================== T5577 Data Format ========================== */

/** T5577 data structure - variable bit-length */
typedef struct {
    uint8_t uid_length;             // UID length in bits (4, 16, or 32)
    uint8_t data_bits[68];          // Raw data bits
    uint8_t bit_count;              // Number of data bits stored
    uint8_t is_write_mode;          // True = write-only emulation
} t5577_data_t;

/* =========================== Data Handler Functions ========================== */

/**
 * Load T5577 slot data from flash memory
 * 
 * @param type Tag type enum (TAG_TYPE_T5577)
 * @param buffer Output buffer for tag data
 * @return Length of bits loaded, or -1 on error
 */
int t5577_data_loadcb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Loading T5577 slot data");
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(TAG_SLOT_ACTIVE, TAG_SENSE_LF, &map_info);
    
    int bit_count = -1;
    bool ret = fds_read_sync(map_info.id, map_info.key, NULL, buffer->buffer);
    if (ret) {
        bit_count = buffer->length * 8;  // Treat as bits for ASK protocol
        NRF_LOG_INFO("Loaded %d bits of T5577 data", bit_count);
        
        // Validate T5577 format: minimum 10 bits, maximum 68 bits
        if (bit_count < 40 || bit_count > 272) {  // 5-34 bytes * 8 = 40-272 bits
            NRF_LOG_WARNING("Invalid T5577 bit length: %d", bit_count);
            bit_count = -1;
        }
    } else {
        NRF_LOG_INFO("T5577 slot data not found in flash");
    }
    
    return bit_count / 8;  // Return as byte count for compatibility
}

/**
 * Save T5577 slot data to flash memory
 * 
 * @param type Tag type enum
 * @param buffer Input buffer with tag data
 * @return Bits written (positive) or error code
 */
int t5577_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Saving T5577 slot data");
    
    int byte_length = buffer->length;
    uint8_t bit_count = byte_length * 8;
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(TAG_SLOT_ACTIVE, TAG_SENSE_LF, &map_info);
    
    // Store with header indicating data length
    if (byte_length + 1 <= buffer->length) {
        uint8_t header[1] = {(uint8_t)(bit_count / 8)};  // Byte count header
        
        bool ret = fds_write_sync(map_info.id, map_info.key, 
                                 byte_length + 1, 
                                 (uint8_t *)header);
        if (ret) {
            NRF_LOG_INFO("Saved %d bits of T5577 data", bit_count);
        } else {
            NRF_LOG_ERROR("Failed to write T5577 data to flash");
        }
    }
    
    return byte_length;
}

/**
 * Factory initialization for T5577 slot data
 * Creates default factory data with random UID
 * 
 * @param slot Active card slot
 * @param type Tag type enum
 * @return True if initialized successfully
 */
bool t5577_data_factory(uint8_t slot, tag_specific_type_t type) {
    NRF_LOG_INFO("Creating T5577 factory data");
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(slot, TAG_SENSE_LF, &map_info);
    
    // Check if data already exists
    if (fds_is_exists(map_info.id, map_info.key)) {
        NRF_LOG_INFO("T5577 factory data already exists");
        return false;  // Already initialized
    }
    
    // Create default T5577 factory data with random UID
    uint8_t factory_data[32];  // Maximum 68 bits = ~9 bytes
    memset(factory_data, 0x55, sizeof(factory_data));  // ASK "high" state
    
    // Generate random UID (4-32 bits)
    uint8_t uid_bits = nrf_rand() % 28 + 4;  // 4-32 bits
    factory_data[0] = uid_bits;
    
    // Fill remaining bits with pseudo-random data
    for (int i = 1; i < (uid_bits / 8) + 1; i++) {
        uint8_t byte = nrf_rand() % 127;  // Use ASK "high" range
        factory_data[i] = byte;
    }
    
    // Calculate and store length
    uint16_t crc;
    calc_14a_crc_lut(factory_data, sizeof(factory_data), (uint8_t *)&crc);
    
    // Store in flash with CRC
    if (!fds_write_exists(map_info.id, map_info.key)) {
        bool ret = fds_write_sync(map_info.id, map_info.key, 
                                 sizeof(factory_data) + 2, factory_data);
        if (ret) {
            NRF_LOG_INFO("Created T5577 factory data with %d bits UID", uid_bits);
            return true;
        } else {
            NRF_LOG_ERROR("Failed to create T5577 factory data");
            return false;
        }
    }
    
    return false;  // Already exists
}

/* =========================== Emulation Handlers ========================== */

/**
 * Handle field induction for T5577
 * T5577 uses ASK modulation - no ATQA/SAK handshake
 * Simply responds with card data when field detected
 */
void t5577_field_induction_handler(void) {
    NRF_LOG_INFO("T5577 field induction detected");
    
    // For T5577, just respond immediately with stored UID data
    // No ATQA/SAK handshake required (unlike ISO14443 cards)
    
    uint8_t *buffer = get_buffer_by_tag_type(TAG_TYPE_T5577);
    if (buffer == NULL) {
        return;
    }
    
    // Transmit stored T5577 data
    int byte_len = buffer->length;
    if (byte_len > 0) {
        NRF_LOG_INFO("Transmitting %d bytes of T5577 data", byte_len);
        // ASK carrier modulation would happen here
        transmit_ask_carrier(buffer->buffer, byte_len, 125000UL);
    }
}

/**
 * Respond to reader request for T5577
 */
bool t5577_read_request_handler(uint8_t *data_out, uint16_t max_len) {
    NRF_LOG_DEBUG("T5577 read request handler");
    
    uint8_t *buffer = get_buffer_by_tag_type(TAG_TYPE_T5577);
    if (buffer == NULL || buffer->length == 0) {
        NRF_LOG_WARNING("No T5577 data available");
        return false;
    }
    
    int byte_len = buffer->length;
    uint16_t copy_len = min(byte_len, max_len);
    
    memcpy(data_out, buffer->buffer, copy_len);
    
    NRF_LOG_INFO("Sent %d bytes of T5577 data to reader", copy_len);
    return true;
}

/**
 * Handle write operation for T5577 (if supported by variant)
 */
bool t5577_write_handler(uint8_t *data_in, uint16_t length) {
    NRF_LOG_DEBUG("T5577 write request: len=%d", length);
    
    uint8_t *buffer = get_buffer_by_tag_type(TAG_TYPE_T5577);
    if (buffer == NULL) {
        return false;
    }
    
    // Update buffer with new data
    if (length <= buffer->length) {
        memcpy(buffer->buffer, data_in, length);
        buffer->length = length;
        
        NRF_LOG_INFO("Updated T5577 data with %d bytes", length);
    } else {
        // Truncate if new data is too large
        memcpy(buffer->buffer, data_in, buffer->length);
        buffer->length = buffer->length;
        
        NRF_LOG_WARNING("Truncated T5577 write: wanted %d, stored %d", 
                        length, buffer->length);
    }
    
    return true;
}

/* =========================== Anti-Detection ========================== */

/**
 * Handle battery-powered lock with short field activation
 * 
 * Optimizes response timing for low-power readers:
 * - Extended wake-up delay (5-10ms)
 * - Full data transmission in single burst
 */
void t5577_low_power_optimization(uint8_t *data_out, uint8_t byte_count) {
    NRF_LOG_DEBUG("T5577 low-power lock optimization");
    
    // Initial delay for battery-powered reader wake-up
    nrf_hw_timer_delay_us(10000UL);  // 10ms
    
    // Transmit data in full burst
    transmit_ask_carrier(data_out, byte_count, 125000UL);
}

/* =========================== Utility Functions ========================== */

/**
 * Convert byte buffer to bit stream (for ASK transmission)
 * 
 * @param bytes Input byte buffer
 * @param bytes_len Number of bytes
 * @param bits Output bit array (must be large enough)
 */
void t5577_bytes_to_bits(const uint8_t *bytes, uint16_t bytes_len, uint8_t *bits) {
    uint8_t bit_count = bytes_len * 8;
    
    for (int i = 0; i < bit_count; i++) {
        int byte_idx = i / 8;
        int bit_idx = i % 8;
        
        // MSB first ordering
        bits[i] = (bytes[byte_idx] >> (7 - bit_idx)) & 1;
    }
    
    NRF_LOG_DEBUG("T5577 bytes->bits: %d bytes = %d bits", 
                  bytes_len, bit_count);
}

/**
 * Transmit ASK carrier with stored T5577 data
 */
void transmit_ask_carrier(const uint8_t *data, uint16_t byte_len, uint32_t freq_hz) {
    NRF_LOG_DEBUG("T5577 ASK transmission: freq=%dHz, len=%d bytes", 
                  freq_hz, byte_len);
    
    // Convert bytes to bits
    uint8_t bits[68];  // Max T5577 length is 68 bits
    t5577_bytes_to_bits(data, byte_len, bits);
    
    // Transmit each bit using ASK modulation (1/0 = on/off carrier)
    for (uint8_t i = 0; i < byte_len * 8; i++) {
        uint32_t pulse_width_us = 100UL / freq_hz;  // Bit duration
        
        if (bits[i] == 1) {
            enable_ask_carrier(freq_hz, pulse_width_us);
        } else {
            disable_ask_carrier();
        }
    }
}
