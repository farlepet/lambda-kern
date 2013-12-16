#include <multiboot.h>
#include <kernel/arch/x86/dev/vga/print.h>

struct multiboot_tag *find_multiboot_table(struct multiboot_header_tag* mboot_tag, u32 type)
{
	u32 size = mboot_tag->size;
	u32 i = 8; // Bypass multiboot_header_tag
	while(i < size)
	{
		struct multiboot_tag *tag = (struct multiboot_tag *)(i + (u32)mboot_tag);
		
		if(tag->type == type) return tag;
		i += tag->size;
		if(i & 0x07) i = (i & ~0x07) + 8; // Entries are aways padded
	}
	return 0; // Couldn't find an appropiate tag
}