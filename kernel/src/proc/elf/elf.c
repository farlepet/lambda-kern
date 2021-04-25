#include <proc/mtask.h>
#include <proc/exec.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <proc/elf.h>
#include <string.h>
#include <video.h>

int elf_find_section(const Elf32_Ehdr *elf, Elf32_Shdr **section, const char *section_name) {
	Elf32_Shdr       *sections     = (Elf32_Shdr *)((uintptr_t)elf + elf->e_shoff);
	const Elf32_Shdr *strTabSec    = &sections[elf->e_shstrndx];
	const char       *strTabSecStr = (const char *)((uintptr_t)elf + strTabSec->sh_offset);

	for(size_t i = 0; i < elf->e_shnum; i++) {
		if(!strcmp(&strTabSecStr[sections[i].sh_name], section_name)) {
			if(section) {
				*section = &sections[i];
			}
			return 0;
		}
	}
	
	/* Section not found */
	return 1;
}

uintptr_t elf_find_data(const Elf32_Ehdr *elf, uintptr_t addr) {
	Elf32_Shdr       *sections     = (Elf32_Shdr *)((uintptr_t)elf + elf->e_shoff);

	for(size_t i = 0; i < elf->e_shnum; i++) {
		if((addr >= sections[i].sh_addr) &&
		   (addr <  (sections[i].sh_addr + sections[i].sh_size))) {
            return (uintptr_t)elf + sections[i].sh_offset +
			       (addr - sections[i].sh_addr);
		}
	}

	return 0;
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
