#ifndef _RCOM_SEND_H_
#define _RCOM_SEND_H_

#include <stddef.h>

int writeInformationFrame(int fd, unsigned char addr, unsigned char *info_ptr,
                          size_t info_size, int msg_nr);

int writeInformationAndRetry(int fd, unsigned char addr,
                             unsigned char *info_ptr, size_t info_size,
                             int msg_nr);

int writeSupervisionFrame(int fd, unsigned char msg_addr,
                          unsigned char msg_ctrl);

int writeSupervisionAndRetry(int fd, unsigned char msg_addr,
                             unsigned char msg_ctrl);

// TODO: Move to a new place after, maybe utils
unsigned char build_BCC2(unsigned char *data, size_t size);

#endif /* _RCOM_SEND_H_ */
