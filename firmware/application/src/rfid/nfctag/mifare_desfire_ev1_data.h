#ifndef MIFARE_DESFIRE_EV1_DATA_H
#define MIFARE_DESFIRE_EV1_DATA_H

#include <stdint.h>
#include "app_util.h"
#include "tag_emulation.h"
#include "nfc_14a_4.h"

/**
 * MIFARE DESFire EV1 Data Handler Header
 * 
 * Provides data handlers for ISO14443-4 emulation
 * Supports low-rate communication mode
 */

/* Type definitions */
typedef int (*desfire_ev1_loadcb_t)(tag_specific_type_t type, tag_data_buffer_t *buffer);
typedef int (*desfire_ev1_savecb_t)(tag_specific_type_t type, tag_data_buffer_t *buffer);
typedef bool (*desfire_ev1_factory_t)(uint8_t slot, tag_specific_type_t type);

/* Data handler function pointers */
extern desfire_ev1_loadcb_t g_desfire_ev1_loadcb;
extern desfire_ev1_savecb_t g_desfire_ev1_savecb;
extern desfire_ev1_factory_t g_desfire_ev1_factory;

/* Initialize DESFire EV1 secure channel */
int desfire_ev1_establish_secure_channel(uint8_t slot, tag_data_buffer_t *buffer);

/* Route APDU to appropriate handler */
bool desfire_ev1_route_apdu(uint8_t apdu_type, uint8_t *buffer, uint16_t length);

/* Calculate DESFire EV1 authentication response */
void desfire_ev1_calc_auth_response(uint8_t *key_encrypted, uint8_t *mac,
                                    uint8_t sector, uint8_t record);

/* Calculate DES3 MAC for DESFire EV1 */
void calculate_des3_mac(uint8_t *key, uint8_t *data, uint8_t *output);

/* Handle SELECT AID command */
bool desfire_ev1_handle_select_aid(uint8_t *buffer, uint16_t length);

/* Initialize communication mode */
bool desfire_ev1_init_communication(uint8_t *buffer, uint16_t length);

/* Read records from emulation */
bool desfire_ev1_read_records(uint8_t *buffer, uint16_t length);

/* Write records to emulation */
bool desfire_ev1_write_records(uint8_t *buffer, uint16_t length);

/* Initialize key generation */
bool desfire_ev1_init_key_generation(uint8_t *buffer, uint16_t length);

/* Generate authentication token */
bool desfire_ev1_generate_auth_token(uint8_t *buffer, uint16_t length);

/* Authenticate with token */
bool desfire_ev1_authenticate_with_token(uint8_t *buffer, uint16_t length);

#endif /* MIFARE_DESFIRE_EV1_DATA_H */
