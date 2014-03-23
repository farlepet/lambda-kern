#include <proc/mtask.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <proc/elf.h>
#include <string.h>

#ifdef ARCH_X86
#include <mm/paging.h>
#endif


ptr_t load_elf(void *file, u32 length, u32 **pdir)
{
	kerror(ERR_BOOTINFO, "Loading elf from %08X of length %08X", file, length);

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

	u32 *pgdir = clone_kpagedir();
	//u32 *phys_pgdir = get_phys_page(pgdir);
	//u32 *pgdir = (u32 *)kernel_cr3;

	ptr_t i = 0;
	for(; i < (ptr_t)(head->e_shentsize * head->e_shnum); i += head->e_shentsize)
	{
		Elf32_Shdr *shdr = (Elf32_Shdr *)((ptr_t)head + (head->e_shoff + i));

		kerror(ERR_BOOTINFO, "shdr[%X/%X] T:%s ADDR:%08X SZ:%08X", (i/head->e_shentsize)+1, head->e_shnum, sht_strings[shdr->sh_type], shdr->sh_addr, shdr->sh_size);

		if(shdr->sh_addr)
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
				kerror(ERR_BOOTINFO, "  -> MAP_PAGE<%08X>[%08X, %08X]", pgdir, (phys + p), (shdr->sh_addr + p));
				pgdir_map_page(pgdir, (phys + p), (void *)(shdr->sh_addr + p), 0x03);
				//map_page((phys + p), (void *)(shdr->sh_addr + p), 0x03);
				kerror(ERR_BOOTINFO, "      -> DONE");
			}
	
		}
	}

	kerror(ERR_BOOTINFO, "ELF Loaded");

	*pdir = pgdir;
	return head->e_entry;
}






char *sht_strings[] =
{
	[SHT_NONE]                      = "NONE",
	[SHT_PROGBITS]                  = "PROG",
	[SHT_SYMTAB]                    = "SYMTAB",
	[SHT_STRTAB]                    = "STRTAB",
	[(SHT_STRTAB+1)...(SHT_NOTE-1)] = "RESERVED",
	[SHT_NOTE]                      = "NOTE",
	[SHT_NOBITS]                    = "NOBITS"
};

