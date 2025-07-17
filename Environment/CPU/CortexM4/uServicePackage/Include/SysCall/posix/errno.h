
#ifndef ERRNO_H
#define ERRNO_H

// Thread-local errno (RTOS-friendly)
#ifdef USE_THREAD_LOCAL_ERRNO
extern int *__errno_location(void);
#define errno (*__errno_location())
#else
extern int errno;
#endif

// Standard error codes with explanations
#define EPERM           1   // Operation not permitted
#define ENOENT          2   // No such file or directory
#define ESRCH           3   // No such process
#define EINTR           4   // Interrupted function call
#define EIO             5   // I/O error
#define ENXIO           6   // No such device or address
#define E2BIG           7   // Argument list too long
#define ENOEXEC         8   // Exec format error
#define EBADF           9   // Bad file descriptor
#define ECHILD         10   // No child processes
#define EAGAIN         11   // Resource temporarily unavailable
#define ENOMEM         12   // Out of memory
#define EACCES         13   // Permission denied
#define EFAULT         14   // Bad address
#define EBUSY          16   // Device or resource busy
#define EEXIST         17   // File exists
#define ENODEV         19   // No such device
#define ENOTDIR        20   // Not a directory
#define EISDIR         21   // Is a directory
#define EINVAL         22   // Invalid argument
#define ENOSPC         28   // No space left on device
#define EPIPE          32   // Broken pipe
#define EDOM           33   // Math argument out of domain
#define ERANGE         34   // Result too large
#define ENOSYS         38   // Function not implemented
#define ENOTSOCK       88   // Not a socket
#define EDESTADDRREQ   89   // Destination address required
#define EMSGSIZE       90   // Message too long
#define EPROTOTYPE     91   // Protocol wrong type for socket
#define ENOPROTOOPT    92   // Protocol not available
#define EPROTONOSUPPORT 93  // Protocol not supported
#define EAFNOSUPPORT   97   // Address family not supported
#define EADDRINUSE     98   // Address already in use
#define EADDRNOTAVAIL  99   // Cannot assign requested address
#define ENETUNREACH   101   // Network is unreachable
#define ECONNABORTED  103   // Software caused connection abort
#define ECONNRESET    104   // Connection reset by peer
#define ENOBUFS       105   // No buffer space available
#define EISCONN       106   // Socket is already connected
#define ENOTCONN      107   // Socket is not connected
#define ETIMEDOUT     110   // Connection timed out
#define ECONNREFUSED  111   // Connection refused
#define EHOSTUNREACH  113   // No route to host
#define EALREADY      114   // Operation already in progress
#define EINPROGRESS   115   // Operation now in progress

#define EUNKNOWN     9999   // Unknown or unclassified error

#endif // ERRNO_H
