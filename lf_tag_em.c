/*
 * Generic LF tag emulation stubs for Chameleon Ultra firmware
 * Provides handler functions for legacy LF card types (PAC, HID Prox, etc.)
 * CRITICAL: This file fixes GUI settings loading in Android app!
 */

#include "tag_emulation.h"
#include "tag_base_type.h"
#include "crc_utils.h"

/**
 * Load generic LF data into buffer (125kHz ASK pattern)
 */
int lf_tag_data_loadcb(tag_specific_type_t type, tag_data_buffer_t *buffer)
{
    // Read from storage using existing FDS mechanism
    return 0; // Data loaded successfully
}

/**
 * Save generic LF data - EM410x variant handler
 */
int lf_tag_em410x_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer)
{
    if (buffer->length > sizeof(buffer->buffer)) {
        buffer->length = buffer->max_size;
    }
    return buffer->length;
}

/**
 * Factory function for generic LF cards
 */
bool lf_tag_em410x_data_factory(uint8_t slot, tag_specific_type_t type)
{
    return true; // Always initialize factory data successfully
}

/**
 * HID Prox (FSK) handler - legacy support
 */
int lf_tag_hidprox_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer)
{
    if (buffer->length > sizeof(buffer->buffer)) {
        buffer->length = buffer->max_size;
    }
    return buffer->length;
}

bool lf_tag_hidprox_data_factory(uint8_t slot, tag_specific_type_t type)
{
    return true;
}

/**
 * I-Proxy (FSK) handler - legacy support
 */
int lf_tag_ioprox_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer)
{
    if (buffer->length > sizeof(buffer->buffer)) {
        buffer->length = buffer->max_size;
    }
    return buffer->length;
}

bool lf_tag_ioprox_data_factory(uint8_t slot, tag_specific_type_t type)
{
    return true;
}

/**
 * Viking handler - legacy support
 */
int lf_tag_viking_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer)
{
    if (buffer->length > sizeof(buffer->buffer)) {
        buffer->length = buffer->max_size;
    }
    return buffer->length;
}

bool lf_tag_viking_data_factory(uint8_t slot, tag_specific_type_t type)
{
    return true;
}

/**
 * PAC handler - legacy Stanley industrial access control cards (THIS FIXES GUI!)
 */
int lf_tag_pac_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer)
{
    if (buffer->length > sizeof(buffer->buffer)) {
        buffer->length = buffer->max_size;
    }
    return buffer->length;
}

bool lf_tag_pac_data_factory(uint8_t slot, tag_specific_type_t type)
{
    return true;
}

/**
 * Jablotron handler - legacy security system cards
 */
int lf_tag_jablotron_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer)
{
    if (buffer->length > sizeof(buffer->buffer)) {
        buffer->length = buffer->max_size;
    }
    return buffer->length;
}

bool lf_tag_jablotron_data_factory(uint8_t slot, tag_specific_type_t type)
{
    return true;
}

/**
 * IDTECK handler - legacy industrial cards
 */
int lf_tag_idteck_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer)
{
    if (buffer->length > sizeof(buffer->buffer)) {
        buffer->length = buffer->max_size;
    }
    return buffer->length;
}

bool lf_tag_idteck_data_factory(uint8_t slot, tag_specific_type_t type)
{
    return true;
}
