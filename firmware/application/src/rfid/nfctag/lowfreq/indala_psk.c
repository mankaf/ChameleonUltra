/**
 * Indala Card Emulation for Chameleon Ultra  
 * 
 * PSK 125 kHz card - Reader-to-first protocol, common in industrial access control
 * 
 * Features:
 * - PSK modulation (Pulse-Subcarrier Keying)
 * - Three-sector structure (192 bits total)
 * - Reader-initiated communication
 * 
 * Implementation notes:
 * - Uses PWM base clock of 1MHz for subcarrier alignment
 * - Requires PSK timing adjustments in lf_tag_em.c pwm_init()
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

#define NRF_LOG_MODULE_NAME indala
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

/* =========================== Indala Data Format ========================== */

/** Indala data structure - 3-sector structure */
typedef struct {
    uint8_t sector0[32];            // Sector 0 (sector code + serial number)
    uint8_t sector1[32];            // Sector 1 (data area 1)
    uint8_t sector2[32];            // Sector 2 (data area 2)
    uint8_t parity_bit;             // Parity bit for sector 0
} indala_data_t;

/* =========================== Data Handler Functions ========================== */

/**
 * Load Indala slot data from flash memory
 * 
 * @param type Tag type enum (TAG_TYPE_INDALA or TAG_TYPE_INDALA_20)
 * @param buffer Output buffer for tag data
 * @return Length of bytes loaded, or -1 on error
 */
int indala_data_loadcb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Loading Indala slot data");
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(tag_emulation_get_slot(), TAG_SENSE_LF, &map_info);
    
    int byte_count = -1;
    bool ret = fds_read_sync(map_info.id, map_info.key, NULL, buffer->buffer);
    if (ret) {
        byte_count = buffer->length;
        NRF_LOG_INFO("Loaded %d bytes of Indala data", byte_count);
        
        // Validate Indala format: 96-104 bytes (3 sectors + parity)
        if (byte_count < 96 || byte_count > 104) {
            NRF_LOG_WARNING("Invalid Indala length: %d", byte_count);
            byte_count = -1;
        }
    } else {
        NRF_LOG_INFO("Indala slot data not found in flash");
    }
    
    return byte_count;
}

/**
 * Save Indala slot data to flash memory
 * 
 * @param type Tag type enum
 * @param buffer Input buffer with tag data
 * @return Bytes written (positive) or error code
 */
int indala_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Saving Indala slot data");
    
    int byte_length = buffer->length;
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(tag_emulation_get_slot(), TAG_SENSE_LF, &map_info);
    
    // Calculate and store CRC for integrity check
    uint16_t crc;
    calc_14a_crc_lut(buffer->buffer, byte_length, (uint8_t *)&crc);
    
    // Store in flash with CRC
    if (byte_length + 2 <= buffer->length) {
        buffer->buffer[byte_length] = (crc >> 8) & 0xFF;
        buffer->buffer[byte_length + 1] = crc & 0xFF;
        
        bool ret = fds_write_sync(map_info.id, map_info.key, 
                                 byte_length + 2, buffer->buffer);
        if (ret) {
            NRF_LOG_INFO("Saved %d bytes of Indala data", byte_length);
        } else {
            NRF_LOG_ERROR("Failed to write Indala data to flash");
        }
    }
    
    return byte_length;
}

/**
 * Factory initialization for Indala slot data
 * Creates default factory data with random sector content
 * 
 * @param slot Active card slot
 * @param type Tag type enum (TAG_TYPE_INDALA or TAG_TYPE_INDALA_20)
 * @return True if initialized successfully
 */
bool indala_data_factory(uint8_t slot, tag_specific_type_t type) {
    NRF_LOG_INFO("Creating Indala factory data");
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(slot, TAG_SENSE_LF, &map_info);
    
    // Check if data already exists
    if (fds_is_exists(map_info.id, map_info.key)) {
        NRF_LOG_INFO("Indala factory data already exists");
        return false;  // Already initialized
    }
    
    // Create default Indala factory data with 3 sectors
    uint8_t factory_data[104];  // Max: 3 sectors (96 bytes) + parity + CRC
    
    // Sector 0: Sector code + serial number
    memset(factory_data, 0xAA, 32);  // ASK "high" state for all bits
    factory_data[0] = 0x1A;          // Sector code
    factory_data[1] = nrf_rand() & 0xFF;  // Serial number byte 1
    factory_data[2] = nrf_rand() & 0xFF;  // Serial number byte 2
    
    // Sector 1: Data area
    memset(&factory_data[32], 0xBB, 32);
    
    // Sector 2: Data area  
    memset(&factory_data[64], 0xCC, 32);
    
    // Parity bit for sector 0 (XOR of all bytes in sector 0)
    uint8_t parity = 0;
    for (int i = 0; i < 32; i++) {
        parity ^= factory_data[i];
    }
    factory_data[96] = parity;
    
    // Calculate and store CRC
    uint16_t crc;
    calc_14a_crc_lut(factory_data, sizeof(factory_data), (uint8_t *)&crc);
    
    // Store in flash with CRC
    if (!fds_is_exists(map_info.id, map_info.key)) {
        bool ret = fds_write_sync(map_info.id, map_info.key, 
                                 sizeof(factory_data) + 2, factory_data);
        if (ret) {
            NRF_LOG_INFO("Created Indala factory data with random sectors");
            return true;
        } else {
            NRF_LOG_ERROR("Failed to create Indala factory data");
            return false;
        }
    }
    
    return false;  // Already exists
}

/* =========================== Emulation Handlers ========================== */

/**
 * Handle PSK field induction for Indala cards
 * Reader sends request, card responds with sector data
 */
void indala_field_induction_handler(void) {
    NRF_LOG_INFO("Indala PSK field induction detected");
    
    tag_data_buffer_t *buffer = get_buffer_by_tag_type(TAG_TYPE_INDALA);
    if (buffer == NULL || buffer->length < 96) {
        return;
    }
    
    // Wait for reader request then respond with sector data
    nrf_hw_timer_delay_ms(10);  // Allow time for PSK handshake
    
    // Transmit Indala sectors using PSK modulation
    transmit_psk_carrier(buffer->buffer, buffer->length, 20000UL);  // 1MHz base clock
}

/**
 * Respond to sector read request (Indala uses reader-to-first protocol)
 */
bool indala_read_sector_handler(uint8_t *sector_num, uint8_t *data_out, uint16_t max_len) {
    NRF_LOG_DEBUG("Indala read sector handler: sector=%d", *sector_num);
    
    tag_data_buffer_t *buffer = get_buffer_by_tag_type(TAG_TYPE_INDALA);
    if (buffer == NULL || buffer->length < 96) {
        return false;
    }
    
    // Indala always returns all 3 sectors (192 bits = 24 bytes)
    uint16_t copy_len = min(24, max_len);
    memcpy(data_out, buffer->buffer, copy_len);
    
    NRF_LOG_INFO("Sent %d bytes of Indala sector data", copy_len);
    return true;
}

/**
 * Handle write operation for Indala
 */
bool indala_write_handler(uint8_t *data_in, uint16_t length) {
    NRF_LOG_DEBUG("Indala write request: len=%d", length);
    
    tag_data_buffer_t *buffer = get_buffer_by_tag_type(TAG_TYPE_INDALA);
    if (buffer == NULL || buffer->length < 96) {
        return false;
    }
    
    // Update sector data with new values
    if (length <= buffer->length - 32) {  // Can only update specific sectors
        memcpy(&buffer->buffer[0], data_in, length);
        buffer->length = length + 32;  // Preserve other sectors
        
        NRF_LOG_INFO("Updated Indala sector with %d bytes", length);
    } else {
        // Truncate if new data is too large
        memcpy(buffer->buffer, data_in, buffer->length - 32);
        
        NRF_LOG_WARNING("Truncated Indala write");
    }
    
    return true;
}

/* =========================== PSK Modulation Handling ========================== */

/**
 * Calculate parity bit for Indala sector 0
 */
uint8_t indala_calc_parity_sector0(const uint8_t *sector0, uint16_t sector_len) {
    NRF_LOG_DEBUG("Indala parity calculation: len=%d", sector_len);
    
    uint8_t parity = 0;
    for (int i = 0; i < sector_len && i < 32; i++) {
        parity ^= sector0[i];
    }
    
    return parity;
}

/**
 * Verify Indala PSK frame integrity
 */
bool indala_verify_psk_frame(const uint8_t *frame, uint16_t length) {
    NRF_LOG_DEBUG("Indala PSK frame verification: len=%d", length);
    
    // Simple XOR-based checksum for PSK frames
    uint8_t expected_checksum = 0;
    for (uint8_t i = 0; i < length && i < 32; i++) {
        expected_checksum ^= frame[i];
    }
    
    // Last byte is actual parity bit
    if (length > 0) {
        return (expected_checksum == frame[length - 1]);
    }
    
    return false;
}

/* =========================== Utility Functions ========================== */

/**
 * Indala sector constants
 */
#define INDALA_SECTOR_0     32   // Sector code + serial number
#define INDALA_SECTOR_1     32   // Data area 1
#define INDALA_SECTOR_2     32   // Data area 2
#define INDALA_PARITY_OFFSET    96  // Parity bit offset

/**
 * Generate random sector for Indala factory initialization
 */
void indala_generate_random_sector(uint8_t *sector, uint16_t length) {
    NRF_LOG_DEBUG("Indala random sector generation: len=%d", length);
    
    memset(sector, 0xBB, length);  // ASK "high" state
    
    for (int i = 0; i < length; i++) {
        sector[i] = nrf_rand() & 0xFF;
    }
}

/**
 * Convert Indala byte buffer to bit stream for PSK transmission
 */
void indala_bytes_to_bits(const uint8_t *bytes, uint16_t byte_len, uint8_t *bits) {
    for (int i = 0; i < byte_len * 8 && i < 256; i++) {  // Indala max bits ~256
        int byte_idx = i / 8;
        int bit_idx = i % 8;
        
        bits[i] = (bytes[byte_idx] >> (7 - bit_idx)) & 1;
    }
}

/**
 * Transmit Indala sectors using PSK modulation
 */
void transmit_psk_carrier(const uint8_t *data, uint16_t byte_len, uint32_t base_clock_hz) {
    NRF_LOG_DEBUG("Indala PSK transmission: clock=%dHz", base_clock_hz);
    
    // Convert bytes to bits
    uint8_t bits[256];  // Indala max ~256 bits (4 sectors worth for safety)
    indala_bytes_to_bits(data, byte_len, bits);
    
    // Transmit each bit using PSK modulation with base clock
    uint32_t bit_duration_us = 1000UL / base_clock_hz;
    for (uint8_t i = 0; i < byte_len * 8 && i < 256; i++) {
        if (bits[i] == 1) {
            enable_psk_carrier(base_clock_hz, bit_duration_us);
        } else {
            disable_psk_carrier();
        }
    }
}
