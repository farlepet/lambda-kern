#include <mod/symbols.h>
#include <mod/module.h>
#include <err/error.h>
#include <proc/proc.h>
#include <proc/elf.h>
#include <sys/stat.h>
#include <mm/alloc.h>
#include <fs/fs.h>
#include <video.h>
#include <string.h>

#if defined(ARCH_X86)
#  include <arch/mm/paging.h>
#endif

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
        kfree(elf_data);
        return 1;
    }

    if(elf_check_header(elf_data)) {
        return 1;
    }

    if(elf_find_section(elf_data, &mod_section, LAMBDA_MODULE_SECTION_NAME)) {
        kfree(elf_data);
        return 1;
    }

    mod_head = (lambda_mod_head_t *)((uintptr_t)elf_data + mod_section->sh_offset);
    if(mod_head->head_magic != LAMBDA_MODULE_HEAD_MAGIC) {
        kfree(elf_data);
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

static int _read_phdr(const Elf32_Ehdr *elf, proc_elf_data_t *elf_data) {
    /* TODO: Merge this with elf_read_phdr */
	
	Elf32_Phdr *prog = (Elf32_Phdr *)((uintptr_t)elf + elf->e_phoff);

	elf_data->dynamic = NULL;

	for(size_t i = 0; i < elf->e_phnum; i++) {
		kdebug(DEBUGSRC_MODULE, "phdr[%2X/%2X] T:%X VADDR: %08X MSZ:%08X FSZ:%08X", i+1, elf->e_phnum, prog[i].p_type, prog[i].p_vaddr, prog[i].p_memsz, prog[i].p_filesz);
		switch(prog[i].p_type) {
			case PT_LOAD: {
				/* TODO: Read entire header first, so we can avoid issues with
				 * overlapping memory regions. Or create more sophisticated mmap
				 * system. */
				/* Allocate physical memory */
				void *phys;
				if(prog[i].p_memsz & 0xFFF) {
					phys = kmamalloc(prog[i].p_memsz + 0x1000, 0x1000); // + 0x1000 so we can align it
					phys = phys + (prog[i].p_vaddr & 0xFFF);
				} else {
					phys = kmamalloc(prog[i].p_memsz, 0x1000);
				}

				/* Copy data and/or clear memory */
				if(prog[i].p_filesz) {
					memcpy(phys, (void *)((uintptr_t)elf + prog[i].p_offset), prog[i].p_filesz);
				}
				if(prog[i].p_filesz < prog[i].p_memsz) {
					memset(phys + prog[i].p_filesz, 0, prog[i].p_memsz - prog[i].p_filesz);
				}

				/* Create MMAP entry: */
				/**mmap_next = (struct kproc_mem_map_ent *)kmalloc(sizeof(struct kproc_mem_map_ent));
				(*mmap_next)->virt_address = prog[i].p_vaddr;
				(*mmap_next)->phys_address = (uintptr_t)phys;
				(*mmap_next)->length       = prog[i].p_memsz;
				(*mmap_next)->next         = NULL;
				mmap_next = &((*mmap_next)->next);*/

#if defined(ARCH_X86)
				/* TODO: Create architecture-independant memory mapping mechanism. */
				uintptr_t start_v = prog[i].p_vaddr & 0xFFFFF000;
                uintptr_t end_v   = ((prog[i].p_vaddr + prog[i].p_memsz) & 0xFFFFF000) | 0xFFF;
                uintptr_t start_p = (uintptr_t)phys & 0xFFFFF000;
                for(uintptr_t pg = 0; (start_v + pg) < end_v; pg += 0x1000) {
					map_page((void *)(start_p + pg), (void *)(start_v + pg), 0x07);
				}
#else
	(void)arch_params;
#endif
			} break;
			case PT_DYNAMIC:
				elf_data->dynamic = (Elf32_Dyn *)prog[i].p_vaddr;
				break;
		}
	}

    return 0;
}

static int _read_relocate(const Elf32_Ehdr *elf) {
    Elf32_Shdr *elf_relplt = NULL;
    Elf32_Shdr *elf_symtab = NULL;
    Elf32_Shdr *elf_strtab = NULL;
    if(elf_find_section(elf, &elf_relplt, ".rel.plt") ||
       elf_find_section(elf, &elf_symtab, ".dynsym") ||
       elf_find_section(elf, &elf_strtab, ".dynstr")) {
        return -1;
    }

	Elf32_Sym *syms   = (Elf32_Sym *)((uintptr_t)elf + elf_symtab->sh_offset);
    char      *strs   = (char *)((uintptr_t)elf + elf_strtab->sh_offset);
    Elf32_Rel *rel    = (Elf32_Rel *)((uintptr_t)elf + elf_relplt->sh_offset);
    size_t     n_rels = elf_relplt->sh_size / elf_relplt->sh_entsize;
    for(size_t i = 0; i < n_rels; i++) {
        size_t sidx = ELF32_R_SYM(rel[i].r_info);
        char *ident = &strs[syms[sidx].st_name];
        kdebug(DEBUGSRC_MODULE, "_read_relocate[%d]: %08X, %08X [%s]",
               i, rel[i].r_offset, rel[i].r_info, ident);

        uintptr_t symaddr = 0;
        if(module_symbol_find(ident, &symaddr)) {
            kdebug(DEBUGSRC_MODULE, "_read_relocate: Could not find symbol %s", ident);
            return -1;
        }

        switch(ELF32_R_TYPE(rel[i].r_info)) {
            case R_386_GLOB_DAT:
            case R_386_JMP_SLOT:
                *((uint32_t *)rel[i].r_offset) = symaddr;
                break;
            
            default:
                kdebug(DEBUGSRC_MODULE, "_read_relocate: Unhandled relocation type: %d", ELF32_R_TYPE(rel[i].r_info));
                return -1;
        }
    }

    return 0;
}

int module_install(struct kfile *file) {
    lambda_mod_head_t *mod_head;
    uintptr_t          mod_base;
    Elf32_Ehdr        *mod_elf;
    symbol_t          *symbols;
    
    kdebug(DEBUGSRC_MODULE, "module_install: Reading module");
    if(module_read(file, &mod_head, &mod_base, &mod_elf)) {
        return -1;
    }
    kdebug(DEBUGSRC_MODULE, "module_install: Checking requirements");
    if(_check_requirements(mod_head, mod_elf)) {
        kfree(mod_elf);
        return -1;
    }
    
    kdebug(DEBUGSRC_MODULE, "module_install: Reading symbol table");
    if(elf_load_symbols(mod_elf, &symbols)) {
        kfree(mod_elf);
        return -1;
    }

    kdebug(DEBUGSRC_MODULE, "module_install: Reading program header");
    proc_elf_data_t elf_data;
    if (_read_phdr(mod_elf, &elf_data)) {
        kfree(mod_elf);
        kfree(symbols);
        return -1;
    }
    
    kdebug(DEBUGSRC_MODULE, "module_install: Handling relocations");
    if (_read_relocate(mod_elf)) {
        /* TODO: Clear allocated memory from phdr */
        kfree(mod_elf);
        kfree(symbols);
        return -1;
    }
    /* TODO */

    mod_head->function(LAMBDA_MODFUNC_START, NULL);
    
    /* TODO: Allocate string size within */
    kdebug(DEBUGSRC_MODULE, "module_install: Generating module entry");
    module_entry_t *modent = (module_entry_t *)kmalloc(sizeof(module_entry_t));
    memset(modent, 0, sizeof(module_entry_t));

    kdebug(DEBUGSRC_MODULE, "module_install: Adding module to list");
    llist_append(&loaded_modules, &modent->list_item);

    return 0;
}

int module_uninstall(module_entry_t *mod) {
    llist_remove(&loaded_modules, &mod->list_item);
    
    kfree(mod);

    return 0;
}
