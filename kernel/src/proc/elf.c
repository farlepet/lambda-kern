#include <proc/mtask.h>
#include <err/error.h>
#include <proc/elf.h>


int load_elf(void *file, u32 length)
{
	kerror(ERR_BOOTINFO, "Loading elf from %08X of length %08X", file, length);

	Elf32_Ehdr *head = file;

	if( (head->e_ident[0] != ELF_IDENT0) ||
		(head->e_ident[1] != ELF_IDENT1) ||
		(head->e_ident[2] != ELF_IDENT2) ||
		(head->e_ident[3] != ELF_IDENT3))
	{
		kerror(ERR_SMERR, "Tried to load an ELF with incorrect header");
		return 1;
	}

	if(head->e_ident[4] != HOST_CLASS)
	{
		kerror(ERR_SMERR, "Tried to load ELF not compatible with current bittiness: %d", head->e_ident[4]);
		return 1;
	}

	if(head->e_type != ET_EXEC)
	{
		kerror(ERR_SMERR, "Tried to load non-executable ELF with type %d", head->e_type);
		return 1;
	}

	if(head->e_machine != HOST_MACHINE)
	{
		kerror(ERR_SMERR, "Tried to load ELF not compatible with current architecture: %d", head->e_machine);
		return 1;
	}

	return 0;
}