#include <proc/mtask.h>

#include <sys/stat.h>

int kfstat(struct kfile *f, struct stat *buf) {
    if(!f)   return -1;
    if(!buf) return -1;

    /* These options currently unsupported */
    buf->st_dev     = 0;
    buf->st_mode    = 0;
    buf->st_nlink   = 0;
    buf->st_rdev    = 0;
    buf->st_atime   = 0;
    buf->st_mtime   = 0;
    buf->st_ctime   = 0;
    buf->st_blksize = 0;
    buf->st_blkcnt  = 0;

    buf->st_uid = f->uid;
    buf->st_gid = f->gid;
    buf->st_ino = f->inode;

    /* Get length of target file: */
    struct kfile *tmp = f;
    while(tmp->flags & FS_SYMLINK) {
        if(!tmp->link) {
            /* TODO: Open file to read symlink? */
        } else {
            tmp = tmp->link;
        }
    }
    
    buf->st_size = tmp->length;

    return 0;
}

int fstat(int fd, struct stat *buf) {
    if(fd < 0) return -1;

    if(fd >= MAX_OPEN_FILES) {
        return -1;
    }

    int idx = proc_by_pid(current_pid);
    if(idx < 0 || idx > MAX_PROCESSES) {
        return -1;
    }

    return kfstat(procs[idx].open_files[fd], buf);
}