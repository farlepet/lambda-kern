#include <arch/init/init.h>

#include <arch/dev/keyb/input.h>
#include <arch/dev/vga/print.h>
#include <arch/io/serial.h>

#include <arch/intr/idt.h>
#include <arch/intr/pit.h>

#include <arch/mm/paging.h>
#include <arch/mm/gdt.h>
#include <arch/mm/mem.h>

#include <err/error.h>
#include <err/panic.h>

#include <intr/intr.h>

#include <mm/alloc.h>

#include <video.h>


static void interrupts_init(void);
static void mm_init(const mboot_t *);

/* TODO: Move this elsewhere, or dynamically allocate */
static hal_io_char_dev_t serial1;

void arch_init(mboot_t *mboot_head) {
    vga_clear();
    disable_interrupts();
	
    serial_init(SERIAL_COM1);
	serial_create_chardev(SERIAL_COM1, &serial1);
	kput_char_dev = &serial1;
	
	kerror(ERR_INFO, "Kernel occupies this memory space: %08X - %08X", &kern_start, &kern_end);

	mm_init(mboot_head);

    interrupts_init();

    // Initialize a second time to enable interrupts
    serial_init(SERIAL_COM1);

    keyb_init();
}


extern void exceptions_init(void); //!< Initializes basic exception handlers. Found in intr/exceptions.asm

/**
 * \brief Initializes interrupts.
 * Initializes based on the target architecture.
 */
static void interrupts_init(void) {
	kerror(ERR_INFO, "Enabling Interrupts");
	
    kerror(ERR_INFO, "  -> GDT");
	gdt_init();
	kerror(ERR_INFO, "  -> IDT");
	idt_init();
	kerror(ERR_INFO, "  -> Exceptions");
	exceptions_init();
	kerror(ERR_INFO, "  -> STI");
	enable_interrupts();

	kerror(ERR_INFO, "Interrupts enabled");

}

static uintptr_t mods_begin = UINTPTR_MAX;
static uintptr_t mods_end   = 0;

/**
 * \brief Initializes memory management.
 * Initializes memory management for the target archetecture.
 * @param head the multiboot header location
 */
static void mm_init(const mboot_t *head) {
	kerror(ERR_INFO, "Initializing memory management");

	if(!head)
		kpanic("Multiboot header pointer is NULL!");

	multiboot_locate_modules(head, &mods_begin, &mods_end);
	size_t upper_mem = multiboot_get_upper_memory(head);
	if(upper_mem == 0) {
		kpanic("Could not get amount of upper mem from multiboot!");
	}
	if(mods_end > 0x100000) {
		upper_mem -= (mods_end - 0x100000);
	}

	kerror(ERR_INFO, "  -> Paging");
	paging_init(mods_end, upper_mem);

	kerror(ERR_INFO, "Memory management enabled");
}
