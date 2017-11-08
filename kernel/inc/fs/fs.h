#ifndef FS_H
#define FS_H

#include <types.h>
#include <fs/kfile.h>


#define PERMISSIONS(u, g, w) ((u << 6) | (g << 3) | (w << 0))


struct kfile *fs_root;

int fs_add_file(struct kfile *file, struct kfile *parent);

uint32_t       fs_read   (struct kfile *f, uint32_t off, uint32_t sz, u8 *buff);
uint32_t       fs_write  (struct kfile *f, uint32_t off, uint32_t sz, u8 *buff);
void           fs_open   (struct kfile *f, uint32_t flags);
void           fs_close  (struct kfile *f);
struct dirent *fs_readdir(DIR *d);
struct kfile  *fs_finddir(struct kfile *f, char *name);
DIR           *fs_opendir(struct kfile *f);
int            fs_mkdir  (struct kfile *f, char *name, uint32_t perms);
int            fs_create (struct kfile *f, char *name, uint32_t perms);
int            fs_ioctl  (struct kfile *f, int req, void *args);

struct kfile *fs_dirfile(DIR *d);

/**
 * \brief Find a file relative to the given directory
 * 
 * @param f Directory for path to be relative to
 * @param path Path
 * 
 * @return kfile representing requested file/directory, NULL if not found
 */
struct kfile *fs_find_file(struct kfile *f, char *path);

void fs_init(void);

void fs_debug(int nfiles);

#endif // FS_H
