#ifndef LAMBDA_VERSION_H
#define LAMBDA_VERSION_H

#include <types.h>

/*
 * Kernel versioning:
 * 
 * Kernel versions are incremented according to the degree to which changes
 * effect the whole, and not arbitrary milestones:
 * 
 *   Patch: Minor changes have been made. Most if not all software (e.g.
 *          drivers) relying on the kernel's internal functionality should
 *          function as before. All user software only relying on syscalls
 *          should function as before, unless relying on undefined or buggy
 *          behaviour.
 * 
 *   Minor: Moderate-to-significant changes have been made. Software relying
 *          on the kernel's internal functionality is not guaranteed to operate
 *          as before. Changes to syscall interface are possible, and some
 *          software/libraries may need to be recompiled against the new kernel.
 * 
 *   Major: Significant changes have been made to the kernel. No software is
 *          guaranteed to remain functional.
 * 
 * When a higher version position is increased, all lower version positions are
 * reset to zero.
 * 
 * When a lower version position reaches 255, the next version will result in the
 * higher version incrementing regardless of the degree of the changes.
 */

#define LAMBDA_VERSION_MAJOR 0
#define LAMBDA_VERSION_MINOR 1
#define LAMBDA_VERSION_PATCH 0

#define LAMBDA_VERSION_STR STR(LAMBDA_VERSION_MAJOR) "." STR(LAMBDA_VERSION_MINOR) "." STR(LAMBDA_VERSION_PATCH)
#define LAMBDA_VERSION_STR_FULL LAMBDA_VERSION_STR "-" KERNEL_GIT

typedef struct {
    uint8_t major; /** Kernel major version. */
    uint8_t minor; /** Kernel minor version. */
    uint8_t patch; /** Kernel patch revision. */
    uint8_t __reserved;
} lambda_version_t;

#define LAMBDA_VERSION { \
    .major = LAMBDA_VERSION_MAJOR, \
    .minor = LAMBDA_VERSION_MINOR, \
    .patch = LAMBDA_VERSION_PATCH  \
}

#endif
