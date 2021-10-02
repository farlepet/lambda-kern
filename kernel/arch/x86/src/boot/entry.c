#include <types.h>

#include <main/main.h>
#include <err/panic.h>
#include <fs/fs.h>

#include <arch/init/init.h>
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

/**
 * Kernel entry-point: performs target-specific system initialization.
 */
__noreturn void kentry(mboot_t *mboot_head, uint32_t magic) {
	if(magic != MBOOT_MAGIC)
		kpanic("Invalid magic number given by the bootloader: 0x%08X", magic);
	
	multiboot_check_commandline(mboot_head);

    arch_init(mboot_head);
	
    fs_init();

	multiboot_check_modules(mboot_head);

    kmain();
}
