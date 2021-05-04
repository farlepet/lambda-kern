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

static int _module_place(const Elf32_Ehdr *elf, const lambda_mod_head_t *mod_head, uintptr_t baseaddr, module_entry_t *mod_ent, const symbol_t *symbols);

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
    
    kdebug(DEBUGSRC_MODULE, "module_install: Generating module entry");
    const char *ident = (char *)elf_find_data(mod_elf, (uintptr_t)mod_head->metadata.ident);
    module_entry_t *modent = (module_entry_t *)kmalloc(sizeof(module_entry_t) + strlen(ident) + 1);
    memset(modent, 0, sizeof(module_entry_t));
    modent->ident = (char *)((uintptr_t)modent + sizeof(module_entry_t));
    memcpy(modent->ident, ident, strlen(ident) + 1);

    uintptr_t baseaddr = 0x80000000;
    kdebug(DEBUGSRC_MODULE, "module_install: Placing and relocation module");
    if(_module_place(mod_elf, mod_head, baseaddr, modent, symbols)) {
        kfree(mod_elf);
        kfree(symbols);
        kfree(modent);
    }

    modent->func(LAMBDA_MODFUNC_START, NULL);
    

    kdebug(DEBUGSRC_MODULE, "module_install: Adding module to list");
    llist_append(&loaded_modules, &modent->list_item);

    return 0;
}

int module_uninstall(module_entry_t *mod) {
    llist_remove(&loaded_modules, &mod->list_item);
    
    kfree(mod);

    return 0;
}


typedef struct {
    uintptr_t addr_orig;
    uintptr_t addr_new;
    uintptr_t addr_phys;
    size_t    size;
} elf_reloc_t;

static uintptr_t _translate_addr(const elf_reloc_t *relocs, uintptr_t addr) {
    size_t i = 0;
    while(relocs[i].addr_new != 0xFFFFFFFF) {
        if((addr >= relocs[i].addr_orig) &&
           (addr < (relocs[i].addr_orig + relocs[i].size))) {
            return relocs[i].addr_new + (addr - relocs[i].addr_orig);
        }
        i++;
    }

    /* TODO: Possibly return 0 as error */
    return addr;
}

static uintptr_t _translate_addr_phys(const elf_reloc_t *relocs, uintptr_t addr) {
    size_t i = 0;
    while(relocs[i].addr_new != 0xFFFFFFFF) {
        if((addr >= relocs[i].addr_orig) &&
           (addr < (relocs[i].addr_orig + relocs[i].size))) {
            return relocs[i].addr_phys + (addr - relocs[i].addr_orig);
        }
        i++;
    }

    return 0;
}

static int _do_reloc(uintptr_t baseaddr, const elf_reloc_t *relocs, uintptr_t symaddr, const Elf32_Rel *rel) {
    (void)baseaddr;
    uintptr_t dataaddr = _translate_addr(relocs, rel->r_offset);
    
    kdebug(DEBUGSRC_MODULE, "_do_reloc: Writing to %08X (%08X)", dataaddr, *(uint32_t *)dataaddr);

    switch(ELF32_R_TYPE(rel->r_info)) {
        case R_386_NONE:
            break;
        case R_386_32:
            *((uint32_t *)dataaddr) += symaddr;
            break;
        case R_386_PC32:
            *((uint32_t *)dataaddr) += (symaddr - dataaddr);
            break;
        case R_386_RELATIVE:
            *((uint32_t *)dataaddr) = _translate_addr(relocs, *((uint32_t *)dataaddr));
            break;
        case R_386_GLOB_DAT:
        case R_386_JMP_SLOT:
            *((uint32_t *)dataaddr) = symaddr;
            break;
        default:
            kdebug(DEBUGSRC_MODULE, "_module_apply_relocs: Unhandled relocation type: %d", ELF32_R_TYPE(rel->r_info));
            return -1;
    }
    
    kdebug(DEBUGSRC_MODULE, "_do_reloc: Wrote %08X to %08X", *(uint32_t *)dataaddr, dataaddr);

    return 0;
}

static int _module_apply_relocs(const Elf32_Ehdr *elf, const elf_reloc_t *relocs, const symbol_t *symbols, uintptr_t baseaddr) {
    Elf32_Shdr *elf_symtab = NULL;
    Elf32_Shdr *elf_strtab = NULL;
    if(elf_find_section(elf, &elf_symtab, ".dynsym") ||
       elf_find_section(elf, &elf_strtab, ".dynstr")) {
        return -1;
    }
	Elf32_Sym *syms = (Elf32_Sym *)((uintptr_t)elf + elf_symtab->sh_offset);
    char      *strs = (char *)((uintptr_t)elf + elf_strtab->sh_offset);

    const Elf32_Shdr *sects = (Elf32_Shdr *)((uintptr_t)elf + elf->e_shoff);
    
    for(size_t i = 0; i < elf->e_shnum; i++) {
        if(sects[i].sh_type != SHT_REL) { continue; }

        const Elf32_Rel *rel    = (Elf32_Rel *)((uintptr_t)elf + sects[i].sh_offset);
        size_t           n_rels = sects[i].sh_size / sects[i].sh_entsize;
        for(size_t j = 0; j < n_rels; j++) {
            size_t sidx = ELF32_R_SYM(rel[j].r_info);
            char *ident = &strs[syms[sidx].st_name];
            kdebug(DEBUGSRC_MODULE, "_module_apply_relocs: %08X, %08X [%s]",
                   rel[j].r_offset, rel[j].r_info, ident);

            uintptr_t symaddr = 0;
            if(!module_symbol_find_kernel(ident, &symaddr)) {
            } else if(!module_symbol_find_module(ident, &symaddr, symbols)) {
                symaddr = _translate_addr(relocs, symaddr);
            } else {
                kdebug(DEBUGSRC_MODULE, "_module_apply_relocs: Could not find symbol %s", ident);
                //return -1;
            }

            if(_do_reloc(baseaddr, relocs, symaddr, &rel[j])) {
                return -1;
            }
        }
    }

    return 0;
}

static void *_alloc_map(uintptr_t virt, size_t len) {
    void *paddr = kmamalloc(len, 0x1000);
#if defined(ARCH_X86)
    /* TODO: Create architecture-independant memory mapping mechanism. */
    uintptr_t start_v =  virt & 0xFFFFF000;
    uintptr_t start_p = (uintptr_t)paddr & 0xFFFFF000;
    for(uintptr_t pg = 0; pg < len; pg += 0x1000) {
        map_page((void *)(start_p + pg), (void *)(start_v + pg), 0x07);
    }
#endif
    return paddr;
}

static int _module_place(const Elf32_Ehdr *elf, const lambda_mod_head_t *mod_head, uintptr_t baseaddr, module_entry_t *mod_ent, const symbol_t *symbols) {
    Elf32_Phdr *prog = (Elf32_Phdr *)((uintptr_t)elf + elf->e_phoff);
	
    size_t n_loads = 0;
    for(size_t i = 0; i < elf->e_phnum; i++) {
        if(prog[i].p_type == PT_LOAD) {
            n_loads++;
        }
    }
    elf_reloc_t *relocs = (elf_reloc_t *)kmalloc((n_loads+1) * sizeof(elf_reloc_t));

    size_t ridx = 0;
    /* NOTE: This is assuming that the binary addresses relative to zero */
    /* Find the most effecient way to allocate and map required memory */
    uintptr_t startaddr = 0;
    uintptr_t lastaddr  = 0;
    for(size_t i = 0; i < elf->e_phnum; i++) {
        if(prog[i].p_type != PT_LOAD) { continue; }
        if(prog[i].p_vaddr > ((lastaddr + 0x2000) & 0xFFFFF000)) {
            /* We found a gap, allocate the previous block */
            relocs[ridx].addr_phys = (uintptr_t)_alloc_map(baseaddr + startaddr, lastaddr - startaddr);
            relocs[ridx].addr_orig = startaddr;
            relocs[ridx].addr_new  = baseaddr + startaddr;
            relocs[ridx].size      = lastaddr - startaddr;
            ridx++;

            startaddr = prog[i].p_vaddr;
        }
        lastaddr = prog[i].p_vaddr + prog[i].p_memsz;
    }
    relocs[ridx].addr_phys = (uintptr_t)_alloc_map(baseaddr + startaddr, lastaddr - startaddr);
    relocs[ridx].addr_orig = startaddr;
    relocs[ridx].addr_new  = baseaddr + startaddr;
    relocs[ridx].size      = lastaddr - startaddr;
    ridx++;
    relocs[ridx].addr_orig = 0xFFFFFFFF;
    relocs[ridx].addr_new  = 0xFFFFFFFF;
    relocs[ridx].size      = 0;


    for(size_t i = 0; i < elf->e_phnum; i++) {
		kdebug(DEBUGSRC_MODULE, "phdr[%2X/%2X] T:%X VADDR: %08X MSZ:%08X FSZ:%08X", i+1, elf->e_phnum, prog[i].p_type, prog[i].p_vaddr, prog[i].p_memsz, prog[i].p_filesz);
		switch(prog[i].p_type) {
			case PT_LOAD: {
                void *phys = (void *)_translate_addr_phys(relocs, prog[i].p_vaddr);

				/* Copy data and/or clear memory */
				if(prog[i].p_filesz) {
					memcpy(phys, (void *)((uintptr_t)elf + prog[i].p_offset), prog[i].p_filesz);
				}
				if(prog[i].p_filesz < prog[i].p_memsz) {
					memset(phys + prog[i].p_filesz, 0, prog[i].p_memsz - prog[i].p_filesz);
				}
            } break;
		}
	}

    kdebug(DEBUGSRC_MODULE, "---------------------------------");
    for(size_t i = 0; i < ridx; i++) {
        kdebug(DEBUGSRC_MODULE, "%08X -> %08X [%08X] (%d, %08X)",
               relocs[i].addr_orig, relocs[i].addr_new,
               relocs[i].addr_phys, relocs[i].size, relocs[i].size);
    }
    kdebug(DEBUGSRC_MODULE, "---------------------------------");


    if(_module_apply_relocs(elf, relocs, symbols, baseaddr)) {
        kfree(relocs);
        return -1;
    }
    
    mod_ent->func = (lambda_mod_func_t)_translate_addr(relocs, (uintptr_t)mod_head->function);

    kfree(relocs);

    return 0;
}