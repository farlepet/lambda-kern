#include <multiboot.h>
#include <err/panic.h>
#include <err/error.h>
#include <fs/initrd.h>
#include <string.h>
#include <video.h>

#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
#  include <arch/io/serial.h>
#  include <arch/mm/paging.h>
#endif

volatile boot_options_t boot_options = {
	.init_ramdisk_name = "",
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
	.init_executable   = "/bin/linit",
#else
	/* TODO: FS not fully implemented on other platforms. */
	.init_executable   = "",
#endif
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
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
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86) // Names of serial ports will likely be different on different systems
		else if(!strcmp(tmp, "-s"))     boot_options.output_serial = SERIAL_COM1;
		else if(!strcmp(tmp, "-sCOM1")) boot_options.output_serial = SERIAL_COM1;
		else if(!strcmp(tmp, "-sCOM2")) boot_options.output_serial = SERIAL_COM2;
		else if(!strcmp(tmp, "-sCOM3")) boot_options.output_serial = SERIAL_COM3;
		else if(!strcmp(tmp, "-sCOM4")) boot_options.output_serial = SERIAL_COM4;
#endif
	}
}

void check_multiboot_modules(struct multiboot_header *mboot_head) {
	kerror(ERR_BOOTINFO, "Loading GRUB modules");

	if(!strlen((const char *)boot_options.init_ramdisk_name)) {
		kerror(ERR_BOOTINFO, "  -> No initrd module specified.");
		return;
	}

	if(!(mboot_head->flags & MBOOT_MODULES)) {
		kerror(ERR_BOOTINFO, "  -> No modules to load");
		return;
	}

	kerror(ERR_BOOTINFO, "Looking for initrd module [%s]", boot_options.init_ramdisk_name);

	struct mboot_module *mod = (struct mboot_module *)mboot_head->mod_addr;
	uint32_t modcnt = mboot_head->mod_count;

	for(uint32_t i = 0; i < modcnt; i++) {
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
		uintptr_t mod_start = (uintptr_t)mod->mod_start;
		uintptr_t mod_end   = (uintptr_t)mod->mod_end;

		uint32_t b = ((mod_start - (uint32_t)firstframe) / 0x1000);
		for(; b < ((mod_end - (uint32_t)firstframe) / 0x1000) + 1; b++) {
			set_frame(b, 1); // Make sure that the module is not overwritten
			map_page((b * 0x1000) + firstframe, (b * 0x1000) + firstframe, 3);
		}
#endif

		kerror(ERR_BOOTINFO, "  -> MOD[%d/%d]: %s", i+1, modcnt, mod->string);
	
#if (!FEATURE_INITRD_EMBEDDED)
		if(!strcmp((char *)mod->string, (const char *)boot_options.init_ramdisk_name)) {
			initrd_mount(fs_root, mod->mod_start, mod->mod_end - mod->mod_start);
		}
#endif /* (!FEATURE_INITRD_EMBEDDED) */
		mod++;
	}
}
