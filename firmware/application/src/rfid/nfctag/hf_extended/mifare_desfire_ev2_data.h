#ifndef MIFARE_DESFIRE_EV2_DATA_H
#define MIFARE_DESFIRE_EV2_DATA_H

#include <stdint.h>
#include "app_util.h"
#include "tag_emulation.h"
#include "nfc_14a_4.h"

/**
 * MIFARE DESFire EV2 Data Handler Header
 * 
 * Provides data handlers for ISO14443-4 emulation with AES support
 * Supports both AES-GCM and AES-CCM cipher suites
 */

/* Type definitions */
typedef int (*desfire_ev2_loadcb_t)(tag_specific_type_t type, tag_data_buffer_t *buffer);
typedef int (*desfire_ev2_savecb_t)(tag_specific_type_t type, tag_data_buffer_t *buffer);
typedef uint8_t *(*desfire_ev2_factory_t)(uint8_t *buffer, tag_specific_type_t type);

/* Data handler function pointers */
extern desfire_ev2_loadcb_t g_desfire_ev2_loadcb;
extern desfire_ev2_savecb_t g_desfire_ev2_savecb;
extern desfire_ev2_factory_t g_desfire_ev2_factory;

int mifare_desfire_ev2_data_loadcb(tag_specific_type_t type, tag_data_buffer_t *buffer);
int mifare_desfire_ev2_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer);
uint8_t *mifare_desfire_ev2_data_factory(uint8_t *buffer, tag_specific_type_t type);
bool mifare_desfire_ev2_data_factory_slot(uint8_t slot, tag_specific_type_t type);

/* Initialize secure channel with AES-GCM */
int desfire_ev2_establish_secure_channel_aes(uint8_t slot, tag_data_buffer_t *buffer);

/* Initialize secure channel with AES-CCM */
int desfire_ev2_establish_secure_channel_ccm(uint8_t slot, tag_data_buffer_t *buffer);

/* Route APDU to appropriate handler */
bool desfire_ev2_route_apdu(uint8_t apdu_type, uint8_t *buffer, uint16_t length);

/* Handle SELECT AID command */
bool desfire_ev2_handle_select_aid(uint8_t *buffer, uint16_t length);

/* Initialize AES-GCM communication */
bool desfire_ev2_init_communication_aes(uint8_t *buffer, uint16_t length);

/* Initialize AES-CCM communication */
bool desfire_ev2_init_communication_ccm(uint8_t *buffer, uint16_t length);

/* Initialize standard communication (backward compatible) */
bool desfire_ev2_init_communication_std(uint8_t *buffer, uint16_t length);

/* Read records with AES-GCM authentication */
bool desfire_ev2_read_records_aes(uint8_t *buffer, uint16_t length);

/* Write records with AES-GCM authentication */
bool desfire_ev2_write_records_aes(uint8_t *buffer, uint16_t length);

/* Initialize key generation */
bool desfire_ev2_init_key_generation(uint8_t *buffer, uint16_t length);

/* Generate auth token with AES-GCM */
bool desfire_ev2_generate_auth_token_aes(uint8_t *buffer, uint16_t length);

/* Read file with AES-GCM */
bool desfire_ev2_read_file_aes(uint8_t *buffer, uint16_t length);

/* Write file with AES-GCM */
bool desfire_ev2_write_file_aes(uint8_t *buffer, uint16_t length);

/* Calculate auth response with AES-GCM */
void desfire_ev2_calc_auth_response_aes(uint8_t *key_encrypted, uint8_t *mac,
                                        uint8_t sector, uint8_t record);

/* Calculate MAC using AES-GCM */
void compute_aes_gcm_mac(uint8_t *key, uint8_t *data, uint8_t *output);

/* Calculate auth response with AES-CCM */
void desfire_ev2_calc_auth_response_ccm(uint8_t *key_encrypted, uint8_t *mac,
                                        uint8_t sector, uint8_t record);

/* Calculate MAC using AES-CCM */
void compute_aes_ccm_mac(uint8_t *key, uint8_t *data, uint8_t *output);

#endif /* MIFARE_DESFIRE_EV2_DATA_H */
