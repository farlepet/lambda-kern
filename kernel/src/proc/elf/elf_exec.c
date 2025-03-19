#include <errno.h>
#include <string.h>

#include <proc/mtask.h>
#include <proc/exec.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <proc/elf.h>
#include <io/output.h>

static void elf_read_phdr(const Elf32_Ehdr *elf, struct kproc_mem_map_ent **mmap_entries, proc_elf_data_t *elf_data, mmu_table_t *mmu_table) {
    if(!mmap_entries || !elf_data) { return; }

    struct kproc_mem_map_ent **mmap_next = mmap_entries;
    
    Elf32_Phdr *prog = (Elf32_Phdr *)((uintptr_t)elf + elf->e_phoff);

    elf_data->dynamic = NULL;

    for(size_t i = 0; i < elf->e_phnum; i++) {
        kdebug(DEBUGSRC_EXEC, ERR_TRACE, "phdr[%2X/%2X] T:%X VADDR: %08X MSZ:%08X FSZ:%08X", i+1, elf->e_phnum, prog[i].p_type, prog[i].p_vaddr, prog[i].p_memsz, prog[i].p_filesz);
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
                *mmap_next = (struct kproc_mem_map_ent *)kmalloc(sizeof(struct kproc_mem_map_ent));
                (*mmap_next)->virt_address = prog[i].p_vaddr;
                (*mmap_next)->phys_address = (uintptr_t)phys;
                (*mmap_next)->length       = prog[i].p_memsz;
                (*mmap_next)->next         = NULL;
                mmap_next = &((*mmap_next)->next);

                mmu_map_table(mmu_table, prog[i].p_vaddr, (uintptr_t)phys, prog[i].p_memsz,
                              (MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_EXEC));
            } break;
            case PT_DYNAMIC:
                elf_data->dynamic = (Elf32_Dyn *)prog[i].p_vaddr;
                break;
        }
    }
}

static uintptr_t elf_exec_common(void *data, uint32_t length, mmu_table_t *mmu_table, symbol_t **symbols, struct kproc_mem_map_ent **mmap_entries, proc_elf_data_t *elf_data) {
    /* TODO: Use this for error-checking */
    (void)length;
    
    const Elf32_Ehdr *head = (Elf32_Ehdr *)data;

    elf_read_phdr(head, mmap_entries, elf_data, mmu_table);

    if(symbols) {
        *symbols = NULL;
        elf_load_symbols(head, symbols);
    }

    if(elf_data->dynamic) {
        size_t i = 0;
        int needed = -1;
        while(elf_data->dynamic[i].d_tag != DT_NULL) {
            const Elf32_Dyn *dyn = &elf_data->dynamic[i];
            switch(dyn->d_tag) {
                case DT_STRTAB:
                    elf_data->dynamic_str     = (const char *)dyn->d_val;
                    break;
                case DT_STRSZ:
                    elf_data->dynamic_str_len = dyn->d_val;
                    break;
                case DT_SYMTAB:
                    elf_data->dynamic_sym     = (const Elf32_Sym *)dyn->d_val;
                    break;
                case DT_NEEDED:
                    /* Deal with this later, we need to ensure we have the string table. */
                    needed = i;
                    break;
                default:
                    break;
            }
            i++;
        }

        if(needed >= 0) {
            kdebug(DEBUGSRC_EXEC, ERR_TRACE, "DT_NEEDED: %s", &elf_data->dynamic_str[elf_data->dynamic[needed].d_val]);
        }
    }

    return head->e_entry;
}

int exec_elf(exec_data_t *exec_data) {
    if(elf_check_header(exec_data->file_data)) {
        return -ENOEXEC;
    }

    exec_data->elf_data = (proc_elf_data_t *)kmalloc(sizeof(proc_elf_data_t));

    exec_data->entrypoint = elf_exec_common(exec_data->file_data, exec_data->file_size, exec_data->mmu_table,
                                            &exec_data->symbols, &exec_data->mmap_entries, exec_data->elf_data);
    if(exec_data->entrypoint == 0) {
        return -EUNSPEC;
    }

    kdebug(DEBUGSRC_EXEC, ERR_TRACE, "Entrypoint: %08X", exec_data->entrypoint);

    return 0;
}
