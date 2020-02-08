#include <mm/alloc.h>
#include <err/error.h>
#include <string.h>
#include <fs/fs.h>

static struct kfile *kfiles;

struct kfile *fs_root;

static uint32_t c_inode = 1;

int fs_add_file(struct kfile *file, struct kfile *parent)
{
	//kerror(ERR_BOOTINFO, "  -> fs_add_file: %s, %d", file->name, file->length);
	file->inode     = c_inode++;
	file->file_lock = 0;

	if(!kfiles) {
		kfiles      = file;
		file->prev  = file;
		file->next  = file;
		file->magic = 0xF11E0000;
		fs_root     = file;
	} else {
		if(parent == NULL) parent = fs_root;

		file->parent = parent;

		// This assumes that the parent is in the filesystem already
		if(parent->child == NULL) {
			parent->child = file;
			file->next = file;
			file->prev = file;
		} else {
			struct kfile *last  = parent->child->prev;
			parent->child->prev = file;
			last->next          = file;
			file->prev          = last;
			file->next          = parent->child;
		}
		
		file->magic        = 0xF11E0000;
	}

	return file->inode;
}

uint32_t fs_read(struct kfile *f, uint32_t off, uint32_t sz, uint8_t *buff)
{
	if(f && f->read)
		return f->read(f, off, sz, buff);
	return 0;
}

uint32_t fs_write(struct kfile *f, uint32_t off, uint32_t sz, uint8_t *buff)
{
	if(f && f->write)
		return f->write(f, off, sz, buff);
	return 0;
}

void fs_open(struct kfile *f, uint32_t flags)
{
	if(f && f->open)
		f->open(f, flags);
	
	else {
		lock(&f->file_lock);
		if(f->open) return; // TODO: Notify the process that the file could not be opened
		f->open_flags = flags | OFLAGS_OPEN;
		unlock(&f->file_lock);
	}
}

void fs_close(struct kfile *f)
{
	if(f && f->close)
		f->close(f);
	
	else {
		lock(&f->file_lock);
		f->open = 0;
		unlock(&f->file_lock);
	}
}

struct dirent *fs_readdir(DIR *d)
{
	if(d && d->dir) {
		if(d->current == NULL) {d->prev = NULL; return NULL; }

		struct dirent *dent = kmalloc(sizeof(struct dirent));

		dent->ino = d->current->inode;
		memcpy(dent->name, d->current->name, FILE_NAME_MAX);

		if(d->current->next == d->dir->child) {
			d->prev = d->current;
			d->current = NULL;
		}
		else {
			d->prev = d->current;
			d->current = d->current->next;
		}

		return dent;
	}
	return NULL;
}

struct kfile *fs_finddir(struct kfile *f, char *name)
{
	if(f)
	{
		struct kfile *file = f->child;

		do {
			if(!strcmp((char *)file->name, name)) {
				return file;
			}
			file = file->next;
		} while(file != f->child);
	}
	return NULL;
}

struct kfile *fs_dirfile(DIR *d) {
	if(d == NULL) return NULL;

	return d->prev;
}

DIR *fs_opendir(struct kfile *f) {
	if(f == NULL) return NULL;

	DIR *stream = (DIR *)kmalloc(sizeof(DIR));

	stream->dir     = f;
	stream->current = f->child;
	stream->prev    = NULL;

	return stream;
}

int fs_mkdir(struct kfile *f, char *name, uint32_t perms)
{
	// TODO: Check name

	if(f && f->mkdir)
		return f->mkdir(f, name, perms);
	return -1;
}

int fs_create(struct kfile *f, char *name, uint32_t perms)
{
	// TODO: Check name

	if(f && f->create)
		return f->create(f, name, perms);
	return -1;
}

int fs_ioctl(struct kfile *f, int req, void *args)
{
	if(f && f->ioctl)
		return f->ioctl(f, req, args);
	return -1;
}


struct kfile *fs_find_file(struct kfile *f, char *path) {
	if(f == NULL) return NULL;
	if(path == NULL) return NULL;
	if(*path == 0) return NULL; // No path given


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
			return fs_find_file(fs_root, path + 1);
		} else {
			return fs_root;
		}
	} else {
		char *dir_sep = strchr(path, '/');
		if(dir_sep != NULL) {
			*dir_sep = '\0';
			dir_sep++;
			f = fs_finddir(f, path);
			// TODO: Check that file is a directory!
			return fs_find_file(f, dir_sep);
		} else {
			return fs_finddir(f, path);
		}
	}

	return NULL;
}



void fs_init()
{
	fs_root = (struct kfile *)kmalloc(sizeof(struct kfile));

	memset(fs_root, 0, sizeof(struct kfile));

	time_t time = 0; // TODO: Find actual time

	fs_root->name[0]    = '\0'; // The root directory has no name
	fs_root->length     = 0;
	fs_root->impl       = 0;
	fs_root->inode      = 0;
	fs_root->uid        = 0;
	fs_root->gid        = 0;
	fs_root->link       = NULL;
	fs_root->open_flags = 0;
	fs_root->pflags     = PERMISSIONS(PERM_RWE, PERM_RE, PERM_RE); // rwxr-xr-x
	fs_root->flags      = FS_DIR;
	fs_root->atime      = time;
	fs_root->mtime      = time;
	fs_root->ctime      = time;

	fs_add_file(fs_root, NULL);
}

void fs_debug(int nfiles)
{
	(void)nfiles;

	struct kfile *f = fs_root->child;
	//DIR *dir = fs_opendir(fs_root);
	//struct dirent *d;
	//d = fs_readdir(dir);

	do {
		kerror(ERR_BOOTINFO, "FSDBG::%c%c%c%c%c%c%c%c%c%s %d %s",
			((f->pflags&0400)?'r':'-'), ((f->pflags&0200)?'w':'-'), ((f->pflags&04000)?'s':((f->pflags&0100)?'x':'-')),
			((f->pflags&040)?'r':'-'), ((f->pflags&020)?'w':'-'), ((f->pflags&02000)?'s':((f->pflags&010)?'x':'-')),
			((f->pflags&04)?'r':'-'), ((f->pflags&02)?'w':'-'), ((f->pflags&01)?'x':'-'),
			((f->pflags&01000)?"T ":" "),
			f->inode, f->name);

		//d = fs_readdir(dir);
		f = f->next;
	} while(f != fs_root->child);
}
