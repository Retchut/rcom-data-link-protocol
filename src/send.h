#ifndef _RCOM_SEND_H_
#define _RCOM_SEND_H_

#include <stddef.h>

int writeInformationFrame(int fd, unsigned char addr, unsigned char cmd,
                          unsigned char *infoPtr, size_t infoSize);

int writeSupervisionFrame(int fd, unsigned char msg_addr,
                          unsigned char msg_ctrl);

int writeSupervisionAndRetry(int fd, unsigned char msg_addr,
                             unsigned char msg_ctrl);

unsigned char buildBCC2(unsigned char *data, size_t size);

#endif /* _RCOM_SEND_H_ */
