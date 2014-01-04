#include <multiboot.h>
#include <fs/initrd.h>
#include <err/error.h>
#include <string.h>

#if defined(ARCH_X86)
#include <mm/paging.h>
#endif

struct multiboot_module_tag *initrd = 0;


static struct header_old_cpio *cpio = 0;

static int n_files = 0;
static struct header_old_cpio files[0x1000]; // Table of initrd file locations
static char filenames[0x1000][128];           // Table of initrd filenames
static u32 filedata[0x1000];                  // Table of initrd data locations


#define n(x) ((x << 16) | (x >> 16))

void initrd_init(struct multiboot_header_tag* mboot_tag, char *name)
{
	kerror(ERR_BOOTINFO, "Loading InitCPIO");

	u32 size = mboot_tag->size;
	u32 i = 8; // Bypass multiboot_header_tag
	while(i < size)
	{
		struct multiboot_tag *tag = (struct multiboot_tag *)(i + (ptr_t)mboot_tag);

		if(tag->type == 3)
		{
			struct multiboot_module_tag *mod = (struct multiboot_module_tag *)tag;

	#if defined(ARCH_X86)
			ptr_t mod_start = (u32)mod->mod_start;
			ptr_t mod_end   = (u32)mod->mod_end;

			u32 b = ((mod_start - (u32)firstframe) / 0x1000);
			for(; b < ((mod_end - (u32)firstframe) / 0x1000); b++)
			{
				set_frame(b, 1); // Make sure that the module is not overwritten
				map_page((b * 0x1000) + firstframe, (b * 0x1000) + firstframe, 3);
			}
	#endif


			if(!strcmp((char *)mod->name, name)) initrd = mod;
		}
		i += tag->size;
		if(i & 0x07) i = (i & ~0x07) + 8; // Entries are aways padded
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

		//files[cidx] = cfile;

		cfile->c_mtime    = n(cfile->c_mtime);
		cfile->c_filesize = n(cfile->c_filesize);

		memcpy(&files[cidx], cfile, sizeof(struct header_old_cpio));

		memcpy(filenames[cidx], (void *)((u32)cfile + sizeof(struct header_old_cpio)), cfile->c_namesize);

		if (!strcmp(filenames[cidx], "TRAILER!!!"))
		{
			memset(&files[cidx], 0, sizeof(struct header_old_cpio));
			return;
		}

		u32 data = ((u32)cfile + sizeof(struct header_old_cpio) + cfile->c_namesize + (cfile->c_namesize & 1));

		kerror(ERR_BOOTINFO, "%08x %08x %08x %08x", *(u32 *)(data), *(u32 *)(data + 4), *(u32 *)(data + 8), *(u32 *)(data + 12));

		filedata[cidx] = data;

		kerror(ERR_BOOTINFO, "Found %s @ %08x w/ size %d w/ data @ %08x [%d]", filenames[cidx], cfile, files[cidx].c_filesize, data, files[cidx].c_namesize);

		cfile = (struct header_old_cpio *)(data + cfile->c_filesize + (cfile->c_filesize & 1));

		cidx++;
		n_files++;
	}
}



void *initrd_find_file(char *name, int *size)
{
	int cidx = 0;
	while(cidx <= n_files)
	{
		kerror(ERR_BOOTINFO, ":Testing %s:%d", filenames[cidx], files[cidx].c_filesize);
		if(!strcmp(name, filenames[cidx]))
		{
			if(size) *size = files[cidx].c_filesize;
			kerror(ERR_BOOTINFO, "files[%d]: %08X", cidx, files[cidx].c_magic);
			return (void *)filedata[cidx];
		}
		cidx++;
	}

	if(size) *size = 0;
	return NULL;
}