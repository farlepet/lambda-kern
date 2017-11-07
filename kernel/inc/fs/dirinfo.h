#ifndef FS_DIRINFO_H
#define FS_DIRINFO_H

#include <fs/dirent.h>

struct dirinfo {
    uint32_t ino;             //!< Inode

    uint32_t n_children;      //!< Number of children
    uint32_t parent_ino;      //!< Inode of parent directory

    char name[FILE_NAME_MAX]; //!< Name of directory
};

#endif