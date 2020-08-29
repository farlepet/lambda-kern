#include <arch/mm/alloc.h>

#include <proc/mtask.h>
#include <proc/exec.h>
#include <err/error.h>
#include <proc/elf.h>
#include <string.h>
#include <video.h>

#if defined(ARCH_X86)
#  include <arch/mm/paging.h>
#  include <arch/proc/user.h>
#endif


ptr_t load_elf(void *file, uint32_t length, uint32_t **pdir) {
	//kerror(ERR_BOOTINFO, "Loading elf from %08X of length %08X", file, length);
	(void)length; // TODO: Error-check with this

	Elf32_Ehdr *head = file;

	if( (head->e_ident[0] != ELF_IDENT0) ||
		(head->e_ident[1] != ELF_IDENT1) ||
		(head->e_ident[2] != ELF_IDENT2) ||
		(head->e_ident[3] != ELF_IDENT3)) {
		kerror(ERR_SMERR, "Tried to load an ELF with incorrect header: %x %c %c %c",
			(head->e_ident[0]),
			(head->e_ident[1]),
			(head->e_ident[2]),
			(head->e_ident[3]));
		return 0;
	}

	if(head->e_ident[4] != HOST_CLASS) {
		kerror(ERR_SMERR, "Tried to load ELF not compatible with current bittiness: %d", head->e_ident[4]);
		return 0;
	}

	if(head->e_type != ET_EXEC) {
		kerror(ERR_SMERR, "Tried to load non-executable ELF with type %d", head->e_type);
		return 0;
	}

	if(head->e_machine != HOST_MACHINE) {
		kerror(ERR_SMERR, "Tried to load ELF not compatible with current architecture: %d", head->e_machine);
		return 0;
	}

	int p = proc_by_pid(current_pid);

	uint32_t *pgdir = clone_kpagedir();
	//uint32_t *pgdir = (uint32_t *)procs[p].cr3;
	
	//uint32_t *phys_pgdir = get_phys_page(pgdir);
	//uint32_t *pgdir = (uint32_t *)kernel_cr3;

	//uint32_t used_addresses[32][2];

	//kerror(ERR_BOOTINFO, "Section Header offset: %08X, size: %08X, #: %d", head->e_shoff, head->e_shentsize, head->e_shnum);

	char     *symStrTab = NULL;
	symbol_t *symbols   = NULL;

	Elf32_Shdr *sections  = (Elf32_Shdr *)((ptr_t)head + head->e_shoff);
	Elf32_Shdr *strTabSec = &sections[head->e_shstrndx];
	const char *strTabStr = (const char *)((ptr_t)head + strTabSec->sh_offset);

	struct kproc_mem_map_ent *mmap_entries;
	struct kproc_mem_map_ent **mmap_next = &mmap_entries;

	for(uint32_t i = 0; i < head->e_shnum; i ++) {
		Elf32_Shdr *shdr = &sections[i];//(Elf32_Shdr *)((ptr_t)head + (head->e_shoff + i));

		kerror(ERR_INFO, "shdr[%X/%X] N:%s T:%s OFF: %08X ADDR:%08X SZ:%08X", i+1, head->e_shnum, &strTabStr[shdr->sh_name], sht_strings[shdr->sh_type], shdr->sh_offset, shdr->sh_addr, shdr->sh_size);

		if(shdr->sh_addr) { // Check if there is a destination address
			void *phys = kmalloc(shdr->sh_size + 0x2000); // + 0x2000 so we can page-align it
			phys = (void *)((ptr_t)phys & ~0xFFF) + 0x1000 + (shdr->sh_addr & 0xFFF);
			//void *phys_phys = get_phys_page(phys);

			//set_pagedir(phys_pgdir);
			if(shdr->sh_type == SHT_NOBITS) memset(phys, 0, shdr->sh_size);
			else memcpy(phys, (void *)((ptr_t)head + shdr->sh_offset), shdr->sh_size);
			//set_pagedir((uint32_t *)kernel_cr3);

			for(ptr_t pg = 0; pg < shdr->sh_size; pg += 0x1000) {
				//kerror(ERR_BOOTINFO, "  -> MAP_PAGE<%08X>[%08X, %08X]", pgdir, (phys + p), (shdr->sh_addr + p));
				pgdir_map_page(pgdir, (phys + pg), (void *)(shdr->sh_addr + pg), 0x07);
				//pgdir_map_page(pgdir, (phys + p), (void *)(shdr->sh_addr + p), 0x03);
				//map_page((phys + p), (void *)(shdr->sh_addr + p), 0x03);
				//kerror(ERR_BOOTINFO, "      -> DONE");
			}

			// Create MMAP entry:
			*mmap_next = (struct kproc_mem_map_ent *)kmalloc(sizeof(struct kproc_mem_map_ent));
			(*mmap_next)->virt_address = shdr->sh_addr;
			(*mmap_next)->phys_address = (uintptr_t)phys;
			(*mmap_next)->length       = shdr->sh_size;
			(*mmap_next)->next         = NULL;
			mmap_next = &((*mmap_next)->next);

		} else if(shdr->sh_type == SHT_SYMTAB) {
			Elf32_Sym *syms = (Elf32_Sym *)((ptr_t)head + shdr->sh_offset);

			uint32_t nSyms = shdr->sh_size / shdr->sh_entsize;
			symStrTab = (char *)    kmalloc(strTabSec->sh_size);
			symbols   = (symbol_t *)kmalloc((nSyms + 1) * sizeof(symbol_t));

			memcpy(symStrTab, strTabStr, strTabSec->sh_size);

			for(uint32_t j = 0; j < nSyms; j++) {
				/*kerror(ERR_BOOTINFO, "  -> [%d]: %s -> %08X:%08X, %04X", j, &strTab[syms[j].st_name],
					syms[j].st_value, syms[j].st_size, syms[j].st_name);*/
				
				symbols[j].name = &symStrTab[syms[j].st_name];
				symbols[j].addr = syms[j].st_value;
				symbols[j].size = syms[j].st_size;
			}

			symbols[nSyms].name = NULL;
			symbols[nSyms].addr = 0xFFFFFFFF;
			symbols[nSyms].size = 0x00000000;
		}
	}

	kerror(ERR_INFO, "Entrypoint: %08X -> %08X", head->e_entry, pgdir_get_page_entry(pgdir, (void *)head->e_entry) & 0xFFFFF000);

	//kerror(ERR_BOOTINFO, "ELF Loaded");

	*pdir = pgdir;
	//return head->e_entry;
	
	// Old way of creating new process:
	int pid = add_user_task_pdir((void *)head->e_entry, "UNNAMED_ELF", 0, PRIO_USERPROG, pgdir);

	p = proc_by_pid(pid);
	procs[p].symbols   = symbols;
	procs[p].symStrTab = symStrTab;
	proc_add_mmap_ents(&procs[p], mmap_entries);
	
	//enter_ring_noargs(procs[p].ring, (void *)head->e_entry);

	return pid;
	//return -1;
}



int exec_elf(void *data, uint32_t length, const char **argv, const char **envp) {
	//kerror(ERR_BOOTINFO, "Executing elf from %08X of length %08X", data, length);
	(void)length; // TODO: Error-check with this

	Elf32_Ehdr *head = data;

	if( (head->e_ident[0] != ELF_IDENT0) ||
		(head->e_ident[1] != ELF_IDENT1) ||
		(head->e_ident[2] != ELF_IDENT2) ||
		(head->e_ident[3] != ELF_IDENT3)) {
		kerror(ERR_SMERR, "Tried to load an ELF with incorrect header: %x %c %c %c",
			(head->e_ident[0]),
			(head->e_ident[1]),
			(head->e_ident[2]),
			(head->e_ident[3]));
		return 0;
	}

	if(head->e_ident[4] != HOST_CLASS) {
		kerror(ERR_SMERR, "Tried to load ELF not compatible with current bittiness: %d", head->e_ident[4]);
		return 0;
	}

	if(head->e_type != ET_EXEC) {
		kerror(ERR_SMERR, "Tried to lSURELYoad non-executable ELF with type %d", head->e_type);
		return 0;
	}

	if(head->e_machine != HOST_MACHINE) {
		kerror(ERR_SMERR, "Tried to load ELF not compatible with current architecture: %d", head->e_machine);
		return 0;
	}

	uint32_t *pgdir = clone_kpagedir();
	//uint32_t *phys_pgdir = get_phys_page(pgdir);
	//uint32_t *pgdir = (uint32_t *)kernel_cr3;
	//uint32_t *pgdir = get_pagedir();

	//uint32_t used_addresses[32][2];

	//kerror(ERR_BOOTINFO, "Section Header offset: %08X, size: %08X, #: %d", head->e_shoff, head->e_shentsize, head->e_shnum);

	char     *symStrTab = NULL;
	symbol_t *symbols   = NULL;

	Elf32_Shdr *sections  = (Elf32_Shdr *)((ptr_t)head + head->e_shoff);
	Elf32_Shdr *strTabSec = &sections[head->e_shstrndx];
	const char *strTabStr = (const char *)((ptr_t)head + strTabSec->sh_offset);

	for(uint32_t i = 0; i < head->e_shnum; i ++) {
		Elf32_Shdr *shdr = &sections[i];//(Elf32_Shdr *)((ptr_t)head + (head->e_shoff + i));

		kerror(ERR_INFO, "shdr[%X/%X] N:%s T:%s OFF: %08X ADDR:%08X SZ:%08X", i+1, head->e_shnum, &strTabStr[shdr->sh_name], sht_strings[shdr->sh_type], shdr->sh_offset, shdr->sh_addr, shdr->sh_size);

		// Check if there is a destination address
		if(shdr->sh_addr) {
			void *phys = kmalloc(shdr->sh_size + 0x2000); // + 0x2000 so we can page-align it
			phys = (void *)((ptr_t)phys & ~0xFFF) + 0x1000 + (shdr->sh_addr & 0xFFF);
			//void *phys_phys = get_phys_page(phys);

			//set_pagedir(phys_pgdir);
			if(shdr->sh_type == SHT_NOBITS) memset(phys, 0, shdr->sh_size);
			else memcpy(phys, (void *)((ptr_t)head + shdr->sh_offset), shdr->sh_size);
			//set_pagedir((uint32_t *)kernel_cr3);

			for(ptr_t pg = 0; pg < shdr->sh_size; pg += 0x1000) {
				//kerror(ERR_BOOTINFO, "  -> MAP_PAGE<%08X>[%08X, %08X]", pgdir, (phys + pg), (shdr->sh_addr + pg));
				pgdir_map_page(pgdir, (phys + pg), (void *)(shdr->sh_addr + pg), 0x07);
				//pgdir_map_page(pgdir, (phys + pg), (void *)(shdr->sh_addr + pg), 0x03);
				//map_page((phys + pg), (void *)(shdr->sh_addr + pg), 0x03);
				//kerror(ERR_BOOTINFO, "      -> DONE");
			}
		} else if(shdr->sh_type == SHT_SYMTAB) {
			Elf32_Sym *syms = (Elf32_Sym *)((ptr_t)head + shdr->sh_offset);

			uint32_t nSyms = shdr->sh_size / shdr->sh_entsize;
			symStrTab = (char *)    kmalloc(strTabSec->sh_size);
			symbols   = (symbol_t *)kmalloc((nSyms + 1) * sizeof(symbol_t));

			memcpy(symStrTab, strTabStr, strTabSec->sh_size);

			for(uint32_t j = 0; j < nSyms; j++) {
				/*kerror(ERR_BOOTINFO, "  -> [%d]: %s -> %08X:%08X, %04X", j, &strTab[syms[j].st_name],
					syms[j].st_value, syms[j].st_size, syms[j].st_name);*/
				
				symbols[j].name = &symStrTab[syms[j].st_name];
				symbols[j].addr = syms[j].st_value;
				symbols[j].size = syms[j].st_size;
			}

			symbols[nSyms].name = NULL;
			symbols[nSyms].addr = 0xFFFFFFFF;
			symbols[nSyms].size = 0x00000000;
		}
	}

	kerror(ERR_INFO, "Entrypoint: %08X -> %08X", head->e_entry, pgdir_get_page_entry(pgdir, (void *)head->e_entry) & 0xFFFFF000);

	//kerror(ERR_BOOTINFO, "ELF Loaded");
	//int pid = add_user_task_pdir((void *)head->e_entry, "UNNAMED_ELF", 0x2000, PRIO_USERPROG, pgdir);

	// TODO: Destroy old symbol table, if applicable!
	// Create symbol table:
	//int p = proc_by_pid(current_pid);
	//procs[p].symbols   = symbols;
	//procs[p].symStrTab = symStrTab;

	//int argc = 0;
	//while(argv[argc] != 0) argc++;

	//memcpy(procs[p].name, "ELF_PROG", 9);

	//enter_ring(procs[p].ring, (void *)head->e_entry, 0, argv, envp);
	//enter_ring(3, (void *)head->e_entry, 0, argv, envp);

	exec_replace_process_image((void *)head->e_entry, "ELF_PROG", pgdir, symbols, symStrTab, argv, envp);

	//return pid;
	return -1;
}




char *sht_strings[] = {
	[SHT_NONE]          = "NONE",
	[SHT_PROGBITS]      = "PROG",
	[SHT_SYMTAB]        = "SYMTAB",
	[SHT_STRTAB]        = "STRTAB",
	[SHT_RELA]          = "RELA",
	[SHT_HASH]          = "HASH",
	[SHT_DYNAMIC]       = "DYNAMIC",
	[SHT_NOTE]          = "NOTE",
	[SHT_NOBITS]        = "NOBITS",
	[SHT_REL]           = "REL",
	[SHT_SHLIB]         = "SHLIB",
	[SHT_DYNSYM]        = "DYNSYM",
	[SHT_INIT_ARRAY]    = "INIT_ARRAY",
	[SHT_FINI_ARRAY]    = "FINI_ARRAY",
	[SHT_PREINIT_ARRAY] = "PREINIT_ARRAY",

	[(SHT_DYNSYM+1)...(SHT_INIT_ARRAY-1)] = "RESERVED"
};
