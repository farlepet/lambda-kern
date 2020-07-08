#include <mm/mm.h>
#include <video.h>
#include <err/panic.h>
#include <err/error.h>
#include <proc/proc.h>

#ifdef ARCH_X86
#  include <mm/paging.h>
#endif

int mm_check_addr(void *addr) {
#if defined(ARCH_X86)
	uint32_t page = get_page_entry(addr);

	if(!(page & 0x01)) return 0; // Page not present

	return (page & 0x02) ? (2) : (1); // Check R/W bit
#else
	// Unimplemented for this architecture
	return 0;
#endif
}

void *mm_translate_proc_addr(struct kproc *proc, const void *addr) {
	#if defined(ARCH_X86)
		return (void *)((pgdir_get_page_entry((uint32_t *)proc->cr3, addr) & 0xFFFFF000 ) | ((uint32_t)addr & 0xFFF));
	#else
		// Unimplemented for this architecture
		return NULL;
	#endif
}