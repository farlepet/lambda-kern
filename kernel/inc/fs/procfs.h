#ifndef PROCFS_H
#define PROCFS_H

#include <fs/kfile.h>
#include <fs/dirinfo.h>

u32   proc_fs_read    (int desc, u32 off, u32 sz, u8 *buff);
u32   proc_fs_write   (int desc, u32 off, u32 sz, u8 *buff);
void  proc_fs_open    (int desc, u32 flags);
void  proc_fs_close   (int desc);
int   proc_fs_mkdir   (int desc, char *name, u32 perms);
int   proc_fs_create  (int desc, char *name, u32 perms);
int   proc_fs_ioctl   (int desc, int req, void *args);
u32   proc_fs_read_blk(int desc, u32 off, u32 sz, u8 *buff);

int   proc_fs_getdirinfo(int desc, struct dirinfo *dinfo);

#endif