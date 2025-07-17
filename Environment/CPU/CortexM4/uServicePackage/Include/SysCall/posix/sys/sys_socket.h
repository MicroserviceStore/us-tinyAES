
#ifndef SYS_SOCKET_H
#define SYS_SOCKET_H

#include <stdint.h>

#define AF_INET        2
#define SOCK_STREAM    1
#define SOCK_DGRAM     2

typedef uint16_t sa_family_t;
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

struct sockaddr {
    sa_family_t sa_family;
    char        sa_data[14];
};

int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, unsigned int addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr *addr, unsigned int *addrlen);
int connect(int sockfd, const struct sockaddr *addr, unsigned int addrlen);
int send(int sockfd, const void *buf, int len, int flags);
int recv(int sockfd, void *buf, int len, int flags);
int close(int sockfd);

#endif // SYS_SOCKET_H
