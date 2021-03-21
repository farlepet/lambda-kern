#include <mm/mm.h>
#include <video.h>
#include <err/panic.h>
#include <err/error.h>
#include <proc/proc.h>
#include <mm/alloc.h>

#if defined(ARCH_X86)
#  include <arch/mm/paging.h>
#endif

int mm_check_addr(void *addr) {
#if defined(ARCH_X86)
	uint32_t page = get_page_entry(addr);

	if(!(page & 0x01)) return 0; // Page not present

	return (page & 0x02) ? (2) : (1); // Check R/W bit
#else
	/* Unimplemented for this architecture, assume address is good. */
	(void)addr;
	return 2;
#endif
}

void *mm_translate_proc_addr(struct kproc *proc, const void *addr) {
	#if defined(ARCH_X86)
		return (void *)((pgdir_get_page_entry((uint32_t *)proc->threads[0].arch.cr3, addr) & 0xFFFFF000 ) | ((uint32_t)addr & 0xFFF));
	#else
		// Unimplemented for this architecture
		(void)proc;
		(void)addr;
		return NULL;
	#endif
}

int mm_proc_mmap_add(struct kproc *proc, uintptr_t phys, uintptr_t virt, size_t sz) {
	kdebug(DEBUGSRC_MM, "mm_proc_mmap_add(%s, %08X, %08X, %d)", proc->name, phys, virt, sz);
	
	struct kproc_mem_map_ent *mmap_ent = (struct kproc_mem_map_ent *)kmalloc(sizeof(struct kproc_mem_map_ent));
	if(!mmap_ent) {
		return -1;
	}

	mmap_ent->virt_address = virt;
	mmap_ent->phys_address = phys;
	mmap_ent->length       = sz;
	mmap_ent->next         = NULL;

	struct kproc_mem_map_ent **last = &proc->mmap;
	while(*last) {
		last = &(*last)->next;
	}
	*last = mmap_ent;

	return 0;
}

int mm_proc_mmap_remove_virt(struct kproc *proc, uintptr_t virt) {
	kdebug(DEBUGSRC_MM, "mm_proc_mmap_remove_virt(%s, %08X)", proc->name, virt);
	
	struct kproc_mem_map_ent *mmap_ent  = proc->mmap;
	struct kproc_mem_map_ent *mmap_prev = NULL;
	
	while(mmap_ent) {
		if(mmap_ent->virt_address == virt) {
			if(mmap_prev) {
				mmap_prev->next = mmap_ent->next;
			} else {
				proc->mmap = mmap_ent->next;
			}
			kfree(mmap_ent);
			return 0;
		}
		mmap_prev = mmap_ent;
		mmap_ent  = mmap_ent->next;
	}

	return -1;
}

int mm_proc_mmap_remove_phys(struct kproc *proc, uintptr_t phys) {
	kdebug(DEBUGSRC_MM, "mm_proc_mmap_remove_phys(%s, %08X)", proc->name, phys);
	
	struct kproc_mem_map_ent *mmap_ent  = proc->mmap;
	struct kproc_mem_map_ent *mmap_prev = NULL;
	
	while(mmap_ent) {
		if(mmap_ent->phys_address == phys) {
			if(mmap_prev) {
				mmap_prev->next = mmap_ent->next;
			} else {
				proc->mmap = mmap_ent->next;
			}
			kfree(mmap_ent);
			return 0;
		}
		mmap_prev = mmap_ent;
		mmap_ent  = mmap_ent->next;
	}

	return -1;
}
