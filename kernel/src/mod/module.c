#include <mod/module.h>
#include <proc/elf.h>
#include <sys/stat.h>
#include <mm/alloc.h>
#include <fs/fs.h>
#include <video.h>
#include <string.h>

static llist_t loaded_modules;

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

int _check_requirements(lambda_mod_head_t *mod_head, Elf32_Ehdr *mod_elf) {
    if(!mod_head->metadata.requirements) {
        /* No requirements, auto-pass */
        return 0;
    }
    
    llist_iterator_t iter;
    module_entry_t  *mod;

    /* TODO: Extract, or relocate, such data prior to arriving here */
    char **reqs = (char **)elf_find_data(mod_elf, (uintptr_t)mod_head->metadata.requirements);
    /* TODO: Potentially enforce maximum number of requirements */
    for(size_t i = 0; reqs[i]; i++) {
        char *req = (char *)elf_find_data(mod_elf, (uintptr_t)reqs[i]);

        int found = 0;
        llist_iterator_init(&loaded_modules, &iter);
        while(!found && llist_iterate(&iter, (void **)&mod)) {
            if(!strcmp(req, mod->ident)) {
                found = 1;
            }
        }
        if(!found) {
            /* Requirement not satisfied */
            return -1;
        }
    }

    return 0;
}

int module_install(struct kfile *file) {

    lambda_mod_head_t *mod_head;
    uintptr_t          mod_base;
    Elf32_Ehdr        *mod_elf;
    
    if(module_read(file, &mod_head, &mod_base, &mod_elf)) {
        return -1;
    }
    if(_check_requirements(mod_head, mod_elf)) {
        return -1;
    }
    /* TODO */
    
    /* TODO: Allocate string size within */
    module_entry_t *modent = (module_entry_t *)kmalloc(sizeof(module_entry_t));
    memset(modent, 0, sizeof(module_entry_t));

    llist_append(&loaded_modules, &modent->list_item);

    return 0;
}

int module_uninstall(module_entry_t *mod) {
    llist_remove(&loaded_modules, &mod->list_item);
    
    kfree(mod);

    return 0;
}
