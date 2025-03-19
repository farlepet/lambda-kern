#include <errno.h>
#include <string.h>

#include <lambda/export.h>
#include <data/cbuff.h>
#include <fs/stream.h>
#include <mm/alloc.h>
#include <proc/atomic/tlock.h>

static ssize_t _read (kfile_hand_t *, size_t off, size_t sz, void *);
static ssize_t _write(kfile_hand_t *, size_t off, size_t sz, const void *);
static int     _open (kfile_t *, kfile_hand_t *);
static int     _close(kfile_hand_t *);

static const file_ops_t _file_ops = {
    .open    = &_open,
    .mkdir   = NULL,
    .create  = NULL,
    .finddir = NULL
};

static const file_hand_ops_t _file_hand_ops = {
    .close   = &_close,
    .read    = &_read,
    .write   = &_write,
    .ioctl   = NULL,
    .readdir = NULL
};

kfile_t *stream_create(int length) {
    if(length < 1) return NULL;

    kfile_t *file = (kfile_t *)kmalloc(sizeof(kfile_t));
    memset(file, 0, sizeof(kfile_t));

    cbuff_t *buff = (cbuff_t *)kmalloc(sizeof(cbuff_t) + length);
    memset(buff, 0, sizeof(cbuff_t));
    buff->buff = (uint8_t *)((uintptr_t)buff + sizeof(cbuff_t));
    buff->size = length;

    file->info = buff;

    // TODO: Should GID and UID be set to something?
    file->pflags = 0777; // TODO
    file->flags = FS_STREAM; // TODO: Should this be a pipe?
    // TODO: Creation time

    file->ops = &_file_ops;

    fs_add_file(file, NULL);

    return file;
}
EXPORT_FUNC(stream_create);

static ssize_t _read(kfile_hand_t *hand, size_t off, size_t sz, void *buff) {
    if(!hand->file || !hand->file->info) return -EINVAL;
    (void)off; // Offset goes unused, this is basically a queue

    kfile_t *f = hand->file;
    uint32_t count = 0;
    int ret;

    while((count < sz) && ((ret = cbuff_get((cbuff_t *)f->info)) >= 0)) {
        ((uint8_t *)buff)[count] = (uint8_t)ret;
        count++;
    }

    f->length -= count;

    return count;
}

static ssize_t _write(kfile_hand_t *hand, size_t off, size_t sz, const void *buff) {
    if(!hand->file || !hand->file->info) return -EINVAL;
    (void)off; // Offset goes unused, this is basically a queue

    kfile_t *f = hand->file;
    uint32_t count = 0;

    while((count < sz) && (cbuff_put(((uint8_t *)buff)[count], (cbuff_t *)f->info) >= 0)) {
        count++;
    }

    f->length += count;

    return count;
}

static int _open(kfile_t *f, kfile_hand_t *hand)
{
    /* TODO: Check open flags */
    tlock_acquire(&f->file_lock);
    hand->ops  = &_file_hand_ops;
    hand->file = f;
    hand->open_flags |= OFLAGS_OPEN;
    tlock_release(&f->file_lock);

    return 0;
}

static int _close(kfile_hand_t *hand)
{
    /* TODO: Check if any other file handles reference the file */

    tlock_acquire(&hand->lock);
    hand->open_flags = 0;
    /*kfree(hand->file->info);*/
    tlock_release(&hand->lock);

    return 0;
}
