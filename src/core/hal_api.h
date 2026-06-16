/**
 * Chameleon Ultra - Hardware Abstraction Layer (HAL) API
 * 
 * Interface to hardware interfaces for all features
 * Status: Framework defined, NOT IMPLEMENTED
 */

#ifndef CHAMELEON_HAL_API_H
#define CHAMELEON_HAL_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Hardware interface types */
typedef enum {
    HAL_INTERFACE_PCI,        /* PCIe device for DMA exploits */
    HAL_INTERFACE_USB,        /* USB controller for fingerprinting */
    HAL_INTERFACE_NETWORK,    /* Network stack for relay attacks */
    HAL_INTERFACE_SERIAL,     /* UART console interception */
    HAL_INTERFACE_POWER,      /* Power analysis interface */
    HAL_INTERFACE_THERMAL     /* Temperature sensing */
} hal_interface_type_t;

/* Hardware device descriptor */
typedef struct {
    uint16_t vendor_id;       /* Vendor ID for PCI/USB */
    uint16_t device_id;       /* Device ID */
    char class_code[4];       /* PCIe/USB class code */
    int bus_slot;             /* Bus/slot/function number */
} hal_device_descriptor_t;

/* Hardware capability flags */
typedef enum {
    HAL_CAP_DMA_ACCESS,       /* DMA buffer access available */
    HAL_CAP_MEMORY_MAPPED_IO, /* Mapped I/O supported */
    HAL_CAP_INTERRUPTS,       /* Interrupt handling */
    HAL_CAP_REGISTERS         /* Device registers accessible */
} hal_capability_flag_t;

/* Initialize HAL subsystem */
int hal_init(void);

/* Cleanup HAL resources */
void hal_cleanup(void);

/* Query available hardware interfaces */
int hal_query_interfaces(hal_interface_type_t* types, size_t max_types);

/* Get device descriptor for named interface */
hal_device_descriptor_t* hal_get_device_descriptor(
    const char* interface_name);

/* Check if capability is available */
int hal_has_capability(hal_interface_type_t iface, hal_capability_flag_t cap);

/* Write to hardware register */
int hal_write_register(const hal_device_descriptor_t* device,
                       uint16_t offset, const uint8_t* data, size_t len);

/* Read from hardware register */
int hal_read_register(const hal_device_descriptor_t* device,
                      uint16_t offset, uint8_t* buffer, size_t len);

/* Memory-mapped I/O access */
void* hal_get_mmap_base(const hal_device_descriptor_t* device);
size_t hal_get_mmap_size(const hal_device_descriptor_t* device);

#ifdef __cplusplus
}
#endif

#endif /* CHAMELEON_HAL_API_H */
