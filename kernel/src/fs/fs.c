#include <mm/alloc.h>
#include <err/error.h>
#include <string.h>
#include <fs/fs.h>

static struct kfile *kfiles;

struct kfile *fs_root;

static u32 c_inode = 1;

int fs_add_file(struct kfile *file)
{
	kerror(ERR_BOOTINFO, "  -> fs_add_file: %s, %d", file->name, file->length);
	file->inode     = c_inode++;
	file->file_lock = 0;

	if(!kfiles)
	{
		kfiles          = file;
		file->prev_file = file;
		file->next_file = file;
		file->magic     = 0xF11E0000;
	}
	else
	{
		struct kfile *last = kfiles->prev_file;
		kfiles->prev_file  = file;
		last->next_file    = file;
		file->next_file    = kfiles;
		file->prev_file    = last;
		file->magic        = 0xF11E0000;
	}

	return file->inode;
}

u32 fs_read(struct kfile *f, u32 off, u32 sz, u8 *buff)
{
	if(f && f->read)
		return f->read(f, off, sz, buff);
	return 0;
}

u32 fs_write(struct kfile *f, u32 off, u32 sz, u8 *buff)
{
	if(f && f->write)
		return f->write(f, off, sz, buff);
	return 0;
}

void fs_open(struct kfile *f, u32 flags)
{
	if(f && f->open)
		f->open(f, flags);
}

void fs_close(struct kfile *f)
{
	if(f && f->close)
		f->close(f);
}

struct dirent *fs_readdir(struct kfile *f, u32 idx)
{
	if(f && f->readdir)
		return f->readdir(f, idx);
	else if(f)
	{
		u32 i     = 0;
		u32 inode = f->inode;

		struct dirent *dent = kmalloc(sizeof(struct dirent));

		struct kfile *file = fs_root->next_file;
		while(file != fs_root)
		{
			if(file->impl == inode) i++;
			if(i == idx)
			{
				dent->ino = file->inode;
				memcpy(dent->name, file->name, FILE_NAME_MAX);
				kerror(ERR_BOOTINFO, "<ino, name, len>: %d, %s, %d", file->inode, file->name, file->length);
				return dent; 
			}
			file = file->next_file;
		}
		kfree(dent);
	}
	return (struct dirent *)0;
}

struct kfile *fs_finddir(struct kfile *f, char *name)
{
	if(f && f->finddir)
		return f->finddir(f, name);
	else if(f)
	{
		struct kfile *file = fs_root;

		int i = 0;
		while(file != fs_root || i == 0)
		{
			if(file->impl == f->inode)
				if(!strcmp((char *)file->name, name))
					return file;
			i++;
			file = file->next_file;
		}
	}
	return (struct kfile *)0;
}

int fs_mkdir(struct kfile *f, char *name, u32 perms)
{
	// TODO: Check name

	if(f && f->mkdir)
		return f->mkdir(f, name, perms);
	return -1;
}

int fs_create(struct kfile *f, char *name, u32 perms)
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



void fs_init()
{
	fs_root = (struct kfile *)kmalloc(sizeof(struct kfile));

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

	// TODO: This should not be this way, it will only work for files OTHER than `/`
	fs_root->open       = 0;
	fs_root->close      = 0;
	fs_root->read       = 0;
	fs_root->write      = 0;
	fs_root->readdir    = 0;
	fs_root->finddir    = 0;
	fs_root->mkdir      = 0;
	fs_root->create     = 0;
	fs_root->ioctl      = 0;

	fs_root->info       = NULL;

	fs_add_file(fs_root);
}

void fs_debug(int nfiles)
{
	struct kfile *f = kfiles;

	while(nfiles--)
	{
		kerror(ERR_BOOTINFO, "FSDBG::%c%c%c%c%c%c%c%c%c%s %d %s",
			((f->pflags&0400)?'r':'-'), ((f->pflags&0200)?'w':'-'), ((f->pflags&04000)?'s':((f->pflags&0100)?'x':'-')),
			((f->pflags&040)?'r':'-'), ((f->pflags&020)?'w':'-'), ((f->pflags&02000)?'s':((f->pflags&010)?'x':'-')),
			((f->pflags&04)?'r':'-'), ((f->pflags&02)?'w':'-'), ((f->pflags&01)?'x':'-'),
			((f->pflags&01000)?"T ":" "),
			f->inode, f->name);

		f = f->next_file;
	}
}
