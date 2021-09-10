#ifndef CONFIG_H
#define CONFIG_H

#include <lambda/platforms.h>

/* Include the kernel debugger task */
#define DEBUGGER 0

/* Color-code the output the kernel produces */
#define KERNEL_COLORCODE 1

/* Embed initrd into kernel */
#define FEATURE_INITRD_EMBEDDED 0

/* Enable Multiboot 1/2 */
#define FEATURE_MULTIBOOT 2

/*#define CONFIG_DISABLE_SAFETY_CHECKS 1*/

#endif
