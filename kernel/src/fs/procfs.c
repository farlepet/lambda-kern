#include <fs/fs.h>
#include <fs/procfs.h>
#include <proc/mtask.h>
#include <err/panic.h>
#include <err/error.h>
#include <fs/dirinfo.h>
#include <mm/alloc.h>
#include <mm/mm.h>

#include <string.h>

uint32_t proc_fs_read(int desc, uint32_t off, uint32_t sz, uint8_t *buff) {
    if(desc > MAX_OPEN_FILES) return 0;

    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return 0;
    
    if(thread->process->open_files[desc]) {
        return fs_read(thread->process->open_files[desc], off, sz, buff);
    }

    return 0;
}

uint32_t proc_fs_read_blk(int desc, uint32_t off, uint32_t sz, uint8_t *buff) {
    if(desc > MAX_OPEN_FILES) return 0;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return 0;

    kfile_hand_t *file = thread->process->open_files[desc];

    if(file) {
        if(file->file && (file->file->flags & FS_STREAM) &&
          !(file->open_flags & OFLAGS_NONBLOCK)) {
            uint32_t count = 0;
            while(count < sz) {
                uint32_t ret = fs_read(file, off, sz - count, buff + count);
                if(ret > 0x80000000) {
                    return count;
                }
                count += ret;
                run_sched(); // Wait
            }
            return count;
        } else {
            off = thread->process->file_position[desc];
            uint32_t n = fs_read(file, off, sz, buff);
            thread->process->file_position[desc] += n;
            return n;
        }
    }

    return 0;
}

uint32_t proc_fs_write(int desc, uint32_t off, uint32_t sz, uint8_t *buff) {
    if(desc > MAX_OPEN_FILES) return 0;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -1;

    if(thread->process->open_files[desc]) {
        return fs_write(thread->process->open_files[desc], off, sz, buff);
    }

    return 0;
}

static int _open_check_flags(struct kfile *file, uint32_t flags) {
    if(flags & OFLAGS_DIRECTORY) {
        if(!(file->flags & FS_DIR)) {
            return -1;
        }
    }

    return 0;
}

int proc_fs_open(const char *name, uint32_t flags) {
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -1;

    if(thread->process->cwd == NULL) {
        return -1;
    }


    char tmp[256];
    // TODO: Check for name length!
    memcpy(tmp, name, strlen(name)+1);
    struct kfile *file = fs_find_file(thread->process->cwd, tmp);
    if(file) {
        // TODO: Check if file is open!!!
        // TODO: Handle errors!
        // TODO: Make sure flags match up!
        if(_open_check_flags(file, flags)) {
            return -1;
        }

        kfile_hand_t *hand = (kfile_hand_t *)kmalloc(sizeof(kfile_hand_t));
        memset(hand, 0, sizeof(kfile_hand_t));

        hand->open_flags = flags;

        if(fs_open(file, hand)) {
            kdebug(DEBUGSRC_FS, ERR_DEBUG, "proc_fs_open: fs_open of %s failed!", name);
            kfree(hand);
            return -1;
        }

        // TODO: Handle errors!
        int ret = proc_add_file(thread->process, hand);
        if (ret > 0) {
            if(!SAFETY_CHECK(hand->open_flags & OFLAGS_OPEN)) {
                kpanic("Open succeeded, but open flag is not set!");
            }
            return ret;
        }

        kfree(hand);
    }

    return -1;
}

int proc_fs_close(int desc) {
    if(desc > MAX_OPEN_FILES) return -1;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -1;

    if(thread->process->open_files[desc]) {
        fs_close(thread->process->open_files[desc]);
        kfree(thread->process->open_files[desc]);
        thread->process->open_files[desc] = NULL;
        return 0; // TODO: Error checking!
    }

    return -1;
}

int proc_fs_mkdir(int desc, char *name, uint32_t perms) {
    if(desc > MAX_OPEN_FILES) return -1;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -1;
    
    if(thread->process->open_files[desc]) {
        return fs_mkdir(thread->process->open_files[desc]->file, name, perms);
    }

    return -1;
}

int proc_fs_create(int desc, char *name, uint32_t perms) {
    if(desc > MAX_OPEN_FILES) return -1;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -1;
    
    if(thread->process->open_files[desc]) {
        return fs_create(thread->process->open_files[desc]->file, name, perms);
    }

    return -1;
}

int proc_fs_ioctl(int desc, int req, void *args) {
    if(desc > MAX_OPEN_FILES) return -1;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -1;
    
    if(thread->process->open_files[desc]) {
        return fs_ioctl(thread->process->open_files[desc], req, args);
    }

    return -1;
}

int proc_fs_getdirinfo(int desc, struct dirinfo *dinfo) {
    if(desc > MAX_OPEN_FILES) return -1;
    if(dinfo == NULL)         return -1;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -1;

    kfile_t *file = thread->process->open_files[desc]->file;

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

int proc_fs_readdir(int desc, uint32_t idx, struct user_dirent *buff, uint32_t buff_size) {
    if(desc > MAX_OPEN_FILES) return -1;
    if(buff == NULL)          return -1;
    if(buff_size == 0)        return -1;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -1;

    kfile_t *file = thread->process->open_files[desc]->file;
    if(file == NULL) return -1;

    /* @todo Bounds checking */

    if(idx == 0) {
        /** . */
        buff->d_ino = file->inode;
        strcpy(buff->d_name, ".");
    } else if(idx == 1) {
        /** .. */
        if(file->parent != NULL) file = file->parent;
        buff->d_ino = file->inode;
        strcpy(buff->d_name, "..");
    } else {
        idx -= 2;
        file = file->child;
        if(file == NULL) return -1;
        
        struct kfile *fchild = file;

        while(idx--) {
            file = file->next;
            if(file == NULL)   return -1;
            if(file == fchild) return -1;
        }

        buff->d_ino = file->inode;
        strcpy(buff->d_name, file->name);
    }

    return (sizeof(struct user_dirent) + strlen(buff->d_name) + 1);
}

int proc_fs_stat(const char *path, kstat_t *buf, uint32_t __unused flags) {
    if(!mm_check_addr(path) ||
       !mm_check_addr(buf)) {
        return -1;
    }
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -1;

    if(thread->process->cwd == NULL) {
        return -1;
    }

    char tmp[256];
    // TODO: Check for name length!
    memcpy(tmp, path, strlen(path)+1);

    /* TODO: Check permissions */
    struct kfile *file = fs_find_file(thread->process->cwd, tmp);
    if(file) {
        return kfstat(file, buf);
    }

    return -1;
}