#include <mm/mm.h>
#include <multiboot.h>
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
	
#ifdef ARCH_X86
	gdt_init();
	paging_init(mem_tag->mem_upper * 1024); // memory in mem_tag in in KiB
#endif
}

/**
 * \brief Allocate a page.
 * Allocate a page the size of the target architecture's pages and using the
 * target architecture's page allocator. Can later be freed by `free_page`
 * @return the location of the page
 * @see free_frame
 */
void *alloc_page()
{
#ifdef ARCH_X86
	return alloc_frame();
#endif
}

/**
 * \brief Free an allocated page.
 * Free a page allocated by `alloc_page`
 * @param page pointer to the page to free
 * @see alloc_page
 */
void free_page(void *page)
{
#ifdef ARCH_X86
	free_frame(page);
#endif
}