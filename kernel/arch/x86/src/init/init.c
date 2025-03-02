#include <arch/init/init.h>

#include <arch/dev/keyb/input.h>
#include <arch/dev/vga/print.h>
#include <arch/io/serial.h>

#include <arch/acpi/acpi.h>
#include <arch/intr/apic/apic.h>
#include <arch/intr/idt.h>
#include <arch/intr/pit.h>
#include <arch/proc/stack_trace.h>

#include <arch/mm/paging.h>
#include <arch/mm/gdt.h>
#include <arch/mm/mem.h>

#include <kern/cmdline.h>
#include <err/error.h>
#include <err/panic.h>

#include <intr/intr.h>

#include <mm/alloc.h>
#include <mm/mm.h>

#include <io/output.h>


static void interrupts_init(void);
static void mm_init(const mboot_t *);

void arch_init(mboot_t *mboot_head) {
    vga_clear();
    disable_interrupts();
    
    kerror(ERR_INFO, "Kernel occupies this memory space: %p - %p", &kern_start, &kern_end);

    kerror(ERR_INFO, "  -> GDT");
    gdt_init();
    
    interrupts_init();

    mm_init(mboot_head);

    cmdline_init();
    cmdline_handle_common();

    acpi_init(mboot_head);
#ifdef CONFIG_X86_APIC
    apic_init();
#endif

    kerror(ERR_INFO, "  -> STI");
    enable_interrupts();

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
    kerror(ERR_INFO, "Configuring Interrupts");
    
    kerror(ERR_INFO, "  -> IDT");
    idt_init();
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
    mm_init_kernel_map();

    kerror(ERR_INFO, "Memory management enabled");
}

void arch_kpanic_hook(void) {
    stack_trace(16, __builtin_frame_address(0), (uint32_t)&stack_trace_here, NULL);
}
