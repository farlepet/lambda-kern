#ifndef LAMBDA_DRV_DRIVER_H
#define LAMBDA_DRV_DRIVER_H

#include <types.h>

#include <lambda/version.h>

typedef enum {
    LAMBDA_DRV_TYPE_NONE    = 0x0000U, /** Invalid. */ 
    LAMBDA_DRV_TYPE_GENERIC = 0x0001U, /** Generic, no defined function. */
    LAMBDA_DRV_TYPE_INPUT   = 0x0002U, /** Input device driver */
    LAMBDA_DRV_TYPE_OUTPUT  = 0x0003U, /** Output device driver */
    LAMBDA_DRV_TYPE_STORAGE = 0x0004U, /** Storage device driver */
    LAMBDA_DRV_TYPE_DISPLAY = 0x0005U, /** Display device driver */
    LAMBDA_DRV_TYPE_HID     = 0x0006U, /** Human interface device driver */
} lambda_drv_type_e;

/**
 * Kernel driver metadata.
 */
typedef struct {
    char            *name;         /** Driver name. */
    char            *description;  /** Driver description. Optional */
    char            *license;      /** Driver license. NONE if no license. */
    char           **authors;      /** Driver authors. Optional */
    char           **requirements; /** List of drivers that must be loaded prior to this one. Optional. */
} __packed lambda_drv_metadata_t;

typedef int (*lambda_drv_func_start_t)();   /** Driver start function. */
typedef int (*lambda_drv_func_stop_t)(int); /** Driver stop function. */

/**
 * Kernel driver functions.
 * 
 * NOTE: All exposed kernel driver functions MUST exit. If continuing execution
 * is required, it must spawn a process.
 */
typedef struct {
    lambda_drv_func_start_t start; /** Start function. */
    lambda_drv_func_stop_t  stop;  /** Stop function. */
} __packed lambda_drv_functions_t;

/**
 * Kernel driver header structure.
 */
typedef struct {
#define LAMBDA_DRV_HEAD_MAGIC   (0xBAC0BEEFUL)
    uint32_t               head_magic;   /** Header magic number, see LAMBDA_DRV_HEAD_MAGIC. */
#define LAMBDA_DRV_HEAD_VERSION (0) /** Current header version. */
    uint16_t               head_version; /** Header version. */
    uint16_t               drv_type;     /** Driver type, see drv_type_e. Use GENERIC if no type applies. */
    lambda_version_t       kernel;       /** Kernel version the driver was compiled for. */

    lambda_drv_metadata_t  metadata;     /** Driver metadata. */
    lambda_drv_functions_t functions;   /** Driver functions. */
} __packed lambda_drv_head_t;

#endif