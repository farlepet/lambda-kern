#include <arch/mm/alloc.h>

#include <proc/mtask.h>
#include <proc/exec.h>
#include <err/error.h>
#include <fs/procfs.h>
#include <proc/elf.h>
#include <mm/mm.h>

#include <string.h>

#if defined(ARCH_X86)
#  include <arch/intr/int.h>
#  include <arch/proc/user.h>
#endif

int execve(const char *filename, const char **argv, const char **envp) {
    int idx = proc_by_pid(current_pid);
    if(idx < 0) return -1;
    struct kproc *proc = &procs[idx];
    kerror(ERR_BOOTINFO, "execve: %s (%08X, %08X)", mm_translate_proc_addr(proc, filename), argv, envp);

    int _exec = proc_fs_open(filename, OFLAGS_READ); //NOTE: The flag here may be wrong
    if(_exec < 0) {
        kerror(ERR_SMERR, "execve: Could not open %s for reading!", filename);
        return -1;
    }

    // TODO: Find size in a more formal manner
    int execsz = proc->open_files[_exec]->length;


    void *execdata = kmalloc(execsz);
    proc_fs_read(_exec, 0, execsz, execdata);

    // TODO: Add executable type handlers in the future, so that they can be
    // registered on-the-fly
    // Check for the filetype:
    if(*(uint32_t *)execdata == ELF_IDENT) { // ELF
        kerror(ERR_INFO, "execve: Determined filetype as ELF");
        return exec_elf(execdata, execsz, argv, envp);
    } else if(*(uint16_t *)execdata == 0x3335) { // SHEBANG, NOTE: Byte order might be wrong!
        kerror(ERR_MEDERR, "execve: No support for shebang yet!");
    } else { // UNKNOWN
        kerror(ERR_MEDERR, "execve: Unknown executable file type: %08X", *(uint32_t *)execdata);
    }

    return -1;
}

void exec_replace_process_image(void *entryp, const char *name, void *pagedir, symbol_t *symbols, char *symbol_string_table, const char **argv, const char **envp) {
    // TODO: Clean this up, separate out portions where possible/sensical
    kerror(ERR_INFO, "exec_replace_process_image @ %08X", entryp);

    int p = proc_by_pid(current_pid);
    struct kproc *proc = &procs[p];
    struct kproc tmp_proc;
    
    // Copy data to temporary struct for easy copying of requred portions
    // Probably innefecient, and could be done better
    memcpy(&tmp_proc, proc, sizeof(struct kproc));

#if defined(ARCH_X86) // Ensure we do not get interrupted
    disable_interrupts();
#endif

    // Clear out old data:
    memset(proc, 0, sizeof(struct kproc));

    memcpy(proc->name, name, strlen(name) + 1);

    proc->pid = tmp_proc.pid;
    proc->uid = tmp_proc.uid;
    proc->gid = tmp_proc.gid;

    proc->type = tmp_proc.type;
    // Not sure whether or not these should carry over:
    memcpy(proc->children, tmp_proc.children, sizeof(proc->children));

    proc->entrypoint = (uint32_t)entryp;

    proc->cwd = tmp_proc.cwd;
    memcpy(proc->open_files, tmp_proc.open_files, sizeof(proc->open_files));
    memcpy(proc->file_position, tmp_proc.file_position, sizeof(proc->file_position));

    proc->symbols = symbols;
    proc->symStrTab = symbol_string_table;

    proc->prio = tmp_proc.prio;

    uint32_t argc = 0;
    while(argv[argc]) argc++;

#if defined(ARCH_X86)
    // Copy architecture-specific bits:
    proc->ring = tmp_proc.ring;
    proc->eip  = (uint32_t)entryp;

    // TODO: Free unused frames
    // TODO: Only keep required portions of pagedir
    //proc->cr3 = tmp_proc.cr3;
    proc->cr3 = (uint32_t)pagedir;

    int kernel = (proc->type & TYPE_KERNEL);

    uint32_t stack_size = tmp_proc.stack_beg - tmp_proc.stack_end;

    uint32_t stack_begin, virt_stack_begin;
    if(!kernel) virt_stack_begin = 0xFF000000;
    else        virt_stack_begin = 0x7F000000;

    #ifdef STACK_PROTECTOR
        stack_begin = (uint32_t)kmalloc(stack_size + 0x2000);
        proc->ebp   = virt_stack_begin + 0x1000;
        proc->ebp  +=             (stack_size + 0x1000);
    #else // STACK_PROTECTOR
        stack_begin = ((uint32_t)kmalloc(stack_size));
        proc->ebp   = virt_stack_begin;
        proc->ebp  +=              stack_size;
    #endif // !STACK_PROTECTOR

    procs[p].esp = procs[p].ebp;

    uint32_t i = 0;
    for(; i < stack_size; i+= 0x1000) {
        if(kernel) pgdir_map_page(pagedir, (void *)(stack_begin + i), (void *)(virt_stack_begin + i), 0x03);
        else       pgdir_map_page(pagedir, (void *)(stack_begin + i), (void *)(virt_stack_begin + i), 0x07);
            //(void *)(procs[p].esp - i), (void *)(procs[p].esp - i), 0x03);
    }

    procs[p].kernel_stack = (uint32_t)kmalloc(PROC_KERN_STACK_SIZE) + PROC_KERN_STACK_SIZE;
    for(i = 0; i < PROC_KERN_STACK_SIZE; i+=0x1000) {
        pgdir_map_page(pagedir, (void *)(procs[p].kernel_stack - i), (void *)(procs[p].kernel_stack - i), 0x03);
    }

    procs[p].stack_end = procs[p].ebp - stack_size;
    procs[p].stack_beg = procs[p].ebp;

    #ifdef STACK_PROTECTOR
    // TODO: Fix stack guarding:
        block_page(procs[p].stack_end - 0x1000); // <-- Problematic line
        block_page(procs[p].stack_beg + 0x1000);
    #endif // STACK_PROTECTOR

    /*proc->kernel_stack      = tmp_proc.kernel_stack;
    proc->kernel_stack_size = tmp_proc.kernel_stack_size;

    proc->stack_beg = tmp_proc.stack_beg;
    proc->stack_end = tmp_proc.stack_end;*/
    // Reset stack:
    //proc->ebp = proc->esp = proc->stack_beg;

    //enable_interrupts();

    //enable_interrupts();

    set_pagedir(pagedir);

    kerror(ERR_INFO, "exec_replace_process_image(): Jumping into process");

    STACK_PUSH(proc->esp, argc);
    STACK_PUSH(proc->esp, argv);
    STACK_PUSH(proc->esp, envp);

    enter_ring_newstack(proc->ring, entryp, 0, argv, envp, (void *)proc->esp);
#endif
}
