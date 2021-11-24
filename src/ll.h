#include <stdbool.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define RECEIVER FALSE
#define TRANSMITTER TRUE

int llopen(int port, bool transmitter);

int llclose(int fd);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char * buffer);
