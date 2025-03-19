#include <errno.h>
#include <sys/stat.h>

#include <lambda/export.h>
#include <proc/mtask.h>


int kfstat(kfile_t *f, kstat_t *buf) {
    if(!f || !buf) return -EINVAL;

    /* These options currently unsupported */
    buf->dev_id     = 0;
    buf->rdev_id    = 0;
    buf->n_link     = 0;

    buf->uid   = f->uid;
    buf->gid   = f->gid;
    buf->inode = f->inode;
    buf->mode  = f->pflags;

    buf->ctime = f->ctime;
    buf->mtime = f->mtime;

    /* TOOD: kfile_t probably should not handle symlinks */
    /* Get length of target file: */
    while(f->flags & FS_SYMLINK) {
        if(!f->link) {
            /* TODO: Open file to read symlink? */
            break;
        } else {
            f = f->link;
        }
    }

    buf->size       = f->length;
    /* @todo Actually determine these */
    buf->blksize    = 1;
    buf->alloc_size = buf->size;

    return 0;
}
EXPORT_FUNC(kfstat);

int fstat(int fd, kstat_t *buf) {
    if(fd < 0 || fd >= MAX_OPEN_FILES) return -EBADF;
    
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) return -EUNSPEC;

    return kfstat(thread->process->open_files[fd]->file, buf);
}
