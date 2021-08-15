#include <lambda/export.h>
#include <data/cbuff.h>
#include <fs/stream.h>
#include <mm/alloc.h>
#include <string.h>

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
    memset(file, 0, sizeof(struct kfile));

    struct cbuff *buff = (struct cbuff *)kmalloc(sizeof(struct cbuff) + length);
    buff->buff  = (uint8_t *)((ptr_t)buff + sizeof(struct cbuff));
    buff->head  = 0;
    buff->tail  = 0;
    buff->count = 0;
    buff->size  = length;

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
    if(!hand->file || !hand->file->info) return 0;
    (void)off; // Offset goes unused, this is basically a queue

    kfile_t *f = hand->file;
    uint32_t count = 0;
    int ret;

    while((count < sz) && !((ret = cbuff_get((struct cbuff *)f->info)) & CBUFF_ERRMSK)) {
        ((uint8_t *)buff)[count] = (uint8_t)ret;
        count++;
    }

    f->length -= count;

    return count;
}

static ssize_t _write(kfile_hand_t *hand, size_t off, size_t sz, const void *buff) {
    if(!hand->file || !hand->file->info) return 0;
    (void)off; // Offset goes unused, this is basically a queue

    kfile_t *f = hand->file;
    uint32_t count = 0;

    while((count < sz) && !(cbuff_put(((uint8_t *)buff)[count], (struct cbuff *)f->info) & CBUFF_ERRMSK)) {
        count++;
    }

    f->length += count;

    return count;
}

static int _open(kfile_t *f, kfile_hand_t *hand)
{
    /* TODO: Check open flags */
	lock(&f->file_lock);
    hand->ops  = &_file_hand_ops;
    hand->file = f;
	hand->open_flags |= OFLAGS_OPEN;
	unlock(&f->file_lock);

    return 0;
}

static int _close(kfile_hand_t *hand)
{
    /* TODO: Check if any other file handles reference the file */

	lock(&hand->lock);
	hand->open_flags = 0;
    /*kfree(hand->file->info);*/
	unlock(&hand->lock);

    return 0;
}
