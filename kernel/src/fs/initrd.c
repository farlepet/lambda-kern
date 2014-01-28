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
static u32 filedata[0x1000];                  // Table of initrd data locations

char *cpio_name = NULL; 

#define n(x) ((x << 16) | (x >> 16))

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

		filedata[cidx] = data;

		//kerror(ERR_BOOTINFO, "Found %s @ %08x w/ size %d w/ data @ %08x [%d]", filenames[cidx], cfile, files[cidx]->c_filesize, data, files[cidx].c_namesize);

		struct kfile *file = (struct kfile *)kmalloc(sizeof(struct kfile));

		memcpy(file->name, cfile + sizeof(struct header_old_cpio), cfile->c_namesize);
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
		
		// TODO: Add R/W O/C etc... functions

		cfile->c_ino = fs_add_file(file);

		cfile = (struct header_old_cpio *)(data + cfile->c_filesize + (cfile->c_filesize & 1));

		cidx++;
		n_files++;
	}
}



void *initrd_find_file(char *name, u32 *size)
{
	if(!initrd)
	{
		kerror(ERR_SMERR, "initrd file requested without valid initrd");
		return 0;
	}

	int cidx = 0;
	while(cidx <= n_files)
	{
		if(!strcmp(name, filenames[cidx]))
		{
			if(size) *size = files[cidx]->c_filesize;
			return (void *)filedata[cidx];
		}
		cidx++;
	}

	if(size) *size = 0;
	return NULL;
}
