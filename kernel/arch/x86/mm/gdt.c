#include <types.h>
#include "gdt.h"

static u8 system_stack[0x2000];

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
	
	TSS[2]  = 0x10; // SS0
	TSS[18] = 0x13; // ES
	TSS[19] = 0x0B; // CS
	TSS[20] = 0x13; // SS
	TSS[21] = 0x13; // DS
	TSS[22] = 0x13; // FS
	TSS[23] = 0x13; // GS

	TSS[1] = (u32)system_stack; // ESP0
	TSS[25] = sizeof(TSS); // IOPB

	load_gdt(GDT, sizeof(GDT));
	seg_reload();
	load_tss();
}
