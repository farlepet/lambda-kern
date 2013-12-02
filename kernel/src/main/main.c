#include <multiboot.h>

void vga_clear()
{
	int i = 0;
	for(; i < 85*25*2; i++)
		*((u8 *)0xB8000 + i) = 0x00;
}

int kmain(struct multiboot __unused *mboot_ptr, u32 __unused initial_stack)
{
	vga_clear();
	*((u8 *)0xB8000) = 'l';
	*((u8 *)0xB8001) = 0x07;
	for(;;) __builtin_ia32_pause();
}