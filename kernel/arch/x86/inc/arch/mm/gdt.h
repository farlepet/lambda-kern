/** \file gdt.h
 *  \brief Contains GDT-related definitions.
 *
 *  Contains a function to initialize the GDT, and a defenition to help
 *  make creating GDT entries easier.
 */

#ifndef GDT_H
#define GDT_H

#include <types.h>

/**
 * \brief Helps create a GDT entry
 * Makes the creation of a GDT entry much cleaner.
 * @param base start of memory that this entry will specify
 * @param limit end of memory that this entry will specify
 * @param flags information about the segment
 */
#define GDT_ENTRY(base, limit, flags) \
    ((((base) & 0xff000000ULL) << 32) |  \
    (((flags) & 0x0000f0ffULL) << 40) |  \
    (((limit) & 0x000f0000ULL) << 32) |  \
    (((base)  & 0x00ffffffULL) << 16) |  \
    ((limit)  & 0x0000ffffULL))

/*#define GDT_ENTRY(base, limit, flags) \
    ((((base)         & 0xff000000ULL) << 32) |  \
    (((flags)         & 0x0000f0ffULL) << 40) |  \
    ((((limit) >> 12) & 0x000f0000ULL) << 32) |  \
    (((base)          & 0x00ffffffULL) << 16) |  \
    (((limit) >> 12)  & 0x0000ffffULL))*/

#define GDT_NULL      GDT_ENTRY(0, 0, 0)

//#define GDT_CODE_KERN GDT_ENTRY(0x00000000, 0x7FFFFFFF, 0xCF9A)
//#define GDT_DATA_KERN GDT_ENTRY(0x00000000, 0x7FFFFFFF, 0xCF92)
#define GDT_CODE_RING0 GDT_ENTRY(0x00000000, 0xFFFFF, 0xCF9A)
#define GDT_DATA_RING0 GDT_ENTRY(0x00000000, 0xFFFFF, 0xCF92)

#define GDT_CODE_RING1 GDT_ENTRY(0x00000000, 0xFFFFF, 0xCFBA)
#define GDT_DATA_RING1 GDT_ENTRY(0x00000000, 0xFFFFF, 0xCFB2)

#define GDT_CODE_RING2 GDT_ENTRY(0x00000000, 0xFFFFF, 0xCFDA)
#define GDT_DATA_RING2 GDT_ENTRY(0x00000000, 0xFFFFF, 0xCFD2)

//#define GDT_CODE_USER GDT_ENTRY(0x80000000, 0xFFFFFFFF, 0xCFFA)
//#define GDT_DATA_USER GDT_ENTRY(0x80000000, 0xFFFFFFFF, 0xCFF2)
#define GDT_CODE_RING3 GDT_ENTRY(0x00000000, 0xFFFFF, 0xCFFA)
#define GDT_DATA_RING3 GDT_ENTRY(0x00000000, 0xFFFFF, 0xCFF2)

typedef struct {
    uint16_t link;
    uint16_t reserved_00;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved_01;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved_02;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved_03;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es;
    uint16_t reserved_04;
    uint16_t cs;
    uint16_t reserved_05;
    uint16_t ss;
    uint16_t reserved_06;
    uint16_t ds;
    uint16_t reserved_07;
    uint16_t fs;
    uint16_t reserved_08;
    uint16_t gs;
    uint16_t reserved_09;
    uint16_t ldtr;
    uint16_t reserved_10;
    uint16_t reserved_11;
    uint16_t iopb_offset;
} __packed tss_t;

/**
 * \brief Initialize the GDT
 * Initializes the GDT then the TSS.
 */
void gdt_init(void);

void tss_set_kern_stack(uint32_t stack);

#endif