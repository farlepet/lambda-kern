#include <drv/driver.h>
#include <proc/elf.h>
#include <sys/stat.h>
#include <mm/alloc.h>
#include <fs/fs.h>
#include <video.h>

int driver_read(struct kfile *file, lambda_drv_head_t **head, uintptr_t *base) {
    Elf32_Ehdr        *elf_data;
    Elf32_Shdr        *drv_section;
    lambda_drv_head_t *drv_head;
    struct stat        file_stat;

    if(!file) {
        return 1;
    }

	if(kfstat(file, &file_stat)) {
        return 1;
    }
    elf_data = (Elf32_Ehdr *)kmalloc(file_stat.st_size);
    if(!elf_data) {
        return 1;
    }
    if(fs_read(file, 0, file_stat.st_size, (void *)elf_data) != file_stat.st_size) {
        return 1;
    }

    if(elf_find_section(elf_data, &drv_section, LAMBDA_DRV_SECTION_NAME)) {
        return 1;
    }

    drv_head = (lambda_drv_head_t *)((uintptr_t)elf_data + drv_section->sh_offset);
    if(drv_head->head_magic != LAMBDA_DRV_HEAD_MAGIC) {
        return 1;
    }

    *head = drv_head;
    *base = drv_section->sh_addr;

    /* TODO: Copy data, and free memory. */

    return 0;
}