#include <stdbool.h>
#include <stddef.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define RECEIVER FALSE
#define TRANSMITTER TRUE

#define SUPERVISION FALSE
#define UNNUMERATED FALSE
#define INFORMATION TRUE

#define BIT(shift) 1 << shift

#define SU_FRAME_SIZE 5
#define I_FRAME_SIZE(s) 6 + s

#define FLAG 0x7E
#define A_SEND_CMD_ADDR 0x03
#define A_RECV_RESP_ADDR 0x03
#define A_RECV_CMD_ADDR 0x01
#define A_SEND_RESP_ADDR 0x01
#define C_INF(n) (n << 6)
#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define C_RR(n) (n << 7) | 0x05
#define C_REJ(n) (n << 7) | 0x01
#define BCC1(a, c) a ^ c

unsigned char buildBCC2(unsigned char *data, size_t size);

int buildFrame(unsigned char *frame, unsigned char addr, unsigned char cmd,
               unsigned char *infoPtr, size_t infoSize);

int writeFrame(int fd, unsigned char *frame, size_t size);

int readFrame(int fd, unsigned char *frame, size_t expectedSize);

bool testFrameEquality(unsigned char *frame1, unsigned char *frame2,
                       size_t size);

int llopen(int port, bool transmitter);

int llclose(int fd);

int llwrite(int fd, unsigned char *buffer, int length);

int llread(int fd, unsigned char *buffer);
