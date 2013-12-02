#include <multiboot.h>
#include <dev/vga/print.h>

int kmain(struct multiboot __unused *mboot_ptr, u32 __unused initial_stack)
{
	vga_clear();
	vga_print("Welcome to Lambda OS!\n");
	for(;;) __builtin_ia32_pause();
}