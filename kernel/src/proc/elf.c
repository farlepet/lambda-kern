#include <proc/mtask.h>
#include <proc/exec.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <proc/elf.h>
#include <string.h>
#include <video.h>

#ifdef ARCH_X86
#include <mm/paging.h>
#include <proc/user.h>
#endif


ptr_t load_elf(void *file, u32 length, u32 **pdir)
{
	//kerror(ERR_BOOTINFO, "Loading elf from %08X of length %08X", file, length);
	(void)length; // TODO: Error-check with this

	Elf32_Ehdr *head = file;

	if( (head->e_ident[0] != ELF_IDENT0) ||
		(head->e_ident[1] != ELF_IDENT1) ||
		(head->e_ident[2] != ELF_IDENT2) ||
		(head->e_ident[3] != ELF_IDENT3))
	{
		kerror(ERR_SMERR, "Tried to load an ELF with incorrect header: %x %c %c %c",
			(head->e_ident[0]),
			(head->e_ident[1]),
			(head->e_ident[2]),
			(head->e_ident[3]));
		return 0;
	}

	if(head->e_ident[4] != HOST_CLASS)
	{
		kerror(ERR_SMERR, "Tried to load ELF not compatible with current bittiness: %d", head->e_ident[4]);
		return 0;
	}

	if(head->e_type != ET_EXEC)
	{
		kerror(ERR_SMERR, "Tried to load non-executable ELF with type %d", head->e_type);
		return 0;
	}

	if(head->e_machine != HOST_MACHINE)
	{
		kerror(ERR_SMERR, "Tried to load ELF not compatible with current architecture: %d", head->e_machine);
		return 0;
	}

	int p = proc_by_pid(current_pid);

	u32 *pgdir = clone_kpagedir();
	//u32 *pgdir = (u32 *)procs[p].cr3;
	
	//u32 *phys_pgdir = get_phys_page(pgdir);
	//u32 *pgdir = (u32 *)kernel_cr3;

	//u32 used_addresses[32][2];

	//kerror(ERR_BOOTINFO, "Section Header offset: %08X, size: %08X, #: %d", head->e_shoff, head->e_shentsize, head->e_shnum);

	char     *symStrTab = NULL;
	symbol_t *symbols   = NULL;

	Elf32_Shdr *sections = (Elf32_Shdr *)((ptr_t)head + head->e_shoff);

	uint32_t i = 0;
	for(; i < head->e_shnum; i ++)
	{
		Elf32_Shdr *shdr = &sections[i];//(Elf32_Shdr *)((ptr_t)head + (head->e_shoff + i));

		kerror(ERR_INFO, "shdr[%X/%X] N:%s T:%s OFF: %08X ADDR:%08X SZ:%08X", i+1, head->e_shnum, sht_strings[shdr->sh_name], sht_strings[shdr->sh_type], shdr->sh_offset, shdr->sh_addr, shdr->sh_size);

		if(shdr->sh_addr) // Check if there is a destination address
		{
			void *phys = kmalloc(shdr->sh_size + 0x2000); // + 0x2000 so we can page-align it
			phys = (void *)((ptr_t)phys & ~0xFFF) + 0x1000 + (shdr->sh_addr & 0xFFF);
			//void *phys_phys = get_phys_page(phys);

			//set_pagedir(phys_pgdir);
			if(shdr->sh_type == SHT_NOBITS) memset(phys, 0, shdr->sh_size);
			else memcpy(phys, (void *)((ptr_t)head + shdr->sh_offset), shdr->sh_size);
			//set_pagedir((u32 *)kernel_cr3);

			ptr_t p = 0;
			for(; p < shdr->sh_size; p += 0x1000)
			{
				//kerror(ERR_BOOTINFO, "  -> MAP_PAGE<%08X>[%08X, %08X]", pgdir, (phys + p), (shdr->sh_addr + p));
				pgdir_map_page(pgdir, (phys + p), (void *)(shdr->sh_addr + p), 0x07);
				//pgdir_map_page(pgdir, (phys + p), (void *)(shdr->sh_addr + p), 0x03);
				//map_page((phys + p), (void *)(shdr->sh_addr + p), 0x03);
				//kerror(ERR_BOOTINFO, "      -> DONE");
			}
		} else if(shdr->sh_type == SHT_SYMTAB) {
			Elf32_Sym *syms = (Elf32_Sym *)((ptr_t)head + shdr->sh_offset);

			Elf32_Shdr *strTabSec = &sections[shdr->sh_link]; //(Elf32_Shdr *)((ptr_t)head + head->e_shentsize * shdr->sh_link);
			char       *strTab    = (char *)((ptr_t)head + strTabSec->sh_offset);

			uint32_t nSyms = shdr->sh_size / shdr->sh_entsize;
			symStrTab = (char *)    kmalloc(strTabSec->sh_size);
			symbols   = (symbol_t *)kmalloc((nSyms + 1) * sizeof(symbol_t));

			memcpy(symStrTab, strTab, strTabSec->sh_size);

			for(uint32_t i = 0; i < nSyms; i++) {
				/*kerror(ERR_BOOTINFO, "  -> [%d]: %s -> %08X:%08X, %04X", i, &strTab[syms[i].st_name],
					syms[i].st_value, syms[i].st_size, syms[i].st_name);*/
				
				symbols[i].name = &symStrTab[syms[i].st_name];
				symbols[i].addr = syms[i].st_value;
				symbols[i].size = syms[i].st_size;
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
	
	//enter_ring_noargs(procs[p].ring, (void *)head->e_entry);

	return pid;
	//return -1;
}



int exec_elf(void *data, u32 length, const char **argv, const char **envp) {
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

	u32 *pgdir = clone_kpagedir();
	//u32 *phys_pgdir = get_phys_page(pgdir);
	//u32 *pgdir = (u32 *)kernel_cr3;
	//u32 *pgdir = get_pagedir();

	//u32 used_addresses[32][2];

	//kerror(ERR_BOOTINFO, "Section Header offset: %08X, size: %08X, #: %d", head->e_shoff, head->e_shentsize, head->e_shnum);

	char     *symStrTab = NULL;
	symbol_t *symbols   = NULL;

	Elf32_Shdr *sections = (Elf32_Shdr *)((ptr_t)head + head->e_shoff);

	uint32_t i = 0;
	for(; i < head->e_shnum; i ++) {
		Elf32_Shdr *shdr = &sections[i];//(Elf32_Shdr *)((ptr_t)head + (head->e_shoff + i));

		kerror(ERR_INFO, "shdr[%X/%X] N:%s T:%s OFF: %08X ADDR:%08X SZ:%08X", i+1, head->e_shnum, sht_strings[shdr->sh_name], sht_strings[shdr->sh_type], shdr->sh_offset, shdr->sh_addr, shdr->sh_size);

		// Check if there is a destination address
		if(shdr->sh_addr) {
			void *phys = kmalloc(shdr->sh_size + 0x2000); // + 0x2000 so we can page-align it
			phys = (void *)((ptr_t)phys & ~0xFFF) + 0x1000 + (shdr->sh_addr & 0xFFF);
			//void *phys_phys = get_phys_page(phys);

			//set_pagedir(phys_pgdir);
			if(shdr->sh_type == SHT_NOBITS) memset(phys, 0, shdr->sh_size);
			else memcpy(phys, (void *)((ptr_t)head + shdr->sh_offset), shdr->sh_size);
			//set_pagedir((u32 *)kernel_cr3);

			ptr_t p = 0;
			for(; p < shdr->sh_size; p += 0x1000) {
				//kerror(ERR_BOOTINFO, "  -> MAP_PAGE<%08X>[%08X, %08X]", pgdir, (phys + p), (shdr->sh_addr + p));
				pgdir_map_page(pgdir, (phys + p), (void *)(shdr->sh_addr + p), 0x07);
				//pgdir_map_page(pgdir, (phys + p), (void *)(shdr->sh_addr + p), 0x03);
				//map_page((phys + p), (void *)(shdr->sh_addr + p), 0x03);
				//kerror(ERR_BOOTINFO, "      -> DONE");
			}
		} else if(shdr->sh_type == SHT_SYMTAB) {
			Elf32_Sym *syms = (Elf32_Sym *)((ptr_t)head + shdr->sh_offset);

			Elf32_Shdr *strTabSec = &sections[shdr->sh_link]; //(Elf32_Shdr *)((ptr_t)head + head->e_shentsize * shdr->sh_link);
			char       *strTab    = (char *)((ptr_t)head + strTabSec->sh_offset);

			uint32_t nSyms = shdr->sh_size / shdr->sh_entsize;
			symStrTab = (char *)    kmalloc(strTabSec->sh_size);
			symbols   = (symbol_t *)kmalloc((nSyms + 1) * sizeof(symbol_t));

			memcpy(symStrTab, strTab, strTabSec->sh_size);

			for(uint32_t i = 0; i < nSyms; i++) {
				/*kerror(ERR_BOOTINFO, "  -> [%d]: %s -> %08X:%08X, %04X", i, &strTab[syms[i].st_name],
					syms[i].st_value, syms[i].st_size, syms[i].st_name);*/
				
				symbols[i].name = &symStrTab[syms[i].st_name];
				symbols[i].addr = syms[i].st_value;
				symbols[i].size = syms[i].st_size;
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
	[SHT_NONE]                      = "NONE",
	[SHT_PROGBITS]                  = "PROG",
	[SHT_SYMTAB]                    = "SYMTAB",
	[SHT_STRTAB]                    = "STRTAB",
	[(SHT_STRTAB+1)...(SHT_NOTE-1)] = "RESERVED",
	[SHT_NOTE]                      = "NOTE",
	[SHT_NOBITS]                    = "NOBITS"
};
