#include <multiboot.h>
#include <fs/initrd.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <string.h>
#include <fs/fs.h>

#if defined(ARCH_X86)
#include <mm/paging.h>
#endif

struct mboot_module *initrd = 0;


static struct header_old_cpio *cpio = 0;

static int n_files = 0;
static struct header_old_cpio *files[0x1000]; // Table of initrd file locations
static char filenames[0x1000][128];           // Table of initrd filenames

char *cpio_name = NULL; 

#define n(x) ((x << 16) | (x >> 16))

static u32 initrd_read(struct kfile *f, u32 off, u32 sz, u8 *buff)
{
	if(!f->info | !cpio) return 0; // We must know where the CPIO file is
	if(off >= f->length) return 0; // We cannot read past the end of the file

	struct header_old_cpio *cfile = (struct header_old_cpio *)f->info;
	u8 *data = (u8 *)((ptr_t)cfile + sizeof(struct header_old_cpio) + cfile->c_namesize + (cfile->c_namesize&1));

	u32 i = off;
	for(; i < (((sz + off) > f->length) ? (u32)(sz + buff) : (f->length)); i++)
	{
		buff[i] = data[i]; 
	}

	return i - off;
}

static u32 initrd_write(struct kfile *f, u32 off, u32 sz, u8 *buff)
{
	// These go unused
	(void)f;
	(void)off;
	(void)sz;
	(void)buff;

	return 0; // There should be no reason to write to the files in the initrd
}

static void initrd_open(struct kfile *f, u32 flags)
{
	lock(&f->file_lock);
	if(f->open) return; // TODO: Notify the process that the file could not be opened
	f->open_flags = flags | OFLAGS_OPEN;
	unlock(&f->file_lock);
}

static void initrd_close(struct kfile *f)
{
	lock(&f->file_lock);
	f->open = 0;
	unlock(&f->file_lock);
}

static struct dirent *initrd_readdir(struct kfile *f, u32 idx)
{
	u32 i = 0;
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
			return dent;
		}
	}

	kfree(dent);

	return NULL; // File could not be found
}

static struct kfile *initrd_finddir(struct kfile *f, char *name)
{
	struct kfile *file = fs_root;

	int i = 0;
	while(file != fs_root || !i)
	{
		if(file->impl == f->inode)
			if(!strcmp((char *)file->name, name))
				return file;
		i++; // We passed root
		file = file->next_file;
	}

	return NULL; // File not found
}




void initrd_init(struct multiboot_header* mboot_head)
{
	kerror(ERR_BOOTINFO, "Loading GRUB modules");

	if(!(mboot_head->flags & MBOOT_MODULES))
	{
		kerror(ERR_BOOTINFO, "  -> No modules to load");
		return;
	}

	struct mboot_module *mod = (struct mboot_module *)mboot_head->mod_addr;
	u32 modcnt = mboot_head->mod_count;

	u32 i = 0;
	while(i < modcnt)
	{
	#if defined(ARCH_X86)
		ptr_t mod_start = (u32)mod->mod_start;
		ptr_t mod_end   = (u32)mod->mod_end;

		u32 b = ((mod_start - (u32)firstframe) / 0x1000);
		for(; b < ((mod_end - (u32)firstframe) / 0x1000) + 1; b++)
		{
			set_frame(b, 1); // Make sure that the module is not overwritten
			map_page((b * 0x1000) + firstframe, (b * 0x1000) + firstframe, 3);
		}
	#endif
		
		if(!strcmp((char *)mod->string, cpio_name)) initrd = mod;
		i++;
		mod++;
	}
	
	if(!initrd)
	{
		kerror(ERR_MEDERR, "  -> Could not locate InitCPIO module!");
		return;
	}

	cpio = (struct header_old_cpio *)initrd->mod_start;

	if(cpio->c_magic != 070707)
	{
		kerror(ERR_MEDERR, "  -> Invalid CPIO magic number");
		return;
	}

	struct header_old_cpio *cfile = (struct header_old_cpio *)cpio;
	int cidx = 0;

	while (1)
	{
		if(cfile->c_magic != 0x71C7)
		{
			kerror(ERR_MEDERR, "  -> Invalid or corrupt InitCPIO!\n");
			return;
		}

		files[cidx] = cfile;

		cfile->c_mtime    = n(cfile->c_mtime);
		cfile->c_filesize = n(cfile->c_filesize);

		memcpy(filenames[cidx], (void *)((u32)cfile + sizeof(struct header_old_cpio)), cfile->c_namesize);

		if (!strcmp(filenames[cidx], "TRAILER!!!"))
		{
			memset(&files[cidx], 0, sizeof(struct header_old_cpio));
			return;
		}

		u32 data = ((u32)cfile + sizeof(struct header_old_cpio) + cfile->c_namesize + (cfile->c_namesize & 1));


		struct kfile *file = (struct kfile *)kmalloc(sizeof(struct kfile));

		memcpy(file->name, filenames[cidx], strlen(filenames[cidx]));

		file->length     = cfile->c_filesize;
		file->impl       = 0; // FIXME
		file->uid        = cfile->c_uid;
		file->gid        = cfile->c_gid;
		file->link       = 0; // FIXME
		file->open_flags = 0;
		file->pflags     = 0; // FIXME
		file->flags      = 0; // FIXME
		file->atime      = cfile->c_mtime;
		file->mtime      = cfile->c_mtime;
		file->ctime      = cfile->c_mtime; // Ehh....
		
		file->read      = &initrd_read;
		file->write     = &initrd_write;
		file->open      = &initrd_open;
		file->close     = &initrd_close;
		file->finddir   = &initrd_finddir;
		file->readdir   = &initrd_readdir;

		file->info      = (void *)cfile;

		cfile->c_ino    = fs_add_file(file);

		cfile = (struct header_old_cpio *)(data + cfile->c_filesize + (cfile->c_filesize & 1));

		cidx++;
		n_files++;
	}
}

