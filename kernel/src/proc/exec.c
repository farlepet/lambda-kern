#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include <proc/mtask.h>
#include <proc/thread.h>
#include <proc/exec.h>
#include <err/error.h>
#include <fs/procfs.h>
#include <fs/fs.h>
#include <proc/elf.h>
#include <mm/alloc.h>
#include <mm/mm.h>

#include <arch/intr/int.h>


static void _exec_replace_process_image(exec_data_t *exec_data);

int execve(const char *filename, const char **argv, const char **envp) {
    kdebug(DEBUGSRC_EXEC, ERR_DEBUG, "execve: %s (%08X, %08X)", filename, argv, envp);
    if(!mm_check_addr(argv)) {
        kdebug(DEBUGSRC_EXEC, ERR_DEBUG, "  -> ARGV invalid address?");
    }
    if(!mm_check_addr(envp)) {
        kdebug(DEBUGSRC_EXEC, ERR_DEBUG, "  -> ENVP invalid address?");
    }

    kdebug(DEBUGSRC_EXEC, ERR_TRACE, "execve MMU: %p", mmu_get_current_table());

    for(int i = 0; argv[i]; i++) {
        kdebug(DEBUGSRC_EXEC, ERR_TRACE, "execve argv[%d]: %08X '%s'", i, argv[i], argv[i]);
    }

    exec_data_t *exec_data = (exec_data_t *)kmalloc(sizeof(exec_data_t));
    if(exec_data == NULL) {
        kdebug(DEBUGSRC_EXEC, ERR_DEBUG, "execve: Could not allocate exec_data struct!\n");
        return -ENOMEM;
    }

    if(fs_read_file_by_path(filename, NULL, &exec_data->file_data, &exec_data->file_size, 0)) {
        kdebug(DEBUGSRC_EXEC, ERR_DEBUG, "execve: Could not open %s!\n", filename);
        kfree(exec_data);
        return -ENOENT;
    }

    exec_data->argv      = argv;
    exec_data->envp      = envp;

    exec_data->mmu_table = mmu_clone_table(mmu_get_kernel_table());

    strncpy(exec_data->name, argv[0], KPROC_NAME_MAX);

    int ret = -EUNSPEC;

    // TODO: Add executable type handlers in the future, so that they can be
    // registered on-the-fly
    // Check for the filetype:
    if(*(uint32_t *)exec_data->file_data == ELF_IDENT) { // ELF
        kdebug(DEBUGSRC_EXEC, ERR_TRACE, "execve: Determined filetype as ELF");
        ret = exec_elf(exec_data);
    } else if(*(uint16_t *)exec_data->file_data == 0x3335) { // SHEBANG, NOTE: Byte order might be wrong!
        kdebug(DEBUGSRC_EXEC, ERR_WARN, "execve: No support for shebang yet!");
    } else { // UNKNOWN
        kdebug(DEBUGSRC_EXEC, ERR_DEBUG, "execve: Unknown executable file type: %08X", *(uint32_t *)exec_data);
    }

    if(ret == 0) {
        _exec_replace_process_image(exec_data);
    }

    /* Above should only return on failure */

    kfree(exec_data->file_data);
    kfree(exec_data);

    return ret;
}

/**
 * Copy and relocate arguments (argv, envp) to the next process image, and
 * push these values to the stack.
 */
static void _exec_copy_arguments(mmu_table_t *mmu_table, const char **argv, const char **envp, char ***n_argv, char ***n_envp) {
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

    mmu_map_table(mmu_table, (uintptr_t)new_buffer, (uintptr_t)new_buffer, data_sz,
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


    /* TODO: Create allocation function wrapper that also maps. */
    void *data = kmalloc(data_sz);
    mmu_map((uintptr_t)data, (uintptr_t)data, data_sz, MMU_FLAG_READ | MMU_FLAG_WRITE);

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

static void _exec_replace_process_image(exec_data_t *exec_data) {
    // TODO: Clean this up, separate out portions where possible/sensical
    kdebug(DEBUGSRC_EXEC, ERR_TRACE, "exec_replace_process_image @ %08X", exec_data->entrypoint);

    kproc_t tmp_proc;

    char **_argv, **_envp;
    void *_arg_alloc = _store_arguments(exec_data->argv, exec_data->envp, &_argv, &_envp);

    // Copy data to temporary struct for easy copying of requred portions
    // Probably innefecient, and could be done better 
    kthread_t *old_thread = mtask_get_curr_thread();
    kproc_t   *curr_proc  = old_thread->process;

    memcpy(&tmp_proc, curr_proc, sizeof(kproc_t));

    disable_interrupts();

    /*
     * Process
     */
    memset(curr_proc, 0, sizeof(kproc_t));

    strncpy(curr_proc->name, exec_data->name, KPROC_NAME_MAX);

    curr_proc->pid    = tmp_proc.pid;
    curr_proc->uid    = tmp_proc.uid;
    curr_proc->gid    = tmp_proc.gid;
    curr_proc->type   = tmp_proc.type;
    curr_proc->domain = tmp_proc.domain;
    curr_proc->parent = tmp_proc.parent;
    curr_proc->cwd    = tmp_proc.cwd;

    curr_proc->mmu_table = exec_data->mmu_table;
    curr_proc->symbols   = exec_data->symbols;
    curr_proc->mmap      = exec_data->mmap_entries;
    curr_proc->elf_data  = exec_data->elf_data;

    curr_proc->list_item      = tmp_proc.list_item;
    curr_proc->list_item.data = curr_proc;

    // Not sure whether or not these should carry over:
    memcpy(curr_proc->children, tmp_proc.children, sizeof(curr_proc->children));

    memcpy(curr_proc->open_files,    tmp_proc.open_files,    sizeof(curr_proc->open_files));
    memcpy(curr_proc->file_position, tmp_proc.file_position, sizeof(curr_proc->file_position));

    /*
     * Thread
     */
    kthread_t *thread = (kthread_t *)kmalloc(sizeof(kthread_t));
    memset(thread, 0, sizeof(kthread_t));
    strncpy(thread->name, curr_proc->name, KPROC_NAME_MAX);

    thread->process    = curr_proc;
    thread->entrypoint = exec_data->entrypoint;
    thread->prio       = old_thread->prio;
    thread->tid        = old_thread->tid;
    thread->stack_size = old_thread->stack_size;

    llist_init(&curr_proc->threads);
    proc_add_thread(curr_proc, thread);

    /* @todo We might be able to reuse the current stacks - otherwise, we will
     * need to free these stacks. */
    arch_setup_thread(thread);

    uint32_t argc = 0;
    while(exec_data->argv[argc]) argc++;

    char **n_argv;
    char **n_envp;

    _exec_copy_arguments(curr_proc->mmu_table, (const char **)_argv, (const char **)_envp,
                         &n_argv, &n_envp);
    kfree(_arg_alloc);

    mmu_set_current_table(curr_proc->mmu_table);

    arch_setup_user_stack(thread, argc, n_argv, n_envp);

    kdebug(DEBUGSRC_EXEC, ERR_DEBUG, "exec_replace_process_image(): Jumping into process");

    thread->flags |= KTHREAD_FLAG_RUNNABLE;
    thread->flags &= ~(KTHREAD_FLAG_RANONCE);

    sched_replace_thread(thread);

    thread_destroy(old_thread);


    run_sched();
}
