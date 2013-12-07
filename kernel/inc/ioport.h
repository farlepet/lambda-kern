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
static inline void outb(u16 port, u8 value)
{
        asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

/**
 * \brief Output a word to a port.
 * Outputs a word on a port.
 * @param port the port the word will be sent to
 * @param value the value of the word to send
 * @see inw
 */
static inline void outw(u16 port, u16 value)
{
        asm volatile("outw %0,%1" : : "a" (value), "dN" (port));
}

/**
 * \brief Output a long word to a port.
 * Outputs a long word on a port.
 * @param port the port the long word will be sent to
 * @param value the value of the long word to send
 * @see inl
 */
static inline void outl(u16 port, u32 value)
{
        asm volatile("outl %0,%1" : : "a" (value), "dN" (port));
}


/**
 * \brief read a byte from a port.
 * Grab a byte from a port.
 * @param port the port the byte will taken from
 * @see outb
 */
static inline u8 inb(u16 port)
{
        u8 ret;
        asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
}

/**
 * \brief read a word from a port.
 * Grab a word from a port.
 * @param port the port the word will taken from
 * @see outw
 */
static inline u16 inw(u16 port)
{
        u16 ret;
        asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
}

/**
 * \brief read a long word from a port.
 * Grab a long word from a port.
 * @param port the port the long word will taken from
 * @see outl
 */
static inline u32 inl(u16 port)
{
        u32 ret;
        asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
}


#endif