/** \file gdt.h
 *  \brief Contains GDT-related definitions.
 *
 *  Contains a function to initialize the GDT, and a defenition to help
 *  make creating GDT entries easier.
 */

#ifndef GDT_H
#define GDT_H

/**
 * \brief Helps create a GDT entry
 * Makes the creation of a GDT entry much cleaner.
 * @param base start of memory that this entry will specify
 * @param limit end of memory that this entry will specify
 * @param flags information about the segment
 */
/*#define GDT_ENTRY(base, limit, flags) \
	((((base)       & 0xff000000ULL) << 32) |  \
	(((flags)       & 0x0000ffffULL) << 40) |  \
	(((limit >> 0) & 0x000f0000ULL) << 32) |  \
	(((base)        & 0x00ffffffULL) << 16) |  \
	((limit >> 0)  & 0x0000ffffULL))*/

#define GDT_ENTRY(base, limit, flags) \
	((((base)       & 0xff000000ULL) << 32) |  \
	(((flags)       & 0x0000f0ffULL) << 40) |  \
	(((limit >> 12) & 0x000f0000ULL) << 32) |  \
	(((base)        & 0x00ffffffULL) << 16) |  \
	((limit >> 12)  & 0x0000ffffULL))

#define GDT_NULL      GDT_ENTRY(0, 0, 0)

//#define GDT_CODE_KERN GDT_ENTRY(0x00000000, 0x7FFFFFFF, 0xCF9A)
//#define GDT_DATA_KERN GDT_ENTRY(0x00000000, 0x7FFFFFFF, 0xCF92)
#define GDT_CODE_RING0 GDT_ENTRY(0x00000000, 0xFFFFFFFF, 0xCF9A)
#define GDT_DATA_RING0 GDT_ENTRY(0x00000000, 0xFFFFFFFF, 0xCF92)

#define GDT_CODE_RING1 GDT_ENTRY(0x00000000, 0xFFFFFFFF, 0xCFBA)
#define GDT_DATA_RING1 GDT_ENTRY(0x00000000, 0xFFFFFFFF, 0xCFB2)

#define GDT_CODE_RING2 GDT_ENTRY(0x00000000, 0xFFFFFFFF, 0xCFDA)
#define GDT_DATA_RING2 GDT_ENTRY(0x00000000, 0xFFFFFFFF, 0xCFD2)

//#define GDT_CODE_USER GDT_ENTRY(0x80000000, 0xFFFFFFFF, 0xCFFA)
//#define GDT_DATA_USER GDT_ENTRY(0x80000000, 0xFFFFFFFF, 0xCFF2)
#define GDT_CODE_RING3 GDT_ENTRY(0x00000000, 0xFFFFFFFF, 0xCFFA)
#define GDT_DATA_RING3 GDT_ENTRY(0x00000000, 0xFFFFFFFF, 0xCFF2)


/**
 * \brief Initialize the GDT
 * Initializes the GDT then the TSS.
 */
void gdt_init(void);

#endif