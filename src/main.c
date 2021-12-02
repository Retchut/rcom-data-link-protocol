#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "ll.h"
#include "send.h"

int main(int argc, char *argv[]) {

  if (!(argc == 3 && strcmp(argv[1], "receiver") == 0) &&
      !(argc == 4 && strcmp(argv[1], "emitter") == 0)) {
    fprintf(stderr, "Usage:\t./recv SerialPort File\n\tex: nserial /dev/tty11 "
                    "pinguim.jpg\n");
    exit(-1);
  }

  bool role = -1;
  int port = -1;
  if (strcmp(argv[1], "emitter") == 0) {
    role = TRANSMITTER;
  } else if (strcmp(argv[1], "receiver") == 0) {
    role = RECEIVER;
  }

  if (isdigit(argv[2][0]) && atoi(argv[2]) >= 0) {
    port = atoi(argv[2]);
  } else {
    fprintf(stderr, "Invalid port specified\n");
    exit(-1);
  }

  int fd = -1;
  if ((fd = llopen(port, role)) == -1) {
    fprintf(stderr, "Error opening port \n");
    exit(-1);
  }

  unsigned char buf[5] = "ABCDE";

  buf[2] = 0x7E;

  if (role == TRANSMITTER) {
    if (llwrite(fd, buf, 5) == -1) {
      fprintf(stderr, "Error writing to port \n");
      exit(-1);
    }

  } else if (role == RECEIVER) {

    unsigned char buf[512];
    if (llread(fd, buf) == -1) {
      fprintf(stderr, "Error reading from port\n");
    }

    printf("Buf is now %s\n", buf);
  }

  if (llclose(fd) == -1) {
    fprintf(stderr, "Error closing port \n");
    exit(-1);
  }

  return 0;
}
