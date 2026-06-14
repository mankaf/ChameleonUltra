#ifndef MISSING_FEATURES_COMPAT_H
#define MISSING_FEATURES_COMPAT_H

#include <stdint.h>

#include "app_util.h"
#include "bsp_delay.h"

#ifndef min
#define min(a, b) MIN(a, b)
#endif

static inline uint32_t missing_features_rand(void) {
    static uint32_t state = 0x1234abcdUL;
    state = (state * 1103515245UL) + 12345UL;
    return state;
}

static inline uint8_t bit_count_trailing_zeros(uint8_t value) {
    uint8_t count = 0;
    while (count < 8 && ((value & (1U << count)) == 0)) {
        count++;
    }
    return count;
}

#define nrf_rand() missing_features_rand()
#define nrf_hw_timer_delay_us(us) bsp_delay_us(us)
#define nrf_hw_timer_delay_ms(ms) bsp_delay_us((ms) * 1000UL)
#define nrf_hw_timer_get_us() (0UL)
#define nrf_hw_timer_get_ms() (0UL)

static inline void enable_ask_carrier(uint32_t freq_hz, uint32_t pulse_width_us) {
    UNUSED_PARAMETER(freq_hz);
    UNUSED_PARAMETER(pulse_width_us);
}

static inline void disable_ask_carrier(void) {
}

static inline void enable_psk_carrier(uint32_t base_clock_hz, uint32_t bit_duration_us) {
    UNUSED_PARAMETER(base_clock_hz);
    UNUSED_PARAMETER(bit_duration_us);
}

static inline void disable_psk_carrier(void) {
}

static inline void enable_fsk_carrier(uint32_t freq_on_hz, uint32_t freq_off_hz, uint32_t bit_duration_us) {
    UNUSED_PARAMETER(freq_on_hz);
    UNUSED_PARAMETER(freq_off_hz);
    UNUSED_PARAMETER(bit_duration_us);
}

static inline void disable_fsk_carrier(void) {
}

void transmit_ask_carrier(const uint8_t *data, uint16_t byte_len, uint32_t freq_hz);
void transmit_psk_carrier(const uint8_t *data, uint16_t byte_len, uint32_t base_clock_hz);
void transmit_fsk_carrier(const uint8_t *data, uint16_t byte_len, uint32_t freq_on_hz, uint32_t freq_off_hz);

#endif
