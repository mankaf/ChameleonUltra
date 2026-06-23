/*
 * PAC card data handler for Chameleon Ultra firmware
 * Implements lf_tag_pac_data_loadcb/savecb/factory functions
 */

#include "tag_emulation.h"
#include "tag_base_type.h"
#include "crc_utils.h"

/**
 * Load PAC card data into buffer (generic LF 125kHz pattern)
 */
int lf_tag_pac_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer)
{
    static uint8_t pac_card_data[64] = {0};
    static uint16_t pac_data_len = 0;
    
    if (buffer->length > sizeof(pac_card_data)) {
        buffer->length = sizeof(pac_card_data);
    }
    
    // For now, use generic LF handler for PAC (fallback to EM410X pattern)
    // Original PAC cards were legacy and not fully implemented
    return lf_tag_em410x_data_savecb(type, buffer);
}

/**
 * Factory function for PAC cards
 */
bool lf_tag_pac_data_factory(uint8_t slot, tag_specific_type_t type)
{
    return lf_tag_em410x_data_factory(slot, type);
}
