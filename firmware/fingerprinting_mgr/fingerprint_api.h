/**
 * Chameleon Ultra - Fingerprinting Manager API
 * 
 * Status: NOT IMPLEMENTED
 */

#ifndef CHAMELEON_FINGERPRINTING_API_H
#define CHAMELEON_FINGERPRINTING_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Hardware Identifier Types
 */
typedef enum {
    FINGERPRINT_MAC_ADDRESS,      // Ethernet MAC address
    FINGERPRINT_SERIAL_NUMBER,    // Device serial number
    FINGERPRINT_HW_UUID,          // Hardware UUID
    FINGERPRINT_CHIP_ID,          // Vendor chip identifier
    FINGERPRINT_MODEL_ID          // Product model identifier
} fingerprint_type_t;

/**
 * Complete hardware fingerprint structure
 */
typedef struct {
    char mac_address[18];         // Ethernet MAC address (XX:XX:XX:XX:XX:XX)
    char serial_number[32];       // Serial number from USB/SMBIOS
    char device_id[64];           // Vendor-specific device ID
    unsigned int hardware_uuid[16];  // Hardware UUID (GUID format)
    char chip_fingerprint[128];   // Chip identifier fingerprint
} hw_fingerprint_t;

/**
 * Spoof data structure for identifier manipulation
 */
typedef struct {
    unsigned int mac_spoof[6];    // MAC address to spoof
    unsigned int device_id[4];    // Vendor ID to emulate
    unsigned int chip_emulation;  // Chip emulation flags
} spoof_data_t;

/**
 * Extract hardware identifiers from network/USB responses
 * 
 * @param[out] out Structure to store extracted fingerprint
 * @return 0 on success, negative error code on failure
 */
int extract_identifiers(hw_fingerprint_t* out);

/**
 * Spoof device hardware identifiers for authentication bypass
 * 
 * @param target Target device identifier (from known device)
 * @param spoof_result Structure to receive spoofed fingerprint
 * @return 0 on success, negative error code on failure
 */
int spoof_identifiers(const hw_fingerprint_t* target,
                      hw_fingerprint_t* spoof_result);

/**
 * Get list of known hardware devices and their fingerprints
 * 
 * @param out Buffer to store device database (caller allocates)
 * @return Number of devices stored on success
 */
size_t get_device_database(hw_fingerprint_t* out, size_t max_size);

/**
 * Add custom device fingerprint to database
 * 
 * @param fp Hardware fingerprint structure
 * @return 0 on success, negative error code on failure
 */
int add_device_to_database(const hw_fingerprint_t* fp);

/**
 * Check if fingerprint matches known device in database
 * 
 * @param fp Fingerprint to check
 * @return Pointer to matching device or NULL if no match
 */
hw_fingerprint_t* match_known_device(const hw_fingerprint_t* fp);

#ifdef __cplusplus
}
#endif

#endif /* CHAMELEON_FINGERPRINTING_API_H */
