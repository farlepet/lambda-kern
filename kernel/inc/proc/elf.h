#ifndef PROC_ELF_H
#define PROC_ELF_H

#include <stdint.h>

#include <mm/symbols.h>
#include <proc/types/exec.h>
#include <proc/types/elf.h>

int elf_find_section(const Elf32_Ehdr *elf, Elf32_Shdr **section, const char *section_name);

uintptr_t elf_find_data(const Elf32_Ehdr *elf, uintptr_t addr);

int elf_check_header(void *data);

int elf_load_symbols(const Elf32_Ehdr *elf, symbol_t **symbols);

int exec_elf(exec_data_t *exec_data);

#endif
