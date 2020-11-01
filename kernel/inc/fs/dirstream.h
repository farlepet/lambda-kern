#ifndef FS_DIRSTREAM_H
#define FS_DIRSTREAM_H

//struct kfile;

struct dirstream {
    struct kfile *dir; //!< kfile representing directory

    struct kfile *current; //!< Current file (next file to be returned by a call to `readdir`)
    struct kfile *prev;    //!< Previous file returned by `readdir`
};

#define DIR struct dirstream

#endif