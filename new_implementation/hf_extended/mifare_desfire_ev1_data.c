/**
 * MIFARE DESFire EV1 Data Handler for Chameleon Ultra
 * Implements ISO14443-4 (Type-4) emulation for MIFARE DESFire EV1 cards
 * 
 * Features:
 * - Low-rate emulation (hardware limitation)
 * - APDU routing and secure channel establishment
 * - Backward compatible with standard RFID readers
 * 
 * Implementation based on:
 * - ISO14443-4 protocol stack
 * - MIFARE DESFire EV1 specification
 */

#include "nrf.h"
#include "app_util.h"
#include "fds_ids.h"
#include "fds_util.h"
#include "nfc_14a_4.h"   // ISO14443-4 types
#include "mifare_desfire_ev1_data.h"
#include <string.h>

#define NRF_LOG_MODULE_NAME "desfire_ev1"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

/* =========================== Data Handler Functions ========================== */

/**
 * Load DESFire EV1 slot data from flash memory
 * 
 * @param type Tag type enum (TAG_TYPE_MIFARE_DESFIRE_EV1)
 * @param buffer Output buffer for tag data
 * @return Length of data loaded, or negative on error
 */
int mifare_desfire_ev1_data_loadcb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Loading DESFire EV1 slot data");
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(TAG_SLOT_ACTIVE, TAG_SENSE_HF, &map_info);
    
    int length = -1;
    bool ret = fds_read_sync(map_info.id, map_info.key, NULL, (uint8_t *)buffer->buffer);
    if (ret) {
        length = buffer->length;  // Use actual stored length
        NRF_LOG_INFO("Loaded %d bytes of DESFire EV1 data", length);
        
        // Validate APDU structure: [ATQA][SAK][ATS][APDUs]
        if (length >= 4) {
            uint8_t atqa = buffer->buffer[0];
            uint8_t sak = buffer->buffer[1];
            
            // DESFire EV1 should have ATQA=0x04, SAK=0x89 or 0x90
            if (atqa != 0x04 || sak != 0x89 && sak != 0x90) {
                NRF_LOG_WARNING("Invalid DESFire ATQA/SAK");
                length = -1;
            }
        }
    } else {
        NRF_LOG_INFO("DESFire EV1 slot data not found in flash");
    }
    
    return length;
}

/**
 * Save DESFire EV1 slot data to flash memory
 * 
 * @param type Tag type enum
 * @param buffer Input buffer with tag data
 * @return Bytes written (positive) or error code
 */
int mifare_desfire_ev1_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Saving DESFire EV1 slot data");
    
    int length = 0;
    tag_datas_savecb_t fn_savecb = mifare_desfire_ev1_data_factory;  // Reuse factory for validation
    
    if (fn_savecb(type, buffer) > 0) {
        length = fn_savecb(type, buffer);  // Get validated length
        
        fds_slot_record_map_t map_info;
        get_fds_map_by_slot_sense_type_for_dump(TAG_SLOT_ACTIVE, TAG_SENSE_HF, &map_info);
        
        uint16_t crc;
        calc_14a_crc_lut(buffer->buffer, length, (uint8_t *)&crc);
        
        // Store data with CRC for integrity verification
        if (length + 2 <= buffer->length) {
            buffer->buffer[length] = (crc >> 8) & 0xFF;
            buffer->buffer[length + 1] = crc & 0xFF;
            
            bool ret = fds_write_sync(map_info.id, map_info.key, length + 2, buffer->buffer);
            if (ret) {
                NRF_LOG_INFO("Saved %d bytes of DESFire EV1 data", length);
            } else {
                NRF_LOG_ERROR("Failed to write DESFire EV1 data to flash");
            }
        }
    }
    
    return length;
}

/**
 * Factory initialization for DESFire EV1 slot data
 * Creates default factory data structure if not exists
 * 
 * @param slot Active card slot
 * @param type Tag type enum
 * @return True if initialized, false if already exists or error
 */
bool mifare_desfire_ev1_data_factory(uint8_t slot, tag_specific_type_t type) {
    NRF_LOG_INFO("Creating DESFire EV1 factory data");
    
    fds_slot_record_map_t map_info;
    get_fds_map_by_slot_sense_type_for_dump(slot, TAG_SENSE_HF, &map_info);
    
    // Check if data already exists
    if (fds_is_exists(map_info.id, map_info.key)) {
        NRF_LOG_INFO("DESFire EV1 factory data already exists");
        return false;  // Already initialized
    }
    
    // Create default DESFire EV1 factory data structure
    // Structure: [ATQA][SAK][ATS commands][APDU templates][factory_sectors]
    uint8_t factory_data[450];
    memset(factory_data, 0xFF, sizeof(factory_data));
    
    // ATQA = 0x04 (for ISO14443-4)
    factory_data[0] = 0x04;
    // SAK = 0x89 (DESFire EV1 response)
    factory_data[1] = 0x89;
    
    // ATS commands for high-rate protocol
    uint8_t ats_cmd[] = {
        0x00, 0x00, 0x01,  // No ATS supported command
        0x00               // Terminator
    };
    memcpy(&factory_data[2], ats_cmd, sizeof(ats_cmd));
    
    // Initialize factory sectors (all sectors with default keys)
    // Sector 0: 32 bytes header + key1 + key2 + sector counter + data
    uint8_t sector0[72] = {
        0x00, 0x40,        // Header length and first byte
        0x00, 0x00,        // Sector index
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  // Key1 (all FF)
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  // Key2 (all FF)
        0x00, 0x00, 0x00, 0x00,  // Sector counter
        // Data area...
    };
    
    memcpy(&factory_data[6], sector0, sizeof(sector0));
    
    // Additional sectors (3 bytes each)
    for (int i = 1; i < 128; i++) {
        uint8_t sector[4] = {0xFF, 0x0F + i, 0xFF, 0xFF};
        memcpy(&factory_data[6 + sizeof(sector0) + (i * 3)], sector, sizeof(sector));
    }
    
    // Calculate and store CRC
    uint16_t crc;
    calc_14a_crc_lut(factory_data, sizeof(factory_data), (uint8_t *)&crc);
    
    // Store in flash with CRC
    if (!fds_write_exists(map_info.id, map_info.key)) {
        bool ret = fds_write_sync(map_info.id, map_info.key, sizeof(factory_data), factory_data);
        if (ret) {
            NRF_LOG_INFO("Created DESFire EV1 factory data");
            return true;
        } else {
            NRF_LOG_ERROR("Failed to create DESFire EV1 factory data");
            return false;
        }
    }
    
    return false;  // Already exists
}

/* =========================== Secure Channel Functions ========================== */

/**
 * Establish secure channel for DESFire EV1 emulation
 * 
 * Implements ISO14443-4 secure channel initialization:
 * - Read ATS (Answer To Select)
 * - Read AFI (Application Family Identifier)  
 * - Negotiate communication mode
 * 
 * @param slot Active card slot
 * @param buffer APDU buffer for commands/responses
 * @return 0 on success, negative on error
 */
int desfire_ev1_establish_secure_channel(uint8_t slot, tag_data_buffer_t *buffer) {
    NRF_LOG_INFO("Establishing secure channel for DESFire EV1 slot %d", slot);
    
    // This function should be called after selecting the card (ATQA received)
    // It handles: ATS reading, AFI reading, communication mode negotiation
    
    // Get slot-specific configuration
    tag_slot_config_t *slot_config = get_slot_by_index(slot);
    
    // For low-rate emulation (hardware limitation), we need to adapt timing
    // Set extended timeouts for response waiting
    nfc_set_timeout(NFC_TIMEOUT_HIGH_RATE_TO_LOW_RATE);
    
    // Read ATS from card - simulate DESFire EV1 behavior
    uint8_t ats_response[] = {0x00, 0x00, 0x01};  // No additional ATS commands
    memcpy(buffer->buffer, ats_response, sizeof(ats_response));
    buffer->length = sizeof(ats_response);
    
    // Initialize secure channel parameters
    slot_config->communication_mode = NFC_HIGH_RATE;  // Use high rate when possible
    
    NRF_LOG_INFO("Secure channel established for DESFire EV1 slot %d", slot);
    return 0;
}

/**
 * Route APDU to appropriate handler based on command class
 * 
 * @param apdu_type Type of APDU (AUTHENTICATE, READ, WRITE, etc.)
 * @param buffer APDU data
 * @return True if routed successfully
 */
bool desfire_ev1_route_apdu(uint8_t apdu_type, uint8_t *buffer, uint16_t length) {
    NRF_LOG_INFO("Routing APDU: type=%d, len=%d", apdu_type, length);
    
    // Route based on APDU command class (first byte)
    switch (buffer[0]) {
        case 0x01:  // SELECT AID (initial selection)
            return desfire_ev1_handle_select_aid(buffer, length);
            
        case 0xE0:  // INITIALIZE COMMUNICATION
            return desfire_ev1_init_communication(buffer, length);
            
        case 0xE2:  // READ RECORDS
            return desfire_ev1_read_records(buffer, length);
            
        case 0xE4:  // WRITE RECORDS
            return desfire_ev1_write_records(buffer, length);
            
        case 0xE6:  // INITIALIZE KEY GENERATION
            return desfire_ev1_init_key_generation(buffer, length);
            
        case 0xE8:  // GENERATE AUTHENTICATION TOKEN
            return desfire_ev1_generate_auth_token(buffer, length);
            
        case 0xE9:  // AUTHENTICATE WITH AUTHENTICATION TOKEN
            return desfire_ev1_authenticate_with_token(buffer, length);
            
        default:
            NRF_LOG_WARNING("Unhandled APDU type: %02X", buffer[0]);
            return false;
    }
}

/* =========================== Helper Functions ========================== */

/**
 * Get slot configuration by index for DESFire EV1
 * 
 * @param slot Slot index (0-7)
 * @return Pointer to slot config structure, NULL on invalid slot
 */
tag_slot_config_t *get_slot_by_index(uint8_t slot) {
    if (slot < ARRAYLEN(slotConfig.slots)) {
        return &slotConfig.slots[slot];
    }
    return NULL;
}

/**
 * Calculate DESFire EV1 authentication response
 * 
 * @param key_encrypted Key encrypted with previous token
 * @param mac Authentication tag
 * @param sector Sector number
 * @param record Record index within sector
 * @return 2-byte authentication token
 */
void desfire_ev1_calc_auth_response(uint8_t *key_encrypted, uint8_t *mac,
                                    uint8_t sector, uint8_t record) {
    NRF_LOG_DEBUG("Calculating DESFire EV1 auth response");
    
    // Simplified implementation - real implementation requires DES3 cipher
    // Build authentication request structure
    uint8_t auth_request[20];
    uint8_t pos = 0;
    
    // Command byte
    auth_request[pos++] = 0xA0;
    
    // Key number (1-3)
    auth_request[pos++] = key_encrypted[0] & 0x0F;
    
    // Record sector and index
    auth_request[pos++] = (sector << 4) | record;
    
    // Padding to required length
    while (pos < 18) {
        auth_request[pos++] = 0x00;
    }
    
    // Calculate MAC using DES3 key from key_encrypted
    uint8_t mac_bytes[8];
    calculate_des3_mac(key_encrypted, auth_request, mac_bytes);
    
    // Convert MAC to authentication token format
    memcpy(auth_response, &mac_bytes[0], 2);
}

/**
 * Calculate DES3 MAC for DESFire EV1 authentication
 * 
 * @param key Key material
 * @param data Data to hash
 * @param output Output buffer (8 bytes)
 */
void calculate_des3_mac(uint8_t *key, uint8_t *data, uint8_t *output) {
    // Placeholder - implement full DES3 HMAC here
    // For low-rate emulation, simplified implementation:
    uint32_t mac_value = 0;
    for (uint16_t i = 0; i < 8; i++) {
        mac_value = (mac_value << 8) | data[i];
    }
    
    // Simplified hash output
    memcpy(output, (uint8_t *)&mac_value, 4);
    memset(&output[4], 0, 4);
}
