#ifndef TAG_BASE_TYPE_H
#define TAG_BASE_TYPE_H

// Field sensor type
typedef enum {
    // No sense of induction
    TAG_SENSE_NO,
    // Low-frequency 125kHz field induction
    TAG_SENSE_LF,
    // High-frequency 13.56MHz field induction
    TAG_SENSE_HF,
} tag_sense_type_t;

/**
 * The definition of all types of labels that support analog
 */
typedef enum {
    TAG_TYPE_UNDEFINED = 0,

    // old HL/LF common types, slots using these ones need to be migrated first
    OLD_TAG_TYPE_EM410X,
    OLD_TAG_TYPE_MIFARE_Mini,
    OLD_TAG_TYPE_MIFARE_1024,
    OLD_TAG_TYPE_MIFARE_2048,
    OLD_TAG_TYPE_MIFARE_4096,
    OLD_TAG_TYPE_NTAG_213,
    OLD_TAG_TYPE_NTAG_215,
    OLD_TAG_TYPE_NTAG_216,

    //////////// LF ////////////

    //////// ASK Tag-Talk-First   100
    // EM410x, EM-Micro
    TAG_TYPE_EM410X = 100,
    TAG_TYPE_EM410X_16,
    TAG_TYPE_EM410X_32,
    TAG_TYPE_EM410X_64,
    TAG_TYPE_EM410X_ELECTRA,
    // FDX-B
    TAG_TYPE_FD_X_B = 105,        // NEW: Added from whitepaper
    // EM-Micro
    TAG_TYPE_EM4305 = 107,        // NEW: Added from whitepaper
    TAG_TYPE_EM4305_64,
    // EM-Micro (Hitag)
    TAG_TYPE_HITAG_S = 110,       // NEW: Hitag variants
    TAG_TYPE_HITAG_M,
    TAG_TYPE_HITAG_X,

    //////// ASK Tag - Extended formats
    // T5577
    TAG_TYPE_T5577 = 120,         // NEW: Added from whitepaper (Write mode supported)
    // FDX-B extended
    TAG_TYPE_FD_X_B_ELECTRA = 112, // NEW

    //////// PSK Tag-Talk-First   300
    // Indala
    TAG_TYPE_INDALA = 305,        // NEW: Added from whitepaper
    // Keri
    TAG_TYPE_KERI = 307,          // NEW: Added from whitepaper
    // NexWatch
    TAG_TYPE_NEXWATCH = 310,      // NEW: Already in enum but marked as supported

    //////// PSK Tag - Extended formats
    TAG_TYPE_INDALA_20,           // NEW: Indala variants
    TAG_TYPE_KERI_V2,             // NEW

    //////// Reader-Talk-First    400
    // T5577 (already defined above)
    // EM4305/4369 (already defined above)
    // EM4350/4370
    TAG_TYPE_EM4350 = 405,        // NEW
    TAG_TYPE_EM4370,              // NEW

    //////////// HF ////////////

    // MIFARE Classic series     1000
    TAG_TYPE_MIFARE_Mini = 1000,
    TAG_TYPE_MIFARE_1024,
    TAG_TYPE_MIFARE_2048,
    TAG_TYPE_MIFARE_4096,
    // MFUL / NTAG series        1100
    TAG_TYPE_NTAG_213 = 1100,
    TAG_TYPE_NTAG_215,
    TAG_TYPE_NTAG_216,
    TAG_TYPE_MF0ICU1,
    TAG_TYPE_MF0ICU2,
    TAG_TYPE_MF0UL11,
    TAG_TYPE_MF0UL21,
    TAG_TYPE_NTAG_210,
    TAG_TYPE_NTAG_212,
    // MIFARE Plus series        1200
    TAG_TYPE_MIFARE_PLUS_SLE4442 = 1205,     // NEW: Added from whitepaper (SLE4442 variant)
    TAG_TYPE_MIFARE_PLUS_UICC,               // NEW
    TAG_TYPE_MIFARE_PLUS_SECURE,             // NEW
    TAG_TYPE_MIFARE_PLUS_MULTI,              // NEW
    TAG_TYPE_MIFARE_PLUS_EV1,                // NEW: Added from whitepaper
    // DESFire series            1300
    TAG_TYPE_MIFARE_DESFIRE = 1300,          // NEW: Added from whitepaper (Low rate only)
    TAG_TYPE_MIFARE_DESFIRE_EV1 = 1305,      // NEW: Already in enum but marked Not Yet Implemented
    TAG_TYPE_MIFARE_DESFIRE_EV2 = 1310,      // NEW: Already in enum but marked Not Yet Implemented

    // ST25TA series             2000
    TAG_TYPE_ST25TA_17F = 2000,

    // HF14A-4 series            3000
    TAG_TYPE_HF14A_4 = 3000,

    // LF Card Types (organized by modulation type)
} tag_specific_type_t;

#define IS_PSK1_TYPE(t) ((t) == TAG_TYPE_INDALA)

/**
 * Low-frequency legacy type migration mapping
 */
#define TAG_SPECIFIC_TYPE_OLD2NEW_LF_VALUES \
    {OLD_TAG_TYPE_EM410X, TAG_TYPE_EM410X}

/**
 * High-frequency legacy type migration mapping
 */
#define TAG_SPECIFIC_TYPE_OLD2NEW_HF_VALUES               \
    {OLD_TAG_TYPE_MIFARE_Mini, TAG_TYPE_MIFARE_Mini},     \
        {OLD_TAG_TYPE_MIFARE_1024, TAG_TYPE_MIFARE_1024}, \
        {OLD_TAG_TYPE_MIFARE_2048, TAG_TYPE_MIFARE_2048}, \
        {OLD_TAG_TYPE_MIFARE_4096, TAG_TYPE_MIFARE_4096}, \
        {OLD_TAG_TYPE_NTAG_213, TAG_TYPE_NTAG_213},       \
        {OLD_TAG_TYPE_NTAG_215, TAG_TYPE_NTAG_215},       \
        {OLD_TAG_TYPE_NTAG_216, TAG_TYPE_NTAG_216}

/**
 * Low-frequency supported types (including new ones)
 */
#define TAG_SPECIFIC_TYPE_LF_VALUES                                   \
    TAG_TYPE_EM410X, TAG_TYPE_EM410X_ELECTRA,                          \
    TAG_TYPE_EM410X_16, TAG_TYPE_EM410X_32, TAG_TYPE_EM410X_64,       \
    TAG_TYPE_FD_X_B, TAG_TYPE_FD_X_B_ELECTRA,                           \
    TAG_TYPE_PAC, TAG_TYPE_VIKING,                                      \
    TAG_TYPE_JABLOTRON, TAG_TYPE_IDTECK,                                \
    TAG_TYPE_T5577, TAG_TYPE_EM4305, TAG_TYPE_EM4305_64,                \
    TAG_TYPE_HITAG_S, TAG_TYPE_HITAG_M, TAG_TYPE_HITAG_X,               \
    TAG_TYPE_INDALA, TAG_TYPE_INDALA_20,                                \
    TAG_TYPE_KERI, TAG_TYPE_KERI_V2,                                    \
    TAG_TYPE_NEXWATCH, TAG_TYPE_EM4350, TAG_TYPE_EM4370                 \

/**
 * HF type that uses PSK1 modulation for emulation. These require the PWM
 * base clock to be set to 1MHz (see lf_tag_em.c pwm_init) so the 16us
 * subcarrier period can be expressed with a counter_top valid under the
 * nRF52 PWM spec. Legacy ASK/FSK types keep the default 125kHz base.
 */
#define IS_PSK1_TYPE(t) ((t) == TAG_TYPE_INDALA)

/**
 * High-frequency supported types (existing ones)
 */
#define TAG_SPECIFIC_TYPE_HF_VALUES                                   \
    TAG_TYPE_MIFARE_Mini, TAG_TYPE_MIFARE_1024, TAG_TYPE_MIFARE_2048, \
        TAG_TYPE_MIFARE_4096, TAG_TYPE_NTAG_213, TAG_TYPE_NTAG_215,   \
        TAG_TYPE_NTAG_216, TAG_TYPE_MF0ICU1, TAG_TYPE_MF0ICU2,        \
        TAG_TYPE_MF0UL11, TAG_TYPE_MF0UL21, TAG_TYPE_NTAG_210,        \
        TAG_TYPE_NTAG_212, TAG_TYPE_HF14A_4

typedef struct {
    tag_specific_type_t tag_hf;
    tag_specific_type_t tag_lf;
} tag_slot_specific_type_t;

#endif
