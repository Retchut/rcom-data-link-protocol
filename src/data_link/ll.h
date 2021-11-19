#ifndef LL_H_
#define LL_H_

#define CTRL_MSG_SIZE 5
#define BCC1(addr, ctrl) (addr ^ ctrl)

#define FLAG 0x7e
#define SENDER_ADDR 0x03
#define SET 0x03
#define EMITTER 0
#define RECEIVER 1

/*
 * @brief llopen is responsible for establishing the connection between the host
 * and some other serial port identified by the file descriptor fd.
 * As such, its job is to send the SET signal followed by reading the UA one.
 */
int llopen(int port, unsigned char role);

int llread();

int llwrite();

int llclose();

#endif // LL_H_
