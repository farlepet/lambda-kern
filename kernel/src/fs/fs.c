#include <fs/fs.h>

struct kfile files[1024]; // TODO: This is VERY bad! Fix malloc inplementation to work using multiple alloc`s per page!!!

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
