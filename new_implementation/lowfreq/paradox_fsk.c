/**
 * Paradox Card Emulation for Chameleon Ultra  
 * 
 * FSK 125 kHz card - Read-to-first protocol, security system applications
 * 
 * Features:
 * - FSK modulation (Frequency-Shift Keying)
 * - Two frequency bands: 38.4 kHz ON / 36.4 kHz OFF
 * - 16-bit base UID + variable data
 * 
 * Implementation notes:
 * - Used in security system access control cards
 * - FSK requires dual-frequency RF front-end support
 */

#include "nrf.h"
#include "app_util.h"
#include "fds_ids.h"
#include "fds_util.h"
#include "tag_emulation.h"
#include "lf_tag_em.h"
#include <string.h>

#define NRF_LOG_MODULE_NAME "paradox"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

/* =========================== Paradox Data Format ========================== */

/** Paradox data structure - 16-bit UID + variable data */
typedef struct {
    uint8_t uid_low;                // Low byte of 16-bit UID
    uint8_t uid_high;               // High byte of 16-bit UID  
    uint8_t data_area[24];          // Variable data area (max 24 bytes)
    uint8_t data_length;            // Data length in bytes
} paradox_data_t;

/* =========================== Data Handler Functions ========================== */

/**
 * Load Paradox slot data from flash memory
 */
int paradox_data_loadcb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Loading Paradox slot data");
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(TAG_SLOT_ACTIVE, TAG_SENSE_LF, &map_info);
    
    int byte_count = -1;
    bool ret = fds_read_sync(map_info.id, map_info.key, NULL, buffer->buffer);
    if (ret) {
        byte_count = buffer->length;
        NRF_LOG_INFO("Loaded %d bytes of Paradox data", byte_count);
        
        // Validate Paradox format: minimum 2 bytes UID + some data
        if (byte_count < 4 || byte_count > 30) {
            NRF_LOG_WARNING("Invalid Paradox length: %d", byte_count);
            byte_count = -1;
        }
    } else {
        NRF_LOG_INFO("Paradox slot data not found in flash");
    }
    
    return byte_count;
}

/**
 * Save Paradox slot data to flash memory
 */
int paradox_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Saving Paradox slot data");
    
    int byte_length = buffer->length;
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(TAG_SLOT_ACTIVE, TAG_SENSE_LF, &map_info);
    
    // Store with CRC
    if (byte_length + 2 <= buffer->length) {
        uint16_t crc;
        calc_14a_crc_lut(buffer->buffer, byte_length, (uint8_t *)&crc);
        
        buffer->buffer[byte_length] = (crc >> 8) & 0xFF;
        buffer->buffer[byte_length + 1] = crc & 0xFF;
        
        bool ret = fds_write_sync(map_info.id, map_info.key, 
                                 byte_length + 2, buffer->buffer);
        if (ret) {
            NRF_LOG_INFO("Saved %d bytes of Paradox data", byte_length);
        } else {
            NRF_LOG_ERROR("Failed to write Paradox data to flash");
        }
    }
    
    return byte_length;
}

/**
 * Factory initialization for Paradox slot data
 */
bool paradox_data_factory(uint8_t slot, tag_specific_type_t type) {
    NRF_LOG_INFO("Creating Paradox factory data");
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(slot, TAG_SENSE_LF, &map_info);
    
    if (fds_is_exists(map_info.id, map_info.key)) {
        return false;
    }
    
    // Create default Paradox factory data with random UID and data area
    uint8_t factory_data[30];
    memset(factory_data, 0xAA, sizeof(factory_data));
    
    // Generate random 16-bit UID (FSK read-first protocol)
    factory_data[0] = nrf_rand() & 0xFF;
    factory_data[1] = nrf_rand() & 0xFF;
    
    // Fill data area with pseudo-random values
    uint8_t data_len = nrf_rand() % 23 + 5;  // 5-27 bytes
    for (uint8_t i = 0; i < data_len && i < sizeof(factory_data) - 2; i++) {
        factory_data[i + 2] = nrf_rand() & 0xFF;
    }
    
    factory_data[byte_length - 1] = data_len;
    
    uint16_t crc;
    calc_14a_crc_lut(factory_data, sizeof(factory_data), (uint8_t *)&crc);
    
    if (!fds_write_exists(map_info.id, map_info.key)) {
        bool ret = fds_write_sync(map_info.id, map_info.key, 
                                 sizeof(factory_data) + 2, factory_data);
        if (ret) {
            NRF_LOG_INFO("Created Paradox factory data");
            return true;
        } else {
            return false;
        }
    }
    
    return false;
}

/* =========================== Emulation Handlers ========================== */

/**
 * Handle FSK field induction for Paradox cards
 */
void paradox_field_induction_handler(void) {
    NRF_LOG_INFO("Paradox FSK field induction detected");
    
    uint8_t *buffer = get_buffer_by_tag_type(TAG_TYPE_PAC);  // Reuse type enum
    if (buffer == NULL || buffer->length < 4) {
        return;
    }
    
    // Wait for reader request then respond
    nrf_hw_timer_delay_us(50UL);  // Small delay for FSK handshake
    
    // Transmit stored Paradox data using FSK modulation
    transmit_fsk_carrier(buffer->buffer, buffer->length, 
                        38400UL, 36400UL);  // ON/OFF frequencies
}

/**
 * Respond to reader request for Paradox UID
 */
bool paradox_read_uid_handler(uint8_t *data_out, uint16_t max_len) {
    NRF_LOG_DEBUG("Paradox read UID handler");
    
    uint8_t *buffer = get_buffer_by_tag_type(TAG_TYPE_PAC);
    if (buffer == NULL || buffer->length < 4) {
        return false;
    }
    
    // Return stored Paradox data to reader
    uint16_t copy_len = min(buffer->length, max_len);
    memcpy(data_out, buffer->buffer, copy_len);
    
    NRF_LOG_INFO("Sent %d bytes of Paradox UID", copy_len);
    return true;
}

/**
 * Handle write operation for Paradox
 */
bool paradox_write_handler(uint8_t *data_in, uint16_t length) {
    NRF_LOG_DEBUG("Paradox write request: len=%d", length);
    
    uint8_t *buffer = get_buffer_by_tag_type(TAG_TYPE_PAC);
    if (buffer == NULL || buffer->length < 4) {
        return false;
    }
    
    // Update data area with new values
    if (length <= buffer->length - 2) {
        memcpy(&buffer->buffer[2], data_in, length);
        buffer->length = length + 2;
        
        NRF_LOG_INFO("Updated Paradox data with %d bytes", length);
    }
    
    return true;
}

/* =========================== Utility Functions ========================== */

/**
 * Calculate Paradox checksum for frame verification
 */
uint8_t paradox_calc_checksum(const uint8_t *frame, uint16_t length) {
    NRF_LOG_DEBUG("Paradox checksum calculation: len=%d", length);
    
    uint8_t checksum = 0;
    for (int i = 0; i < length; i++) {
        checksum ^= frame[i];
    }
    
    return checksum;
}

/**
 * Verify Paradox FSK frame with checksum
 */
bool paradox_verify_checksum(const uint8_t *frame, uint16_t length) {
    NRF_LOG_DEBUG("Paradox checksum verification: len=%d", length);
    
    if (length < 1) {
        return false;
    }
    
    // Last byte is expected checksum
    uint8_t expected_checksum = paradox_calc_checksum(frame, length - 1);
    return (expected_checksum == frame[length - 1]);
}

/**
 * Transmit Paradox data using FSK modulation
 */
void transmit_fsk_carrier(const uint8_t *data, uint16_t byte_len, 
                          uint32_t freq_on_hz, uint32_t freq_off_hz) {
    NRF_LOG_DEBUG("Paradox FSK transmission: %dHz/%dHz", freq_on_hz, freq_off_hz);
    
    if (byte_len == 0) return;
    
    // Convert bytes to bits
    uint8_t bits[256];
    for (int i = 0; i < byte_len * 8 && i < 256; i++) {
        int byte_idx = i / 8;
        int bit_idx = i % 8;
        bits[i] = (data[byte_idx] >> (7 - bit_idx)) & 1;
    }
    
    // Transmit each bit using FSK modulation
    uint32_t bit_duration_us = 1000UL / freq_on_hz;
    for (uint8_t i = 0; i < byte_len * 8 && i < 256; i++) {
        if (bits[i] == 1) {
            enable_fsk_carrier(freq_on_hz, freq_off_hz, bit_duration_us);
        } else {
            disable_fsk_carrier();
        }
    }
}
