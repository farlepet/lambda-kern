#ifndef MOD_MODULE_H
#define MOD_MODULE_H

#include <lambda/mod/module.h>
#include <proc/elf.h>
#include <fs/kfile.h>
#include <data/llist.h>

/**
 * Loaded module information
 */
typedef struct {
    char *ident; /** Module identifier */

    lambda_mod_func_t func;

    llist_item_t list_item;
} module_entry_t;

int module_read(struct kfile *file, lambda_mod_head_t **head, uintptr_t *base, Elf32_Ehdr **elf);

int module_install(struct kfile *file);

int module_uninstall(module_entry_t *mod);

#endif