/**
 * Chameleon Ultra - Side-Channel Monitor API
 * 
 * Status: NOT IMPLEMENTED
 */

#ifndef CHAMELEON_SIDECHANNEL_API_H
#define CHAMELEON_SIDECHANNEL_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Side-Channel Attack Type Enumeration
 */
typedef enum {
    SC_POWER,           // Power analysis attack
    SC_EMI,             // Electromagnetic emanation capture
    SC_TIMING,          // Cache/timing measurements
    SC_THERMAL,         // Thermal profiling
    SC_NOISE            // Electrical noise correlation
} sc_attack_type_t;

/**
 * Measurement configuration structure
 */
typedef struct {
    sc_attack_type_t attack_type;
    uint32_t sample_rate_hz;       // Sampling frequency
    size_t buffer_size_bytes;      // Measurement buffer size
    int target_device_id;          // Target device identifier
} sc_measurement_config_t;

/**
 * Measurement result structure
 */
typedef struct {
    uint64_t timestamp_us;         // Measurement timestamp (microseconds)
    size_t samples_count;          // Number of samples collected
    double mean_value;             // Mean measurement value
    double std_deviation;          // Standard deviation
    sc_attack_type_t type;         // Attack type used
} sc_measurement_result_t;

/**
 * Initialize side-channel monitoring subsystem
 * 
 * @param config Configuration for measurements
 * @return 0 on success, negative error code on failure
 */
int sc_monitor_start(sc_measurement_config_t* config);

/**
 * Stop active side-channel monitoring
 * 
 * @return 0 on success, negative error code on failure
 */
int sc_monitor_stop(void);

/**
 * Collect latest measurement results
 * 
 * @param result Buffer to store measurement result (must be valid)
 * @param timeout_ms Maximum time to wait for data
 * @return 0 if data available, -1 if timeout occurred
 */
int sc_measurement_collect(sc_measurement_result_t* result,
                          uint32_t timeout_ms);

/**
 * Get current monitoring status
 * 
 * @return 1 if actively monitoring, 0 otherwise
 */
int sc_is_monitoring(void);

/**
 * Configure oscilloscope connection for power analysis
 * 
 * @param probe_path Path to oscilloscope device
 * @param sample_rate Target sampling rate (Hz)
 * @return 0 on success, negative error code on failure
 */
int sc_oscilloscope_connect(const char* probe_path, uint32_t sample_rate);

/**
 * Configure EMI capture for electromagnetic analysis
 * 
 * @param probe_path Path to near-field probe device
 * @return 0 on success, negative error code on failure
 */
int sc_emitter_probe_connect(const char* probe_path);

/**
 * Enable/disable power analysis mode
 * 
 * @param enable 1 to enable, 0 to disable
 * @return 0 on success
 */
int sc_enable_power_analysis(int enable);

/**
 * Get side-channel statistics
 */
struct sc_stats {
    uint64_t total_measurements;    // Total measurements taken
    uint64_t power_samples;         // Power analysis samples
    uint64_t timing_samples;        // Timing measurements
    uint64_t emi_samples;           // EMI capture samples
};

sc_stats_t* sc_get_stats(void);

#ifdef __cplusplus
}
#endif

#endif /* CHAMELEON_SIDECHANNEL_API_H */
