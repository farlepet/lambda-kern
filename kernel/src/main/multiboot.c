#include <multiboot.h>
#include <err/error.h>
#include <string.h>
#include <video.h>

#if defined(ARCH_X86)
#include <io/serial.h>
#endif

/**
 * Finds the first multiboot entry with a certain type value, then returns it
 * as a multiboot_tag.
 * 
 * @param mboot_tag the pointer to the multiboot tags header
 * @param type the type value to look for
 * @see multiboot_tag
 */
struct multiboot_tag *find_multiboot_table(struct multiboot_header_tag* mboot_tag, u32 type)
{
	u32 size = mboot_tag->size;
	u32 i = 8; // Bypass multiboot_header_tag
	while(i < size)
	{
		struct multiboot_tag *tag = (struct multiboot_tag *)(i + (ptr_t)mboot_tag);
		
		if(tag->type == type) return tag;
		i += tag->size;
		if(i & 0x07) i = (i & ~0x07) + 8; // Entries are aways padded
	}
	return 0; // Couldn't find an appropiate tag
}


extern int output_serial;
/**
 * Checks the kernel commandline and does things accordingly.
 *
 * @param mboot_tag pointer to the multiboot tags header
 */
void check_commandline(struct multiboot_header_tag *mboot_tag)
{
	struct multiboot_cmdline_tag *cmdtag = (struct multiboot_cmdline_tag *)find_multiboot_table(mboot_tag, 1);
	if(!cmdtag)
	{
		kerror(ERR_MEDERR, "Commandline tag not found!");
		return;
	}
	char *cmd = (char *)cmdtag->cmdline;
	char *end = cmd + strlen(cmd) + 1;
	char tmp[1000];
	int i;
	while(cmd < end)
	{
		i = 0;
		while(*cmd == ' ') cmd++;
		while(*cmd != ' ') tmp[i++] = *cmd++;
		tmp[i] = 0;

#if defined(ARCH_X86)
		if     (!strcmp(tmp, "-s"))     output_serial = SERIAL_COM1;
		else if(!strcmp(tmp, "-sCOM1")) output_serial = SERIAL_COM1;
		else if(!strcmp(tmp, "-sCOM2")) output_serial = SERIAL_COM2;
		else if(!strcmp(tmp, "-sCOM3")) output_serial = SERIAL_COM3;
		else if(!strcmp(tmp, "-sCOM4")) output_serial = SERIAL_COM4;
#endif
	}
}