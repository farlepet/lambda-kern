#ifndef FS_KFILE_H
#define FS_KFILE_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

typedef struct kfile kfile_t;
typedef struct kfile_hand kfile_hand_t;

#include <data/types/llist.h>
#include <fs/dirent.h>
#include <fs/dirstream.h>
#include <proc/atomic/types/tlock.h>
#include <time/time.h>

/* NOTE: Some of these flags don't matter after the file is opened, it might be
 * useful to either map or simply mask off these flags. */
#define OFLAGS_READ      0x00000001
#define OFLAGS_WRITE     0x00000002
#define OFLAGS_APPEND    0x00000004
#define OFLAGS_CREATE    0x00000008
#define OFLAGS_DSYNC     0x00000010
#define OFLAGS_EXCL      0x00000020
#define OFLAGS_NOCTTY    0x00000040
#define OFLAGS_NONBLOCK  0x00000080
#define OFLAGS_RSYNC     0x00000100
#define OFLAGS_SYNC      0x00000200
#define OFLAGS_TRUNC     0x00000400
#define OFLAGS_CLOEXEC   0x00000800
#define OFLAGS_DIRECTORY 0x00000800
#define OFLAGS_OPEN      0x80000000

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


typedef int            (*fileop_close_f)  (kfile_hand_t *);
typedef ssize_t        (*fileop_read_f)   (kfile_hand_t *, size_t, size_t, void *);
typedef ssize_t        (*fileop_write_f)  (kfile_hand_t *, size_t, size_t, const void *);
typedef int            (*fileop_ioctl_f)  (kfile_hand_t *, int, void *);
typedef struct dirent *(*fileop_readdir_f)(kfile_hand_t *, DIR *); // TODO: This probably shouldn't exist in this form

typedef struct {
    fileop_close_f   close;
    fileop_read_f    read;
    fileop_write_f   write;
    fileop_ioctl_f   ioctl;
    fileop_readdir_f readdir;
} file_hand_ops_t;


typedef int      (*fileop_open_f)  (kfile_t *, kfile_hand_t *);
typedef int      (*fileop_mkdir_f) (kfile_t *, const char *, uint32_t);
typedef int      (*fileop_create_f)(kfile_t *, const char *, uint32_t);
typedef kfile_t *(*fileop_finddir_f)(kfile_t *, const char *); // TODO: This is unnecessary

typedef struct {
    fileop_open_f    open;
    fileop_mkdir_f   mkdir;
    fileop_create_f  create;
    fileop_finddir_f finddir;
} file_ops_t;

/**
 * Kernel representation of a file
 * 
 * Not to be confused with a handle for an open file.
 */
struct kfile
{
    char name[FILE_NAME_MAX]; //!< Filename [TODO: This could be moved elsewhere, else throw it at the end of the file with size zero]
    size_t    length;         //!< Length of the file
    uint32_t  impl;           //!< Owning i-node
    uint32_t  inode;          //!< i-node
    uint32_t  uid;            //!< User ID
    uint32_t  gid;            //!< Group ID

    kfile_t  *link;           //!< Used for symlinks

    uint32_t  pflags;         //!< Premissions: r/w/e
    uint32_t  flags;          //!< File type, etc...

    time_t  atime;            //!< Access time
    time_t  mtime;            //!< Modification time
    time_t  ctime;            //!< Creation time

    const file_ops_t *ops;

    void *info;               //!< Driver-specific information (eg: Hard-disk, partition, and offset for a file on a HDD)

    tlock_t file_lock;        //!< Make sure only one process can access this at a time

    kfile_t *parent;          //!< Pointer to parent directory

    llist_item_t list_item;   //!< Linked list item
    llist_t      children;    //!< List of children
};

/**
 * Kernel representation of an open file. May be multiple instances for any
 * given kfile_t.
 */
struct kfile_hand {
    uint32_t open_flags; //!< Flags used for opening file

    const file_hand_ops_t *ops;
    
    tlock_t lock;        //!< Make sure only one process can access this at a time

    kfile_t *file;       //!< Pointer to file itself
};

#endif
