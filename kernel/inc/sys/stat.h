#ifndef SYS_STAT_H
#define SYS_STAT_H

#include <fs/kfile.h>

#include <sys/types.h>

/**
 * @brief Kernel representatino of file information
 */
typedef struct kstat {
    /** Device ID */
    uint32_t dev_id;
    /** Special file device ID */
    uint32_t rdev_id;
    /** inode number */
    uint64_t inode;
    /** Unix permissions */
    uint32_t mode;
    /** Number of hard links to this file */
    uint32_t n_link;
    /** Owning user ID */
    uint32_t uid;
    /** Owner group ID */
    uint32_t gid;
    /** File size in bytes */
    uint64_t size;
    /** Actual size of allocated space for file on storage medium */
    uint64_t alloc_size;
    /** Access time, in nanoseconds since 1970-01-01 UTC */
    int64_t atime;
    /** Creation time, in nanoseconds since 1970-01-01 UTC */
    int64_t btime;
    /** Status change time, in nanoseconds since 1970-01-01 UTC */
    int64_t ctime;
    /** Modification time, in nanoseconds since 1970-01-01 UTC */
    int64_t mtime;
    /** Preferred block size, in bytes */
    uint32_t blksize;
} kstat_t;

int kfstat(kfile_t *f, kstat_t *buf);

int fstat(int fd, kstat_t *buf);

#endif