#include <arch/init/init.h>

#include <arch/io/serial.h>
#include <arch/dev/keyb/input.h>
#include <arch/dev/vga/print.h>

#include <arch/intr/idt.h>
#include <arch/intr/pit.h>

#include <arch/mm/paging.h>
#include <arch/mm/alloc.h>
#include <arch/mm/gdt.h>
#include <arch/mm/mem.h>

#include <err/error.h>
#include <err/panic.h>

#include <intr/intr.h>


static void interrupts_init(void);
static void mm_init(struct multiboot_header *mboot_head);

void arch_init(struct multiboot_header *mboot_head) {
    vga_clear();
    disable_interrupts();
	
    serial_init(SERIAL_COM1);
	
	kerror(ERR_BOOTINFO, "Kernel occupies this memory space: %08X - %08X", &kern_start, &kern_end);

	mm_init(mboot_head);

    interrupts_init();

    // Initialize a second time to enable interrupts
    serial_init(SERIAL_COM1);

    keyb_init();
}


extern void exceptions_init(); //!< Initializes basic exception handlers. Found in intr/exceptions.asm

/**
 * \brief Initializes interrupts.
 * Initializes based on the target architecture.
 */
static void interrupts_init(void) {
	kerror(ERR_BOOTINFO, "Enabling Interrupts");
	
    kerror(ERR_BOOTINFO, "  -> GDT");
	gdt_init();
	kerror(ERR_BOOTINFO, "  -> IDT");
	idt_init();
	kerror(ERR_BOOTINFO, "  -> Exceptions");
	exceptions_init();
	kerror(ERR_BOOTINFO, "  -> STI");
	enable_interrupts();

	kerror(ERR_BOOTINFO, "Interrupts enabled");

}

static ptr_t mods_begin = 0xFFFFFFFF;
static ptr_t mods_end   = 0x00000000;

static void mm_locate_modules(struct multiboot_header *mboot_head) {
	struct mboot_module *mod = (struct mboot_module *)mboot_head->mod_addr;
	uint32_t modcnt = mboot_head->mod_count;

	uint32_t i = 0;
	while(i < modcnt) {
		ptr_t mod_start = mod->mod_start;
		ptr_t mod_end   = mod->mod_end;

		kerror(ERR_BOOTINFO, "mm_init_alloc: multiboot module: %08X->%08X", mod_start, mod_end);

		if(mod_start < mods_begin) mods_begin = mod_start;
		if(mod_end   > mods_end)   mods_end   = mod_end;

		uint32_t b = ((mod_start - (uint32_t)firstframe) / 0x1000);
		for(; b < ((mod_end - (uint32_t)firstframe) / 0x1000) + 1; b++)
		{
			set_frame(b, 1); // Make sure that the module is not overwritten
			map_page((b * 0x1000) + firstframe, (b * 0x1000) + firstframe, 3);
		}
		i++;
		mod++;
	}

	kerror(ERR_BOOTINFO, "mm_init_alloc: Address space used by mulitboot modules: %08X->%08X", mods_begin, mods_end);

	if(mods_end == 0) mods_end = FRAMES_START;
}

/**
 * \brief Initializes memory management.
 * Initializes memory management for the target archetecture.
 * @param mboot_tag the multiboot header location
 */
static void mm_init(struct multiboot_header *mboot_head) {
	kerror(ERR_BOOTINFO, "Initializing memory management");

	if(!mboot_head)
		kpanic("Multiboot header pointer is NULL!");
	if(!(mboot_head->flags & MBOOT_MEMINFO))
		kpanic("Multiboot header entries mem_* are not available!");

	mm_locate_modules(mboot_head);

	kerror(ERR_BOOTINFO, "  -> Paging");
	paging_init(mods_end, mboot_head->mem_upper * 1024); // memory in mem_tag is in KiB

	kerror(ERR_BOOTINFO, "Memory management enabled");
}
