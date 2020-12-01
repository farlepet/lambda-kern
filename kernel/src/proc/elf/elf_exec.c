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

static void elf_read_phdr(Elf32_Ehdr *elf, struct kproc_mem_map_ent **mmap_entries, proc_elf_data_t *elf_data, arch_task_params_t *arch_params) {
	if(!mmap_entries || !elf_data) { return; }

	struct kproc_mem_map_ent **mmap_next = mmap_entries;
	
	Elf32_Phdr *prog = (Elf32_Phdr *)((uintptr_t)elf + elf->e_phoff);

	elf_data->dynamic = NULL;

	for(size_t i = 0; i < elf->e_phnum; i++) {
		kerror(ERR_INFO, "phdr[%2X/%2X] T:%X VADDR: %08X MSZ:%08X FSZ:%08X", i+1, elf->e_phnum, prog[i].p_type, prog[i].p_vaddr, prog[i].p_memsz, prog[i].p_filesz);
		switch(prog[i].p_type) {
			case PT_LOAD: {
				/* TODO: Read entire header first, so we can avoid issues with
				 * overlapping memory regions. Or create more sophisticated mmap
				 * system. */
				/* Allocate physical memory */
				void *phys = kmalloc(prog[i].p_memsz + 0x2000); // + 0x2000 so we can page-align it
				phys = (void *)((uintptr_t)phys & ~0xFFF) + 0x1000 + (prog[i].p_vaddr & 0xFFF);

				/* Copy data and/or clear memory */
				if(prog[i].p_filesz) {
					memcpy(phys, (void *)((uintptr_t)elf + prog[i].p_offset), prog[i].p_filesz);
				}
				if(prog[i].p_filesz < prog[i].p_memsz) {
					memset(phys + prog[i].p_filesz, 0, prog[i].p_memsz - prog[i].p_filesz);
				}

				/* Create MMAP entry: */
				*mmap_next = (struct kproc_mem_map_ent *)kmalloc(sizeof(struct kproc_mem_map_ent));
				(*mmap_next)->virt_address = prog[i].p_vaddr;
				(*mmap_next)->phys_address = (uintptr_t)phys;
				(*mmap_next)->length       = prog[i].p_memsz;
				(*mmap_next)->next         = NULL;
				mmap_next = &((*mmap_next)->next);

#if defined(ARCH_X86)
				/* TODO: Create architecture-independant memory mapping mechanism. */
				for(uintptr_t pg = 0; pg < prog[i].p_memsz; pg += 0x1000) {
					pgdir_map_page(arch_params->pgdir, (phys + pg), (void *)(prog[i].p_vaddr + pg), 0x07);
				}
#endif
			} break;
			case PT_DYNAMIC:
				elf_data->dynamic = (Elf32_Dyn *)prog[i].p_vaddr;
				break;
		}
	}
}

static uintptr_t elf_exec_common(void *data, uint32_t length, arch_task_params_t *arch_params, char **symStrTab, symbol_t **symbols, struct kproc_mem_map_ent **mmap_entries, proc_elf_data_t *elf_data) {
	/* TODO: Use this for error-checking */
	(void)length;
#if defined(ARCH_ARMV7)
	(void)arch_params;
#endif
	
	Elf32_Ehdr *head = (Elf32_Ehdr *)data;

	if(symStrTab) *symStrTab = NULL;
	if(symbols)   *symbols   = NULL;

	Elf32_Shdr *sections     = (Elf32_Shdr *)((uintptr_t)head + head->e_shoff);
	Elf32_Shdr *strTabSec    = &sections[head->e_shstrndx];
	const char *strTabSecStr = (const char *)((uintptr_t)head + strTabSec->sh_offset);
	Elf32_Shdr *strTab       = NULL;
	const char *strTabStr    = NULL;

	elf_read_phdr(head, mmap_entries, elf_data, arch_params);

	for(uint32_t i = 0; i < head->e_shnum; i ++) {
		if(sections[i].sh_type == SHT_STRTAB &&
		   &sections[i] != strTabSec) {
			strTab    = &sections[i];
			strTabStr = (const char *)((uintptr_t)head + sections[i].sh_offset);
			break;
		}
	}

	for(uint32_t i = 0; i < head->e_shnum; i ++) {
		Elf32_Shdr *shdr = &sections[i];

		if(shdr->sh_type <= SHT_PREINIT_ARRAY) {
			kerror(ERR_INFO, "shdr[%2X/%2X] N:%s T:%s OFF: %08X ADDR:%08X SZ:%08X", i+1, head->e_shnum, &strTabSecStr[shdr->sh_name], sht_strings[shdr->sh_type], shdr->sh_offset, shdr->sh_addr, shdr->sh_size);
		} else {
			kerror(ERR_INFO, "shdr[%2X/%2X] N:%s T:%X OFF: %08X ADDR:%08X SZ:%08X", i+1, head->e_shnum, &strTabSecStr[shdr->sh_name], shdr->sh_type, shdr->sh_offset, shdr->sh_addr, shdr->sh_size);
		}

		if((shdr->sh_type == SHT_SYMTAB) &&
		   (symStrTab) && (symbols) && (strTabStr)) {
			Elf32_Sym *syms = (Elf32_Sym *)((uintptr_t)head + shdr->sh_offset);

			uint32_t nSyms = shdr->sh_size / shdr->sh_entsize;
			*symStrTab = (char *)    kmalloc(strTab->sh_size);
			*symbols   = (symbol_t *)kmalloc((nSyms + 1) * sizeof(symbol_t));

			memcpy(*symStrTab, strTabStr, strTab->sh_size);

			for(uint32_t j = 0; j < nSyms; j++) {
				(*symbols)[j].name = &(*symStrTab)[syms[j].st_name];
				(*symbols)[j].addr = syms[j].st_value;
				(*symbols)[j].size = syms[j].st_size;
			}

			(*symbols)[nSyms].name = NULL;
			(*symbols)[nSyms].addr = 0xFFFFFFFF;
			(*symbols)[nSyms].size = 0x00000000;
		}
	}

	if(elf_data->dynamic) {
		size_t i = 0;
		int needed = -1;
		while(elf_data->dynamic[i].d_tag != DT_NULL) {
			const Elf32_Dyn *dyn = &elf_data->dynamic[i];
			switch(dyn->d_tag) {
				case DT_STRTAB:
					elf_data->dynamic_str     = (const char *)dyn->d_val;
					break;
				case DT_STRSZ:
					elf_data->dynamic_str_len = dyn->d_val;
					break;
				case DT_SYMTAB:
					elf_data->dynamic_sym     = (const Elf32_Sym *)dyn->d_val;
					break;
				case DT_NEEDED:
					/* Deal with this later, we need to ensure we have the string table. */
					needed = i;
					break;
				default:
					break;
			}
			i++;
		}

		if(needed >= 0) {
			kerror(ERR_BOOTINFO, "DT_NEEDED: %s", &elf_data->dynamic_str[elf_data->dynamic[needed].d_val]);
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
	if(elf_check_header(file)) {
		return -1;
	}

	arch_task_params_t arch_params;
#if defined(ARCH_X86)
	arch_params.pgdir = clone_kpagedir();
#endif
	
	char                     *symStrTab;
	symbol_t                 *symbols;
	struct kproc_mem_map_ent *mmap_entries;
	proc_elf_data_t           elf_data;

	uintptr_t entrypoint = elf_exec_common(file, length, &arch_params, &symStrTab, &symbols, &mmap_entries, &elf_data);
	if(entrypoint == 0) {
		return -1;
	}


	kerror(ERR_INFO, "Entrypoint: %08X", entrypoint);
	
	// Old way of creating new process:
	int pid = add_user_task_arch((void *)entrypoint, "UNNAMED_ELF", 0, PRIO_USERPROG, &arch_params);

	struct kproc *proc = proc_by_pid(pid);
	proc->symbols   = symbols;
	proc->symStrTab = symStrTab;
	proc_add_mmap_ents(proc, mmap_entries);
	
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
	proc_elf_data_t           elf_data;

	uintptr_t entrypoint = elf_exec_common(data, length, &arch_params, &symStrTab, &symbols, &mmap_entries, &elf_data);
	if(entrypoint == 0) {
		return -1;
	}

	kerror(ERR_INFO, "Entrypoint: %08X", entrypoint);

	// TODO: Add generated memory map to this process

	exec_replace_process_image((void *)entrypoint, "ELF_PROG", &arch_params, symbols, symStrTab, argv, envp);

	/* We shouldn't get this far */
	return -1;
}
