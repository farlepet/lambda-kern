#include <mod/module.h>
#include <proc/elf.h>
#include <sys/stat.h>
#include <mm/alloc.h>
#include <fs/fs.h>
#include <video.h>

int module_read(struct kfile *file, lambda_mod_head_t **head, uintptr_t *base, Elf32_Ehdr **elf) {
    Elf32_Ehdr        *elf_data;
    Elf32_Shdr        *mod_section;
    lambda_mod_head_t *mod_head;
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

    if(elf_find_section(elf_data, &mod_section, LAMBDA_MODULE_SECTION_NAME)) {
        return 1;
    }

    mod_head = (lambda_mod_head_t *)((uintptr_t)elf_data + mod_section->sh_offset);
    if(mod_head->head_magic != LAMBDA_MODULE_HEAD_MAGIC) {
        return 1;
    }

    *head = mod_head;
    *base = mod_section->sh_addr;

    if(elf) {
        *elf = elf_data;
    } else {
        /* TODO: Copy data, and free memory. */
    }

    return 0;
}