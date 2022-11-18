#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <sys/types.h>

#include <fs/kfile.h>

#define PERMISSIONS(u, g, w) ((u << 6) | (g << 3) | (w << 0))

int fs_add_file(kfile_t *file, kfile_t *parent);

ssize_t        fs_read   (kfile_hand_t *f, uint32_t off, uint32_t sz, void *buff);
ssize_t        fs_write  (kfile_hand_t *f, uint32_t off, uint32_t sz, const void *buff);
int            fs_open   (kfile_t *f, kfile_hand_t *hand);
int            fs_close  (kfile_hand_t *f);
struct dirent *fs_readdir(DIR *d);
kfile_t       *fs_finddir(kfile_t *f, const char *name);
DIR           *fs_opendir(kfile_t *f);
int            fs_mkdir  (kfile_t *f, const char *name, uint32_t perms);
int            fs_create (kfile_t *f, const char *name, uint32_t perms);
int            fs_ioctl  (kfile_hand_t *f, int req, void *args);

kfile_t *fs_dirfile(DIR *d);

kfile_hand_t *fs_handle_create(void);
kfile_hand_t *fs_handle_create_open(kfile_t *f, uint32_t flags);
int fs_handle_destroy(kfile_hand_t *hand);
int fs_read_file_by_path(const char *path, kfile_t *cwd, void **buff, size_t *sz, size_t max_sz);

/**
 * \brief Find a file relative to the given directory
 * 
 * @param f Directory for path to be relative to
 * @param path Path
 * 
 * @return kfile representing requested file/directory, NULL if not found
 */
kfile_t *fs_find_file(kfile_t *f, const char *path);

/**
 * @brief Returns root node of virtual filesystem
 *
 * @return kfile_t* Pointer to root node
 */
kfile_t *fs_get_root(void);

void fs_init(void);

#endif // FS_H
