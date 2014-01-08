#include <mm/mm.h>
#include <video.h>
#include <err/panic.h>

#ifdef ARCH_X86
#include <mm/paging.h>
#include <mm/gdt.h>
#endif

/**
 * \brief Initializes memory management.
 * Initializes memory management for the target archetecture.
 * @param mboot_tag the multiboot header location
 */
void mm_init(struct multiboot_header_tag *mboot_tag)
{
	struct multiboot_basic_memory_tag *mem_tag = (struct multiboot_basic_memory_tag *)find_multiboot_table(mboot_tag, 4);

	if(!mem_tag)
		kpanic("No memory information tag found in multiboot tags!");

#if defined(ARCH_X86)
	gdt_init();
	paging_init(mem_tag->mem_upper * 1024); // memory in mem_tag is in KiB
#endif
}
