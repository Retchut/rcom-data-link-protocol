#ifndef WRITE_H_
#define WRITE_H_

#include <stddef.h>

int writeInformationFrame(int fd, unsigned char addr, unsigned char cmd,
                          unsigned char *infoPtr, size_t infoSize);

int writeSupervisionFrame(int fd, unsigned char msg_addr,
                          unsigned char msg_ctrl);

unsigned char buildBCC2(unsigned char *data, size_t size);
#endif // WRITE_H_
