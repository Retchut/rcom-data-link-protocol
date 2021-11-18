#ifndef LL_H_
#define LL_H_

/*
 * @brief llopen is responsible for establishing the connection between the host
 * and some other serial port identified by the file descriptor fd.
 * As such, its job is to send the SET signal followed by reading the UA one.
 */
int llopen(int port, unsigned char role);

#endif // LL_H_
