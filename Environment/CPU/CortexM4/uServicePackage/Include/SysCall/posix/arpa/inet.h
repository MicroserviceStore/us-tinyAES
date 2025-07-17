
#ifndef ARPA_INET_H
#define ARPA_INET_H

#include <stdint.h>

typedef uint32_t in_addr_t;

in_addr_t inet_addr(const char *cp);
char* inet_ntoa(struct in_addr in);
int inet_pton(int af, const char *src, void *dst);

#endif // ARPA_INET_H
