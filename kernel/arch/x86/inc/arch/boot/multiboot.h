/** \file multiboot.h
 *  \brief Contains structures related to multiboot-compatible bootloaders.
 *
 *  Contains structures that help with dealing with the information
 *  multiboot-compatible bootloaders (i.e. GRUB) give us.
 */

#ifndef ARCH_BOOT_MULTIBOOT_H
#define ARCH_BOOT_MULTIBOOT_H

#include <stddef.h>
#include <stdint.h>

#include <arch/acpi/acpi_data.h>

#pragma pack (push, 1)

#if (CONFIG_MULTIBOOT_VERSION == 1)
#  define MBOOT_MAGIC (0x2BADB002UL)

#  define MBOOT_MEMINFO 1
#  define MBOOT_BOOTDEV 2
#  define MBOOT_CMDLINE 4
#  define MBOOT_MODULES 8
#  define MBOOT_SYMBOLS (16 | 32)
#  define MBOOT_MEMMAP  64
#  define MBOOT_DRIVES  128
#  define MBOOT_CONFIG  256
#  define MBOOT_LOADNAM 512
#  define MBOOT_APMTAB  1024
#  define MBOOT_VBEINFO 2046

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mod_count;
    uint32_t mod_addr;

    uint8_t  syms[16];

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
} mboot_t;

typedef struct {
    uint32_t mod_start;
    uint32_t mod_end;
    uint32_t string;
    uint32_t pad;
} mboot_module_t;

typedef struct {
    uint32_t magic;
#  define MBOOT_HEAD_MAGIC (0x1BADB002UL)
    uint32_t flags;
    uint32_t checksum;
} mboot_head_t;

#elif (CONFIG_MULTIBOOT_VERSION == 2)
#  define MBOOT_MAGIC (0x36D76289UL)

typedef struct {
    uint32_t size;
    uint32_t _reserved;
    uint8_t  tags[];
} mboot_t;

typedef struct {
    struct {
        uint16_t type;
        uint16_t flags;
        uint32_t size;
        uint32_t tags[6];
    } inforeq;
    struct {
        uint16_t type;
        uint16_t flags;
        uint32_t size;
    } end;
} _tags_t;

typedef struct {
    uint32_t magic;
#  define MBOOT_HEAD_MAGIC (0xE85250D6UL)
    uint32_t architecture;
#  define MBOOT_HEAD_ARCHITECTURE_X86    (0UL)
#  define MBOOT_HEAD_ARCHITECTURE_MIPS32 (4UL)
    uint32_t header_length;
    uint32_t checksum;
    _tags_t  tags;
} mboot_head_t;

typedef enum {
    MBOOT_TAGTYPE_END             = 0,
    MBOOT_TAGTYPE_CMDLINE         = 1,
    MBOOT_TAGTYPE_BOOTLOADER_NAME = 2,
    MBOOT_TAGTYPE_MODULE          = 3,
    MBOOT_TAGTYPE_BASIC_MEMINFO   = 4,
    MBOOT_TAGTYPE_BOOTDEV         = 5,
    MBOOT_TAGTYPE_MMAP            = 6,
    MBOOT_TAGTYPE_VBE             = 7,
    MBOOT_TAGTYPE_FRAMEBUFFER     = 8,
    MBOOT_TAGTYPE_ELF_SECTIONS    = 9,
    MBOOT_TAGTYPE_APM             = 10,
    MBOOT_TAGTYPE_EFI32           = 11,
    MBOOT_TAGTYPE_EFI64           = 12,
    MBOOT_TAGTYPE_SMBIOS          = 13,
    MBOOT_TAGTYPE_ACPI_OLD        = 14,
    MBOOT_TAGTYPE_ACPI_NEW        = 15,
    MBOOT_TAGTYPE_NETWORK         = 16,
    MBOOT_TAGTYPE_EFI_MMAP        = 17,
    MBOOT_TAGTYPE_EFI_BS          = 18,
    MBOOT_TAGTYPE_EFI21_IH        = 19,
    MBOOT_TAGTYPE_EFI64_IH        = 20,
    MBOOT_TAGTYPE_LOAD_BASE_ADDR  = 21
} mboot_tag_type_e;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint8_t  data[];
} mboot_tag_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    char     cmdline[];
} mboot_tag_cmdline_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
    char     name[];
} mboot_tag_module_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t size_lower;
    uint32_t size_upper;
} mboot_tag_basicmem_t;

/* Note: Same format is used for old and new, but the length of the structure
 * will differ. */
typedef struct {
    uint32_t type;
    uint32_t size;
    acpi_rsdp_desc_t rsdp;
} mboot_tag_acpi_rsdp_t;
#endif


#pragma pack (pop)

/**
 * Checks the kernel commandline and does things accordingly.
 *
 * @param head pointer to the multiboot tags header
 */
void multiboot_check_commandline(const mboot_t *head);

void multiboot_check_modules(const mboot_t *head);

void multiboot_locate_modules(const mboot_t *head, uintptr_t *start, uintptr_t *end);

size_t multiboot_get_upper_memory(const mboot_t *head);

#if (CONFIG_MULTIBOOT_VERSION == 2)
const mboot_tag_t *multiboot_find_tag(const mboot_t *head, uint32_t type, uint32_t idx);
#endif

#endif
