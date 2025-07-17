
#ifndef UNISTD_H
#define UNISTD_H

int close(int fd);
int read(int fd, void *buf, unsigned int count);
int write(int fd, const void *buf, unsigned int count);

#endif // UNISTD_H
