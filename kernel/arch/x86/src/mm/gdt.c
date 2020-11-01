#include <arch/mm/gdt.h>

#include <types.h>

static uint8_t system_stack[0x2000];

uint32_t TSS[26] = { 0, }; //!< The Task State Segment

uint64_t GDT[10] =          //!< The Global Descriptor Table
{
	GDT_NULL,       // 00

	GDT_CODE_RING0, // 08
	GDT_DATA_RING0, // 10

	GDT_CODE_RING1, // 18
	GDT_DATA_RING1, // 20

	GDT_CODE_RING2, // 28
	GDT_DATA_RING2, // 30

	GDT_CODE_RING3, // 38
	GDT_DATA_RING3, // 40

	GDT_NULL        // 48 <- Placeholder for TSS
};

extern void load_gdt(uint64_t *, uint32_t); //!< Sets the GDT pointer with `lidt`
extern void seg_reload();         //!< Reloads the segment registers
extern void load_tss();           //!< Sets the TSS descriptor

/**
 * \brief Initialize the GDT
 * Initializes the GDT then the TSS.
 */
void gdt_init()
{
	GDT[9] = GDT_ENTRY((uint32_t)&TSS, sizeof(TSS)-1, 0x40E9/*0x4089*//*0x0089*/); // Or should it be 0x00E9?
	
	TSS[2]  = 0x10; // SS0
	//TSS[4]  = 0x21; // SS1
	//TSS[6]  = 0x32; // SS2

	TSS[18] = 0x13; // ES
	TSS[19] = 0x0B; // CS
	TSS[20] = 0x13; // SS
	TSS[21] = 0x13; // DS
	TSS[22] = 0x13; // FS
	TSS[23] = 0x13; // GS

	TSS[1] = (uint32_t)system_stack; // ESP0
	//TSS[3] = (uint32_t)system_stack; // ESP1
	//TSS[5] = (uint32_t)system_stack; // ESP2

	TSS[25] = sizeof(TSS); // IOPB

	load_gdt(GDT, sizeof(GDT));
	seg_reload();
	load_tss();
}

void tss_set_kern_stack(uint32_t stack) {
	TSS[1] = stack;
}