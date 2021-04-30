#ifndef DRV_MODULE_H
#define DRV_MODULE_H

#include <lambda/mod/module.h>
#include <proc/elf.h>
#include <fs/kfile.h>

int module_read(struct kfile *file, lambda_mod_head_t **head, uintptr_t *base, Elf32_Ehdr **elf);

#endif