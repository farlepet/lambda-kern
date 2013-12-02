#ifndef IOPORT_H
#define IOPORT_H

#include <types.h>

static inline void outb(u16 port, u8 value)
{
        asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

static inline void outw(u16 port, u16 value)
{
        asm volatile("outw %0,%1" : : "a" (value), "dN" (port));
}

static inline void outl(u16 port, u32 value)
{
        asm volatile("outl %0,%1" : : "a" (value), "dN" (port));
}



static inline u8 inb(u16 port)
{
        u8 ret;
        asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
}

static inline u16 inw(u16 port)
{
        u16 ret;
        asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
}

static inline u32 inl(u16 port)
{
        u32 ret;
        asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
}


#endif