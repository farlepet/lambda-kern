#ifndef INITRD_H
#define INITRD_H

#include <stdint.h>
#include <fs/fs.h>

#ifdef CONFIG_EMBEDDED_INITRD
/* Linked in through initrd.o */
#  ifdef CONFIG_EMBEDDED_INITRD_LZOP
extern int _binary_initrd_cpio_start;
extern int _binary_initrd_cpio_end;
#    define INITRD_START _binary_initrd_cpio_start
#    define INITRD_END   _binary_initrd_cpio_start
#  else
extern int _binary_initrd_cpio_lzo_start;
extern int _binary_initrd_cpio_lzo_end;
#    define INITRD_START _binary_initrd_cpio_lzo_start
#    define INITRD_END   _binary_initrd_cpio_lzo_start
#  endif
#endif

/**
 * @brief Load INITRD
 * 
 * @param mntpoint where to mount initrd
 * @param initrd   pointer to CPIO structure in memory
 * @param len      length of initrd data
 */
void initrd_mount(struct kfile *mntpoint, uintptr_t initrd, size_t len);

/**
 * @brief CPIO header
 */
struct header_old_cpio {
    uint16_t c_magic;    //!< Magic number (070707)
    uint16_t c_dev;      //!< Device number
    uint16_t c_ino;      //!< Inode number
    uint16_t c_mode;     //!< Mode
    uint16_t c_uid;      //!< UID of owner
    uint16_t c_gid;      //!< GID of owner
    uint16_t c_nlink;    //!< Number of links to this file
    uint16_t c_rdev;     //!< Device number (for block/char devices)
    uint32_t c_mtime;    //!< Modification time (seconds since epoch)
    uint16_t c_namesize; //!< Bytes in name, including NULL byte
    uint32_t c_filesize; //!< Size of file (MAX 4 GiB)
} __packed;

/**
 * @brief CPIO c_mode values
 */
enum old_cpio_mode {
    CPIO_MODE_SOCKET    = 0140000, //!< Socket
    CPIO_MODE_SYMLINK   = 0120000, //!< Symbolic link
    CPIO_MODE_REGULAR   = 0100000, //!< Regular file
    CPIO_MODE_BLOCK     = 0060000, //!< Block device
    CPIO_MODE_DIRECTORY = 0040000, //!< Directory
    CPIO_MODE_CHAR      = 0020000, //!< Character device
    CPIO_MODE_PIPE      = 0010000, //!< Named pipe or FIFO
    CPIO_MODE_SUID      = 0004000, //!< SUID
    CPIO_MODE_SGID      = 0002000, //!< SGID
    CPIO_MODE_STICKY    = 0001000, //!< Sticky bit

    CPIO_MODE_PERM_MASK = 0000777  //!< POSIX permissions
};

#endif // INITRD_H
