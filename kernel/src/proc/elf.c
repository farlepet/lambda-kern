#include <proc/mtask.h>
#include <proc/exec.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <proc/elf.h>
#include <string.h>
#include <video.h>

#if defined(ARCH_X86)
#  include <arch/mm/paging.h>
#  include <arch/proc/user.h>
#endif

static ptr_t elf_exec_common(void *data, uint32_t length, arch_task_params_t *arch_params, char **symStrTab, symbol_t **symbols, struct kproc_mem_map_ent **mmap_entries) {
	/* TODO: Use this for error-checking */
	(void)length;
#if defined(ARCH_ARMV7)
	(void)arch_params;
#endif
	
	Elf32_Ehdr *head = (Elf32_Ehdr *)data;

	if(symStrTab) *symStrTab = NULL;
	if(symbols)   *symbols   = NULL;

	Elf32_Shdr *sections     = (Elf32_Shdr *)((ptr_t)head + head->e_shoff);
	Elf32_Shdr *strTabSec    = &sections[head->e_shstrndx];
	const char *strTabSecStr = (const char *)((ptr_t)head + strTabSec->sh_offset);
	Elf32_Shdr *strTab       = NULL;
	const char *strTabStr    = NULL;

	for(uint32_t i = 0; i < head->e_shnum; i ++) {
		if(sections[i].sh_type == SHT_STRTAB &&
		   &sections[i] != strTabSec) {
			strTab    = &sections[i];
			strTabStr = (const char *)((ptr_t)head + sections[i].sh_offset);
			break;
		}
	}

	struct kproc_mem_map_ent **mmap_next = mmap_entries;

	for(uint32_t i = 0; i < head->e_shnum; i ++) {
		Elf32_Shdr *shdr = &sections[i];//(Elf32_Shdr *)((ptr_t)head + (head->e_shoff + i));

		kerror(ERR_INFO, "shdr[%X/%X] N:%s T:%s OFF: %08X ADDR:%08X SZ:%08X", i+1, head->e_shnum, &strTabSecStr[shdr->sh_name], sht_strings[shdr->sh_type], shdr->sh_offset, shdr->sh_addr, shdr->sh_size);

		if(shdr->sh_addr) { // Check if there is a destination address
			void *phys = kmalloc(shdr->sh_size + 0x2000); // + 0x2000 so we can page-align it
			phys = (void *)((ptr_t)phys & ~0xFFF) + 0x1000 + (shdr->sh_addr & 0xFFF);
			//void *phys_phys = get_phys_page(phys);

			//set_pagedir(phys_pgdir);
			if(shdr->sh_type == SHT_NOBITS) memset(phys, 0, shdr->sh_size);
			else memcpy(phys, (void *)((ptr_t)head + shdr->sh_offset), shdr->sh_size);
			//set_pagedir((uint32_t *)kernel_cr3);

#if defined(ARCH_X86)
			for(ptr_t pg = 0; pg < shdr->sh_size; pg += 0x1000) {
				//kerror(ERR_BOOTINFO, "  -> MAP_PAGE<%08X>[%08X, %08X]", pgdir, (phys + p), (shdr->sh_addr + p));
				pgdir_map_page(arch_params->pgdir, (phys + pg), (void *)(shdr->sh_addr + pg), 0x07);
				//pgdir_map_page(pgdir, (phys + p), (void *)(shdr->sh_addr + p), 0x03);
			}
#endif

			if(mmap_entries) {
				// Create MMAP entry:
				*mmap_next = (struct kproc_mem_map_ent *)kmalloc(sizeof(struct kproc_mem_map_ent));
				(*mmap_next)->virt_address = shdr->sh_addr;
				(*mmap_next)->phys_address = (uintptr_t)phys;
				(*mmap_next)->length       = shdr->sh_size;
				(*mmap_next)->next         = NULL;
				mmap_next = &((*mmap_next)->next);
			}

		} else if((shdr->sh_type == SHT_SYMTAB) &&
		          (symStrTab) && (symbols) && (strTabStr)) {
			Elf32_Sym *syms = (Elf32_Sym *)((ptr_t)head + shdr->sh_offset);

			uint32_t nSyms = shdr->sh_size / shdr->sh_entsize;
			*symStrTab = (char *)    kmalloc(strTab->sh_size);
			*symbols   = (symbol_t *)kmalloc((nSyms + 1) * sizeof(symbol_t));

			memcpy(*symStrTab, strTabStr, strTab->sh_size);

			for(uint32_t j = 0; j < nSyms; j++) {
				/*kerror(ERR_BOOTINFO, "  -> [%d]: %s -> %08X:%08X, %04X", j, &strTabStr[syms[j].st_name],
					syms[j].st_value, syms[j].st_size, syms[j].st_name);*/
				(*symbols)[j].name = &(*symStrTab)[syms[j].st_name];
				(*symbols)[j].addr = syms[j].st_value;
				(*symbols)[j].size = syms[j].st_size;
			}

			(*symbols)[nSyms].name = NULL;
			(*symbols)[nSyms].addr = 0xFFFFFFFF;
			(*symbols)[nSyms].size = 0x00000000;
		}
	}

	return head->e_entry;
}

static int elf_check_header(void *data) {
	Elf32_Ehdr *head = (Elf32_Ehdr *)data;

	if( (head->e_ident[0] != ELF_IDENT0) ||
		(head->e_ident[1] != ELF_IDENT1) ||
		(head->e_ident[2] != ELF_IDENT2) ||
		(head->e_ident[3] != ELF_IDENT3)) {
		kerror(ERR_SMERR, "Tried to load an ELF with incorrect header: %x %c %c %c",
			(head->e_ident[0]),
			(head->e_ident[1]),
			(head->e_ident[2]),
			(head->e_ident[3]));
		return 1;
	}

	if(head->e_ident[4] != HOST_CLASS) {
		kerror(ERR_SMERR, "Tried to load ELF not compatible with current bittiness: %d", head->e_ident[4]);
		return 1;
	}

	if(head->e_type != ET_EXEC) {
		kerror(ERR_SMERR, "Tried to load non-executable ELF with type %d", head->e_type);
		return 1;
	}

	if(head->e_machine != HOST_MACHINE) {
		kerror(ERR_SMERR, "Tried to load ELF not compatible with current architecture: %d", head->e_machine);
		return 1;
	}

	return 0;
}

int load_elf(void *file, uint32_t length) {
	//kerror(ERR_BOOTINFO, "Loading elf from %08X of length %08X", file, length);

	if(elf_check_header(file)) {
		return -1;
	}

	int p = proc_by_pid(current_pid);

	arch_task_params_t arch_params;
#if defined(ARCH_X86)
	arch_params.pgdir = clone_kpagedir();
#endif
	
	char                     *symStrTab;
	symbol_t                 *symbols;
	struct kproc_mem_map_ent *mmap_entries;

	ptr_t entrypoint = elf_exec_common(file, length, &arch_params, &symStrTab, &symbols, &mmap_entries);
	if(entrypoint == 0) {
		return -1;
	}


	kerror(ERR_INFO, "Entrypoint: %08X", entrypoint);
	
	// Old way of creating new process:
	int pid = add_user_task_arch((void *)entrypoint, "UNNAMED_ELF", 0, PRIO_USERPROG, &arch_params);

	p = proc_by_pid(pid);
	procs[p].symbols   = symbols;
	procs[p].symStrTab = symStrTab;
	proc_add_mmap_ents(&procs[p], mmap_entries);
	
	return pid;
}



int exec_elf(void *data, uint32_t length, const char **argv, const char **envp) {
	//kerror(ERR_BOOTINFO, "Executing elf from %08X of length %08X", data, length);

	if(elf_check_header(data)) {
		return 0;
	}

	arch_task_params_t arch_params;
#if defined(ARCH_X86)
	arch_params.pgdir = clone_kpagedir();
#endif

	char                     *symStrTab;
	symbol_t                 *symbols;
	struct kproc_mem_map_ent *mmap_entries;

	ptr_t entrypoint = elf_exec_common(data, length, &arch_params, &symStrTab, &symbols, &mmap_entries);
	if(entrypoint == 0) {
		return -1;
	}

	kerror(ERR_INFO, "Entrypoint: %08X", entrypoint);

	// TODO: Add generated memory map to this process

	exec_replace_process_image((void *)entrypoint, "ELF_PROG", &arch_params, symbols, symStrTab, argv, envp);

	/* We shouldn't get this far */
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
