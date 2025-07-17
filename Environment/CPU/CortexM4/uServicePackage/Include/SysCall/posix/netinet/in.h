
#ifndef NETINET_IN_H
#define NETINET_IN_H

#include <stdint.h>

#define INADDR_ANY ((in_addr_t) 0x00000000)

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

struct in_addr {
    in_addr_t s_addr;
};

struct sockaddr_in {
    uint16_t        sin_family;
    in_port_t       sin_port;
    struct in_addr  sin_addr;
    char            sin_zero[8];
};

// Swap bytes for 16-bit
#ifndef __bswap16
#define __bswap16(x)        ((uint16_t)(((x) >> 8) | ((x) << 8)))
#endif

#ifndef __bswap32
#define __bswap32(x)        ((uint16_t)(((x) >> 24) & 0x000000FF) | (((x) >> 8)  & 0x0000FF00) | (((x) << 8)  & 0x00FF0000) | (((x) << 24) & 0xFF000000))
#endif

// Host-to-network and network-to-host
#define htons(x) __bswap16(x)
#define ntohs(x) __bswap16(x)
#define htonl(x) __bswap32(x)
#define ntohl(x) __bswap32(x)

#endif // NETINET_IN_H
