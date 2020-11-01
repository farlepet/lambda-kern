#ifndef PROCFS_H
#define PROCFS_H

#include <fs/kfile.h>
#include <fs/dirinfo.h>

uint32_t proc_fs_read  (int desc, uint32_t off, uint32_t sz, uint8_t *buff);
uint32_t proc_fs_write (int desc, uint32_t off, uint32_t sz, uint8_t *buff);
int proc_fs_open       (const char *name, uint32_t flags);
int proc_fs_close      (int desc);
int proc_fs_mkdir      (int desc, char *name, uint32_t perms);
int proc_fs_create     (int desc, char *name, uint32_t perms);
int proc_fs_ioctl      (int desc, int req, void *args);
uint32_t proc_fs_read_blk(int desc, uint32_t off, uint32_t sz, uint8_t *buff);

int proc_fs_getdirinfo(int desc, struct dirinfo *dinfo);

#endif