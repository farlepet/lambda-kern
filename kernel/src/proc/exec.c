#include <proc/mtask.h>
#include <proc/exec.h>
#include <err/error.h>
#include <fs/procfs.h>
#include <proc/elf.h>
#include <mm/alloc.h>
#include <mm/mm.h>


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