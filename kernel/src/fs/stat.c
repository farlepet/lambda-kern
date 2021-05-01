#include <lambda/export.h>
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
EXPORT_FUNC(kfstat);

int fstat(int fd, struct stat *buf) {
    if(fd < 0 || fd >= MAX_OPEN_FILES) return -1;
    if(!curr_proc)                     return -1;

    return kfstat(curr_proc->open_files[fd], buf);
}