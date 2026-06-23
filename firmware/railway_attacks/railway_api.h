/**
 * Chameleon Ultra - Relay Attack API
 * 
 * Firmware interface for implementing relay attack functionality.
 * Status: NOT IMPLEMENTED
 */

#ifndef CHAMELEON_RELAY_API_H
#define CHAMELEON_RELAY_API_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Relay Session Structure - Maintains state for bidirectional traffic relay
 */
typedef struct {
    uint64_t session_id;                    // Unique relay session identifier
    struct sockaddr_storage origin_addr;    // Source endpoint address
    struct sockaddr_storage dest_addr;      // Target endpoint address
    time_t start_time;                      // Session start timestamp
    time_t deadline_ms;                     // Session expiration (milliseconds)
    uint32_t packets_sent;                  // Total packets forwarded
    uint32_t bytes_forwarded;               // Total bytes forwarded
    uint32_t retransmits;                   // Packet retransmission count
    int is_encrypted;                       // Whether relayed traffic is encrypted
    void *tls_session_data;                 // TLS session context (if applicable)
} relay_session_t;

/**
 * Endpoint Configuration Structure
 */
typedef struct {
    union {
        struct sockaddr_in4 ipv4;
        struct sockaddr_in6 ipv6;
        struct sockaddr_un unix;
    } addr;
    uint16_t port;
    char domain[256];                       // Domain name for TLS/SNI
} endpoint_config_t;

/**
 * Relay Packet Forward Callback
 * Called when packet needs to be forwarded between endpoints
 */
typedef int (*relay_forward_callback)(uint8_t *packet, size_t len, void *user_data);

/**
 * Relay Session Statistics
 */
typedef struct {
    uint64_t total_packets;                 // Total packets processed
    uint64_t total_bytes;                   // Total bytes processed
    uint32_t active_sessions;               // Currently active sessions
    uint32_t completed_sessions;            // Sessions that completed normally
    uint32_t failed_sessions;               // Sessions that failed
} relay_stats_t;

/**
 * Initialize relay subsystem for attacks
 * 
 * @param interface Network interface to use for relaying (e.g., "eth0")
 * @param listen_port Port to listen on for client connections
 * @param relay_port Port to relay traffic to destination
 * @return 0 on success, negative error code on failure
 */
int relay_subsystem_init(const char *interface, 
                         uint16_t listen_port, 
                         uint16_t relay_port);

/**
 * Clean up relay subsystem resources
 */
void relay_subsystem_cleanup(void);

/**
 * Get current relay subsystem statistics
 * 
 * @return Pointer to stats structure (caller owns the pointer)
 */
relay_stats_t *relay_get_stats(void);

/**
 * Establish a new relay connection between origin and destination
 * 
 * @param origin Endpoint configuration for source
 * @param destination Endpoint configuration for target
 * @param timeout_ms Connection timeout in milliseconds
 * @return Pointer to relay session on success, NULL on failure
 */
relay_session_t *relay_connect(endpoint_config_t *origin,
                               endpoint_config_t *destination,
                               int timeout_ms);

/**
 * Terminate a relay session
 * 
 * @param session Relay session to close
 */
void relay_close(relay_session_t *session);

/**
 * Forward data packet between relay endpoints
 * 
 * @param session Active relay session
 * @param data Packet data buffer
 * @param len Length of packet data
 * @return 0 on success, negative error code on failure
 */
int relay_forward_packet(relay_session_t *session,
                         const uint8_t *data,
                         size_t len);

/**
 * Get session remaining lifetime in milliseconds
 * 
 * @param session Relay session
 * @return Remaining lifetime or -1 if session has expired
 */
time_t relay_get_remaining_lifetime(relay_session_t *session);

/**
 * Check if relay session is still active
 * 
 * @param session Relay session
 * @return 1 if active, 0 if closed/expired
 */
int relay_is_active(relay_session_t *session);

/**
 * Set TLS session context for encrypted traffic relay
 * 
 * @param session Relay session
 * @param tls_ctx TLS context pointer
 * @return 0 on success
 */
int relay_set_tls_context(relay_session_t *session, void *tls_ctx);

/**
 * Get proxy URL for current relay configuration
 * 
 * @return String representation of proxy URL (caller must free)
 */
char *relay_get_proxy_url(void);

/**
 * Set relay timeout policy
 * 
 * @param default_timeout_ms Default session timeout in milliseconds
 * @return 0 on success
 */
int relay_set_default_timeout(uint32_t default_timeout_ms);

/**
 * Enable/disable SSL/TLS interception (MITM mode)
 * 
 * @param enable 1 to enable MITM, 0 to disable
 * @return 0 on success
 */
int relay_enable_mitm(int enable);

#ifdef __cplusplus
}
#endif

#endif /* CHAMELEON_RELAY_API_H */
