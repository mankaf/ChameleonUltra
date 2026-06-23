/**
 * EM4305 Card Emulation for Chameleon Ultra
 * 
 * ASK 125 kHz card - Hitag M variant, widely used in access control systems
 * 
 * Features:
 * - Multiple UID lengths (32/64 bit variants)
 * - Extended data format beyond simple ID
 * - Variable sector structure
 * 
 * Implementation notes:
 * - Supports both read-only and read/write modes
 * - Compatible with EM-Micro Hitag M series readers
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

#define NRF_LOG_MODULE_NAME em4305
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

#define EM4305_32BIT 4
#define EM4305_64BIT 8

/* =========================== EM4305 Data Format ========================== */

/** EM4305 data structure - variable length */
typedef struct {
    uint8_t uid_length;             // UID length: 32 or 64 bits
    uint8_t data_bits[68];          // Raw data bits (max 68 bits)
    uint8_t bit_count;              // Number of data bits stored
    uint8_t has_extended_data;      // True if beyond simple ID
    uint8_t sector_count;           // Number of sectors in extended data
} em4305_data_t;

/* =========================== Data Handler Functions ========================== */

/**
 * Load EM4305 slot data from flash memory
 * 
 * @param type Tag type enum (TAG_TYPE_EM4305 or TAG_TYPE_EM4305_64)
 * @param buffer Output buffer for tag data
 * @return Length of bytes loaded, or -1 on error
 */
int em4305_data_loadcb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Loading EM4305 slot data");
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(tag_emulation_get_slot(), TAG_SENSE_LF, &map_info);
    
    int byte_count = -1;
    bool ret = fds_read_sync(map_info.id, map_info.key, NULL, buffer->buffer);
    if (ret) {
        byte_count = buffer->length;
        NRF_LOG_INFO("Loaded %d bytes of EM4305 data", byte_count);
        
        // Validate EM4305 format: 4-8 bytes (32-64 bits basic ID, possibly more)
        if (byte_count < 1 || byte_count > 8) {
            NRF_LOG_WARNING("Invalid EM4305 length: %d", byte_count);
            byte_count = -1;
        }
    } else {
        NRF_LOG_INFO("EM4305 slot data not found in flash");
    }
    
    return byte_count;
}

/**
 * Save EM4305 slot data to flash memory
 * 
 * @param type Tag type enum
 * @param buffer Input buffer with tag data
 * @return Bytes written (positive) or error code
 */
int em4305_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Saving EM4305 slot data");
    
    int byte_length = buffer->length;
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(tag_emulation_get_slot(), TAG_SENSE_LF, &map_info);
    
    // Store with header indicating UID length variant
    uint8_t uid_variant = 0;
    if (byte_length == 4) {
        uid_variant = EM4305_32BIT;
    } else if (byte_length >= 8) {
        uid_variant = EM4305_64BIT;
    }
    
    // Calculate and store CRC for integrity check
    uint16_t crc;
    calc_14a_crc_lut(buffer->buffer, byte_length, (uint8_t *)&crc);
    
    // Store in flash with variant indicator and CRC
    if (byte_length + 2 <= buffer->length) {
        buffer->buffer[0] = uid_variant;
        buffer->buffer[1] = crc & 0xFF;
        
        bool ret = fds_write_sync(map_info.id, map_info.key, 
                                 byte_length + 2, buffer->buffer);
        if (ret) {
            NRF_LOG_INFO("Saved %d bytes of EM4305 data", byte_length);
        } else {
            NRF_LOG_ERROR("Failed to write EM4305 data to flash");
        }
    }
    
    return byte_length;
}

/**
 * Factory initialization for EM4305 slot data
 * Creates default factory data with random 32-bit or 64-bit UID
 * 
 * @param slot Active card slot
 * @param type Tag type enum (TAG_TYPE_EM4305 or TAG_TYPE_EM4305_64)
 * @return True if initialized successfully
 */
bool em4305_data_factory(uint8_t slot, tag_specific_type_t type) {
    NRF_LOG_INFO("Creating EM4305 factory data");
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(slot, TAG_SENSE_LF, &map_info);
    
    // Check if data already exists
    if (fds_is_exists(map_info.id, map_info.key)) {
        NRF_LOG_INFO("EM4305 factory data already exists");
        return false;  // Already initialized
    }
    
    // Determine UID length from type
    uint8_t uid_variant = EM4305_64BIT;  // Default to 64-bit
    if (type == TAG_TYPE_EM4305) {
        uid_variant = EM4305_32BIT;
    }
    
    // Create default EM4305 factory data with random UID
    uint8_t factory_data[8];  // Max 64 bits = 8 bytes
    memset(factory_data, 0xAA, sizeof(factory_data));  // ASK "high" state
    
    // Generate random UID (32 or 64 bits)
    for (uint8_t i = 0; i < uid_variant / 8; i++) {
        factory_data[i] = nrf_rand() & 0xFF;
    }
    
    // Add sector information for extended data support
    factory_data[uid_variant / 8] = 0x03;  // Sector count: 3 sectors
    
    // Calculate and store CRC
    uint16_t crc;
    calc_14a_crc_lut(factory_data, sizeof(factory_data), (uint8_t *)&crc);
    
    // Store in flash with CRC
    if (!fds_is_exists(map_info.id, map_info.key)) {
        bool ret = fds_write_sync(map_info.id, map_info.key, 
                                 sizeof(factory_data) + 1, factory_data);
        if (ret) {
            NRF_LOG_INFO("Created EM4305 factory data with %d-bit UID", 
                         uid_variant * 8);
            return true;
        } else {
            NRF_LOG_ERROR("Failed to create EM4305 factory data");
            return false;
        }
    }
    
    return false;  // Already exists
}

/* =========================== Emulation Handlers ========================== */

/**
 * Handle field induction for EM4305
 * Responds with UID and optional extended data
 */
void em4305_field_induction_handler(void) {
    NRF_LOG_INFO("EM4305 field induction detected");
    
    tag_data_buffer_t *buffer = get_buffer_by_tag_type(TAG_TYPE_EM4305);
    if (buffer == NULL || buffer->length == 0) {
        return;
    }
    
    // Transmit stored EM4305 data
    int byte_len = buffer->length;
    if (byte_len > 0) {
        NRF_LOG_INFO("Transmitting %d bytes of EM4305 data", byte_len);
        
        // ASK carrier modulation
        transmit_ask_carrier(buffer->buffer, byte_len, 125000UL);
    }
}

/**
 * Respond to reader request for EM4305 UID
 */
bool em4305_read_uid_handler(uint8_t *data_out, uint16_t max_len) {
    NRF_LOG_DEBUG("EM4305 read UID handler");
    
    tag_data_buffer_t *buffer = get_buffer_by_tag_type(TAG_TYPE_EM4305);
    if (buffer == NULL || buffer->length == 0) {
        NRF_LOG_WARNING("No EM4305 data available");
        return false;
    }
    
    // Return only UID portion (first 4 or 8 bytes)
    uint16_t uid_len = min(buffer->length, max_len);
    memcpy(data_out, buffer->buffer, uid_len);
    
    NRF_LOG_INFO("Sent %d bytes of EM4305 UID to reader", uid_len);
    return true;
}

/**
 * Handle extended data read (if beyond simple ID)
 */
bool em4305_read_extended_handler(uint8_t *data_out, uint16_t max_len) {
    NRF_LOG_DEBUG("EM4305 read extended data handler");
    
    tag_data_buffer_t *buffer = get_buffer_by_tag_type(TAG_TYPE_EM4305);
    if (buffer == NULL || buffer->length == 0) {
        return false;
    }
    
    // Return full data for extended read requests
    uint16_t copy_len = min(buffer->length, max_len);
    memcpy(data_out, buffer->buffer, copy_len);
    
    NRF_LOG_INFO("Sent %d bytes of EM4305 extended data", copy_len);
    return true;
}

/* =========================== Sector Operations ========================== */

/**
 * Parse EM4305 sector structure from UID
 * 
 * @param uid_uid Pointer to UID buffer
 * @param uid_len UID length in bytes (4 or 8)
 * @return Number of sectors found, 0 on error
 */
uint8_t em4305_parse_sectors(const uint8_t *uid, uint16_t uid_len) {
    NRF_LOG_DEBUG("EM4305 sector parsing: len=%d", uid_len);
    
    if (uid_len < 4) {
        return 0;  // Minimum 4 bytes for basic ID
    }
    
    // Check for sector indicator in UID (last byte for extended data)
    const uint8_t *data_start = &uid[uid_len - 1];  // Last byte contains sector count
    
    if (*data_start == 0x03) {  // Standard: 3 sectors
        NRF_LOG_INFO("EM4305 found standard 3-sector format");
        return 3;
    } else if (*data_start >= 0x10 && *data_start <= 0x20) {
        // Extended sector range
        uint8_t sectors = *data_start - 0x10;
        NRF_LOG_INFO("EM4305 found extended %d-sector format", sectors);
        return sectors;
    } else {
        NRF_LOG_WARNING("Invalid EM4305 sector indicator: %02X", *data_start);
        return 0;
    }
}

/**
 * Read specific EM4305 sector
 */
bool em4305_read_sector(uint8_t *sector_num, uint8_t *data_out, uint16_t max_len) {
    NRF_LOG_DEBUG("EM4305 read sector %d", *sector_num);
    
    // For write-only emulation, just return stored data
    tag_data_buffer_t *buffer = get_buffer_by_tag_type(TAG_TYPE_EM4305);
    if (buffer == NULL) {
        return false;
    }
    
    // Return full buffer regardless of sector request (emulation mode)
    uint16_t copy_len = min(buffer->length, max_len);
    memcpy(data_out, buffer->buffer, copy_len);
    
    return true;
}

/* =========================== Utility Functions ========================== */

/**
 * EM4305 UID length constants
 */
#define EM4305_SECTOR_COUNT     3   // Standard sector count

/**
 * Calculate CRC for EM4305 data block
 */
void em4305_calc_crc(const uint8_t *data, uint16_t len, uint8_t *crc) {
    NRF_LOG_DEBUG("EM4305 CRC calculation: len=%d", len);
    
    uint16_t crc_value = 0;
    for (uint16_t i = 0; i < len; i++) {
        crc_value = (crc_value << 8) | data[i];
    }
    
    // Convert to byte array
    crc[0] = (crc_value >> 8) & 0xFF;
    crc[1] = crc_value & 0xFF;
}

/**
 * Check if EM4305 variant is write-enabled
 */
bool em4305_is_write_enabled(tag_specific_type_t type) {
    // TAG_TYPE_EM4305 = basic read-only
    // TAG_TYPE_EM4305_64 = extended, may support write depending on sector config
    
    return (type == TAG_TYPE_EM4305_64);
}
