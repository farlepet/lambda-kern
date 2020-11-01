#include <multiboot.h>
#include <fs/initrd.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <string.h>

#include <libgen.h>

static struct header_old_cpio *cpio = 0;

static int n_files = 0;
/* TODO: To save space, check number of files present, then kmalloc this array. */
static struct header_old_cpio *files[0x1000]; // Table of initrd file locations
static char filenames[0x1000][128];           // Table of initrd filenames

#define n(x) ((x << 16) | (x >> 16))

static uint32_t initrd_read(struct kfile *f, uint32_t off, uint32_t sz, uint8_t *buff) {
	if(!f->info | !cpio) return 0; // We must know where the CPIO file is
	if(off >= f->length) return 0; // We cannot read past the end of the file

	uint8_t *data = f->info;

	uint32_t i   = 0;
	uint32_t end = (sz + off) > f->length ? (f->length - off) : sz;
	for(; i < end; i++) {
		buff[i] = data[off + i];
	}

	return i;
}

static uint32_t initrd_write(struct kfile *f, uint32_t off, uint32_t sz, uint8_t *buff) {
	// These go unused
	(void)f;
	(void)off;
	(void)sz;
	(void)buff;

	return 0; // There should be no reason to write to the files in the initrd
}

static void initrd_open(struct kfile *f, uint32_t flags) {
	lock(&f->file_lock);

	/* TODO: Maybe find a way to handle symlinks more globally, rather than
	 * specifically withing the fs handler. */
	if(f->flags & FS_SYMLINK) {
		/* We need to find the file this symlinks to */
		/* TODO: Ensure we do not exceed 127 characters */
		char symlink[128];
		memcpy(symlink, f->info, f->length);
		symlink[f->length] = '\0';

		f->link = fs_find_file(f->parent, symlink);
		if(f->link) {
			fs_open(f->link, flags);
		} else {
			return;
		}
	}
	
	/* TODO: Prevent opening for writing */
	f->open_flags = flags | OFLAGS_OPEN;

	unlock(&f->file_lock);
}

/*static void initrd_close(struct kfile *f) {
	lock(&f->file_lock);
	f->open = 0;
	unlock(&f->file_lock);
}*/



void initrd_mount(struct kfile *mntpoint, uintptr_t initrd, size_t __unused len) {
	cpio = (struct header_old_cpio *)initrd;

	if(cpio->c_magic != 070707) {
		kerror(ERR_MEDERR, "  -> Invalid CPIO magic number");
		return;
	}

	struct header_old_cpio *cfile = (struct header_old_cpio *)cpio;
	int cidx = 0;

	while (1) {
		if(cfile->c_magic != 0x71C7) {
			kerror(ERR_MEDERR, "  -> Invalid or corrupt InitCPIO!\n");
			return;
		}

		files[cidx] = cfile;

		cfile->c_mtime    = n(cfile->c_mtime);
		cfile->c_filesize = n(cfile->c_filesize);

		memcpy(filenames[cidx], (void *)((uint32_t)cfile + sizeof(struct header_old_cpio)), cfile->c_namesize);

		if (!strcmp(filenames[cidx], "TRAILER!!!")) {
			memset(&files[cidx], 0, sizeof(struct header_old_cpio));
			return;
		}

		uint32_t data = ((uint32_t)cfile + sizeof(struct header_old_cpio) + cfile->c_namesize + (cfile->c_namesize & 1));


		struct kfile *file = (struct kfile *)kmalloc(sizeof(struct kfile));
		memset(file, 0, sizeof(struct kfile));

		char *name = basename(filenames[cidx]);
		memcpy(file->name, name, strlen(name));

		char *path = dirname(filenames[cidx]);

		struct kfile *dir = mntpoint;
		if(path[0] != '.') {
			while(1) {
				for(uint32_t i = 0; i < strlen(path); i++) {
					if(path[i] == '/') {
						char *nextPath = &path[i+1];
						path[i] = '\0';

						//kerror(ERR_BOOTINFO, "initrd: looking for dir: [%s]", path);
						dir = fs_finddir(dir, path);
						kerror(ERR_BOOTINFO, "  -> %08X", dir);
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
		file->impl       = dir->inode; //fs_root->inode; // FIXME
		file->uid        = cfile->c_uid;
		file->gid        = cfile->c_gid;
		file->link       = 0; // FIXME
		file->open_flags = 0;
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
		file->atime      = cfile->c_mtime;
		file->mtime      = cfile->c_mtime;
		file->ctime      = cfile->c_mtime; // Ehh....
		
		file->read      = &initrd_read;
		file->write     = &initrd_write;
		file->open      = &initrd_open;

		file->info      = (void *)data;

		cfile->c_ino    = fs_add_file(file, dir);

		cfile = (struct header_old_cpio *)(data + cfile->c_filesize + (cfile->c_filesize & 1));

		cidx++;
		n_files++;
	}
}
