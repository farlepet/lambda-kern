#include <mm/paging.h>
#include <err/error.h>
#include <types.h>

void handle_page_fault()
{
	u32 cr2; asm ("mov %%cr2, %0" : "=r" (cr2));

	kerror(ERR_MEDERR, "Page fault at 0x%08X", cr2);

	// TODO: Gather more information about the page fault
	for(;;);
}