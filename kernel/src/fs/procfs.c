#include <fs/fs.h>
#include <fs/procfs.h>
#include <proc/mtask.h>
#include <fs/dirinfo.h>
#include <string.h>

u32 proc_fs_read(int desc, u32 off, u32 sz, u8 *buff) {
    if(desc > MAX_OPEN_FILES) return 0;
    
    int idx = proc_by_pid(current_pid);
    if(idx < 0) return 0;
    struct kproc *proc = &procs[idx];

    if(proc->open_files[desc]) {
        return fs_read(proc->open_files[desc], off, sz, buff);
    }

    return 0;
}

u32 proc_fs_read_blk(int desc, u32 off, u32 sz, u8 *buff) {
    if(desc > MAX_OPEN_FILES) return 0;
    
    int idx = proc_by_pid(current_pid);
    if(idx < 0) return 0;
    struct kproc *proc = &procs[idx];

    if(proc->open_files[desc]) {
        if((proc->open_files[desc]->flags & FS_STREAM) && !(proc->open_files[desc]->open_flags & OFLAGS_NONBLOCK)) {
            uint32_t count = 0;
            while(count < sz) {
                uint32_t ret = fs_read(proc->open_files[desc], off, sz - count, buff + count);
                if(ret > 0x80000000) {
                    return count;
                }
                count += ret;
                run_sched(); // Wait
            }
            return count;
        } else {
            return fs_read(proc->open_files[desc], off, sz, buff);
        }
    }

    return 0;
}

u32 proc_fs_write(int desc, u32 off, u32 sz, u8 *buff) {
    if(desc > MAX_OPEN_FILES) return 0;
    
    int idx = proc_by_pid(current_pid);
    if(idx < 0) return 0;
    struct kproc *proc = &procs[idx];

    if(proc->open_files[desc]) {
        return fs_write(proc->open_files[desc], off, sz, buff);
    }

    return 0;
}

void proc_fs_open(int desc, u32 flags) {
    if(desc > MAX_OPEN_FILES) return;
    
    int idx = proc_by_pid(current_pid);
    if(idx < 0) return;
    struct kproc *proc = &procs[idx];

    if(proc->open_files[desc]) {
        fs_open(proc->open_files[desc], flags);
    }
}

void proc_fs_close(int desc) {
    if(desc > MAX_OPEN_FILES) return;
    
    int idx = proc_by_pid(current_pid);
    if(idx < 0) return;
    struct kproc *proc = &procs[idx];

    if(proc->open_files[desc]) {
        fs_close(proc->open_files[desc]);
    }
}

int proc_fs_mkdir(int desc, char *name, u32 perms) {
    if(desc > MAX_OPEN_FILES) return -1;
    
    int idx = proc_by_pid(current_pid);
    if(idx < 0) return -1;
    struct kproc *proc = &procs[idx];

    if(proc->open_files[desc]) {
        return fs_mkdir(proc->open_files[desc], name, perms);
    }

    return -1;
}

int proc_fs_create(int desc, char *name, u32 perms) {
    if(desc > MAX_OPEN_FILES) return -1;
    
    int idx = proc_by_pid(current_pid);
    if(idx < 0) return -1;
    struct kproc *proc = &procs[idx];

    if(proc->open_files[desc]) {
        return fs_create(proc->open_files[desc], name, perms);
    }

    return -1;
}

int proc_fs_ioctl(int desc, int req, void *args) {
    if(desc > MAX_OPEN_FILES) return -1;
    
    int idx = proc_by_pid(current_pid);
    if(idx < 0) return -1;
    struct kproc *proc = &procs[idx];

    if(proc->open_files[desc]) {
        return fs_ioctl(proc->open_files[desc], req, args);
    }

    return -1;
}

int proc_fs_getdirinfo(int desc, struct dirinfo *dinfo) {
    if(desc > MAX_OPEN_FILES) return -1;
    if(dinfo == NULL) return -1;

    int idx = proc_by_pid(current_pid);
    if(idx < 0) return -1;
    struct kproc *proc = &procs[idx];

    struct kfile *file = proc->open_files[desc];

    if(file) {
        dinfo->ino        = file->inode;
        if(file->parent) {
            dinfo->parent_ino = file->parent->inode;
        } else {
            dinfo->parent_ino = 0;
        }
        // Count children
        struct kfile *tmp = file->child;
        if(tmp) {
            do {
                dinfo->n_children++;
                tmp = tmp->next;
            } while(tmp && tmp != file->child);
        }

        memcpy(dinfo->name, file->name, FILE_NAME_MAX);
    }

    return 0;
}