#include <fs/fs.h>

static struct kfile *kfiles;


static u32 c_inode = 0;

int fs_add_file(struct kfile *file)
{
	file->inode = c_inode++;

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
	return (struct dirent *)0;
}

struct kfile *fs_finddir(struct kfile *f, char *name)
{
	if(f && f->finddir)
		return f->finddir(f, name);
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
