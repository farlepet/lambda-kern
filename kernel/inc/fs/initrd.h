#ifndef INITRD_H
#define INITRD_H

#include <multiboot.h>
#include <types.h>

void initrd_init(struct multiboot_header_tag* mboot_tag, char *name);
void *initrd_find_file(char *name, u32 *size);

struct header_old_cpio
{
	u16 c_magic;
	u16 c_dev;
	u16 c_ino;
	u16 c_mode;
	u16 c_uid;
	u16 c_gid;
	u16 c_nlink;
	u16 c_rdev;
	u32 c_mtime;
	u16 c_namesize;
	u32 c_filesize;
} __packed;


#endif // INITRD_H