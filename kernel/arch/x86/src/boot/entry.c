#include <types.h>
#include <string.h>

#include <io/output.h>
#include <kern/cmdline.h>
#include <main/main.h>
#include <err/panic.h>
#include <fs/fs.h>

#include <arch/init/init.h>
#include <arch/io/serial.h>
#include <arch/boot/multiboot.h>
#include <arch/mm/paging.h>
#include <arch/types/mmu.h>
#include <arch/registers.h>

#if (CONFIG_MULTIBOOT_VERSION == 2)
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
#elif (CONFIG_MULTIBOOT_VERSION == 1)
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
__noreturn void kern_premap(mboot_t *, uint32_t);

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
    output_set_dev(&serial1);

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

/* @todo Clean all this up, once it is confirmed working */

__section(.boot.bss) __align(PAGE_SZ)
static uint32_t __init_pagedir[PGDIR_ENTRIES];
__section(.boot.bss) __align(PAGE_SZ)
static uint32_t __init_pagetables[PGTBL_ENTRIES];

__used
__align(8)
static uint8_t  __newstack[0x2000];
__used
static mboot_t *__mboot_head;
__used
static uint32_t __mboot_magic;

__section(.boot.text)
__noreturn void kern_premap(mboot_t *mboot_head, uint32_t magic) {
    uintptr_t virt_addr = (uintptr_t)&kern_start;

    /* Clear out page directory */
    for(int i = 0; i < PGDIR_ENTRIES; i++) {
        __init_pagedir[i] = 0;
    }

    /* Temporarially map lower 4 MiB */
    for(int i = 0; i < PGTBL_ENTRIES; i++) {
        __init_pagetables[i] = (i * PAGE_SZ)           |
                               PAGE_TABLE_FLAG_PRESENT |
                               PAGE_TABLE_FLAG_WRITABLE;
    }

    __init_pagedir[0] = ((uint32_t)__init_pagetables) |
                        PAGE_DIRECTORY_FLAG_PRESENT   |
                        PAGE_DIRECTORY_FLAG_WRITABLE;

    __init_pagedir[(virt_addr >> 22)] = ((uint32_t)__init_pagetables) |
                                        PAGE_DIRECTORY_FLAG_PRESENT   |
                                        PAGE_DIRECTORY_FLAG_WRITABLE;
    
    register_cr3_write((uint32_t)&__init_pagedir);

    uint32_t cr0 = register_cr0_read();
    cr0 |= CR0_FLAG_PG;
    register_cr0_write(cr0);

    __mboot_head  = mboot_head;
    __mboot_magic = magic;

    asm volatile("mov $(__newstack + 0x2000), %esp\n"
                 "mov %esp, %ebp\n"
                 "push __mboot_magic\n"
                 "push __mboot_head\n"
                 "call kentry\n");

    kentry(mboot_head, magic);
}
