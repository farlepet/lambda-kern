#include <proc/mtask.h>
#include <proc/exec.h>
#include <err/error.h>
#include <fs/procfs.h>
#include <proc/elf.h>
#include <mm/alloc.h>


int execve(const char *filename, const char **argv, const char **envp) {
	kerror(ERR_INFO, "execve: %s (%08X, %08X)", filename, argv, envp);

    int _exec = proc_fs_open(filename, OFLAGS_READ); //NOTE: The flag here may be wrong
    if(_exec < 0) {
        kerror(ERR_SMERR, "execve: Could not open %s for reading!", filename);
        return -1;
    }

    // TODO: Find size in a more formal manner
    int idx = proc_by_pid(current_pid);
    if(idx < 0) return -1;
    struct kproc *proc = &procs[idx];
    int execsz = proc->open_files[_exec]->length;

    void *execdata = kmalloc(execsz);
    proc_fs_read(_exec, 0, execsz, execdata);

    // TODO: Add executable type handlers in the future, so that they can be
    // registered on-the-fly
    // Check for the filetype:
    if(*(u32 *)execdata == ELF_IDENT) { // ELF
        kerror(ERR_INFO, "execve: Determined filetype as ELF");
        return exec_elf(execdata, execsz, argv, envp);
    } else if(*(u16 *)execdata == 0x3335) { // SHEBANG, NOTE: Byte order might be wrong!
        kerror(ERR_MEDERR, "execve: No support for shebang yet!");
    } else { // UNKNOWN
        kerror(ERR_MEDERR, "execve: Unknown executable file type: %08X", *(u32 *)execdata);
    }

    return -1;
}