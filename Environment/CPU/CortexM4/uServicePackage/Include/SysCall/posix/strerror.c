
#include "errno.h"

const char* strerror(int errnum) {
    switch (errnum) {
        case EPERM: return "Operation not permitted";
        case ENOENT: return "No such file or directory";
        case ESRCH: return "No such process";
        case EINTR: return "Interrupted function call";
        case EIO: return "I/O error";
        case ENOMEM: return "Out of memory";
        case EACCES: return "Permission denied";
        case EINVAL: return "Invalid argument";
        case ENOSYS: return "Function not implemented";
        case ENOTSOCK: return "Not a socket";
        case EADDRINUSE: return "Address already in use";
        case EADDRNOTAVAIL: return "Address not available";
        case ECONNREFUSED: return "Connection refused";
        case ECONNRESET: return "Connection reset by peer";
        case ETIMEDOUT: return "Connection timed out";
        case ENOTCONN: return "Socket not connected";
        case EISCONN: return "Socket already connected";
        case ENETUNREACH: return "Network unreachable";
        case EUNKNOWN: return "Unknown error";
        default: return "Unknown error code";
    }
}
