#ifndef ERRNO_H
#define ERRNO_H

/* POSIX errno codes, numbering based off of BSD */

#define EPERM            1         /**< Operation not permitted */
#define ENOENT           2         /**< No such file or directory */
#define ESRCH            3         /**< No such process */
#define EINTR            4         /**< Interrupted function */
#define EIO              5         /**< I/O error */
#define ENXIO            6         /**< No such device or address */
#define E2BIG            7         /**< Argument list too long */
#define ENOEXEC          8         /**< Executable file format error */
#define EBADF            9         /**< Bad file descriptor */
#define ECHILD          10         /**< No child processes */
#define EDEADLK         11         /**< Reasource deadlock would occur */
#define ENOMEM          12         /**< Not enough space */
#define EACCES          13         /**< Permission denied */
#define EFAULT          14         /**< Bad address */
#define EBUSY           16         /**< Device or resource busy */
#define EEXIST          17         /**< File exists */
#define EXDEV           18         /**< Improper hard link */
#define ENODEV          19         /**< No such device */
#define ENOTDIR         20         /**< Not a directory or a symbolic link to a directory*/
#define EISDIR          21         /**< Is a directory */
#define EINVAL          22         /**< Invalid argument */
#define ENFILE          23         /**< Too many files open in system */
#define EMFILE          24         /**< File descriptor value too large */
#define ENOTTY          25         /**< Inappropriate I/O control operation */
#define ETXTBSY         26         /**< Text file busy */
#define EFBIG           27         /**< File too large */
#define ENOSPC          28         /**< No space left on device */
#define ESPIPE          29         /**< Invalid seek */
#define EROFS           30         /**< Read-only filesystem*/
#define EMLINK          31         /**< Too many hard links */
#define EPIPE           32         /**< Broken pipe */
#define EDOM            33         /**< Mathematics argument out of domain of function */
#define ERANGE          34         /**< Result too large */
#define EAGAIN          35         /**< Resource unavailable, try again */
#define EWOULDBLOCK     EAGAIN     /**< Operation would block */
#define EINPROGRESS     36         /**< Operation in progress */
#define EALREADY        37         /**< Connection already in progress*/
#define ENOTSOCK        38         /**< Not a socket*/
#define EDESTADDRREQ    39         /**< Destination address required */
#define EMSGSIZE        40         /**< Message too large */
#define EPROTOTYPE      41         /**< Protocol wrong type for socket */
#define ENOPROTOOPT     42         /**< Protocol not available */
#define EPROTONOSUPPORT 43         /**< Protocol not supported */
#define ESOCKTNOSUPPORT 44         /**< Socket type not supported */
#define EOPNOTSUPP      45         /**< Operation not support on socket */
#define ENOTSUP         EOPNOTSUPP /**< Not supported */
#define EAFNOSUPPORT    47         /**< Address family not supported */
#define EADDRINUSE      48         /**< Address in use */
#define EADDRNOTAVAIL   49         /**< Address not available */
#define ENETDOWN        50         /**< Network is down */
#define ENETUNREACH     51         /**< Network unreachable */
#define ENETRESET       52         /**< Connection aborted by network */
#define ECONNABORTED    53         /**< Connection aborted */
#define ECONNRESET      54         /**< Connection reset */
#define ENOBUFS         55         /**< No buffer space available */
#define EISCONN         56         /**< Socket is connected */
#define ENOTCONN        57         /**< The socket is not connected */
#define ESHUTDOWN       58         /**< */
#define ETIMEDOUT       60         /**< Connection timed out */
#define ECONNREFUSED    61         /**< Connection refused */
#define ELOOP           62         /**< Too many levels of symbolic links */
#define ENAMETOOLONG    63         /**< Filename too long */
#define EHOSTUNREACH    65         /**< Host is unreachable */
#define ENOTEMPTY       66         /**< Directory not empty */
#define EDQUOT          69         /**< Reserved */
#define ESTALE          70         /**< Stale NFS file handle */
#define ENOLCK          77         /**< No locks available */
#define ENOSYS          78         /**< Functionality not supported */
#define EIDRM           82         /**< Identifier removed */
#define ENOMSG          83         /**< No message of desired type */
#define EOVERFLOW       84         /**< Value too large to be stored in datatype */
#define ECANCELED       85         /**< Operation cancelled */
#define EILSEQ          86         /**< Illegal byte sequence */
#define EBADMSG         89         /**< Bad message */
#define EMULTIHOP       90         /**< Reserved */
#define ENOLINK         91         /**< Reserved */
#define ENOTRECOVERABLE 95         /**< State not recoverable */
#define EOWNERDEAD      96         /**< Previous owner died */

/* Non-POSIX error codes, to be used within the kernel only */

#define EUNSPEC         512        /**< Unspecified error - catch-all for any errors that may need to be defined in the future */
#define ENODATA         513        /**< No or not enough data available */

#endif

