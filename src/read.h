#ifndef _RCOM_READ_H_
#define _RCOM_READ_H_

int readSupervisionFrame(int fd);

int readInformationFrameResponse(int fd);

int readInformationMessage(int fd, unsigned char *stuffed_msg);

#endif /* _RCOM_READ_H_ */
