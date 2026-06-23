/**
 * Chameleon Ultra - Decryption Engine API
 * 
 * Status: NOT IMPLEMENTED
 */

#ifndef CHAMELEON_DECORPTION_API_H
#define CHAMELEON_DECORPTION_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Protocol Type Enumeration
 */
typedef enum {
    PROTOCOL_TLS,
    PROTOCOL_SSH,
    PROTOCOL_HTTPS,
    PROTOCOL_WIRELESS_AES,
    PROTOCOL_CUSTOM
} protocol_type_t;

/**
 * Key Material Structure (for decryption)
 */
typedef struct {
    uint8_t* key_material;     // Intercepted key material buffer
    size_t key_material_len;   // Length of key material
    int32_t key_slot;          // Crypto accelerator slot ID (if available)
} key_material_t;

/**
 * Decrypted Payload Structure
 */
typedef struct {
    uint8_t* payload;          // Decrypted data buffer
    size_t payload_len;        // Decrypted data length
    protocol_type_t protocol;  // Protocol that was decrypted
    uint64_t decrypt_time_ns;  // Decryption time in nanoseconds
} decrypted_payload_t;

/**
 * Decrypt a captured packet using available key material
 * 
 * @param ctx Decryptor context (contains protocol state, session info)
 * @param pkt Captured packet buffer
 * @param len Packet length
 * @param[out] out Decrypted payload structure (caller frees)
 * @return 0 on success, negative error code on failure
 */
int decrypt_packet(void* ctx, const uint8_t* pkt, size_t len, 
                   decrypted_payload_t** out);

/**
 * Load decryption keys from memory dump or captured traffic
 * 
 * @param key_type Type of key to extract (master session key, etc.)
 * @param source Source of key material (memory address, file path)
 * @return Pointer to loaded key structure on success, NULL on failure
 */
key_material_t* load_key_material(key_type_t key_type, const char* source);

/**
 * Initialize decryption engine with protocol parsers
 * 
 * @return 0 on success, negative error code on failure
 */
int decryptor_init(void);

/**
 * Shutdown decryption engine and release resources
 */
void decryptor_cleanup(void);

/**
 * Get current decryption performance statistics
 */
struct decryptor_stats {
    uint64_t total_packets;              // Total packets processed
    uint64_t decrypted_packets;          // Packets successfully decrypted
    uint64_t failed_decryptions;         // Failed decryption attempts
    double avg_decrypt_time_us;          // Average decryption time (microseconds)
} decryptor_get_stats(void);

#ifdef __cplusplus
}
#endif

#endif /* CHAMELEON_DECORPTION_API_H */
