#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include <lambda/export.h>
#include <mod/symbols.h>
#include <mod/module.h>
#include <err/error.h>
#include <proc/thread.h>
#include <proc/proc.h>
#include <proc/elf.h>
#include <mm/alloc.h>
#include <fs/fs.h>
#include <io/output.h>

static llist_t loaded_modules;

/* TODO: Better method for choosing where to place module */
static uintptr_t _current_base = KERNEL_OFFSET + 0x10000000;

static lambda_mod_head_t *_module_place(const Elf32_Ehdr *elf, const lambda_mod_head_t *mod_head, uintptr_t baseaddr, module_entry_t *mod_ent, symbol_t *symbols);

int module_read(kfile_hand_t *file, lambda_mod_head_t **head, uintptr_t *base, Elf32_Ehdr **elf) {
    Elf32_Ehdr        *elf_data;
    Elf32_Shdr        *mod_section;
    lambda_mod_head_t *mod_head;
    kstat_t            file_stat;

    if(!file) {
        return -EINVAL;
    }

    TRY_OR_RET(kfstat(file->file, &file_stat));

    elf_data = (Elf32_Ehdr *)kmalloc((size_t)file_stat.size);
    if(!elf_data) {
        return -ENOMEM;
    }
    if((size_t)fs_read(file, 0, sizeof(Elf32_Ehdr), (void *)elf_data) != sizeof(Elf32_Ehdr)) {
        kfree(elf_data);
        return -EUNSPEC;
    }
    if(elf_check_header(elf_data)) {
        kfree(elf_data);
        return -EUNSPEC;
    }

    if((size_t)fs_read(file, 0, (size_t)file_stat.size, (void *)elf_data) != file_stat.size) {
        kfree(elf_data);
        return -EUNSPEC;
    }
    if(elf_find_section(elf_data, &mod_section, LAMBDA_MODULE_SECTION_NAME)) {
        kfree(elf_data);
        return -EUNSPEC;
    }

    mod_head = (lambda_mod_head_t *)((uintptr_t)elf_data + mod_section->sh_offset);
    if(mod_head->head_magic != LAMBDA_MODULE_HEAD_MAGIC) {
        kfree(elf_data);
        return -EUNSPEC;
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

static int _check_requirements(lambda_mod_head_t *mod_head) {
    if(!mod_head->metadata.requirements) {
        /* No requirements, auto-pass */
        return 0;
    }
    
    llist_iterator_t iter;
    module_entry_t  *mod;

    const char *const *reqs = (const char *const *)mod_head->metadata.requirements;
    /* TODO: Potentially enforce maximum number of requirements */
    for(size_t i = 0; reqs[i]; i++) {
        int found = 0;
        llist_iterator_init(&loaded_modules, &iter);
        while(!found && llist_iterate(&iter, (void **)&mod)) {
            if(!strcmp(reqs[i], mod->ident)) {
                found = 1;
            }
        }
        if(!found) {
            /* Requirement not satisfied */
            kdebug(DEBUGSRC_MODULE, ERR_WARN, "%s: Requirement not satisfied: %s", mod_head->metadata.name, reqs[i]);
            return -EUNSPEC;
        }
    }

    return 0;
}

int module_install(kfile_hand_t *file) {
    /* @todo A lot of work is needed here in order to prevent memory leaks */
    lambda_mod_head_t *mod_head;
    lambda_mod_head_t *mod_head_reloc;
    uintptr_t          mod_base;
    Elf32_Ehdr        *mod_elf;
    symbol_t          *symbols;
    
    kdebug(DEBUGSRC_MODULE, ERR_DEBUG, "module_install: Reading module");
    if(module_read(file, &mod_head, &mod_base, &mod_elf)) {
        return -EUNSPEC;
    }

    kdebug(DEBUGSRC_MODULE, ERR_TRACE, "module_install: Generating module entry");
    const char *ident = (char *)elf_find_data(mod_elf, (uintptr_t)mod_head->metadata.ident);
    module_entry_t *modent = (module_entry_t *)kmalloc(sizeof(module_entry_t) + strlen(ident) + 1);
    memset(modent, 0, sizeof(module_entry_t));
    modent->ident = (char *)((uintptr_t)modent + sizeof(module_entry_t));
    memcpy(modent->ident, ident, strlen(ident) + 1);

    kdebug(DEBUGSRC_MODULE, ERR_TRACE, "module_install: Reading symbol table");
    if(elf_load_symbols(mod_elf, &symbols)) {
        kfree(mod_elf);
        return -EUNSPEC;
    }

    /* This is potentially wasteful if the module ends up getting rejected, but
     * it makes it much easier to perform the requisite checks, and shouldn't be
     * big deal since the calling process (should) be priviledged. Alternatively,
     * we can do a partial placement first. */
    kdebug(DEBUGSRC_MODULE, ERR_TRACE, "module_install: Placing and relocating module");
    mod_head_reloc = _module_place(mod_elf, mod_head, _current_base, modent, symbols);
    if(mod_head_reloc == NULL) {
        /* @todo Remove module */
        kfree(mod_elf);
        kfree(symbols);
        kfree(modent);

        return -EUNSPEC;
    }
    _current_base += 0x10000; /* TODO: Actually determine module space */

    kdebug(DEBUGSRC_MODULE, ERR_TRACE, "module_install: Checking requirements");
    if(_check_requirements(mod_head_reloc)) {
        kfree(mod_elf);
        kfree(symbols);
        kfree(modent);

        return -EUNSPEC;
    }

    kdebug(DEBUGSRC_MODULE, ERR_TRACE, "module_install: Calling function @ %p", modent->func);
    if(modent->func(LAMBDA_MODFUNC_START, modent)) {
        kdebug(DEBUGSRC_MODULE, ERR_WARN, "module_install: Module function return non-zero!");
        /* @todo: remove module */
    }

    kdebug(DEBUGSRC_MODULE, ERR_TRACE, "module_install: Adding module to list");
    modent->list_item.data = (void *)modent;
    llist_append(&loaded_modules, &modent->list_item);

    return 0;
}

int module_uninstall(module_entry_t *mod) {
    mod->func(LAMBDA_MODFUNC_STOP, mod);
    /* @todo Actually remove module */
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
            kdebug(DEBUGSRC_MODULE, ERR_WARN, "_module_apply_relocs: Unhandled relocation type: %d", ELF32_R_TYPE(rel->r_info));
            return -EUNSPEC;
    }
    
    kdebug(DEBUGSRC_MODULE, ERR_TRACE, "_do_reloc: Wrote %08X to %08X [%d]", *(uint32_t *)dataaddr, dataaddr, ELF32_R_TYPE(rel->r_info));

    return 0;
}

#define _RELOC_NEED_SYM(R) ((ELF32_R_TYPE(R.r_info) != R_386_NONE) && \
                            (ELF32_R_TYPE(R.r_info) != R_386_RELATIVE))

static int _module_apply_relocs(const Elf32_Ehdr *elf, const elf_reloc_t *relocs, const symbol_t *symbols, uintptr_t baseaddr) {
    Elf32_Shdr *elf_symtab = NULL;
    Elf32_Shdr *elf_strtab = NULL;
    if(elf_find_section(elf, &elf_symtab, ".dynsym") ||
       elf_find_section(elf, &elf_strtab, ".dynstr")) {
        return -EUNSPEC;
    }
    Elf32_Sym *syms = (Elf32_Sym *)((uintptr_t)elf + elf_symtab->sh_offset);
    char      *strs = (char *)((uintptr_t)elf + elf_strtab->sh_offset);

    const Elf32_Shdr *sects = (Elf32_Shdr *)((uintptr_t)elf + elf->e_shoff);
    
    for(size_t i = 0; i < elf->e_shnum; i++) {
        if(sects[i].sh_type != SHT_REL) { continue; }

        const Elf32_Rel *rel    = (Elf32_Rel *)((uintptr_t)elf + sects[i].sh_offset);
        size_t           n_rels = sects[i].sh_size / sects[i].sh_entsize;
        for(size_t j = 0; j < n_rels; j++) {
            uintptr_t symaddr = 0;
            if(_RELOC_NEED_SYM(rel[j])) {
                size_t sidx = ELF32_R_SYM(rel[j].r_info);
                char *ident = &strs[syms[sidx].st_name];
                kdebug(DEBUGSRC_MODULE, ERR_TRACE, "_module_apply_relocs: %08X, %08X [%s]",
                       rel[j].r_offset, rel[j].r_info, ident);
                
                if(!module_symbol_find_kernel(ident, &symaddr)) {
                } else if(!module_symbol_find_module(ident, &symaddr, symbols)) {
                    symaddr = _translate_addr(relocs, symaddr);
                } else {
                    /* Check other loaded modules */
                    llist_iterator_t iter;
                    module_entry_t  *ent;
                    int              found = 0;
                    llist_iterator_init(&loaded_modules, &iter);
                    while(!found &&
                        llist_iterate(&iter, (void **)&ent)) {
                        if(ent->symbols &&
                        !module_symbol_find_module(ident, &symaddr, ent->symbols)) {
                            found = 1;
                        }
                    }
                    if (!found) {
                        kdebug(DEBUGSRC_MODULE, ERR_WARN, "_module_apply_relocs: Could not find symbol %s", ident);
                        return -EUNSPEC;
                    }
                }
            } else {
                kdebug(DEBUGSRC_MODULE, ERR_TRACE, "_module_apply_relocs: %08X, %08X",
                       rel[j].r_offset, rel[j].r_info);
            }

            if(_do_reloc(baseaddr, relocs, symaddr, &rel[j])) {
                return -EUNSPEC;
            }
        }
    }

    return 0;
}

static void *_alloc_map(uintptr_t virt, size_t len) {
    void *paddr = kmamalloc(len, 0x1000);
    /* TODO: Possibly do not make all pages writable and executable */
    mmu_map(virt, (uintptr_t)paddr, len, MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_EXEC | MMU_FLAG_KERNEL);

    return paddr;
}

static lambda_mod_head_t *_module_place(const Elf32_Ehdr *elf, const lambda_mod_head_t *mod_head, uintptr_t baseaddr, module_entry_t *mod_ent, symbol_t *symbols) {
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

    if(_module_apply_relocs(elf, relocs, symbols, baseaddr)) {
        kfree(relocs);
        return NULL;
    }

    kdebug(DEBUGSRC_MODULE, ERR_TRACE, "Translating module symbol table");
    for(size_t i = 0; symbols[i].addr != 0xFFFFFFFF; i++) {
        symbols[i].addr = _translate_addr(relocs, symbols[i].addr);
        kdebug(DEBUGSRC_MODULE, ERR_TRACE, "  %d: [%s] => %08X", i, symbols[i].name, symbols[i].addr);
    }
    
    kdebug(DEBUGSRC_MODULE, ERR_TRACE, "Mod func: %p -> %p", mod_head->function, mod_ent->func);
    
    Elf32_Shdr *mod_section;
    if(elf_find_section(elf, &mod_section, LAMBDA_MODULE_SECTION_NAME)) {
        return NULL;
    }
    lambda_mod_head_t *new_mod_head = (lambda_mod_head_t *)_translate_addr(relocs, mod_section->sh_addr);
    mod_ent->func    = new_mod_head->function;
    mod_ent->symbols = symbols;

    kfree(relocs);

    return new_mod_head;
}




int module_start_thread(module_entry_t *mod, void (*entry)(void *), void *data, const char *name) {
    if((mod == NULL) ||
       (entry == NULL)) {
        return -EINVAL;
    }
    
    static char _name[256];
    if(name == NULL) {
        strncpy(_name, mod->ident, 255);
        _name[255] = '\0';
        name = _name;
    }

    size_t tidx = 0;
    for(; tidx < MOD_THREAD_MAX; tidx++) {
        if(mod->threads[tidx] == 0) {
            break;
        }
    }
    if(tidx == MOD_THREAD_MAX) {
        kdebug(DEBUGSRC_MODULE, ERR_ERROR, "module_start_thread: Ran out of thread slots!");
        return -EUNSPEC;
    }

    /* TODO: Allow configuration of stack size and priority? */
    int tid = thread_spawn((uintptr_t)entry, data, name, CONFIG_PROC_KERN_STACK_SIZE, PRIO_DRIVER);
    if(tid < 0) {
        return tid;
    }

    mod->threads[tidx] = tid;

    return tid;
}
EXPORT_FUNC(module_start_thread);
