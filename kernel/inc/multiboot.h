/** \file multiboot.h
 *  \brief Contains structures related to multiboot-compatible bootloaders.
 *
 *  Contains structures that help with dealing with the information
 *  multiboot-compatible bootloaders (i.e. GRUB) give us.
 */

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <types.h>

/**
 * \brief ELF information given by bootloader
 * ELF file information as presented by the multiboot structure.
 * @see multiboot
 */
struct multiboot_elf_section_header_table
{
	u32 num;
	u32 size;
	u32 addr;
	u32 shndx;
};

/**
 * \brief A structure defining the information the bootloader passes us.
 * Details the information structure given to us by a multiboot-compatible
 * bootloader.
 */
struct multiboot
{
	u32 flags;       //!< Multiboot version
	u32 mem_lower;   //!< Available lower memory
	u32 mem_upper;   //!< Available lower memory
	u32 boot_device; //!< The device the kernel booted from
	u32 cmdline;     //!< Command line that was passed to the kernel
	u32 mods_count;  //!< Number of modules in memory
	u32 mods_addr;   //!< Address of pointer to first module
	struct multiboot_elf_section_header_table elf_head_table;
	u32 mmap_length; //!< Number of memory map entries
	u32 mmap_addr;   //!< Address of first memory map entry
} __packed;

/**
 * \brief Structure of the memory map.
 * Structure of the memory map given to us in the multiboot structure
 * by multiboot-compatible bootloaders
 * @see multiboot
 */
struct multiboot_memory_map
{
	u32 size;
	u32 base_addr_low;  //!< Lower 32 bits of memory address
	u32 base_addr_high; //!< Upper 32 bits of memory address
	u32 length_low;     //!< Lower 32 bits of region length
	u32 length_high;    //!< Upper 32 bits of region length
	u32 type;           //!< Type of memory region
} __packed;

/**
 * \brief Struture of an entry in the list of modules.
 * Structure of the module list the the multiboot header provides us.
 * @see multiboot
 */
struct multiboot_mod_list
{
	u32 mod_start; //!< Start address of module
	u32 mod_end;   //!< End address of module
	u32 cmdline;   //!< Address of module command line
	u32 pad;       //!< Padding to make this 16 bytes
} __packed;

#endif