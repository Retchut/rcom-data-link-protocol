#ifndef DEFINES_H_
#define DEFINES_H_

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define RECEIVER FALSE
#define TRANSMITTER TRUE
#define UNKNOW_ROLE -1

#define BIT(shift) 1 << shift

#define SU_FRAME_SIZE 5
#define I_FRAME_SIZE(s) 6 + s

#define NUM_TRIES 3

#define FLAG 0x7E
#define A_SEND_CMD 0x03
#define A_SEND_RSP 0x01
#define A_RECV_CMD 0x01
#define A_RECV_RSP 0x03
#define C_INF(n) (n << 6)
#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define C_RR(n) (n << 7) | 0x05
#define C_REJ(n) (n << 7) | 0x01
#define BCC1(a, c) a ^ c

#endif // DEFINES_H_
