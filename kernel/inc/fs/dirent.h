#ifndef FS_DIRENT_H
#define FS_DIRENT_H

#include <stdint.h>

#define FILE_NAME_MAX 256

struct dirent {
    uint32_t ino;                 //!< i-node of the file
    char     name[FILE_NAME_MAX]; //!< name of the file
};

#endif