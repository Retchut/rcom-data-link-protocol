#ifndef DEFINES_H_
#define DEFINES_H_

// Serial port
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */

// Roles
#define RECEIVER 0
#define TRANSMITTER 1
#define UNKNOWN_ROLE -1

#define BIT(shift) 1 << shift

// Frame Size
#define SU_FRAME_SIZE 5
#define I_FRAME_SIZE(s) 7 + s

// Message Sending
#define MAX_DATA_CHUNK_SIZE 1024
#define MAX_FRAME_SIZE MAX_DATA_CHUNK_SIZE * 2 + 7
#define NUM_TRIES 3
#define REJECTED -2

// Message Information
#define FLAG 0x7E
#define ESCAPE 0x7d
#define FLAG_ESCAPE 0x5E
#define ESCAPE_ESCAPE 0x5D
#define A_SEND_CMD 0x03
#define A_SEND_RSP 0x01
#define A_RECV_CMD 0x01
#define A_RECV_RSP 0x03
#define C_SET 0x03
#define C_DISC 0x0B
#define C_RR0 0x05
#define C_RR1 0x85
#define C_REJ0 0x01
#define C_REJ1 0x81
#define C_S0 0x00
#define C_S1 0x40
#define C_UA 0x07
#define C_RR(n) (n << 7) | 0x05
#define C_REJ(n) (n << 7) | 0x01
#define C_S(n) (((n) % 2) << 6)
#define BCC1(a, c) a ^ c
#define C_INF(n) (n << 6)

#endif // DEFINES_H_
