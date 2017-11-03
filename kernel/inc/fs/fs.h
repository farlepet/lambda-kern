#ifndef FS_H
#define FS_H

#include <types.h>
#include <fs/kfile.h>


#define PERMISSIONS(u, g, w) ((u << 6) | (g << 3) | (w << 0))


struct kfile *fs_root;

int fs_add_file(struct kfile *file);

uint32_t       fs_read   (struct kfile *f, uint32_t off, uint32_t sz, u8 *buff);
uint32_t       fs_write  (struct kfile *f, uint32_t off, uint32_t sz, u8 *buff);
void           fs_open   (struct kfile *f, uint32_t flags);
void           fs_close  (struct kfile *f);
struct dirent *fs_readdir(struct kfile *f, uint32_t idx);
struct kfile  *fs_finddir(struct kfile *f, char *name);
int            fs_mkdir  (struct kfile *f, char *name, uint32_t perms);
int            fs_create (struct kfile *f, char *name, uint32_t perms);
int            fs_ioctl  (struct kfile *f, int req, void *args);

void fs_init(void);

void fs_debug(int nfiles);

#endif // FS_H
