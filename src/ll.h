#ifndef _RCOM_LL_H_
#define _RCOM_LL_H_

#include <stdbool.h>

int llopen(int fd, bool transmitter);

int llclose(int fd);

int llwrite(int fd, unsigned char *buffer, unsigned int length);

int llread(int fd, unsigned char *buffer);

#endif /* _RCOM_LL_H_ */
