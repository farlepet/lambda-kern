#include <proc/mtask.h>
#include <proc/exec.h>
#include <err/error.h>
#include <fs/procfs.h>
#include <fs/fs.h>
#include <proc/elf.h>
#include <mm/alloc.h>
#include <mm/mm.h>

#include <string.h>
#include <sys/stat.h>

#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
#  include <arch/intr/int.h>
#  include <arch/proc/user.h>
#endif

int execve(const char *filename, const char **argv, const char **envp) {
    kdebug(DEBUGSRC_EXEC, ERR_DEBUG, "execve: %s (%08X, %08X)", filename, argv, envp);
    if(!mm_check_addr(argv)) {
        kdebug(DEBUGSRC_EXEC, ERR_DEBUG, "  -> ARGV invalid address?");
    }
    if(!mm_check_addr(envp)) {
        kdebug(DEBUGSRC_EXEC, ERR_DEBUG, "  -> ENVP invalid address?");
    }

    kdebug(DEBUGSRC_EXEC, ERR_TRACE, "execve MMU: %p", (uintptr_t)mmu_get_current_table());

    for(int i = 0; argv[i]; i++) {
        kdebug(DEBUGSRC_EXEC, ERR_TRACE, "execve argv[%d]: %08X '%s'", i, argv[i], argv[i]);
    }
	
    void  *exec_data;
    size_t exec_size;

    if(fs_read_file_by_path(filename, NULL, &exec_data, &exec_size, 0)) {
		kdebug(DEBUGSRC_EXEC, ERR_DEBUG, "execve: Could not open %s!\n", argv[1]);
		return -1;
	}

    // TODO: Add executable type handlers in the future, so that they can be
    // registered on-the-fly
    // Check for the filetype:
    if(*(uint32_t *)exec_data == ELF_IDENT) { // ELF
        kdebug(DEBUGSRC_EXEC, ERR_TRACE, "execve: Determined filetype as ELF");
        return exec_elf(exec_data, exec_size, argv, envp);
    } else if(*(uint16_t *)exec_data == 0x3335) { // SHEBANG, NOTE: Byte order might be wrong!
        kdebug(DEBUGSRC_EXEC, ERR_WARN, "execve: No support for shebang yet!");
    } else { // UNKNOWN
        kdebug(DEBUGSRC_EXEC, ERR_DEBUG, "execve: Unknown executable file type: %08X", *(uint32_t *)exec_data);
    }

    return -1;
}

/**
 * Copy and relocate arguments (argv, envp) to the next process image, and
 * push these values to the stack.
 */
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
    
    kdebug(DEBUGSRC_EXEC, ERR_TRACE, "exec_copy_arguments(): ARGV: %08X, ENVP: %08X, SZ: %d",
           argv, envp, data_sz);

    /* TODO: We are wasting space when doing this, we could add it to the process' memory map such
     * that it is available for the process. */
    char *new_buffer = kmamalloc(data_sz, 0x1000);
    char **new_argv  = (char **)new_buffer;
    char **new_envp  = NULL;

    mmu_map_table(thread->process->mmu_table, (uintptr_t)new_buffer, (uintptr_t)new_buffer, data_sz,
                  (MMU_FLAG_READ | MMU_FLAG_WRITE));

    size_t c_off = (argc + envc + 2) * sizeof(char *);
    
    size_t i;
    for(i = 0; i < argc; i++) {
        new_argv[i] = &new_buffer[c_off];
        memcpy(&new_buffer[c_off], argv[i], strlen(argv[i]) + 1);
        kdebug(DEBUGSRC_EXEC, ERR_TRACE, " -> ARG[%d]: %s", i, &new_buffer[c_off]);
        c_off += strlen(argv[i]) + 1;
    }
    new_argv[i] = NULL;

    new_envp = &new_argv[i+1];
    for(i = 0; i < envc; i++) {
        new_envp[i] = &new_buffer[c_off];
        memcpy(&new_buffer[c_off], envp[i], strlen(envp[i]) + 1);
        kdebug(DEBUGSRC_EXEC, ERR_TRACE, " -> ENV[%d]: %s", i, &new_buffer[c_off]);
        c_off += strlen(envp[i]) + 1;
    }
    new_envp[i] = NULL;

    if(c_off != data_sz) {
        kdebug(DEBUGSRC_EXEC, ERR_TRACE, "  -> c_off (%d) != data_sz (%d)!", c_off, data_sz);
    }

    (void)thread;

    *n_argv = new_argv;
    *n_envp = new_envp;

    kdebug(DEBUGSRC_EXEC, ERR_TRACE, "  -> New locations: ARGV: %08X, ENVP: %08X", *n_argv, *n_envp);    
}

static void *_store_arguments(const char **argv, const char **envp, char ***_argv, char ***_envp) {
    /* At minimum, we have two null pointers. */
    size_t data_sz = 2 * sizeof(char *);
    size_t argc    = 0;
    size_t envc    = 0;
    size_t off     = 0;
    
    for(; argv[argc]; argc++) {
        /* argv pointer + argv[i] length + NULL terminator */
        data_sz += sizeof(char *) + strlen(argv[argc]) + 1;
    }
    for(; envp[envc]; envc++) {
        /* envp pointer + envp[i] length + NULL terminator */
        data_sz += sizeof(char *) + strlen(envp[envc]) + 1;
    }


    void *data = kmalloc(data_sz);

    *_argv = (char **)data;
    *_envp = &(*_argv)[argc + 1];
    
    off = (argc + envc + 2) * sizeof(char *);

    for(size_t i = 0; i < argc; i++) {
        (*_argv)[i] = (char *)(data + off);
        memcpy((*_argv)[i], argv[i], strlen(argv[i]) + 1);
        off += strlen(argv[i]) + 1;
    }
    (*_argv)[argc] = NULL;
    
    for(size_t i = 0; i < envc; i++) {
        (*_envp)[i] = (char *)(data + off);
        memcpy((*_envp)[i], envp[i], strlen(envp[i]) + 1);
        off += strlen(envp[i]) + 1;
    }
    (*_envp)[envc] = NULL;
    
    if(off != data_sz) {
        kdebug(DEBUGSRC_EXEC, ERR_TRACE, "  -> off (%d) != data_sz (%d)!", off, data_sz);
    }

    return data;
}

void exec_replace_process_image(void *entryp, const char *name, arch_task_params_t *arch_params, symbol_t *symbols, const char **argv, const char **envp) {
    // TODO: Clean this up, separate out portions where possible/sensical
    kdebug(DEBUGSRC_EXEC, ERR_TRACE, "exec_replace_process_image @ %08X", entryp);

    struct kproc tmp_proc;
    
    char **_argv, **_envp;
    void *_arg_alloc = _store_arguments(argv, envp, &_argv, &_envp);

    // Copy data to temporary struct for easy copying of requred portions
    // Probably innefecient, and could be done better 
    kthread_t *old_thread   = mtask_get_curr_thread();
    kproc_t *curr_proc = old_thread->process;
    memcpy(&tmp_proc, curr_proc, sizeof(struct kproc));

    disable_interrupts();

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

    /* @todo: Free old thread. */
    /* @todo: Dequeue old threads */
	llist_init(&curr_proc->threads);
	kthread_t *thread = (kthread_t *)kmalloc(sizeof(kthread_t));
    memset(thread, 0, sizeof(kthread_t));

    thread->process    = curr_proc;
    thread->entrypoint = (uint32_t)entryp;
    thread->prio       = old_thread->prio;
    thread->tid        = old_thread->tid;
    thread->stack_size = old_thread->stack_size;
    memcpy(thread->name, curr_proc->name, strlen(curr_proc->name) + 1);
    proc_add_thread(curr_proc, thread);

    curr_proc->cwd = tmp_proc.cwd;
    memcpy(curr_proc->open_files, tmp_proc.open_files, sizeof(curr_proc->open_files));
    memcpy(curr_proc->file_position, tmp_proc.file_position, sizeof(curr_proc->file_position));

    curr_proc->symbols = symbols;

    curr_proc->list_item = tmp_proc.list_item;
    curr_proc->list_item.data = curr_proc;


#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
    // Copy architecture-specific bits:
    curr_proc->arch.ring = tmp_proc.arch.ring;
    thread->arch.eip  = (uint32_t)entryp;
    
    // TODO: Free unused frames
    // TODO: Only keep required portions of pagedir
    //proc->cr3 = tmp_proc.cr3;
    curr_proc->mmu_table = (mmu_table_t *)arch_params->pgdir;
#endif

    proc_create_stack(thread);
    proc_create_kernel_stack(thread);

    //enable_interrupts();

    uint32_t argc = 0;
    while(argv[argc]) argc++;

    char **n_argv;
    char **n_envp;

    exec_copy_arguments(thread, (const char **)_argv, (const char **)_envp, &n_argv, &n_envp);
    kfree(_arg_alloc);

    mmu_set_current_table(curr_proc->mmu_table);

#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
    thread->arch.esp = thread->arch.ebp;

    STACK_PUSH(thread->arch.esp, n_envp);
    STACK_PUSH(thread->arch.esp, n_argv);
    STACK_PUSH(thread->arch.esp, argc);
#endif

    kdebug(DEBUGSRC_EXEC, ERR_DEBUG, "exec_replace_process_image(): Jumping into process");

    thread->flags |= KTHREAD_FLAG_RUNNABLE;

    sched_enqueue_thread(thread);

#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
    enter_ring_newstack(curr_proc->arch.ring, entryp, (void *)thread->arch.esp);
#endif
}
