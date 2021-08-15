#include <multiboot.h>
#include <fs/initrd.h>
#include <err/error.h>
#include <err/panic.h>
#include <mm/alloc.h>
#include <string.h>

#include <libgen.h>

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

static struct header_old_cpio *cpio = NULL;

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
		return -1;
	}
	/* TODO: Further check open flags/permissions */
	
	lock(&f->file_lock);

	if(f->flags & FS_SYMLINK) {
		/* We need to find the file this symlinks to */
		/* TODO: Ensure we do not exceed 127 characters */
		char symlink[128];
		memcpy(symlink, f->info, f->length);
		symlink[f->length] = '\0';

		f->link = fs_find_file(f->parent, symlink);
		if(f->link) {
			fs_open(f->link, hand);
		} else {
			/* TODO: Find link */
			unlock(&f->file_lock);
			return -1;
		}
	}

	hand->file = f;
	hand->ops  = &_file_hand_ops;

	hand->open_flags |= OFLAGS_OPEN;

	unlock(&f->file_lock);

	return 0;
}

/*static void _close(kfile_t *f) {
	lock(&f->file_lock);
	f->open = 0;
	unlock(&f->file_lock);
}*/



void initrd_mount(kfile_t *mntpoint, uintptr_t initrd, size_t __unused len) {
	cpio = (struct header_old_cpio *)initrd;

	if(cpio->c_magic != 070707) {
		kerror(ERR_MEDERR, "  -> Invalid CPIO magic number");
		return;
	}

	struct header_old_cpio *cfile = (struct header_old_cpio *)cpio;

	while (1) {
		if(cfile->c_magic != 070707) {
			kerror(ERR_MEDERR, "  -> Invalid or corrupt InitCPIO!\n");
			return;
		}

		cfile->c_mtime    = CPIO2LE(cfile->c_mtime);
		cfile->c_filesize = CPIO2LE(cfile->c_filesize);

		char *filename = (char *)((uintptr_t)cfile + sizeof(struct header_old_cpio));
		
		if (!strcmp(filename, "TRAILER!!!")) {
			/* End of file list. */
			return;
		}

		uintptr_t data = ((uint32_t)cfile + sizeof(struct header_old_cpio) + cfile->c_namesize + (cfile->c_namesize & 1));

		kfile_t *file = (kfile_t *)kmalloc(sizeof(kfile_t));
		if(!file) {
			kpanic("Could not allocate enough memory for initrd file structures!");
		}
		memset(file, 0, sizeof(kfile_t));

		char *name = basename(filename);
		memcpy(file->name, name, strlen(name));

		char *path = dirname(filename);

		kfile_t *dir = mntpoint;
		if(path[0] != '.') {
			while(1) {
				for(uint32_t i = 0; i < strlen(path); i++) {
					if(path[i] == '/') {
						char *nextPath = &path[i+1];
						path[i] = '\0';

						dir = fs_finddir(dir, path);
						if(dir == NULL) { // Default to '/'
							dir = fs_root;
							break;
						}

						if(*nextPath) {
							path = nextPath;
							continue;
						}
					}
				}
				dir = fs_finddir(dir, path);
				if(dir == NULL) {
					dir = fs_root;
				}
				break;
			}
		}
		
		
		file->length     = cfile->c_filesize;
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
		file->mtime      = cfile->c_mtime;
		file->ctime      = cfile->c_mtime;
		
		file->ops       = &_file_ops;

		file->info      = (void *)data;

		cfile->c_ino    = fs_add_file(file, dir);

		cfile = (struct header_old_cpio *)(data + cfile->c_filesize + (cfile->c_filesize & 1));

		initrd_n_files++;
	}
}
