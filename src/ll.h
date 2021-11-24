#include <stdbool.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define RECEIVER FALSE
#define TRANSMITTER TRUE

#define BIT(shift) 1 << shift

#define FLAG        0x7E
#define A_WRT_CMD   0x03
#define A_RCV_RESP  0x03
#define A_RCV_CMD   0x01
#define A_WRT_RESP  0x01
#define C_INF(n)    (n << 6)
#define C_SET       0x03
#define C_DISC      0x0B
#define C_UA        0x07
#define C_RR(n)     (n << 7) | 0x05
#define C_REJ(n)    (n << 7) | 0x01

int llopen(int port, bool transmitter);

int llclose(int fd);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char * buffer);

int writeFrame(char *buffer, int length);