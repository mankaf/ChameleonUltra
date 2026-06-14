#ifndef RELAY_CORE_H
#define RELAY_CORE_H

#include <stdint.h>

/**
 * Relay Attack Core Header
 * 
 * Provides multi-antenna synchronization and timing-based relay bypass
 */

/* Initialize relay attack support */
void relay_core_init(void);

/* Start relay attack mode for specified card slot */
bool relay_core_start(uint8_t slot);

/* Stop relay attack mode */
void relay_core_stop(void);

/* Relay challenge from reader to emulated card */
int relay_core_relay_challenge(uint8_t *challenge, uint16_t challenge_len);

/* Relay response from emulated card back to reader */
int relay_core_relay_response(uint8_t *response, uint16_t response_len);

/* Add time-stamping for relay detection bypass */
bool relay_core_add_timestamp(uint8_t *challenge, uint16_t challenge_len);

/* Calculate optimal response delay for relay attack */
uint32_t relay_core_calc_response_delay(void);

/* Check if relay attack is currently active */
bool relay_core_is_active(void);

/* Activate specific antenna hop for relay attack */
bool relay_core_activate_hop(uint8_t hop_index);

/* Deactivate antenna hop */
void relay_core_deactivate_hop(uint8_t hop_index);

/* Get list of active antenna hops */
uint8_t relay_core_get_active_hop_count(void);

/* Get relay response buffer (reusable) */
uint8_t *relay_core_get_response_buffer(void);

/* Get relay challenge buffer (reusable) */
uint8_t *relay_core_get_challenge_buffer(void);

/* Legacy compatibility wrapper */
int relay_core_relay(int8_t *challenge, uint16_t len);

#endif /* RELAY_CORE_H */
