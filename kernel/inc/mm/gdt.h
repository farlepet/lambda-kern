#ifndef GDT_H
#define GDT_H

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

// Initialize the GDT and TSS
void gdt_init();

#endif