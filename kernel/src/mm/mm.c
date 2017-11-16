#include <mm/mm.h>
#include <video.h>
#include <err/panic.h>
#include <err/error.h>

#ifdef ARCH_X86
#include <mm/paging.h>
#include <mm/alloc.h>
#include <mm/gdt.h>
#include <mm/mem.h>
#endif

static ptr_t mods_begin = 0xFFFFFFFF;
static ptr_t mods_end   = 0x00000000;

static void mm_locate_modules(struct multiboot_header *mboot_head) {
	struct mboot_module *mod = (struct mboot_module *)mboot_head->mod_addr;
	uint32_t modcnt = mboot_head->mod_count;

	

	uint32_t i = 0;
	while(i < modcnt)
	{
	#if defined(ARCH_X86)
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
	#endif
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
void mm_init(struct multiboot_header *mboot_head)
{
	kerror(ERR_BOOTINFO, "Initializing memory management");

	if(!mboot_head)
		kpanic("Multiboot header pointer is NULL!");
	if(!(mboot_head->flags & MBOOT_MEMINFO))
		kpanic("Multiboot header entries mem_* are not available!");

#if defined(ARCH_X86)
	mm_locate_modules(mboot_head);

	kerror(ERR_BOOTINFO, "  -> Paging");
	paging_init(mods_end, mboot_head->mem_upper * 1024); // memory in mem_tag is in KiB

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