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
	u32 flags;
	u32 mem_lower;
	u32 mem_upper;
	u32 boot_device;
	u32 cmdline;
	u32 mod_count;
	u32 mod_addr;

	u8 syms[16];

	u32 mmap_len;
	u32 mmap_addr;
	u32 drives_len;
	u32 drives_addr;
	u32 config_table;
	u32 bootloader_name;
	u32 apm_table;

	u32 vbe_control_info;
	u32 vbe_mode_info;
	u16 vbe_mode;
	u16 vbe_seg;
	u16 vbe_off;
	u16 vbe_len;
};

struct mboot_module
{
	u32 mod_start;
	u32 mod_end;
	u32 string;
	u32 pad;
};

#pragma pack (pop)


/**
 * Checks the kernel commandline and does things accordingly.
 *
 * @param mboot_head pointer to the multiboot tags header
 */
void check_commandline(struct multiboot_header *mboot_head);

#endif