#include <mm/mm.h>
#include <video.h>
#include <err/panic.h>
#include <err/error.h>

#ifdef ARCH_X86
#include <mm/paging.h>
#include <mm/gdt.h>
#endif

/**
 * \brief Initializes memory management.
 * Initializes memory management for the target archetecture.
 * @param mboot_tag the multiboot header location
 */
void mm_init(struct multiboot_header *mboot_head)
{
	kerror(ERR_BOOTINFO, "Initializing memory management");

	if(!mboot_head)
		kpanic("Multiboot header pointer is NULL!");
	if(!(mboot_head->flags & MBOOT_MEMINFO))
		kpanic("Multiboot header entries mem_* are not available!");

#if defined(ARCH_X86)
	kerror(ERR_BOOTINFO, "  -> Paging");
	paging_init(mboot_head->mem_upper * 1024); // memory in mem_tag is in KiB
#endif
	
	kerror(ERR_BOOTINFO, "Memory management enabled");
}


int mm_check_addr(void *addr) {
#if defined(ARCH_X86)
	u32 page = get_page_entry(addr);

	if(!(page & 0x01)) return 0; // Page not present

	return (page & 0x02) ? (2) : (1); // Check R/W bit
#else
	// Unimplemented for this architecture
	return 0;
#endif
}