/**
 * Chameleon Ultra - Relay Network Stack Extension API
 * 
 * Interface for network stack hooks and extension points
 * Status: NOT IMPLEMENTED - Stub header created
 */

#ifndef CHAMELEON_NETWORK_STACK_API_H
#define CHAMELEON_NETWORK_STACK_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Network hook type enumeration for relay attack extension points
 */
typedef enum {
    HOOK_PRE_SEND,         /* Before packet send - intercept opportunity */
    HOOK_POST_RECV,        /* After packet receive - process intercepted data */
    HOOK_IP_INPUT,         /* IP layer input hook */
    HOOK_UDP_OUTPUT        /* UDP output interception point */
} network_hook_type_t;

/**
 * Network hook callback structure
 */
typedef struct {
    network_hook_type_t hook_type;      /* Hook type identifier */
    void* target_device;                /* Target device handle */
    uint8_t* buffer;                    /* Packet buffer (if applicable) */
    size_t buffer_len;                  /* Buffer length */
    void* user_data;                    /* Application-specific data */
} network_hook_context_t;

/**
 * Register network hook callback
 * 
 * @param hook_type Type of hook to register
 * @param callback Callback function pointer (NOT IMPLEMENTED)
 * @param user_data User data for callback
 * @return 0 on success, negative error code on failure
 */
int network_register_hook(network_hook_type_t hook_type,
                          int (*callback)(const network_hook_context_t* ctx),
                          void* user_data);

/**
 * Unregister network hook callback
 * 
 * @param hook_type Type of hook to unregister
 * @return 0 on success
 */
int network_unregister_hook(network_hook_type_t hook_type);

/**
 * Query available network extension points
 * 
 * @param[out] hooks Array of available hook types
 * @param max_hooks Maximum number of entries to populate
 * @return Number of available hooks
 */
size_t network_list_hooks(void** hooks, size_t max_hooks);

/**
 * Check if hook is currently registered/active
 * 
 * @param hook_type Hook type to check
 * @return 1 if active, 0 otherwise
 */
int network_is_hook_active(network_hook_type_t hook_type);

/**
 * Get target device handle for current interception session
 */
void* network_get_target_handle(void);

/**
 * Initialize network stack extension subsystem
 */
int network_extension_init(void);

/**
 * Cleanup network stack extension resources
 */
void network_extension_cleanup(void);

/**
 * Process intercepted packet data (hook callback entry point stub)
 * 
 * @param ctx Hook context structure
 * @return 0 to continue, non-zero to drop/skip
 */
int network_process_intercept(const network_hook_context_t* ctx);

#ifdef __cplusplus
}
#endif

#endif /* CHAMELEON_NETWORK_STACK_API_H */
