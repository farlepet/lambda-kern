#include <proc/mtask.h>
#include <proc/exec.h>
#include <err/error.h>
#include <fs/procfs.h>
#include <proc/elf.h>
#include <mm/alloc.h>
#include <mm/mm.h>

#include <string.h>
#include <sys/stat.h>

#if defined(ARCH_X86)
#  include <arch/intr/int.h>
#  include <arch/proc/user.h>
#  include <arch/mm/paging.h>
#endif

int execve(const char *filename, const char **argv, const char **envp) {
    kerror(ERR_BOOTINFO, "execve: %s (%08X, %08X)", filename, argv, envp);
    if(!mm_check_addr(argv)) {
        kerror(ERR_BOOTINFO, "  -> ARGV invalid address?");
    }
    if(!mm_check_addr(envp)) {
        kerror(ERR_BOOTINFO, "  -> ENVP invalid address?");
    }

#if defined(ARCH_X86)
    kerror(ERR_BOOTINFO, "execve pgdir: %08X", get_pagedir());
#endif

    for(int i = 0; argv[i]; i++) {
        kerror(ERR_BOOTINFO, "execve argv[%d]: %08X '%s'", i, argv[i], argv[i]);
    }

    int _exec = proc_fs_open(filename, OFLAGS_READ); //NOTE: The flag here may be wrong
    if(_exec < 0) {
        kerror(ERR_SMERR, "execve: Could not open %s for reading!", filename);
        return -1;
    }

    /* TODO: Use regular fstat with _exec */
    struct stat exec_stat;
    fstat(_exec, &exec_stat);
    int execsz = exec_stat.st_size;


    void *execdata = kmalloc(execsz);
    proc_fs_read(_exec, 0, execsz, execdata);

    // TODO: Add executable type handlers in the future, so that they can be
    // registered on-the-fly
    // Check for the filetype:
    if(*(uint32_t *)execdata == ELF_IDENT) { // ELF
        kdebug(DEBUGSRC_EXEC, "execve: Determined filetype as ELF");
        return exec_elf(execdata, execsz, argv, envp);
    } else if(*(uint16_t *)execdata == 0x3335) { // SHEBANG, NOTE: Byte order might be wrong!
        kerror(ERR_MEDERR, "execve: No support for shebang yet!");
    } else { // UNKNOWN
        kerror(ERR_MEDERR, "execve: Unknown executable file type: %08X", *(uint32_t *)execdata);
    }

    return -1;
}

/**
 * Copy and relocate arguments (argv, envp) to the next process image, and
 * push these values to the stack.
 */
#if defined(ARCH_ARMV7)
__unused
#endif
static void exec_copy_arguments(kthread_t *thread, const char **argv, const char **envp, char ***n_argv, char ***n_envp) {
    size_t   data_sz = 0;
    uint32_t argc = 0;
    uint32_t envc = 0;

    for(size_t i = 0; argv[i]; i++) {
        /* argv pointer + argv[i] length + NULL terminator */
        data_sz += sizeof(char *) + strlen(argv[i]) + 1;
        argc++;
    }
    
    for(size_t i = 0; envp[i]; i++) {
        /* argv pointer + argv[i] length + NULL terminator */
        data_sz += sizeof(char *) + strlen(envp[i]) + 1;
        envc++;
    }

    /* NULL pointers at the end of argv and envp */
    data_sz += sizeof(char *) * 2;
    
    kdebug(DEBUGSRC_EXEC, "exec_copy_arguments(): ARGV: %08X, ENVP: %08X, SZ: %d",
           argv, envp, data_sz);

    /* TODO: Find a better way to map some memory for this process, rather than potentially
     * re-mapping kernel memory to allow user access */

    char *new_buffer = kmalloc(data_sz);
    char **new_argv  = (char **)new_buffer;
    char **new_envp  = NULL;

#if defined(ARCH_X86)
    for(ptr_t p = (ptr_t)new_buffer & 0xFFFFF000; p < ((ptr_t)new_buffer + data_sz); p += 0x1000) {
        pgdir_map_page((uint32_t *)thread->process->arch.cr3, (void *)p, (void *)p, 0x07);
    }
#endif

    size_t c_off = (argc + envc + 2) * sizeof(char *);
    
    size_t i;
    for(i = 0; i < argc; i++) {
        new_argv[i] = &new_buffer[c_off];
        memcpy(&new_buffer[c_off], argv[i], strlen(argv[i]) + 1);
        c_off += strlen(argv[i]) + 1;
    }
    new_argv[i] = NULL;

    new_envp = &new_argv[i+1];
    for(i = 0; i < envc; i++) {
        new_envp[i] = &new_buffer[c_off];
        memcpy(&new_buffer[c_off], envp[i], strlen(envp[i]) + 1);
        c_off += strlen(envp[i]) + 1;
    }
    new_envp[i] = NULL;

    if(c_off != data_sz) {
        kdebug(DEBUGSRC_EXEC, "  -> c_off (%d) != data_sz (%d)!", c_off, data_sz);
    }

    (void)thread;

    *n_argv = new_argv;
    *n_envp = new_envp;

    kdebug(DEBUGSRC_EXEC, "  -> New locations: ARGV: %08X, ENVP: %08X", *n_argv, *n_envp);    
}

void exec_replace_process_image(void *entryp, const char *name, arch_task_params_t *arch_params, symbol_t *symbols, char *symbol_string_table, const char **argv, const char **envp) {
    // TODO: Clean this up, separate out portions where possible/sensical
    kdebug(DEBUGSRC_EXEC, "exec_replace_process_image @ %08X", entryp);

    struct kproc tmp_proc;
    
    // Copy data to temporary struct for easy copying of requred portions
    // Probably innefecient, and could be done better
    memcpy(&tmp_proc, curr_proc, sizeof(struct kproc));

#if defined(ARCH_X86) // Ensure we do not get interrupted
    disable_interrupts();
#endif

    // Clear out old data:
    memset(curr_proc, 0, sizeof(struct kproc));

    memcpy(curr_proc->name, name, strlen(name) + 1);

    curr_proc->pid = tmp_proc.pid;
    curr_proc->uid = tmp_proc.uid;
    curr_proc->gid = tmp_proc.gid;

    curr_proc->type = tmp_proc.type;
    curr_proc->parent = tmp_proc.parent;
    // Not sure whether or not these should carry over:
    memcpy(curr_proc->children, tmp_proc.children, sizeof(curr_proc->children));

    curr_proc->threads[0].entrypoint = (uint32_t)entryp;

    curr_proc->cwd = tmp_proc.cwd;
    memcpy(curr_proc->open_files, tmp_proc.open_files, sizeof(curr_proc->open_files));
    memcpy(curr_proc->file_position, tmp_proc.file_position, sizeof(curr_proc->file_position));

    curr_proc->symbols = symbols;
    curr_proc->symStrTab = symbol_string_table;

    curr_proc->threads[0].prio = tmp_proc.threads[0].prio;

    curr_proc->next = tmp_proc.next;
    curr_proc->prev = tmp_proc.prev;

#if defined(ARCH_X86)
    // Copy architecture-specific bits:
    curr_proc->arch.ring = tmp_proc.arch.ring;
    curr_proc->threads[0].arch.eip  = (uint32_t)entryp;

    // TODO: Free unused frames
    // TODO: Only keep required portions of pagedir
    //proc->cr3 = tmp_proc.cr3;
    curr_proc->arch.cr3 = (uint32_t)arch_params->pgdir;

    int kernel = (curr_proc->type & TYPE_KERNEL);

    uint32_t stack_size = tmp_proc.threads[0].arch.stack_beg - tmp_proc.threads[0].arch.stack_end;

    uint32_t stack_begin, virt_stack_begin;
    if(!kernel) virt_stack_begin = 0xFF000000;
    else        virt_stack_begin = 0x7F000000;

    #ifdef STACK_PROTECTOR
        stack_begin = (uint32_t)kmalloc(stack_size + 0x2000);
        curr_proc->ebp   = virt_stack_begin + 0x1000;
        curr_proc->ebp  += (stack_size + 0x1000);
    #else // STACK_PROTECTOR
        stack_begin = ((uint32_t)kmalloc(stack_size));
        curr_proc->threads[0].arch.ebp   = virt_stack_begin;
        curr_proc->threads[0].arch.ebp  += stack_size;
    #endif // !STACK_PROTECTOR

    curr_proc->threads[0].arch.esp = curr_proc->threads[0].arch.ebp;

    uint32_t i = 0;
    for(; i < stack_size; i+= 0x1000) {
        if(kernel) pgdir_map_page(arch_params->pgdir, (void *)(stack_begin + i), (void *)(virt_stack_begin + i), 0x03);
        else       pgdir_map_page(arch_params->pgdir, (void *)(stack_begin + i), (void *)(virt_stack_begin + i), 0x07);
            //(void *)(procs[p].esp - i), (void *)(procs[p].esp - i), 0x03);
    }

    curr_proc->threads[0].arch.kernel_stack = (uint32_t)kmalloc(PROC_KERN_STACK_SIZE) + PROC_KERN_STACK_SIZE;
    for(i = 0; i < PROC_KERN_STACK_SIZE; i+=0x1000) {
        pgdir_map_page(arch_params->pgdir, (void *)(curr_proc->threads[0].arch.kernel_stack - i), (void *)(curr_proc->threads[0].arch.kernel_stack - i), 0x03);
    }

    curr_proc->threads[0].arch.stack_end = curr_proc->threads[0].arch.ebp - stack_size;
    curr_proc->threads[0].arch.stack_beg = curr_proc->threads[0].arch.ebp;

    #ifdef STACK_PROTECTOR
    // TODO: Fix stack guarding:
        block_page(curr_proc->arch.stack_end - 0x1000); // <-- Problematic line
        block_page(curr_proc->arch.stack_beg + 0x1000);
    #endif // STACK_PROTECTOR

    /*proc->kernel_stack      = tmp_proc.kernel_stack;

    proc->stack_beg = tmp_proc.stack_beg;
    proc->stack_end = tmp_proc.stack_end;*/
    // Reset stack:
    //proc->ebp = proc->esp = proc->stack_beg;

    //enable_interrupts();

    //enable_interrupts();


    uint32_t argc = 0;
    while(argv[argc]) argc++;

    char **n_argv;
    char **n_envp;

    exec_copy_arguments(&curr_proc->threads[0], argv, envp, &n_argv, &n_envp);
    
    set_pagedir(arch_params->pgdir);

    STACK_PUSH(curr_proc->threads[0].arch.esp, n_envp);
    STACK_PUSH(curr_proc->threads[0].arch.esp, n_argv);
    STACK_PUSH(curr_proc->threads[0].arch.esp, argc);    

    kdebug(DEBUGSRC_EXEC, "exec_replace_process_image(): Jumping into process");

    enter_ring_newstack(curr_proc->arch.ring, entryp, (void *)curr_proc->threads[0].arch.esp);
#else
    /* TODO */
    (void)arch_params;
    (void)argv;
    (void)envp;
#endif
}
