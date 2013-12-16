#include <mm/mm.h>
#include <multiboot.h>
#include <video.h>

#ifdef ARCH_X86
#include <kernel/arch/x86/mm/paging.h>
#include <kernel/arch/x86/mm/gdt.h>
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
	{
		kprint("No memory information tag found!\n");
		for(;;);
	}
	
#ifdef ARCH_X86
	gdt_init();
	paging_init(mem_tag->mem_upper * 1024); // memory in mem_tag in in KiB
#endif
	
}