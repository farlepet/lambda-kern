#include <lambda/export.h>
#include <proc/mtask.h>

#include <sys/stat.h>

int kfstat(kfile_hand_t *hand, struct stat *buf) {
    if(!hand || !hand->file || !buf) return -1;

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

    const kfile_t *f = hand->file;

    buf->st_uid = f->uid;
    buf->st_gid = f->gid;
    buf->st_ino = f->inode;

    /* TOOD: kfile_t probably should not handle symlinks */
    /* Get length of target file: */
    while(f->flags & FS_SYMLINK) {
        if(!f->link) {
            /* TODO: Open file to read symlink? */
        } else {
            f = f->link;
        }
    }
    
    buf->st_size = f->length;

    return 0;
}
EXPORT_FUNC(kfstat);

int fstat(int fd, struct stat *buf) {
    if(fd < 0 || fd >= MAX_OPEN_FILES) return -1;
    if(!curr_proc)                     return -1;

    return kfstat(curr_proc->open_files[fd], buf);
}