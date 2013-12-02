#include <multiboot.h>
#include <mm/paging.h>
#include <mm/gdt.h>
#include <intr/idt.h>
#include <dev/vga/print.h>
#include <ioport.h>

int kmain(struct multiboot __unused *mboot_ptr, u32 __unused initial_stack)
{
	vga_clear();
	gdt_init();
	idt_init();
	paging_init();
	vga_print("Welcome to Lambda OS!\n");

	outb(0x21,0xfd);
	outb(0xa1,0xff);

	__sti;

	for(;;) __builtin_ia32_pause();
}