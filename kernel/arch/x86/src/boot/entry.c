#include <types.h>
#include <string.h>

#include <video.h>
#include <kern/cmdline.h>
#include <main/main.h>
#include <err/panic.h>
#include <fs/fs.h>

#include <arch/init/init.h>
#include <arch/io/serial.h>
#include <arch/boot/multiboot.h>

#if (FEATURE_MULTIBOOT == 2)
__section(.mboot)
__align(8)
__used
static mboot_head_t _boot_head = {
	.magic         = MBOOT_HEAD_MAGIC,
	.architecture  = MBOOT_HEAD_ARCHITECTURE_X86,
	.header_length = sizeof(_boot_head),
	.checksum      = -(MBOOT_HEAD_MAGIC + MBOOT_HEAD_ARCHITECTURE_X86 + sizeof(_boot_head)),
	.tags          = (_tags_t) {
		.inforeq = {
			.type  = 1,
			.flags = 0,
			.size  = sizeof(_boot_head.tags.inforeq), 
			.tags  = {
				MBOOT_TAGTYPE_CMDLINE,
				MBOOT_TAGTYPE_MODULE,
				MBOOT_TAGTYPE_BASIC_MEMINFO,
				MBOOT_TAGTYPE_MMAP,
				MBOOT_TAGTYPE_ACPI_OLD,
				MBOOT_TAGTYPE_ACPI_NEW,
			}
		},
		.end = {
			.type  = 0,
			.flags = 0,
			.size  = 8
		}
	}
};
#elif (FEATURE_MULTIBOOT == 1)
__section(.mboot)
__align(8)
__used
static mboot_head_t _boot_head = {
	.magic = MBOOT_HEAD_MAGIC,
	.flags = 0x03,
	.checksum = -(MBOOT_HEAD_MAGIC + 0x03)
};
#endif

__noreturn void kentry(mboot_t *, uint32_t);

/* TODO: Move this elsewhere, or dynamically allocate */
static hal_io_char_dev_t serial1;

/**
 * Kernel entry-point: performs target-specific system initialization.
 */
__noreturn void kentry(mboot_t *mboot_head, uint32_t magic) {
	if(magic != MBOOT_MAGIC)
		kpanic("Invalid magic number given by the bootloader: 0x%08X", magic);
	
    serial_init(SERIAL_COM1);
	serial_create_chardev(SERIAL_COM1, &serial1);
	kput_char_dev = &serial1;
	
	multiboot_check_commandline(mboot_head);

    arch_init(mboot_head);
	
	const char *ser = cmdline_getstr("serial");
	if(ser != NULL) {
		if(!strcmp(ser, "COM1"))      boot_options.output_serial = SERIAL_COM1;
		else if(!strcmp(ser, "COM2")) boot_options.output_serial = SERIAL_COM2;
		else if(!strcmp(ser, "COM3")) boot_options.output_serial = SERIAL_COM3;
		else if(!strcmp(ser, "COM4")) boot_options.output_serial = SERIAL_COM4;
        else                          boot_options.output_serial = SERIAL_COM1;
	} else if(cmdline_getbool("serial")) {
		boot_options.output_serial = SERIAL_COM1;
	}

    fs_init();

	multiboot_check_modules(mboot_head);

    kmain();
}
