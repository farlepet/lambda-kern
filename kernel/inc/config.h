#ifndef CONFIG_H
#define CONFIG_H

/* Kernel configuration file. This file is included in every C file through
 * compiler flags. */

#include <lambda/platforms.h>
#include <lambda/config_defs.h>

/* Include the kernel debugger task */
#define DEBUGGER 0

/* Color-code the output the kernel produces */
#define KERNEL_COLORCODE 1

/* Embed initrd into kernel */
#define FEATURE_INITRD_EMBEDDED 0

/* Enable Multiboot 1/2 */
#define FEATURE_MULTIBOOT 2

/* Disable safety checks.
 * TODO: Integrate with CONFIG_STRICTNESS */
/*#define CONFIG_DISABLE_SAFETY_CHECKS 1*/

/* Level of assertion strictness
 * It is useful to set this high for development testing. */
#define CONFIG_STRICTNESS (LAMBDA_STRICTNESS_ALL)

#endif
