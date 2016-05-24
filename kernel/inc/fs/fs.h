#ifndef FS_H
#define FS_H

#include <types.h>
#include <time/time.h>
#include <proc/atomic.h>

#define FILE_NAME_MAX 256

#define OFLAGS_OPEN   1
#define OFLAGS_WRITE  2
#define OFLAGS_READ   4
#define OFLAGS_APPEND 8
#define OFLAGS_CREATE 16

#define FS_FILE     1
#define FS_DIR      2
#define FS_CHARDEV  4
#define FS_BLCKDEV  8
#define FS_PIPE     16
#define FS_SYMLINK  32
#define FS_MNTPOINT 64


#define PERM_E   01
#define PERM_W   02
#define PERM_WE  03
#define PERM_R   04
#define PERM_RE  05
#define PERM_RW  06
#define PERM_RWE 07

#define PERMISSIONS(u, g, w) ((u << 6) | (g << 3) | (w << 0))

struct dirent
{
	u32   ino;                 //!< i-node of the file
	char  name[FILE_NAME_MAX]; //!< name of the file
};

struct kfile //!< Kernel representation of a file
{
	char name[FILE_NAME_MAX]; //!< Filename
	u32  length;              //!< Length of the file
	u32  impl;                //!< Owning i-node
	u32  inode;               //!< i-node
	u32  uid;                 //!< User ID
	u32  gid;                 //!< Group ID

	struct kfile *link;       //!< Used for symlinks

	u32  open_flags;          //!< Open flags
	u32  pflags;              //!< Premissions: r/w/e
	u32  flags;               //!< File type, etc...

	time_t  atime;            //!< Access time
	time_t  mtime;            //!< Modification time
	time_t  ctime;            //!< Creation time

	u32  (*read) (struct kfile *, u32, u32, u8 *);     //!< Read from a file
	u32  (*write)(struct kfile *, u32, u32, u8 *);     //!< Write to a file
	void (*open) (struct kfile *, u32);                //!< Open a file
	void (*close)(struct kfile *);                     //!< Close a file
	struct dirent *(*readdir)(struct kfile *, u32);    //!< Request a dirent structure
	struct kfile  *(*finddir)(struct kfile *, char *); //!< Find a file using a filename
	int (*mkdir) (struct kfile *, char *, u32);        //!< Create a directory
	int (*create)(struct kfile *, char *, u32);        //!< Create a file
	int (*ioctl) (struct kfile *, int, void *);        //!< I/O control

	void *info;              //!< Driver-specific information (eg: Hard-disk, partition, and offset for a file on a HDD)

	lock_t file_lock;        //!< Make sure only one process can access this at a time

	u32 magic;               //!< Verify this is a valid file entry (0xF11E0000)
	struct kfile *prev_file; //!< Next file in the linked-list
	struct kfile *next_file; //!< Previous file in the linked-list
};

struct kfile *fs_root;

int fs_add_file(struct kfile *file);

u32            fs_read   (struct kfile *f, u32 off, u32 sz, u8 *buff);
u32            fs_write  (struct kfile *f, u32 off, u32 sz, u8 *buff);
void           fs_open   (struct kfile *f, u32 flags);
void           fs_close  (struct kfile *f);
struct dirent *fs_readdir(struct kfile *f, u32 idx);
struct kfile  *fs_finddir(struct kfile *f, char *name);
int            fs_mkdir  (struct kfile *f, char *name, u32 prems);
int            fs_create (struct kfile *f, char *name, u32 perms);
int            fs_ioctl  (struct kfile *f, int req, void *args);

void fs_init(void);

void fs_debug(int nfiles);

#endif // FS_H
