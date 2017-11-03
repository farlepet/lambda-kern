#ifndef FS_KFILE_H
#define FS_KFILE_H

#include <types.h>
#include <stdint.h>
#include <time/time.h>
#include <fs/dirent.h>
#include <proc/atomic.h>

#define OFLAGS_OPEN     1
#define OFLAGS_WRITE    2
#define OFLAGS_READ     4
#define OFLAGS_APPEND   8
#define OFLAGS_CREATE   16
#define OFLAGS_NONBLOCK 32

#define FS_FILE     1
#define FS_DIR      2
#define FS_CHARDEV  4
#define FS_BLCKDEV  8
#define FS_PIPE     16
#define FS_SYMLINK  32
#define FS_MNTPOINT 64
#define FS_STREAM   128


#define PERM_E   01
#define PERM_W   02
#define PERM_WE  03
#define PERM_R   04
#define PERM_RE  05
#define PERM_RW  06
#define PERM_RWE 07

struct kfile //!< Kernel representation of a file
{
	char name[FILE_NAME_MAX]; //!< Filename
	uint32_t  length;              //!< Length of the file
	uint32_t  impl;                //!< Owning i-node
	uint32_t  inode;               //!< i-node
	uint32_t  uid;                 //!< User ID
	uint32_t  gid;                 //!< Group ID

	struct kfile *link;       //!< Used for symlinks

	uint32_t  open_flags;          //!< Open flags
	uint32_t  pflags;              //!< Premissions: r/w/e
	uint32_t  flags;               //!< File type, etc...

	time_t  atime;            //!< Access time
	time_t  mtime;            //!< Modification time
	time_t  ctime;            //!< Creation time

	uint32_t (*read) (struct kfile *, uint32_t, uint32_t, u8 *);     //!< Read from a file
	uint32_t (*write)(struct kfile *, uint32_t, uint32_t, u8 *);     //!< Write to a file
	void     (*open) (struct kfile *, uint32_t);                //!< Open a file
	void     (*close)(struct kfile *);                     //!< Close a file
	struct dirent *(*readdir)(struct kfile *, uint32_t);    //!< Request a dirent structure
	struct kfile  *(*finddir)(struct kfile *, char *); //!< Find a file using a filename
	int (*mkdir) (struct kfile *, char *, uint32_t);        //!< Create a directory
	int (*create)(struct kfile *, char *, uint32_t);        //!< Create a file
	int (*ioctl) (struct kfile *, int, void *);        //!< I/O control

	void *info;              //!< Driver-specific information (eg: Hard-disk, partition, and offset for a file on a HDD)

	lock_t file_lock;        //!< Make sure only one process can access this at a time

	uint32_t magic;               //!< Verify this is a valid file entry (0xF11E0000)
	struct kfile *prev_file; //!< Next file in the linked-list
	struct kfile *next_file; //!< Previous file in the linked-list
};

#endif