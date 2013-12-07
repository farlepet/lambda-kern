#include <types.h>
#include <mm/gdt.h>

u32 TSS[26] = { 0, }; //!< The Task State Segment

u64 GDT[6] =          //!< The Global Descriptor Table
{
	GDT_NULL,
	GDT_CODE_KERN,
	GDT_DATA_KERN,
	GDT_CODE_USER,
	GDT_DATA_USER
};

extern void load_gdt(u64 *, u32); //!< Sets the GDT pointer with `lidt`
extern void seg_reload();         //!< Reloads the segment registers
extern void load_tss();           //!< Sets the TSS descriptor

/**
 * \brief Initialize the GDT
 * Initializes the GDT then the TSS.
 */
void gdt_init()
{
	GDT[5] = GDT_ENTRY((u32)&TSS, sizeof(TSS), 0x0089); // Or should it be 0x00E9?
	TSS[2] = 0x10;

	load_gdt(GDT, sizeof(GDT));
	seg_reload();
	load_tss();
}