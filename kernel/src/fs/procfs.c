#include <errno.h>
#include <string.h>

#include <fs/fs.h>
#include <fs/procfs.h>
#include <proc/mtask.h>
#include <err/panic.h>
#include <err/error.h>
#include <fs/dirinfo.h>
#include <mm/alloc.h>
#include <mm/mm.h>


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
    if(!thread) return -EUNSPEC;

    if(thread->process->open_files[desc]) {
        return fs_write(thread->process->open_files[desc], off, sz, buff);
    }

    return 0;
}

static int _open_check_flags(struct kfile *file, uint32_t flags) {
    if(flags & OFLAGS_DIRECTORY) {
        if(!(file->flags & FS_DIR)) {
            return -ENOTDIR;
        }
    }

    return 0;
}

int proc_fs_open(const char *name, uint32_t flags) {
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -EUNSPEC;

    if(thread->process->cwd == NULL) {
        return -EUNSPEC;
    }

    struct kfile *file = fs_find_file(thread->process->cwd, name);
    if(!file) {
        return -ENOENT;
    }

    // TODO: Check if file is open!!!
    // TODO: Handle errors!
    // TODO: Make sure flags match up!
    TRY_OR_RET(_open_check_flags(file, flags));

    kfile_hand_t *hand = (kfile_hand_t *)kmalloc(sizeof(kfile_hand_t));
    memset(hand, 0, sizeof(kfile_hand_t));

    hand->open_flags = flags;

    if(fs_open(file, hand)) {
        kdebug(DEBUGSRC_FS, ERR_DEBUG, "proc_fs_open: fs_open of %s failed!", name);
        kfree(hand);
        return -EUNSPEC;
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

    return -ENFILE;
}

int proc_fs_close(int desc) {
    if(desc > MAX_OPEN_FILES) return -EBADF;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -EUNSPEC;

    if(thread->process->open_files[desc]) {
        fs_close(thread->process->open_files[desc]);
        kfree(thread->process->open_files[desc]);
        thread->process->open_files[desc] = NULL;
        return 0; // TODO: Error checking!
    }

    return -EBADF;
}

int proc_fs_mkdir(int desc, char *name, uint32_t perms) {
    if(desc > MAX_OPEN_FILES) return -EBADF;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -EUNSPEC;
    
    if(thread->process->open_files[desc]) {
        return fs_mkdir(thread->process->open_files[desc]->file, name, perms);
    }

    return -EBADF;
}

int proc_fs_create(int desc, char *name, uint32_t perms) {
    if(desc > MAX_OPEN_FILES) return -EBADF;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -EUNSPEC;
    
    if(thread->process->open_files[desc]) {
        return fs_create(thread->process->open_files[desc]->file, name, perms);
    }

    return -EBADF;
}

int proc_fs_ioctl(int desc, int req, void *args) {
    if(desc > MAX_OPEN_FILES) return -EBADF;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -EUNSPEC;
    
    if(thread->process->open_files[desc]) {
        return fs_ioctl(thread->process->open_files[desc], req, args);
    }

    return -EBADF;
}

int proc_fs_getdirinfo(int desc, struct dirinfo *dinfo) {
    if(desc > MAX_OPEN_FILES) return -EBADF;
    if(dinfo == NULL)         return -EINVAL;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -EUNSPEC;

    kfile_t *file = thread->process->open_files[desc]->file;

    if(file) {
        dinfo->ino        = file->inode;
        if(file->parent) {
            dinfo->parent_ino = file->parent->inode;
        } else {
            dinfo->parent_ino = 0;
        }
        dinfo->n_children = llist_count(&file->children);

        memcpy(dinfo->name, file->name, FILE_NAME_MAX);
    }

    return 0;
}

int proc_fs_readdir(int desc, uint32_t idx, struct user_dirent *buff, uint32_t buff_size) {
    if(desc > MAX_OPEN_FILES) return -EBADF;
    if(buff == NULL)          return -EINVAL;
    if(buff_size == 0)        return -EINVAL;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -EUNSPEC;

    kfile_t *file = thread->process->open_files[desc]->file;
    if(file == NULL) return -EBADF;

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

        llist_item_t *child = llist_get(&file->children, idx);
        if(child == NULL) { return -EUNSPEC; }
        file = (kfile_t *)child->data;

        buff->d_ino = file->inode;
        strcpy(buff->d_name, file->name);
    }

    return (sizeof(struct user_dirent) + strlen(buff->d_name) + 1);
}

int proc_fs_stat(const char *path, kstat_t *buf, uint32_t __unused flags) {
    if(!mm_check_addr(path) ||
       !mm_check_addr(buf)) {
        return -EINVAL;
    }
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -EUNSPEC;

    if(thread->process->cwd == NULL) {
        return -EUNSPEC;
    }

    /* TODO: Check permissions */
    struct kfile *file = fs_find_file(thread->process->cwd, path);
    if(!file) {
        return -ENOENT;
    }

    return kfstat(file, buf);
}

int proc_fs_access(int dirfd, const char *pathname, uint32_t __unused mode, uint32_t flags) {
    kthread_t *thread = mtask_get_curr_thread();

    kfile_t *dir = thread->process->cwd;
    if(flags & SYSCALL_ACCESS_FLAG_CWDOVER) {
        if((dirfd < 0) ||
           (dirfd >= MAX_OPEN_FILES) ||
           (thread->process->open_files[dirfd] == NULL)) {
            /* @todo Return error code */
            return -EBADF;
        }
        dir = thread->process->open_files[dirfd]->file;
    } else {
        dir = thread->process->cwd;
    }

    kfile_t *file = fs_find_file(dir, pathname);
    if(file == NULL) {
        return -ENOENT;
    }

    /* @todo Actually check permissions - right now this only check existence! */
    return 0;
}
