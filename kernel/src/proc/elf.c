#include <proc/mtask.h>
#include <err/error.h>
#include <proc/elf.h>


int load_elf(void *file, u32 length)
{
	kerror(ERR_BOOTINFO, "Loading elf from %08X of length %08X", file, length);

	Elf32_Ehdr *head = file;

	if(*(u32 *)&head->e_ident[0] != ELF_IDENT)
	{
		kerror(ERR_SMERR, "Tried to load an ELF with incorrect header: %08X %08x %08x", *(u32 *)&head->e_ident[0], *(u32 *)&head->e_ident[4], *(u32 *)&head->e_ident[8]);
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