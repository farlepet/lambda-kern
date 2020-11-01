/** \file ioport.h
 *  \brief Contains functions to read from and write to  ports.
 *
 *  Contains functions that allow you to read and write dato to and from
 *  ports in C.
 */

#ifndef IOPORT_H
#define IOPORT_H

#include <types.h>

/**
 * \brief Output a byte to a port.
 * Outputs a byte on a port.
 * @param port the port the byte will be sent to
 * @param value the value of the byte to send
 * @see inb
 */
static inline void outb(uint16_t port, uint8_t value) {
        asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

/**
 * \brief Output a word to a port.
 * Outputs a word on a port.
 * @param port the port the word //!< in intr.asm word to send
 * @see inw
 */
static inline void outw(uint16_t port, uint16_t value) {
        asm volatile("outw %0,%1" : : "a" (value), "dN" (port));
}

/**
 * \brief Output a long word to a port.
 * Outputs a long word on a port.
 * @param port the port the long word will be sent to
 * @param value the value of the long word to send
 * @see inl
 */
static inline void outl(uint16_t port, uint32_t value) {
        asm volatile("outl %0,%1" : : "a" (value), "dN" (port));
}


/**
 * \brief read a byte from a port.
 * Grab a byte from a port.
 * @param port the port the byte will taken from
 * @see outb
 */
static inline uint8_t inb(uint16_t port) {
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
}

/**
 * \brief read a word from a port.
 * Grab a word from a port.
 * @param port the port the word will taken from
 * @see outw
 */
static inline uint16_t inw(uint16_t port) {
        uint16_t ret;
        asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
}

/**
 * \brief read a long word from a port.
 * Grab a long word from a port.
 * @param port the port the long word will taken from
 * @see outl
 */
static inline uint32_t inl(uint16_t port) {
        uint32_t ret;
        asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
}


#endif