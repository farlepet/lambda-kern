/** \file multiboot.h
 *  \brief Contains structures related to multiboot-compatible bootloaders.
 *
 *  Contains structures that help with dealing with the information
 *  multiboot-compatible bootloaders (i.e. GRUB) give us.
 */

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <types.h>

#pragma pack (push, 1)

#define MBOOT_MEMINFO 1
#define MBOOT_BOOTDEV 2
#define MBOOT_CMDLINE 4
#define MBOOT_MODULES 8
#define MBOOT_SYMBOLS (16 | 32)
#define MBOOT_MEMMAP  64
#define MBOOT_DRIVES  128
#define MBOOT_CONFIG  256
#define MBOOT_LOADNAM 512
#define MBOOT_APMTAB  1024
#define MBOOT_VBEINFO 2046

struct multiboot_header
{
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	uint32_t cmdline;
	uint32_t mod_count;
	uint32_t mod_addr;

	uint8_t syms[16];

	uint32_t mmap_len;
	uint32_t mmap_addr;
	uint32_t drives_len;
	uint32_t drives_addr;
	uint32_t config_table;
	uint32_t bootloader_name;
	uint32_t apm_table;

	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint16_t vbe_mode;
	uint16_t vbe_seg;
	uint16_t vbe_off;
	uint16_t vbe_len;
};

struct mboot_module
{
	uint32_t mod_start;
	uint32_t mod_end;
	uint32_t string;
	uint32_t pad;
};

#pragma pack (pop)


/**
 * Checks the kernel commandline and does things accordingly.
 *
 * @param mboot_head pointer to the multiboot tags header
 */
void check_commandline(struct multiboot_header *mboot_head);

#endif