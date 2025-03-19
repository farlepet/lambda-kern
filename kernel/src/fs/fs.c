#include "types.h"
#include <errno.h>
#include <string.h>

#include <lambda/config_defs.h>
#include <lambda/export.h>
#include <data/llist.h>
#include <err/error.h>
#include <err/panic.h>
#include <mm/alloc.h>
#include <fs/fs.h>
#include <proc/atomic/tlock.h>

#include <sys/stat.h>

static kfile_t *_fs_root = NULL;

static uint32_t c_inode = 1;

int fs_add_file(kfile_t *file, kfile_t *parent) {
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_LOWIMPACT)
    if (!_fs_root) {
        kpanic("VFS not yet initialized!");
    }
    if (!file) {
        kpanic("file is NULL!");
    }
#endif
    //kerror(ERR_BOOTINFO, "  -> fs_add_file: %s, %d", file->name, file->length);
    file->inode     = c_inode++;
    TRY_OR_RET(tlock_init(&file->file_lock));

    if(parent == NULL) parent = _fs_root;

    file->list_item.data = file;

    llist_append(&parent->children, &file->list_item);
    file->parent = parent;

    llist_init(&file->children);

    return file->inode;
}
EXPORT_FUNC(fs_add_file);

ssize_t fs_read(kfile_hand_t *hand, uint32_t off, uint32_t sz, void *buff) {
    if(!SAFETY_CHECK(hand) ||
       !SAFETY_CHECK(hand->ops)) {
        return 0;
    }
    
    if(hand->ops->read) {
        return hand->ops->read(hand, off, sz, buff);
    }

    return 0;
}
EXPORT_FUNC(fs_read);

ssize_t fs_write(kfile_hand_t *hand, uint32_t off, uint32_t sz, const void *buff) {
    if(!SAFETY_CHECK(hand) ||
       !SAFETY_CHECK(hand->ops)) {
        return 0;
    }

    if(hand->ops->write) {
        return hand->ops->write(hand, off, sz, buff);
    }

    return 0;
}
EXPORT_FUNC(fs_write);

int fs_open(kfile_t *f, kfile_hand_t *hand) {
    if((f == NULL) ||
       (hand == NULL)) {
       return -EINVAL;
    }

    if(hand->open_flags & OFLAGS_OPEN) {
        /* File is already open, or flags incorrectly set */
        return -EINVAL;
    }

    if(f->ops && f->ops->open) {
        return f->ops->open(f, hand);
    } else {
        /* TODO: Check requested flags */
        /* TODO: Keep track of active references in kfile_t */
        hand->open_flags |= OFLAGS_OPEN;
        hand->file = f;
    }

    return 0;
}
EXPORT_FUNC(fs_open);

int fs_close(kfile_hand_t *hand) {
    if(hand == NULL) return -EINVAL;
    
    if(SAFETY_CHECK(hand->ops) &&
       hand->ops->close) {
        hand->ops->close(hand);
    }

    tlock_acquire(&hand->lock);
    hand->open_flags = 0;
    tlock_release(&hand->lock);

    return 0;
}
EXPORT_FUNC(fs_close);

struct dirent *fs_readdir(DIR *d) {
    if(d && d->dir) {
        if(d->current == NULL) { d->prev = NULL; return NULL; }

        struct dirent *dent = kmalloc(sizeof(struct dirent));

        dent->ino = d->current->inode;
        memcpy(dent->name, d->current->name, FILE_NAME_MAX);

        d->prev = d->current;
        if(d->current->list_item.next == d->dir->children.list) {
            d->current = NULL;
        } else {
            d->current = (kfile_t *)d->current->list_item.next->data;
        }

        return dent;
    }
    return NULL;
}
EXPORT_FUNC(fs_readdir);

kfile_t *fs_finddir(kfile_t *f, const char *name) {
    if(f && f->children.list) {
        llist_iterator_t iter;
        kfile_t         *file;

        llist_iterator_init(&f->children, &iter);
        while(llist_iterate(&iter, (void **)&file)) {
            if(!strcmp((char *)file->name, name)) {
                return file;
            }
        }
    }
    return NULL;
}
EXPORT_FUNC(fs_finddir);

kfile_t *fs_dirfile(DIR *d) {
    if(!SAFETY_CHECK(d)) return NULL;

    return d->prev;
}

DIR *fs_opendir(kfile_t *f) {
    if(!SAFETY_CHECK(f)) return NULL;

    DIR *stream = (DIR *)kmalloc(sizeof(DIR));

    stream->dir     = f;
    stream->current = (kfile_t *)f->children.list->data;
    stream->prev    = NULL;

    return stream;
}

int fs_mkdir(kfile_t *f, const char *name, uint32_t perms)
{
    if(!SAFETY_CHECK(f) || SAFETY_CHECK(f->ops) || !f->ops->mkdir) {
        return -EINVAL;
    }
    // TODO: Check name

    return f->ops->mkdir(f, name, perms);
}

int fs_create(kfile_t *f, const char *name, uint32_t perms)
{
    if(!SAFETY_CHECK(f) || SAFETY_CHECK(f->ops) || !f->ops->create) {
        return -EINVAL;
    }
    // TODO: Check name

    return f->ops->create(f, name, perms);
}
EXPORT_FUNC(fs_create);

int fs_ioctl(kfile_hand_t *hand, int req, void *args)
{
    if(!SAFETY_CHECK(hand) || SAFETY_CHECK(hand->ops) || !hand->ops->ioctl) {
        return -EINVAL;
    }

    return hand->ops->ioctl(hand, req, args);
}
EXPORT_FUNC(fs_ioctl);


kfile_t *fs_find_file(kfile_t *f, const char *path) {
    if(path == NULL) return NULL;
    if(*path == 0)   return NULL; // No path given

    if(f == NULL) {
        f = _fs_root;
    }

    if(path[0] == '.') {
        if(path[1] == '.') { // [..]
            if(strlen(path) > 2) {
                if(path[2] == '/') path += 3;
                else               path += 2;
                return fs_find_file(f->parent, path);
            } else {
                return f->parent;
            }
        } else { // [.]
            if(strlen(path) > 1) {
                if(path[1] == '/') path += 2;
                else               path += 1;
                return fs_find_file(f, path);
            } else {
                return f;
            }
        } 
    } else if(path[0] == '/') {
        if(strlen(path) > 1) {
            return fs_find_file(_fs_root, path + 1);
        } else {
            return _fs_root;
        }
    } else {
        char *dir_sep = strchr(path, '/');
        if(dir_sep != NULL) {
            /* Create temporary string to hold the directory. TODO: Ensure
             * the length is reasonable. This method is also somewhat
             * inneffecient. If we create a wrapper for this function, we
             * could do one allocation, then this function cah modify the
             * contents freely. */
            char *dir = (char *)kmalloc(dir_sep - path + 1);
            memcpy(dir, path, dir_sep - path);
            dir[dir_sep - path] = '\0';
            dir_sep++;
    
            f = fs_finddir(f, dir);
            kfree(dir);
            
            if(f) {
                return fs_find_file(f, dir_sep);
            } else {
                return NULL;
            }
        } else {
            return fs_finddir(f, path);
        }
    }

    return NULL;
}
EXPORT_FUNC(fs_find_file);


kfile_hand_t *fs_handle_create(void) {
    kfile_hand_t *ret = (kfile_hand_t *)kmalloc(sizeof(kfile_hand_t));
    if(!ret) {
        return NULL;
    }
    memset(ret, 0, sizeof(kfile_hand_t));
    return ret;
}

int fs_handle_destroy(kfile_hand_t *hand) {
    if(!hand) {
        return -EINVAL;
    }
    kfree(hand);
    return 0;
}

kfile_hand_t *fs_handle_create_open(kfile_t *f, uint32_t flags) {
    if(!f) {
        return NULL;
    }
    
    kfile_hand_t *hand = fs_handle_create();
    if(!hand) {
        return NULL;
    }
    
    hand->open_flags = flags;
    if(fs_open(f, hand)) {
        fs_handle_destroy(hand);
        return NULL;
    }

    return hand;
}

static int __read_file(kfile_hand_t *file, void **buff, size_t *sz, size_t max_sz) {
    /* NOTE: Not performing checks on input, as that should have been done by the caller */
    if(max_sz == 0) {
        max_sz = SIZE_MAX;
    }
    
    kstat_t _stat;
    if(kfstat(file->file, &_stat)) {
        return -1;
    }

    if(_stat.size < max_sz) {
        max_sz = (size_t)_stat.size;
    }

    *buff = kmalloc(max_sz);
    if(*buff == NULL) {
        return -ENOMEM;
    }

    ssize_t read = fs_read(file, 0, max_sz, *buff);
    if(read < 0) {
        kfree(*buff);
        return read;
    }

    /* TODO: Should we error on the case where read != max_sz? */
    *sz = (size_t)read;

    return 0;
}

int fs_read_file_by_path(const char *path, kfile_t *cwd, void **buff, size_t *sz, size_t max_sz) {
    if((path   == NULL) ||
       (buff   == NULL) ||
       (sz     == NULL)) {
        return -EINVAL;
    }
    
    if(cwd == NULL) {
        cwd = _fs_root;
    }
    kfile_t *f = fs_find_file(cwd, path);
    if(f == NULL) {
        return -ENOENT;
    }

    kfile_hand_t *hand = fs_handle_create_open(f, OFLAGS_READ);
    if(hand == NULL) {
        return -EUNSPEC;
    }

    int ret = __read_file(hand, buff, sz, max_sz);

    fs_handle_destroy(hand);

    return ret;
}

kfile_t *fs_get_root(void) {
    return _fs_root;
}

void fs_init() {
    _fs_root = (kfile_t *)kmalloc(sizeof(kfile_t));
    if(!_fs_root) {
        kerror(ERR_EMER, "fs_init(): Failed to allocate memory for root!");
        return;
    }

    memset(_fs_root, 0, sizeof(kfile_t));

    time_t time = 0; // TODO: Find actual time

    _fs_root->name[0]    = '\0'; // The root directory has no name
    _fs_root->length     = 0;
    _fs_root->impl       = 0;
    _fs_root->inode      = 0;
    _fs_root->uid        = 0;
    _fs_root->gid        = 0;
    _fs_root->link       = NULL;
    _fs_root->pflags     = PERMISSIONS(PERM_RWE, PERM_RE, PERM_RE); // rwxr-xr-x
    _fs_root->flags      = FS_DIR;
    _fs_root->atime      = time;
    _fs_root->mtime      = time;
    _fs_root->ctime      = time;

    llist_init(&_fs_root->children);
}
