#ifndef FS_H
#define FS_H

#include <types.h>
#include <time/time.h>

#define FILE_NAME_MAX 256

struct dirent
{
	u32   ino;                 //!< i-node of the file
	char *name[FILE_NAME_MAX]; //!< name of the file
};

struct kfile //!< Kernel representation of a file
{
	char *name[FILE_NAME_MAX]; //!< Filename
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
	struct dirent *(*readdir)(struct kfile *, u32);    //!< Request a dirent ctructure
	struct kfile  *(*finddir)(struct kfile *, char *); //!< Find a file using a filename
	int (*mkdir) (char *, u32);                        //!< Create a directory
	int (*create)(char *, u32);                        //!< Create a file
	int (*ioctl) (struct kfile *, int, void *);        //!< I/O control
};

#endif // FS_H
