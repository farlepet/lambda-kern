/** \file multiboot.h
 *  \brief Contains structures related to multiboot-compatible bootloaders.
 *
 *  Contains structures that help with dealing with the information
 *  multiboot-compatible bootloaders (i.e. GRUB) give us.
 */

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <types.h>

/// Information about the multiboot tags
struct multiboot_header_tag
{
	u32 size;     //!< Total size of multiboot tags
	u32 reserved; //!< Always 0
};

/// General structure of a multiboot tag
struct multiboot_tag
{
	u32 type; //!< Type of multiboot tag
	u32 size; //!< Size of multiboot tag
};

/// Amount of consecutive memory available
struct multiboot_basic_memory_tag
{
	u32 type;      //!< Always 4
	u32 size;      //!< Always 16
	u32 mem_lower; //!< Usable memory below 1MiB in KiB
	u32 mem_upper; //!< Amount of consecutive usable momory in KiB
};

/// The device the kernel booted from
struct multiboot_boot_device_tag
{
	u32 type;          //!< Always 5
	u32 size;          //!< Always 20
	u32 biosdev;       //!< BIOS drive number
	u32 partition;     //!< Top-level partition number
	u32 sub_partition; //!< Sub-partition number
};

/// The commandline given to the kernel
struct multiboot_cmdline_tag
{
	u32 type;      //!< Always 1
	u32 size;      //!< Size of tag
	u8  cmdline[]; //!< C-style string containing command line
};

/// Any modules specified to be loaded
struct multiboot_module_tag
{
	u32   type;      //!< Always 3
	u32   size;      //!< Size of tag
	void *mod_start; //!< Start address of module
	void *mod_end;   //!< End address of module
	u8    name[];    //!< Name of module
};

/// Information on the ELF headers
struct multiboot_elf_headers_tag
{
	u32 type;     //!< Always 9
	u32 size;     //!< Size of tag
	u16 num;
	u16 entsize;
	u16 shndx;
	u16 reserved;
	// Section headers are at end of this tag
};

/// Map of system memory
struct multiboot_memory_map_tag
{
	u32 type;          //!< Always 6
	u32 size;          //!< Size of tag
	u32 entry_size;    //!< Size of each entry (Always a multiple of 8)
	u32 entry_version; //!< Version of the entries
	// Entries continue on from here
};

/// Entry used by multiboot_memory_map_tag
struct multiboot_memory_map_entry
{
	u64 base_addr; //!< Starting physical address of memory region
	u64 length;    //!< Size of memory region in bytes
	u32 type;      //!< Type of address region: 1 == Usable RAM, 2 == ACPI imformation, 3 == Must be preserved upon hibernation
	u32 reserved;  //!< Always 0
};

/// Name of bootloader kernel loaded from
struct multiboot_bootloader_name_tag
{
	u32 type;   //!< Always 2
	u32 size;   //!< Size of tag
	u8  name[]; //!< Name of bootloader
};

/// APM information table
struct multiboot_apm_table_tag
{
	u32 type;        //!< Always 10
	u32 size;        //!< Always 28
	u16 version;     //!< Version
	u16 cseg;        //!< Protected mode 32-bit code segment
	u32 offset;      //!< Entry point offset
	u16 cseg_16;     //!< Protected mode 16-bit code segment
	u16 dseg;        //!< Protected mode 16-bit date segment
	u16 flags;       //!< Flags
	u16 cseg_len;    //!< Length of the protected mode 32-bit code segment
	u16 cseg_16_len; //!< Length of the protected mode 16-bit code segment
	u16 dseg_len;    //!< Length of the protected mode 16-bit data segment
};

/// VBE information
struct multiboot_vbe_tag
{
	u32 type;                  //!< Always 7
	u32 size;                  //!< Always 784
	u16 vbe_mode;              //!< Current video mode
	u16 vbe_interface_seg;     //!< VBE interface table segment
	u16 vbe_interface_off;     //!< VBE interface table offset
	u16 vbe_interface_len;     //!< VBE interface table length
	u8  vbe_control_info[512]; //!< VBE control information
	u8  vbe_mode_info[256];    //!< VBE mode information
};




/**
 * \brief Finds the first multiboot entry with a certain type value.
 * Finds the first multiboot entry with a certain type value, then returns it
 * as a multiboot_tag.
 * @param mboot_tag the pointer to the multiboot tags header
 * @param type the type value to look for
 * @see multiboot_tag
 */
struct multiboot_tag *find_multiboot_table(struct multiboot_header_tag* mboot_tag, u32 type);

#endif