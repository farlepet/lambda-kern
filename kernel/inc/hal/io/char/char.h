#ifndef HAL_IO_CHAR_CHAR_H
#define HAL_IO_CHAR_CHAR_H

#include <types.h>

typedef struct {
    void (*putc)(void *data, int c); //!< Send a character
    int  (*getc)(void *data);        //!< Receive a character
    int  (*chavail)(void *data);     //!< Check if character is available

    void *data;

#define HAL_IO_CHARDEV_CAP_INPUT  (1UL << 0)
#define HAL_IO_CHARDEV_CAP_OUTPUT (1UL << 1)
    uint32_t cap; //!< Capabilities
} hal_io_char_dev_t;

/**
 * Write character to character device.
 */
static inline void hal_io_char_dev_putc(hal_io_char_dev_t *dev, int c) {
    if(!dev) return;
    if(!(dev->cap & HAL_IO_CHARDEV_CAP_OUTPUT) || !dev->putc) return;

    return dev->putc(dev->data, c);
}

/**
 * Read character from character device.
 */
static inline int hal_io_char_dev_getc(hal_io_char_dev_t *dev) {
    if(!dev) return -1;
    if(!(dev->cap & HAL_IO_CHARDEV_CAP_INPUT) || !dev->getc) return -1;

    return dev->getc(dev->data);
}

/**
 * Check if character is available from character device.
 */
static inline int hal_io_char_dev_chavail(hal_io_char_dev_t *dev) {
    if(!dev) return -1;
    if(!(dev->cap & HAL_IO_CHARDEV_CAP_INPUT) || !dev->getc) return -1;

    return dev->chavail(dev->data);
}

#endif
