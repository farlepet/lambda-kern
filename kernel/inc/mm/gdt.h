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
#define GDT_ENTRY(base, limit, flags) \
	((((base)  & 0xff000000ULL) << 32) |  \
	(((flags)  & 0x0000ffffULL) << 40) |  \
	(((limit)  & 0x000f0000ULL) << 32) |  \
	(((base)   & 0x00ffffffULL) << 16) |  \
	((limit)   & 0x0000ffffULL))

#define GDT_NULL      GDT_ENTRY(0, 0, 0)
#define GDT_CODE_KERN GDT_ENTRY(0, 0xFFFFFFFF, 0xCF9A)
#define GDT_DATA_KERN GDT_ENTRY(0, 0xFFFFFFFF, 0xCF92)
#define GDT_CODE_USER GDT_ENTRY(0, 0xFFFFFFFF, 0xCFFA)
#define GDT_DATA_USER GDT_ENTRY(0, 0xFFFFFFFF, 0xCFF2)

/**
 * \brief Initialize the GDT
 * Initializes the GDT then the TSS.
 */
void gdt_init();

#endif