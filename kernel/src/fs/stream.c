#include <lambda/export.h>
#include <data/cbuff.h>
#include <fs/stream.h>
#include <mm/alloc.h>
#include <string.h>

static uint32_t stream_read (struct kfile *f, uint32_t off, uint32_t sz, uint8_t *buff);
static uint32_t stream_write(struct kfile *f, uint32_t off, uint32_t sz, uint8_t *buff);
static void     stream_open (struct kfile *f, uint32_t flags);
static void     stream_close(struct kfile *f);


struct kfile *stream_create(int length) {
    if(length < 1) return NULL;

    struct kfile *file = (struct kfile *)kmalloc(sizeof(struct kfile));
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

    file->read  = stream_read;
    file->write = stream_write;
    file->open  = stream_open;
    file->close = stream_close;
    // TODO: Possibly implement IOCTL?

    fs_add_file(file, NULL);

    return file;
}
EXPORT_FUNC(stream_create);

static uint32_t stream_read(struct kfile *f, uint32_t off, uint32_t sz, uint8_t *buff) {
    if(!f->info) return 0;
    (void)off; // Offset goes unused, this is basically a queue

    uint32_t count = 0;
    int ret;

    while((count < sz) && !((ret = cbuff_get((struct cbuff *)f->info)) & CBUFF_ERRMSK)) {
        buff[count] = (uint8_t)ret;
        count++;
    }

    f->length -= count;

    return count;
}

static uint32_t stream_write(struct kfile *f, uint32_t off, uint32_t sz, uint8_t *buff) {
    if(!f->info) return 0;
    (void)off; // Offset goes unused, this is basically a queue

    uint32_t count = 0;

    while((count < sz) && !(cbuff_put(buff[count], (struct cbuff *)f->info) & CBUFF_ERRMSK)) {
        count++;
    }

    f->length += count;

    return count;
}

static void stream_open(struct kfile *f, uint32_t flags)
{
	lock(&f->file_lock);
	if(f->open) return; // TODO: Notify the process that the file could not be opened
	f->open_flags = flags | OFLAGS_OPEN;
	unlock(&f->file_lock);
}

static void stream_close(struct kfile *f)
{
	lock(&f->file_lock);
	f->open = 0;
    kfree(f->info);
	unlock(&f->file_lock);
}