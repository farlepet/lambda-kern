#include <multiboot.h>
#include <err/panic.h>
#include <err/error.h>
#include <string.h>
#include <video.h>

#if defined(ARCH_X86)
#  include <arch/io/serial.h>
#endif

volatile boot_options_t boot_options = {
	.init_ramdisk_name = "",
	.init_executable   = "/bin/linit",
#if defined(ARCH_X86)
	.output_serial     = 0,
#endif
};

/**
 * Checks the kernel commandline and does things accordingly.
 *
 * @param mboot_tag pointer to the multiboot tags header
 */
void check_commandline(struct multiboot_header *mboot_head) {
	if(!(mboot_head->flags & MBOOT_CMDLINE)) {
		kerror(ERR_BOOTINFO, "No commandline provided");
		return;
	}
	
	char *cmd = (char *)mboot_head->cmdline;
	char *end = cmd + strlen(cmd) + 1;
	char tmp[1000];
	
	while(cmd < end) {
		kerror(ERR_BOOTINFO, "CMD: %08X END: %08X", cmd, end);
		int i = 0;
		if(*cmd == 0) break;
		while(*cmd == ' ') cmd++;
		while(*cmd != ' ' && *cmd != 0) tmp[i++] = *cmd++;
		tmp[i] = 0;
		
		kerror(ERR_BOOTINFO, "  cmd: %s", tmp);

		if       (!strcmp(tmp, "-cpio")) {
			/* Load CPIO ramdisk */
			while(*cmd == ' ') cmd++;
			if(*cmd == 0) kpanic("No module name supplied after `-cpio`");
			
			size_t j = 0;
			for(; (cmd[j] && cmd[j] != ' ' && j < (INITRD_MODULE_MAX_LEN - 1)); j++) {
				boot_options.init_ramdisk_name[j] = cmd[j];
			}
			boot_options.init_ramdisk_name[j] = '\0';
			
			while(*cmd && *cmd != ' ') cmd++;
		} else if(!strcmp(tmp, "-init")) {
			/* Specify init executable */
			while(*cmd == ' ') cmd++;
			if(*cmd == 0) kpanic("No executable name supplied after `-init`");
			
			size_t j = 0;
			for(; (cmd[j] && cmd[j] != ' ' && j < (INITEXEC_PATH_MAX_LEN - 1)); j++) {
				boot_options.init_executable[j] = cmd[j];
			}
			boot_options.init_executable[j] = '\0';
			
			while(*cmd && *cmd != ' ') cmd++;
		} else if(!strcmp(tmp, "-kterm")) {
			/* Launch the kterm shell rather than spawning the init executable */
			boot_options.init_executable[0] = '\0';
		}
#if defined(ARCH_X86) // Names of serial ports will likely be different on different systems
		else if(!strcmp(tmp, "-s"))     boot_options.output_serial = SERIAL_COM1;
		else if(!strcmp(tmp, "-sCOM1")) boot_options.output_serial = SERIAL_COM1;
		else if(!strcmp(tmp, "-sCOM2")) boot_options.output_serial = SERIAL_COM2;
		else if(!strcmp(tmp, "-sCOM3")) boot_options.output_serial = SERIAL_COM3;
		else if(!strcmp(tmp, "-sCOM4")) boot_options.output_serial = SERIAL_COM4;
#endif
	}
}
