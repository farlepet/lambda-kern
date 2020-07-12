#include <multiboot.h>
#include <err/panic.h>
#include <err/error.h>
#include <string.h>
#include <video.h>

#if defined(ARCH_X86)
#  include <arch/io/serial.h>
#endif

extern int   output_serial;
extern char *cpio_name;
/**
 * Checks the kernel commandline and does things accordingly.
 *
 * @param mboot_tag pointer to the multiboot tags header
 */
void check_commandline(struct multiboot_header *mboot_head)
{
	if(!(mboot_head->flags & MBOOT_CMDLINE))
	{
		kerror(ERR_BOOTINFO, "No commandline provided");
		return;
	}
	
	char *cmd = (char *)mboot_head->cmdline;
	char *end = cmd + strlen(cmd) + 1;
	char tmp[1000];
	int i;
	while(cmd < end)
	{
		kerror(ERR_BOOTINFO, "CMD: %08X END: %08X", cmd, end);
		i = 0;
		if(*cmd == 0) break;
		while(*cmd == ' ') cmd++;
		while(*cmd != ' ' && *cmd != 0) tmp[i++] = *cmd++;
		tmp[i] = 0;

		if     (!strcmp(tmp, "-cpio"))
		{
			if(*cmd == 0) kpanic("No module name supplied after `-cpio`");
			while(*cmd == ' ') cmd++;
			cpio_name = cmd; // The name of the initrd module
			while(*cmd != ' ' && *cmd != 0) cmd++;
		}
#if defined(ARCH_X86) // Names of serial ports will likely be different on different systems
		else if(!strcmp(tmp, "-s"))     output_serial = SERIAL_COM1;
		else if(!strcmp(tmp, "-sCOM1")) output_serial = SERIAL_COM1;
		else if(!strcmp(tmp, "-sCOM2")) output_serial = SERIAL_COM2;
		else if(!strcmp(tmp, "-sCOM3")) output_serial = SERIAL_COM3;
		else if(!strcmp(tmp, "-sCOM4")) output_serial = SERIAL_COM4;
#endif
	}
}
