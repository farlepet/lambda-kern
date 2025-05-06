#include <errno.h>
#include <libgen.h>
#include <string.h>

#include <crypto/comp/comp.h>
#include <err/error.h>
#include <err/panic.h>
#include <fs/initrd.h>
#include <mm/alloc.h>
#include <proc/atomic/tlock.h>

static int     _open(kfile_t *, kfile_hand_t *);
static ssize_t _read(kfile_hand_t *, size_t, size_t, void *);

static const file_ops_t _file_ops = {
    .open    = &_open,
    .mkdir   = NULL,
    .create  = NULL,
    .finddir = NULL
};

static const file_hand_ops_t _file_hand_ops = {
    .close   = NULL,
    .read    = &_read,
    .write   = NULL,
    .ioctl   = NULL,
    .readdir = NULL
};

static const struct header_old_cpio *cpio = NULL;

static size_t initrd_n_files = 0;

#define CPIO2LE(x) ((x << 16) | (x >> 16)) //!< Convert 32-bit CPIO values to little-endian

static ssize_t _read(kfile_hand_t *hand, size_t off, size_t sz, void *buff) {
    if(!hand->file ||
       !hand->file->info) return 0; // We must know where the CPIO data is

    kfile_t *f = hand->file;

    if(off >= f->length) return 0; // We cannot read past the end of the file

    uint8_t *data = f->info;

    uint32_t len = ((sz + off) > f->length) ? (f->length - off) : sz;
    memcpy(buff, &data[off], len);

    return len;
}

static int _open(kfile_t *f, kfile_hand_t *hand) {
    if(hand->open_flags & OFLAGS_WRITE) {
        kdebug(DEBUGSRC_FS, ERR_DEBUG, "initrd: _open: Attempted to open file for writing!");
        return -EROFS;
    }
    /* TODO: Further check open flags/permissions */
    
    tlock_acquire(&f->file_lock);

    if(f->flags & FS_SYMLINK) {
        /* We need to find the file this symlinks to */
        /* TODO: Ensure we do not exceed 127 characters */
        char symlink[128];
        memcpy(symlink, f->info, f->length);
        symlink[f->length] = '\0';
        tlock_release(&f->file_lock);
    
        f->link = fs_find_file(f->parent, symlink);
        
        if(f->link) {
            return fs_open(f->link, hand);
        } else {
            kdebug(DEBUGSRC_FS, ERR_DEBUG, "initrd: _open: Could not open following symlink!");
            return -ENOENT;
        }
    }

    hand->file = f;
    hand->ops  = &_file_hand_ops;

    hand->open_flags |= OFLAGS_OPEN;

    tlock_release(&f->file_lock);

    return 0;
}

/*static void _close(kfile_t *f) {
    tlock_acquire(&f->file_lock);
    f->open = 0;
    tlock_release(&f->file_lock);
}*/



void initrd_mount(kfile_t *mntpoint, uintptr_t initrd, size_t len) {
    cpio = (const struct header_old_cpio *)initrd;

#ifdef CONFIG_CRYPTO_COMP_LZOP
    if (!memcmp((void *)initrd, "\x89LZO", 4)) {
        kdebug(DEBUGSRC_FS, ERR_INFO, "  -> Decompressing InitCPIO");

        size_t comp_len = len;
        crypto_comp_handle_t handle = { .format = CRYPTO_COMP_FMT_LZOP, };
        int ret = crypto_comp_decompress(&handle, (void *)initrd, comp_len, (void **)&cpio, &len);
        if (ret) {
            kdebug(DEBUGSRC_FS, ERR_CRIT, "  -> Failed to decompress InitCPIO: %d", ret);
            return;
        }
        /* TODO: Free compressed data */
    }
#endif
    (void)len;

    if(cpio->c_magic != 070707) {
        kdebug(DEBUGSRC_FS, ERR_CRIT, "  -> Invalid CPIO magic number");
        return;
    }

    /* cpio data is read-only, so we need to copy the filename to get the basename */
    char *fname = (char *)kmalloc(FILE_NAME_MAX + 1);

    const struct header_old_cpio *cfile = cpio;

    while (1) {
        if(cfile->c_magic != 070707) {
            kdebug(DEBUGSRC_FS, ERR_CRIT, "  -> Invalid or corrupt InitCPIO!\n");
            kfree(fname);
            return;
        }


        const char *filename = (const char *)((uintptr_t)cfile + sizeof(struct header_old_cpio));
        
        if (!strcmp(filename, "TRAILER!!!")) {
            /* End of file list. */
            kfree(fname);
            return;
        }

        uintptr_t data = ((uint32_t)cfile + sizeof(struct header_old_cpio) + cfile->c_namesize + (cfile->c_namesize & 1));

        uint32_t filesize = CPIO2LE(cfile->c_filesize);

        kfile_t *file = (kfile_t *)kmalloc(sizeof(kfile_t));
        if(!file) {
            kpanic("Could not allocate enough memory for initrd file structures!");
        }
        memset(file, 0, sizeof(kfile_t));

        memset(fname, 0, FILE_NAME_MAX + 1);
        strncpy(fname, filename, FILE_NAME_MAX);

        char *name = basename(fname);
        memcpy(file->name, name, strlen(name));

        char *path = dirname(fname);

        kfile_t *dir = fs_find_file(mntpoint, path);
        if(dir == NULL) {
            /* TODO: Skip the file, or halt processing of mounting */
            kdebug(DEBUGSRC_FS, ERR_WARN, "initrd: Could not find directory for file `%s`, defaulting to root.", path);
            dir = fs_get_root();
        }

        file->length     = filesize;
        file->impl       = dir->inode;
        file->uid        = cfile->c_uid;
        file->gid        = cfile->c_gid;
        file->link       = 0; /* NOTE: This is handled upon first open, if applicable. */
        file->pflags     = cfile->c_mode & 07777;
        switch(cfile->c_mode & 0170000) {
            case CPIO_MODE_SOCKET:
                file->flags |= 0; // FIXME: Socket
                break;

            case CPIO_MODE_SYMLINK:
                /* Presently, symbolic links are traced during opening. */
                file->flags |= FS_SYMLINK;
                break;

            case CPIO_MODE_REGULAR:
                file->flags |= FS_FILE;
                break;

            case CPIO_MODE_BLOCK:
                file->flags |= FS_BLCKDEV;
                break;

            case CPIO_MODE_DIRECTORY:
                file->flags |= FS_DIR;
                break;

            case CPIO_MODE_CHAR:
                file->flags |= FS_CHARDEV;
                break;

            case CPIO_MODE_PIPE:
                file->flags |= FS_PIPE;
                break;
        }

        /* CPIO only gives us modification time. */
        file->atime      = 0;              /* Start access time afresh at each mount. */
        file->mtime      = CPIO2LE(cfile->c_mtime);
        file->ctime      = CPIO2LE(cfile->c_mtime);
        
        file->ops       = &_file_ops;

        file->info      = (void *)data;

        //cfile->c_ino    = fs_add_file(file, dir);
        fs_add_file(file, dir);

        cfile = (const struct header_old_cpio *)(data + filesize + (filesize & 1));

        initrd_n_files++;
    }
}
