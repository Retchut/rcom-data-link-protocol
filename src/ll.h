#ifndef LL_H_
#define LL_H_

#include <stdbool.h>

int llopen(int fd, bool transmitter);

int llclose(int fd);

int llwrite(int fd, unsigned char *buffer, int length);

int llread(int fd, unsigned char *buffer);

#endif // LL_H_
