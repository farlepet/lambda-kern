#include <arch/mm/gdt.h>

#include <types.h>

static uint8_t system_stack[0x2000];

static tss_t TSS = { 0, };

static uint64_t GDT[10] =          //!< The Global Descriptor Table
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
extern void seg_reload(void);               //!< Reloads the segment registers
extern void load_tss(void);                 //!< Sets the TSS descriptor

/**
 * \brief Initialize the GDT
 * Initializes the GDT then the TSS.
 */
void gdt_init(void) {
    TSS.ss0 = 0x10;
    TSS.ss1 = 0x21;
    TSS.ss2 = 0x32;

    TSS.esp0 = (uint32_t)system_stack;

    TSS.es = 0x13;
    TSS.cs = 0x0B;
    TSS.ss = 0x13;
    TSS.ds = 0x13;
    TSS.fs = 0x13;
    TSS.gs = 0x13;

    TSS.iopb_offset = sizeof(tss_t);
    
    GDT[9] = GDT_ENTRY((uint32_t)&TSS, sizeof(TSS)-1, 0x40E9);

    load_gdt(GDT, sizeof(GDT));
    seg_reload();
    load_tss();
}

void tss_set_kern_stack(uint32_t stack) {
    TSS.esp0 = stack;
}