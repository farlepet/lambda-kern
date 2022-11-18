#ifndef SYS_TYPES_H
#define SYS_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <limits.h>

typedef uint32_t dev_t;
typedef uint32_t ino_t;
typedef uint32_t mode_t;
typedef uint16_t nlink_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef size_t   off_t;
typedef uint32_t blksize_t;
typedef uint32_t blkcnt_t;

#if (SIZE_MAX == 0xFFFFFFFFFFFFFFFF)
typedef int64_t ssize_t;
#else
typedef int32_t ssize_t;
#endif

#endif
