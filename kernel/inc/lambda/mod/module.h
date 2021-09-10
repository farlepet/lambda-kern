#ifndef LAMBDA_MOD_MODULE_H
#define LAMBDA_MOD_MODULE_H

#include <types.h>

#include <lambda/version.h>

#define LAMBDA_MODULE_SECTION_NAME ".lambda_module_data"

/* @todo It might be useful to add a driver-specific logging function */

/**
 * Kernel module metadata.
 */
typedef struct {
    char            *ident;        /** Module identifier/name. Must not contain spaces. */
    char            *name;         /** Module human-readable name. */
    char            *description;  /** Module description. Optional */
    char            *license;      /** Module license. NONE if no license. */
    char           **authors;      /** Module authors. Optional */
    char           **requirements; /** List of drivers that must be loaded prior to this one. Optional. */
} __packed lambda_mod_metadata_t;

typedef enum {
    LAMBDA_MODFUNC_START = 0,
    LAMBDA_MODFUNC_STOP  = 1,
} lambda_mod_func_e;

typedef int (*lambda_mod_func_t)(uint32_t, void *); /** Module management function. */

/**
 * Kernel module header structure.
 */
typedef struct {
#define LAMBDA_MODULE_HEAD_MAGIC   (0xBAC0BEEFUL)
    uint32_t               head_magic;   /** Header magic number, see LAMBDA_DRV_HEAD_MAGIC. */
#define LAMBDA_MODULE_HEAD_VERSION (0) /** Current header version. */
    uint16_t               head_version; /** Header version. */
    uint16_t               _reserved;    /** Reserved */
    lambda_version_t       kernel;       /** Kernel version the driver was compiled for. */
    lambda_mod_func_t      function;     /** Module management function. */

    lambda_mod_metadata_t  metadata;     /** Module metadata. */
} __packed lambda_mod_head_t;

#define MODULE_HEADER                                         \
    static lambda_mod_head_t                                  \
    __attribute__((__used__,                                  \
                  __section__((LAMBDA_MODULE_SECTION_NAME)))) \
    __mod_head

#endif